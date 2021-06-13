#include "../Pal/Color.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Pal
        {
            Color::Color( uint8_t r, uint8_t g, uint8_t b, uint8_t a /* = 255 */ ) :
                R( r ), G( g ), B( b ), A( a )
            {}
        }
    }
}
