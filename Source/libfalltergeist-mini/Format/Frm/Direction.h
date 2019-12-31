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

#pragma once

#include <cstdint>
#include <vector>

#include "../Frm/Frame.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Frm
        {
            class Direction
            {
            protected:
                std::vector<Frame> _Frames;

            public:
                int16_t  ShiftX     = 0;
                int16_t  ShiftY     = 0;
                uint32_t DataOffset = 0;
                uint8_t  Index      = 0; // helps in range-based for() loops, it is NOT directory id

            public:
                Direction()                   = default;
                Direction( Direction&& )      = default;
                Direction( const Direction& ) = delete;
                Direction& operator=( const Direction& ) = delete;
                Direction& operator=( Direction&& ) = default;
                ~Direction()                        = default;

            public:
                const Frame& GetFrame( uint16_t frame ) const;

                std::vector<Frame>&       Frames();
                const std::vector<Frame>& Frames() const;

                inline uint16_t FramesSize() const
                {
                    return static_cast<uint16_t>( _Frames.size() );
                }

                uint16_t MaxFrameWidth() const;
                uint16_t MaxFrameHeight() const;
            };
        }
    }
}
