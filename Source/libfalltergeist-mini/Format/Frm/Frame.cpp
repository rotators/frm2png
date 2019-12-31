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

#include <cstdint>
#include <vector>

#include "../Frm/Frame.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Frm
        {
            Frame::Frame( uint16_t width, uint16_t height, int16_t offsetX, int16_t offsetY ) :
                // protected
                _ColorIndex( width * height, 0 ),
                // public
                Width( width ),
                Height( height ),
                OffsetX( offsetX ),
                OffsetY( offsetY )
            {}

            uint8_t Frame::ColorIndex( uint16_t x, uint16_t y ) const
            {
                if( x >= Width || y >= Height )
                    return 0;

                return _ColorIndex.at( Width * y + x );
            }

            uint8_t* Frame::ColorIndexData()
            {
                return _ColorIndex.data();
            }
        }
    }
}
