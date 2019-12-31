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

    static constexpr uint8_t DIR_NE  = 0;
    static constexpr uint8_t DIR_SE  = 2;
    static constexpr uint8_t DIR_NW  = 5;
    static constexpr uint8_t DIR_MAX = 6;
    // static constexpr uint8_t DIR_E  = 1;
    // static constexpr uint8_t DIR_SW = 3;
    // static constexpr uint8_t DIR_W  = 4;

    // should be <uint16,uint16> but it's used for temporary values during .frm -> .png offsets conversion
    typedef std::vector<std::pair<int32_t, int32_t>> PngOffsets;

    // converts .frm frames offsets to .png frames offsets
    // finds minimum size required to draw all .png frames
    static PngOffsets ConvertOffsets( const std::vector<Falltergeist::Format::Frm::Frame>& frames, uint32_t& minWidth, uint32_t& minHeight, Logging& logVerbose )
    {
        // init conversion

        int32_t spotX = 0, spotY = 0;

        // result[F].first  = frame F, offset X
        // result[F].second = frame F, offset Y
        std::vector<std::pair<int32_t, int32_t>> result;

        for( const auto& frame : frames )
        {
            if( !frame.Index )
            {
                // cache first .frm frame spot position

                spotX = frame.Width / 2;
                spotY = frame.Height;

                result.emplace_back( 0, 0 );
            }
            else
            {
                // apply .frm offsets to cached spot

                spotX += frame.OffsetX;
                spotY += frame.OffsetY;

                // convert .frm spot to .png offset
                // requires further processing, as results are negative for many directions at this step

                result.emplace_back( spotX - frame.Width / 2, spotY - frame.Height );

                // shift position of first frame, if needed

                if( result.back().first < 0 )
                    result.front().first = std::max( result.front().first, -result.back().first );

                if( result.back().second < 0 )
                    result.front().second = std::max( result.front().second, -result.back().second );
            }

            logVerbose << "offset frame:" + std::to_string( frame.Index ) + " " + std::to_string( frame.OffsetX ) + "," + std::to_string( frame.OffsetY ) + " -> " + std::to_string( result.back().first ) + "," + std::to_string( result.back().second );
        }

        // finish conversion, find .png image size

        minWidth = minHeight = 0;
        for( const auto& frame : frames )
        {
            // apply first frame offset to other frames

            auto& offset = result[frame.Index];

            if( frame.Index )
            {
                // logVerbose << "offset ? frame:" + std::to_string( frame.Index ) + " " + std::to_string( offset.first ) + "," + std::to_string( offset.second ) + " + " + std::to_string( result.front().first ) + "," + std::to_string( result.front().second );

                offset.first += result.front().first;   // X
                offset.second += result.front().second; // Y
            }

            // logVerbose << "offset ! frame:" + std::to_string( frame.Index ) + " " + std::to_string( offset.first ) + "," + std::to_string( offset.second );

            minWidth  = std::max<uint32_t>( minWidth, offset.first + frame.Width );
            minHeight = std::max<uint32_t>( minHeight, offset.second + frame.Height );
        }

        return result;
    }

    // copy pixels from .frm to .png, starting at given position; adjusts RGB
    static void DrawFrame( const PngGeneratorData& data, const Falltergeist::Format::Frm::Frame& frame, PngImage& image, const uint32_t pngX = 0, const uint32_t pngY = 0 )
    {
        for( uint16_t y = 0, h = frame.Height; y < h; y++ )
        {
            for( uint16_t x = 0, w = frame.Width; x < w; x++ )
            {
                const Falltergeist::Format::Pal::Color* color = data.Pal.color( frame.ColorIndex( x, y ) );
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
        uint16_t maxWidth  = data.Frm.MaxFrameWidth();
        uint16_t maxHeight = data.Frm.MaxFrameHeight();

        PngImage image( maxWidth * data.Frm.FramesPerDirection, maxHeight * data.Frm.DirectionsSize() );

        for( const auto& dir : data.Frm.Directions() )
        {
            uint16_t frameIdx = 0;
            for( const auto& frame : dir.Frames() )
            {
                const uint32_t pngX = maxWidth * frameIdx;
                const uint32_t pngY = maxHeight * dir.Index;

                DrawFrame( data, frame, image, pngX, pngY );
                frameIdx++;
            }
        }

        logVerbose << "write png = " + data.PngPath + data.PngBasename + data.PngExtension + " = " + std::to_string( image.width() ) + "x" + std::to_string( image.height() );
        PngWriter png( data.PngPath + data.PngBasename + data.PngExtension );
        png.write( image );
    }

    // create multiple animated .png files (one per direction)
    static void GeneratorAnim( const PngGeneratorData& data, Logging& logVerbose )
    {
        for( const auto& dir : data.Frm.Directions() )
        {
            const std::string pngName  = data.PngPath + data.PngBasename + "_" + std::to_string( dir.Index ) + data.PngExtension;
            uint32_t          pngWidth = 0, pngHeight = 0;

            // TODO support acTL + fcTL + IDAT variant
            static constexpr bool firstIsAnim = false;

            logVerbose << "direction " + std::to_string( dir.Index ) << 1;
            PngOffsets offsets = ConvertOffsets( dir.Frames(), pngWidth, pngHeight, logVerbose );

            logVerbose << "write png = " + pngName + " = " + std::to_string( pngWidth ) + "x" + std::to_string( pngHeight ) << 1;
            PngWriter png( pngName );

            png.writeAnimHeader( pngWidth, pngHeight, dir.FramesSize() + ( firstIsAnim ? 0 : 1 ), 0, !firstIsAnim );

            if( !firstIsAnim )
            {
                // if first image is not supposed to be part of animation, copy of first frame is added and moved to center
                // software supporting APNG will ignore it, anything else will use that as image to display

                auto&    frame = dir.Frames().front();
                PngImage defaultImage( pngWidth, pngHeight );

                // draw .png frame

                const uint32_t pngX = pngWidth / 2 - frame.Width / 2;
                const uint32_t pngY = pngHeight / 2 - frame.Height / 2;

                DrawFrame( data, frame, defaultImage, pngX, pngY );

                png.writeAnimFrame( defaultImage,
                                    0, 0, // offsets
                                    0, 0, // delay
                                    0, 0  // dispose, blend
                );
            }

            // draw .png frames
            for( const auto& frame : dir.Frames() )
            {
                logVerbose << "draw frame:" + std::to_string( frame.Index ) + " @ " + std::to_string( offsets[frame.Index].first ) + "," + std::to_string( offsets[frame.Index].second ) + " -> " + std::to_string( frame.Width ) + "x" + std::to_string( frame.Height );

                PngImage image( frame.Width, frame.Height );
                DrawFrame( data, frame, image );

                png.writeAnimFrame( image, offsets[frame.Index].first, offsets[frame.Index].second, 0, data.Frm.FramesPerSecond / 2, PNG_DISPOSE_OP_BACKGROUND, PNG_BLEND_OP_SOURCE );
            }

            png.writeAnimEnd();
            logVerbose << -2;
        }
    }

    // create single animated .png files with all directions included
    static void GeneratorAnimPacked( const PngGeneratorData& data, Logging& logVerbose )
    {
        const std::string pngName  = data.PngPath + data.PngBasename + data.PngExtension;
        uint32_t          pngWidth = 0, pngWidthLeft = 0, pngWidthRight = 0, pngHeight = 0, pngRightX = 0;
        constexpr uint8_t pngSpacing = 4;

        // TODO support acTL + fcTL + IDAT variant
        static constexpr bool firstIsAnim = false;

        // TODO? fallback to 'anim' generator?
        if( data.Frm.DirectionsSize() != DIR_MAX )
            throw std::runtime_error( "GeneratorAnimPacked() - .frm file must contain frames for exactly " + std::to_string( DIR_MAX ) + " directions" );

        // cache size/offsets of all directions

        // dirSize[D].first  = direction D, width
        // dirSize[D].second = direction D, height
        std::vector<std::pair<uint32_t, uint32_t>> dirSize;

        // dirOffsets[D][F].first  = direction D, frame F, offset X
        // dirOffsets[D][F].second = direction D, frame F, offset Y
        std::vector<PngOffsets> dirOffsets;

        for( const auto& dir : data.Frm.Directions() )
        {
            logVerbose << "direction " + std::to_string( dir.Index ) << 1;

            uint32_t   dirWidth = 0, dirHeight = 0;
            PngOffsets offsets = ConvertOffsets( dir.Frames(), dirWidth, dirHeight, logVerbose );

            dirSize.emplace_back( dirWidth, dirHeight );
            dirOffsets.emplace_back( offsets );

            logVerbose << -1;
        }

        // find .png image size

        for( uint8_t leftIdx = DIR_NW, rightIdx = DIR_NE; rightIdx <= DIR_SE; leftIdx--, rightIdx++ )
        {
            logVerbose << "dirSize " + std::to_string( leftIdx ) + ":" + std::to_string( dirSize[leftIdx].first ) + "," + std::to_string( dirSize[leftIdx].second ) + " " + std::to_string( rightIdx ) + ":" + std::to_string( dirSize[rightIdx].first ) + "," + std::to_string( dirSize[rightIdx].second );

            pngWidthLeft  = std::max( pngWidthLeft, dirSize[leftIdx].first );
            pngWidthRight = std::max( pngWidthLeft, dirSize[rightIdx].first );

            pngWidth = std::max( pngWidth, pngWidthLeft + pngSpacing + pngWidthRight );
            pngHeight += ( rightIdx > DIR_NE ? pngSpacing : 0 ) + std::max( dirSize[leftIdx].second, dirSize[rightIdx].second );
            pngRightX = std::max( pngRightX, dirSize[leftIdx].first + pngSpacing );
        }

        logVerbose << "write png = " + pngName + " = " + std::to_string( pngWidth ) + "x" + std::to_string( pngHeight );
        PngWriter png( pngName );

        png.writeAnimHeader( pngWidth, pngHeight, data.Frm.FramesPerDirection + ( firstIsAnim ? 0 : 1 ), 0, !firstIsAnim );

        // TODO defaultImage
        if( !firstIsAnim )
        {
            PngImage defaultImage( pngWidth, pngHeight );
            png.writeAnimFrame( defaultImage, 0, 0, 0, 0, 0, 0 );
        }

        //

        for( uint16_t frameIdx = 0; frameIdx < data.Frm.FramesPerDirection; frameIdx++ )
        {
            logVerbose << "frame " + std::to_string( frameIdx ) << 1;
            PngImage image( pngWidth, pngHeight );

            for( const auto& dir : data.Frm.Directions() )
            {
                logVerbose << "direction " + std::to_string( dir.Index ) << 1;

                uint32_t pngX = 0, pngY = 0, pngRow = ( dir.Index <= DIR_SE ? dir.Index : DIR_NW - dir.Index );
                logVerbose << "pngRow = " + std::to_string( pngRow );

                const Falltergeist::Format::Frm::Frame& frame = data.Frm.GetFrame( dir.Index, frameIdx );

                // move *E frames to right column
                if( dir.Index <= DIR_SE )
                    pngX = pngRightX;
                // align *W frames to right
                else
                    pngX = pngRightX - pngSpacing - dirSize[dir.Index].first;

                // align frames to bottom

                logVerbose << "pngCurrDirs = " + std::to_string( dir.Index ) + "," + std::to_string( std::abs( DIR_NW - dir.Index ) );
                pngY += std::max( dirSize[dir.Index].second, dirSize[std::abs( DIR_NW - dir.Index )].second ) - dirSize[dir.Index].second;

                // apply vertical spacing (except NE/NW)

                pngY += pngSpacing * pngRow;

                // move frames to their rows (except NE/NW)

                while( pngRow )
                {
                    logVerbose << "pngPrevDirs = " + std::to_string( DIR_MAX - pngRow ) + "," + std::to_string( pngRow - 1 );
                    pngY += std::max( dirSize[DIR_MAX - pngRow].second, dirSize[pngRow - 1].second );
                    pngRow--;
                }

                logVerbose << "pngX = " + std::to_string( pngX );
                logVerbose << "pngY = " + std::to_string( pngY );

                DrawFrame( data, frame, image, pngX + dirOffsets[dir.Index][frame.Index].first, pngY + dirOffsets[dir.Index][frame.Index].second );
                logVerbose << -1;
            }

            png.writeAnimFrame( image,
                                0, 0, // offsets
                                0, data.Frm.FramesPerSecond / 2,
                                PNG_DISPOSE_OP_BACKGROUND, PNG_BLEND_OP_SOURCE );
            logVerbose << -1;
        }

        png.writeAnimEnd();
    }

    //
    // used by main application
    //

    std::unordered_map<std::string, PngGeneratorFunc> Generator;

    void InitPngGenerators()
    {
        Generator["legacy"]      = &GeneratorLegacy;
        Generator["anim"]        = &GeneratorAnim;
        Generator["anim-packed"] = &GeneratorAnimPacked;
    }
}
