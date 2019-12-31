/*
 * Copyright (c) 2015-2018 Falltergeist developers
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

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <string>

#include "../Dat/Stream.h"
#include "../Frm/File.h"
#include "../Frm/Frame.h"
#include "../Pal/File.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Frm
        {
            static constexpr uint8_t DIR_MAX = 6;

            File::File( Dat::Stream&& stream )
            {
                stream.setPosition( 0 );

                Version            = stream.uint32();
                FramesPerSecond    = stream.uint16();
                ActionFrame        = stream.uint16();
                FramesPerDirection = stream.uint16();

                uint16_t shiftX[DIR_MAX];
                uint16_t shiftY[DIR_MAX];
                uint32_t dataOffset[DIR_MAX];

                for( uint8_t dir = 0; dir < DIR_MAX; dir++ )
                    shiftX[dir] = stream.uint16();
                for( uint8_t dir = 0; dir < DIR_MAX; dir++ )
                    shiftY[dir] = stream.uint16();
                for( uint8_t dir = 0; dir < DIR_MAX; dir++ )
                {
                    dataOffset[dir] = stream.uint32();
                    if( dir > 0 && dataOffset[dir - 1] == dataOffset[dir] )
                        continue;

                    // TODO:Rotators Direction::Direction( index, dataOffset, shiftX, shiftY )
                    _Directions.emplace_back();

                    auto& direction = _Directions.back();

                    direction.Index      = dir;
                    direction.DataOffset = dataOffset[dir];
                    direction.ShiftX     = shiftX[dir];
                    direction.ShiftY     = shiftY[dir];
                }

                // for each direction
                for( auto& direction : _Directions )
                {
                    // jump to frames data at frames area
                    stream.setPosition( direction.DataOffset + 62 );

                    // read all frames
                    for( uint16_t frameIdx = 0; frameIdx < FramesPerDirection; frameIdx++ )
                    {
                        uint16_t width  = stream.uint16();
                        uint16_t height = stream.uint16();

                        // Number of pixels for this frame
                        // We don't need this, because we already have width*height
                        stream.uint32();

                        int16_t offsetX = stream.int16();
                        int16_t offsetY = stream.int16();

                        direction.Frames().emplace_back( width, height, offsetX, offsetY );

                        auto& frame = direction.Frames().back();
                        frame.Index = frameIdx;

                        // Pixels data
                        stream.readBytes( frame.ColorIndexData(), frame.Width * frame.Height );
                    }
                }
            }

            const Frame& File::GetFrame( uint8_t dir, uint16_t frame ) const
            {
                if( dir >= _Directions.size() )
                    throw std::runtime_error( "Falltergeist::Format::Frm::File::GetFrame() - invalid direction '" + std::to_string( dir ) + "'" );

                return _Directions.at( dir ).GetFrame( frame );
            }

            uint16_t File::MaxFrameWidth() const
            {
                uint16_t width = 0;
                std::for_each( _Directions.begin(), _Directions.end(), [&width]( const Direction& dir ) {
                    width = std::max( width, dir.MaxFrameWidth() );
                } );

                return width;
            }

            uint16_t File::MaxFrameHeight() const
            {
                uint16_t height = 0;
                std::for_each( _Directions.begin(), _Directions.end(), [&height]( const Direction& dir ) {
                    height = std::max( height, dir.MaxFrameHeight() );
                } );

                return height;
            }

            const std::vector<Direction>& File::Directions() const
            {
                return _Directions;
            }
        }
    }
}
