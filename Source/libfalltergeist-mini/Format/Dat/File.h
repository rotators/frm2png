#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>

#include "Entry.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Dat
        {
            class File
            {
                public:
                    File();
                    File(const std::string& pathToFile);

                    std::string filename() const;
                    File* setFilename(const std::string& filename);

                    // an pointer to an entry with given name or nullptr if no such entry exists
                    Entry* entry(const std::string& filename);

                    File* readBytes(char* destination, uint32_t numberOfBytes);
                    File* skipBytes(uint32_t numberOfBytes);
                    File* setPosition(uint32_t position);
                    uint32_t position();
                    uint32_t size();

                    File& operator>>(int32_t &value);
                    File& operator>>(uint32_t &value);
                    File& operator>>(int16_t &value);
                    File& operator>>(uint16_t &value);
                    File& operator>>(int8_t &value);
                    File& operator>>(uint8_t &value);
                    File& operator>>(Entry &entry);

                protected:
                    std::unordered_map<std::string, Dat::Entry> _entries;
                    std::ifstream _stream;
                    std::string _filename;
                    void _initialize();
            };
        }
    }
}
