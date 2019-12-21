#pragma once

#include "../Dat/Item.h"
#include "../Dat/Stream.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Dat
        {
            // A simple file
            class MiscFile : public Item
            {
                public:
                    MiscFile(Stream&& stream);

                    Stream& stream();

                protected:
                    Stream _stream;
            };
        }
    }
}
