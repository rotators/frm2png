#pragma once

#include <cstdint>
#include <vector>

#include "../Dat/Item.h"
#include "../Dat/Stream.h"
#include "../Pal/Color.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Pal
        {
            class File : public Dat::Item
            {
            protected:
                std::vector<Color> _Colors;

            public:
                File( const std::vector<Color>& colors );
                File( Dat::Stream&& stream );

                const Color* color( unsigned index ) const;

            protected:
                void PostProcess();

            public:
                const Color& Get( size_t index ) const;
                void         RGBMultiplier( uint8_t multiplier = 4 );
            };
        }
    }
}
