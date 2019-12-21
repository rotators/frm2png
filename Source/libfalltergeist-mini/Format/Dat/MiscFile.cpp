#include <utility>

#include "../Dat/MiscFile.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Dat
        {
            MiscFile::MiscFile(Stream&& stream) : _stream(std::move(stream))
            {
            }

            Stream& MiscFile::stream()
            {
                return _stream;
            }
        }
    }
}
