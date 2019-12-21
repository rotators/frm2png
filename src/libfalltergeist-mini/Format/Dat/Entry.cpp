#include <algorithm>
#include <cstdint>

#include "../Dat/Entry.h"
#include "../Dat/File.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Dat
        {
            Entry::Entry(File* datFile)
            {
                _datFile = datFile;
            }

            std::string Entry::filename() const
            {
                return _filename;
            }

            void Entry::setFilename(std::string value)
            {
                std::replace(value.begin(), value.end(), '\\','/');
                std::transform(value.begin(), value.end(), value.begin(), ::tolower);
                _filename = value;
            }

            uint32_t Entry::packedSize() const
            {
                return _packedSize;
            }

            void Entry::setPackedSize(std::uint32_t value)
            {
                _packedSize = value;
            }

            uint32_t Entry::unpackedSize() const
            {
                return _unpackedSize;
            }

            void Entry::setUnpackedSize(std::uint32_t value)
            {
                _unpackedSize = value;
            }

            uint32_t Entry::dataOffset() const
            {
                return _dataOffset;
            }

            void Entry::setDataOffset(std::uint32_t value)
            {
                _dataOffset = value;
            }

            bool Entry::compressed() const
            {
                return _compressed;
            }

            void Entry::setCompressed(bool value)
            {
                _compressed = value;
            }

            File* Entry::datFile()
            {
                return _datFile;
            }
        }
    }
}
