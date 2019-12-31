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
#include <stdexcept>
#include <string>

#include "../Frm/Direction.h"
#include "../Frm/Frame.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Frm
        {
            const Frame& Direction::GetFrame( uint16_t frame ) const
            {
                if( frame >= _Frames.size() )
                    throw std::runtime_error( "Falltergeist::Format::Frm::Direction::GetFrame() - invalid frame '" + std::to_string( frame ) + "'" );

                return _Frames.at( frame );
            }

            std::vector<Frame>& Direction::Frames()
            {
                return _Frames;
            }

            const std::vector<Frame>& Direction::Frames() const
            {
                return _Frames;
            }

            uint16_t Direction::MaxFrameWidth() const
            {
                return std::max_element( _Frames.begin(), _Frames.end(), []( const Frame& a, const Frame& b ) {
                           return a.Width < b.Width;
                       } )
                    ->Width;
            }

            uint16_t Direction::MaxFrameHeight() const
            {
                return std::max_element( _Frames.begin(), _Frames.end(), []( const Frame& a, const Frame& b ) {
                           return a.Height < b.Height;
                       } )
                    ->Height;
            }
        }
    }
}
