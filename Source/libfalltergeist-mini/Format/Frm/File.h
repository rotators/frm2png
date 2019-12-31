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

#include "../Dat/Item.h"
#include "../Dat/Stream.h"
#include "../Frm/Direction.h"
#include "../Frm/Frame.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Frm
        {
            class Direction;

            class File : public Dat::Item
            {
            protected:
                std::vector<Direction> _Directions;

            public:
                uint32_t Version            = 0;
                uint16_t FramesPerSecond    = 0;
                uint16_t FramesPerDirection = 0;
                uint16_t ActionFrame        = 0;

            public:
                File( Dat::Stream&& stream );

            public:
                const Frame& GetFrame( uint8_t dir, uint16_t frame ) const;

                uint16_t MaxFrameWidth() const;
                uint16_t MaxFrameHeight() const;

                const std::vector<Direction>& Directions() const;

                inline uint8_t DirectionsSize() const
                {
                    return static_cast<uint8_t>( _Directions.size() );
                }
            };
        }
    }
}
