#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "../Dat/Stream.h"
#include "../Pal/Color.h"
#include "../Pal/File.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Pal
        {
            File::File( const std::vector<Color>& colors )
            {
                _Colors = colors;

                PostProcess();
            }

            File::File( Dat::Stream&& stream )
            {
                stream.setPosition( 3 );

                _Colors.emplace_back( 0, 0, 0, 0 ); // zero color (transparent)

                for( uint8_t i = 0; i != 255; ++i )
                {
                    uint8_t r = stream.uint8();
                    uint8_t g = stream.uint8();
                    uint8_t b = stream.uint8();

                    _Colors.emplace_back( r, g, b );
                }

                PostProcess();
            }

            void File::PostProcess()
            {
                uint16_t index = 0;
                for( auto& color : _Colors )
                {
                    color.Index = index++;

                    // I guess this section requires a little explanation
                    // Thing is, original engine uses indexed/paletted textures
                    // and palette contains 'magic' animated colors
                    // we can, ofcourse use paletted textures, and different shaders for rgba (png) textures,
                    // but, while hackish, this is probably simpler/faster.
                    // So, we have 6 groups of animations, with 1-6 colors each
                    // Red and Alpha components denotes 'magical' color,
                    // while Green denotes group, and Blue - index.
                    // Because in OpenGL colors are floats from 0.0 to 1.0
                    // we need numbers, that being divided by 255 will give 'round' values,
                    // this number is 51.
                    // Then, in shaders, we'll calculate resulting index by multiplying
                    // original value by 255 and then dividing by 51.
                    // So, color 0.2 will give: 0.2*255/51=1, all we need now is to subtract 1.
                    // So resulting formulae is: index = color.b * 255 / 51 -1;

                    if( color.Index >= 229 && color.Index <= 254 )
                    {
                        // magic, sorry
                        color.A            = 51;
                        color.R            = 153;
                        color.NoMultiplier = true;
                    }
                    else if( color.Index == 0 || color.Index == 255 )
                        color.NoMultiplier = true;

                    // SLIME
                    if( color.Index >= 229 && color.Index <= 232 )
                    {
                        color.G = 0; //
                        color.B = static_cast<uint8_t>( ( color.Index - 229 ) * 51 );
                    }
                    // MONITORS
                    else if( color.Index >= 233 && color.Index <= 237 )
                    {
                        color.G = 51; //
                        color.B = static_cast<uint8_t>( ( color.Index - 233 ) * 51 );
                    }
                    // SLOW FIRE
                    else if( color.Index >= 238 && color.Index <= 242 )
                    {
                        color.G = 102; //
                        color.B = static_cast<uint8_t>( ( color.Index - 238 ) * 51 );
                    }
                    // FAST FIRE
                    else if( color.Index >= 243 && color.Index <= 247 )
                    {
                        color.G = 153; //
                        color.B = static_cast<uint8_t>( ( color.Index - 243 ) * 51 );
                    }
                    // SHORE
                    else if( color.Index >= 248 && color.Index <= 253 )
                    {
                        color.G = 204; //
                        color.B = static_cast<uint8_t>( ( color.Index - 248 ) * 51 );
                    }
                    // ALARM
                    else if( color.Index == 254 )
                    {
                        color.G = 255; //
                        color.B = 0;
                    }
                }
            }

            const Color& File::Get( size_t index ) const
            {
                return _Colors.at( index );
            }

            void File::RGBMultiplier( uint8_t multiplier /* = 4 */ )
            {
                if( multiplier < 2 || multiplier > 4 )
                    throw std::runtime_error( "Falltergist::Format::Pal::File::RGBMultiplier() - invalid multiplier '" + std::to_string( multiplier ) + "'" );

                for( auto& color : _Colors )
                {
                    if( color.NoMultiplier )
                        continue;

                    uint32_t r = color.R * multiplier;
                    uint32_t g = color.G * multiplier;
                    uint32_t b = color.B * multiplier;

                    if( r >= UINT8_MAX )
                        throw std::runtime_error( "Falltergeist::Format::Pal::File::RGBMultiplier() - [" + std::to_string( color.Index ) + "][R] invalid multiplier '" + std::to_string( multiplier ) + "' : " + std::to_string( color.R ) + " * " + std::to_string( multiplier ) + " = " + std::to_string( r ) );
                    else if( g >= UINT8_MAX )
                        throw std::runtime_error( "Falltergeist::Format::Pal::File::RGBMultiplier() - [" + std::to_string( color.Index ) + "][G] invalid multiplier '" + std::to_string( multiplier ) + "' : " + std::to_string( color.G ) + " * " + std::to_string( multiplier ) + " = " + std::to_string( g ) );
                    else if( b >= UINT8_MAX )
                        throw std::runtime_error( "Falltergeist::Format::Pal::File::RGBMultiplier() - [" + std::to_string( color.Index ) + "][B] invalid multiplier '" + std::to_string( multiplier ) + "' : " + std::to_string( color.B ) + " * " + std::to_string( multiplier ) + " = " + std::to_string( b ) );

                    color.R = static_cast<uint8_t>( r );
                    color.G = static_cast<uint8_t>( g );
                    color.B = static_cast<uint8_t>( b );
                }
            }
        }
    }
}
