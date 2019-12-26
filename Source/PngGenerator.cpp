/*
 * Copyright (c) 2019 Rotators
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

// C++ standard includes
#include <algorithm>
#include <cstdint>
#include <functional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

// frm2png includes
#include "Logging.h"
#include "PngGenerator.h"
#include "PngImage.h"
#include "PngWriter.h"

// falltergeist includes
#include "Format/Frm/File.h"
#include "Format/Pal/Color.h"
#include "Format/Pal/File.h"

namespace frm2png
{
    PngGeneratorData::PngGeneratorData( Falltergeist::Format::Frm::File&& frm, Falltergeist::Format::Pal::File&& pal ) :
        Frm( std::move( frm ) ),
        Pal( std::move( pal ) )
    {}

    //
    // helpers
    //

    typedef std::vector<std::pair<int32_t, int32_t>> PngOffsets;

    // converts .frm frames offsets to .png frames offsets
    // finds minimum size required to draw all .png frames
    static PngOffsets ConvertOffsets( const std::vector<Falltergeist::Format::Frm::Frame>& frames, uint32_t& minWidth, uint32_t& minHeight, Logging& logVerbose )
    {
        // init conversion

        int32_t spotX = 0, spotY = 0;

        std::vector<std::pair<int32_t, int32_t>> result;

        uint16_t frameIdx = 0;
        for( const auto& frame : frames )
        {
            if( !frameIdx )
            {
                // save first .frm frame spot position

                spotX = frame.width() / 2;
                spotY = frame.height();

                result.emplace_back( 0, 0 );
            }
            else
            {
                // apply .frm offsets to cached spot
                spotX += frame.offsetX();
                spotY += frame.offsetY();

                // convert .frm spot to .png offset

                result.emplace_back( spotX - frame.width() / 2, spotY - frame.height() );

                // shift position of first frame, if needed

                if( result.back().first < 0 )
                    result.front().first = std::max( result.front().first, -result.back().first );

                if( result.back().second < 0 )
                    result.front().second = std::max( result.front().second, -result.back().second );
            }

            logVerbose << "offset frame:" + std::to_string( frameIdx ) + " " + std::to_string( frame.offsetX() ) + "," + std::to_string( frame.offsetY() ) + " -> " + std::to_string( result.back().first ) + "," + std::to_string( result.back().second );
            frameIdx++;
        }

        // finish conversion, find .png image size

        frameIdx = minWidth = minHeight = 0;
        for( auto& offset : result )
        {
            // apply first frame offset to other frames

            if( frameIdx )
            {
                offset.first += result.front().first;
                offset.second += result.front().second;
            }

            minWidth  = std::max<uint32_t>( minWidth, offset.first + frames.at( frameIdx ).width() );
            minHeight = std::max<uint32_t>( minHeight, offset.second + frames.at( frameIdx ).height() );

            frameIdx++;
        }

        return result;
    }

    // copy pixels from .frm to .png, starting at given position; adjusts RGB
    static void DrawFrame( const PngGeneratorData& data, const Falltergeist::Format::Frm::Frame& frame, PngImage& image, const uint32_t pngX = 0, const uint32_t pngY = 0 )
    {
        for( uint16_t y = 0, h = frame.height(); y < h; y++ )
        {
            for( uint16_t x = 0, w = frame.width(); x < w; x++ )
            {
                const Falltergeist::Format::Pal::Color* color = data.Pal.color( frame.index( x, y ) );
                image.setPixel( pngX + x, pngY + y, color->red() * data.RgbMultiplier, color->green() * data.RgbMultiplier, color->blue() * data.RgbMultiplier, color->alpha() );
            }
        }
    }

    //
    // generators
    //

    // original generator by falltergeist team
    // all frames are drawn as single static image; each direction draws frames in its own row, from left to right
    static void GeneratorLegacy( const PngGeneratorData& data, Logging& logVerbose )
    {
        uint16_t maxWidth  = data.Frm.maxFrameWidth();
        uint16_t maxHeight = data.Frm.maxFrameHeight();

        PngImage image( maxWidth * data.Frm.framesPerDirection(), maxHeight * static_cast<uint8_t>( data.Frm.directions().size() ) );

        uint8_t dirIdx = 0;
        for( const auto& dir : data.Frm.directions() )
        {
            uint16_t frameIdx = 0;
            for( const auto& frame : dir.frames() )
            {
                const uint32_t pngX = maxWidth * frameIdx;
                const uint32_t pngY = maxHeight * dirIdx;

                DrawFrame( data, frame, image, pngX, pngY );
                frameIdx++;
            }
            dirIdx++;
        }

        logVerbose << "write png = " + data.PngPath + data.PngBasename + data.PngExtension + " = " + std::to_string( image.width() ) + "x" + std::to_string( image.height() );
        PngWriter png( data.PngPath + data.PngBasename + data.PngExtension );
        png.write( image );
    }

    // create multiple animated .png files (one per direction)
    static void GeneratorAPNG( const PngGeneratorData& data, Logging& logVerbose )
    {
        uint8_t dirIdx = 0;
        for( const auto& dir : data.Frm.directions() )
        {
            const std::string pngName  = data.PngPath + data.PngBasename + "_" + std::to_string( dirIdx ) + data.PngExtension;
            uint32_t          pngWidth = 0, pngHeight = 0;
            // TODO support acTL + fcTL + IDAT variant
            static constexpr bool firstIsAnim = false;

            logVerbose << "direction " + std::to_string( dirIdx ) << 1;
            PngOffsets offsets = ConvertOffsets( dir.frames(), pngWidth, pngHeight, logVerbose );

            logVerbose << "write png = " + pngName + " = " + std::to_string( pngWidth ) + "x" + std::to_string( pngHeight ) << 1;
            PngWriter png( pngName );

            png.writeAnimHeader( pngWidth, pngHeight, static_cast<uint16_t>( dir.frames().size() ) + ( firstIsAnim ? 0 : 1 ), 0, !firstIsAnim );

            if( !firstIsAnim )
            {
                // if first image is not supposed to be part of animation, copy of first frame is added and moved to center
                // software supporting APNG will ignore it, anything else will use that as image to display

                auto&    frame = dir.frames().front();
                PngImage defaultImage( pngWidth, pngHeight );

                // draw .png frame

                const uint32_t pngX = pngWidth / 2 - frame.width() / 2;
                const uint32_t pngY = pngHeight / 2 - frame.height() / 2;

                DrawFrame( data, frame, defaultImage, pngX, pngY );

                png.writeAnimFrame( defaultImage, 0, 0, 0, 0, 0, 0 );
            }

            // draw .png frames
            uint16_t frameIdx = 0;
            for( const auto& frame : dir.frames() )
            {
                logVerbose << "draw frame:" + std::to_string( frameIdx ) + " @ " + std::to_string( offsets.at( frameIdx ).first ) + "," + std::to_string( offsets.at( frameIdx ).second ) + " -> " + std::to_string( frame.width() ) + "x" + std::to_string( frame.height() );

                PngImage image( frame.width(), frame.height() );
                DrawFrame( data, frame, image );

                png.writeAnimFrame( image, offsets.at( frameIdx ).first, offsets.at( frameIdx ).second, 0, data.Frm.framesPerSecond() / 2, PNG_DISPOSE_OP_BACKGROUND, PNG_BLEND_OP_SOURCE );

                frameIdx++;
            }

            png.writeAnimEnd();
            dirIdx++;
            logVerbose << -2;
        }
    }

    std::unordered_map<std::string, PngGeneratorFunc> Generator;

    void InitPngGenerators()
    {
        Generator["legacy"] = &GeneratorLegacy;
        Generator["apng"]   = &GeneratorAPNG;
    }
}
