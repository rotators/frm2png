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

namespace Falltergeist
{
    namespace Format
    {
        namespace Frm
        {
            class Frame
            {
            protected:
                std::vector<uint8_t> _ColorIndex;

            public:
                uint16_t Width   = 0;
                uint16_t Height  = 0;
                int16_t  OffsetX = 0;
                int16_t  OffsetY = 0;
                uint16_t Index   = 0;

            public:
                Frame() = default;
                Frame( uint16_t width, uint16_t height, int16_t offsetX, int16_t offsetY );
                Frame( const Frame& other ) = delete;
                Frame( Frame&& other )      = default;
                Frame& operator=( const Frame& ) = delete;
                Frame& operator=( Frame&& ) = default;
                ~Frame()                    = default;

            public:
                uint8_t  ColorIndex( uint16_t x, uint16_t y ) const;
                uint8_t* ColorIndexData();
            };
        }
    }
}
