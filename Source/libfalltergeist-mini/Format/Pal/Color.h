#pragma once

#include <cstdint>

namespace Falltergeist
{
    namespace Format
    {
        namespace Pal
        {
            class Color
            {
            public:
                uint16_t Index = 0;

                uint8_t R     = 0;
                uint8_t G     = 0;
                uint8_t B     = 0;
                uint8_t A     = 255;
                bool    NoMultiplier = false;

            public:
                Color() = delete;
                Color( uint8_t r, uint8_t g, uint8_t b, uint8_t = 255 );
                ~Color() = default;
            };
        }
    }
}
