#include <algorithm>
#include "../Dat/Stream.h"
#include "../Frm/File.h"
#include "../Pal/File.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Frm
        {
            File::File(Dat::Stream&& stream)
            {
                stream.setPosition(0);

                _version = stream.uint32();
                _framesPerSecond = stream.uint16();
                _actionFrame = stream.uint16();
                _framesPerDirection = stream.uint16();

                uint16_t shiftX[6];
                uint16_t shiftY[6];
                uint32_t dataOffset[6];
                for (unsigned int i = 0; i != 6; ++i) shiftX[i] = stream.uint16();
                for (unsigned int i = 0; i != 6; ++i) shiftY[i] = stream.uint16();
                for (unsigned int i = 0; i != 6; ++i)
                {
                    dataOffset[i] = stream.uint32();
                    if (i > 0 && dataOffset[i-1] == dataOffset[i])
                    {
                        continue;
                    }

                    _directions.emplace_back();
                    auto& direction = _directions.back();
                    direction.setDataOffset(dataOffset[i]);
                    direction.setShiftX(shiftX[i]);
                    direction.setShiftY(shiftY[i]);
                }

                // for each direction
                for (auto& direction : _directions)
                {
                    // jump to frames data at frames area
                    stream.setPosition(direction.dataOffset() + 62);

                    // read all frames
                    for (unsigned i = 0; i != _framesPerDirection; ++i)
                    {
                        uint16_t width = stream.uint16();
                        uint16_t height = stream.uint16();

                        direction.frames().emplace_back(width, height);
                        auto& frame = direction.frames().back();

                        // Number of pixels for this frame
                        // We don't need this, because we already have width*height
                        stream.uint32();

                        frame.setOffsetX(stream.int16());
                        frame.setOffsetY(stream.int16());

                        // Pixels data
                        stream.readBytes(frame.data(), frame.width() * frame.height());
                    }
                }
            }

            uint32_t File::version() const
            {
                return _version;
            }

            uint16_t File::framesPerSecond() const
            {
                return _framesPerSecond;
            }

            uint16_t File::framesPerDirection() const
            {
                return _framesPerDirection;
            }

            uint16_t File::actionFrame() const
            {
                return _actionFrame;
            }

            const std::vector<Direction>& File::directions() const
            {
                return _directions;
            }

            uint16_t File::maxFrameWidth() const
            {
                uint16_t width = 0;
                std::for_each( _directions.begin(), _directions.end(), [&width]( const Direction& dir )
                {
                    width = std::max( width, dir.maxFrameWidth() );
                });

                return width;
            }

            uint16_t File::maxFrameHeight() const
            {
                uint16_t height = 0;
                std::for_each( _directions.begin(), _directions.end(), [&height]( const Direction& dir )
                {
                    height = std::max( height, dir.maxFrameHeight() );
                });

                return height;
            }

            int16_t File::offsetX(unsigned int direction, unsigned int frame) const
            {
                if (direction >= _directions.size()) direction = 0;
                return _directions.at(direction).frames().at(frame).offsetX();
            }

            int16_t File::offsetY(unsigned int direction, unsigned int frame) const
            {
                if (direction >= _directions.size()) direction = 0;
                return _directions.at(direction).frames().at(frame).offsetY();
            }
        }
    }
}
