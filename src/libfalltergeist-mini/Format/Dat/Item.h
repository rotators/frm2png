#pragma once

#include <string>

namespace Falltergeist
{
    namespace Format
    {
        namespace Dat
        {
            // A base class for all game resource files
            class Item
            {
                public:
                    virtual ~Item() {}

                    Item& setFilename(const std::string& filename);
                    std::string filename();

                protected:
                    std::string _filename;
            };
        }
    }
}
