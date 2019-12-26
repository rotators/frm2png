#include <stdexcept>

#include "../Dat/File.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Dat
        {
            File::File()
            {
                _initialize();
            }

            File::File(const std::string& filename)
            {
                setFilename(filename);
                _initialize();
            }

            std::string File::filename() const
            {
                return _filename;
            }

            File* File::setFilename(const std::string& filename)
            {
                _filename = filename;
                return this;
            }

            void File::_initialize()
            {
                _stream.open(filename(), std::ios_base::binary);
                if (!_stream.is_open())
                    throw std::runtime_error("Format::Dat::File::_initialize() - can't open stream: " + filename());

                unsigned int FileSize;
                unsigned int filesTreeSize;
                unsigned int filesTotalNumber;

                // reading data size from dat file
                setPosition(size() - 4);
                *this >> FileSize;
                if (FileSize != size())
                    throw std::runtime_error("Format::Dat::File::items() - wrong file size");

                // reading size of files tree
                setPosition(size() - 8);
                *this >> filesTreeSize;

                // reading total number of items in dat file
                setPosition(size() - filesTreeSize - 8);
                *this >> filesTotalNumber;

                //reading files data one by one
                for (unsigned int i = 0; i != filesTotalNumber; ++i)
                {
                    Entry entry(this);
                    *this >> entry;
                    _entries.emplace(entry.filename(), std::move(entry));
                }
            }

            File* File::setPosition(unsigned int position)
            {
                _stream.seekg(position, std::ios::beg);
                return this;
            }

            unsigned int File::position()
            {
                return static_cast<unsigned>(_stream.tellg());
            }

            unsigned int File::size(void)
            {
                auto oldPosition = _stream.tellg();
                _stream.seekg(0,std::ios::end);
                auto currentPosition = _stream.tellg();
                _stream.seekg(oldPosition, std::ios::beg);
                return static_cast<unsigned>(currentPosition);
            }

            File* File::skipBytes(unsigned int numberOfBytes)
            {
                setPosition(position() + numberOfBytes);
                return this;
            }

            File* File::readBytes(char* destination, unsigned int numberOfBytes)
            {
                unsigned int position = this->position();
                _stream.read(destination, numberOfBytes);
                setPosition(position + numberOfBytes);
                return this;
            }

            Entry* File::entry(const std::string& filename)
            {
                auto entryIt = _entries.find(filename);
                if (entryIt != _entries.end()) {
                    return &entryIt->second;
                }
                return nullptr;
            }

            File& File::operator>>(int32_t &value)
            {
                readBytes(reinterpret_cast<char *>(&value), sizeof(value));
                return *this;
            }

            File& File::operator>>(uint32_t &value)
            {
                return *this >> (int32_t&) value;
            }

            File& File::operator>>(int16_t &value)
            {
                readBytes(reinterpret_cast<char *>(&value), sizeof(value));
                return *this;
            }

            File& File::operator>>(uint16_t &value)
            {
                return *this >> (int16_t&) value;
            }

            File& File::operator>>(int8_t &value)
            {
                readBytes(reinterpret_cast<char *>(&value), sizeof(value));
                return *this;
            }

            File& File::operator>>(uint8_t &value)
            {
                return *this >> (int8_t&) value;
            }

            File& File::operator>>(Entry& entry)
            {
                uint32_t filenameSize;
                uint8_t compressed;
                uint32_t unpackedSize;
                uint32_t packedSize;
                uint32_t dataOffset;

                *this >> filenameSize;

                std::string filename;
                filename.resize(filenameSize);
                readBytes(&filename[0], filenameSize);
                entry.setFilename(filename);

                *this >> compressed >> unpackedSize >> packedSize >> dataOffset;

                entry.setCompressed((bool) compressed);
                entry.setUnpackedSize(unpackedSize);
                entry.setPackedSize(packedSize);
                entry.setDataOffset(dataOffset);

                return *this;
            }
        }
    }
}
