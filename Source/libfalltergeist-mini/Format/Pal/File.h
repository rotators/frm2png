#pragma once

#include <vector>

#include "../Dat/Item.h"
#include "../Pal/Color.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Dat
        {
            class Stream;
        }

        namespace Pal
        {
            class File : public Dat::Item
            {
                public:
                    File(const std::vector<Color>& colors);
                    File(Dat::Stream&& stream);

                    const Color* color(unsigned index) const;

                protected:
                    void PostProcess();

                    std::vector<Color> _colors;
            };
        }
    }
}
