#include <algorithm>
#include "../Frm/Direction.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Frm
        {
            int16_t Direction::shiftX() const
            {
                return _shiftX;
            }

            void Direction::setShiftX(int16_t value)
            {
                _shiftX = value;
            }

            int16_t Direction::shiftY() const
            {
                return _shiftY;
            }

            void Direction::setShiftY(int16_t value)
            {
                _shiftY = value;
            }

            uint32_t Direction::dataOffset() const
            {
                return _dataOffset;
            }

            uint16_t Direction::maxFrameWidth() const
            {
                return std::max_element( _frames.begin(), _frames.end(), []( const Frame& a, const Frame& b )
                {
                    return a.width() < b.width();
                })->width();
            }

            uint16_t Direction::maxFrameHeight() const
            {
                return std::max_element( _frames.begin(), _frames.end(), []( const Frame& a, const Frame& b )
                {
                    return a.height() < b.height();
                })->height();
            }

            void Direction::setDataOffset(uint32_t value)
            {
                _dataOffset = value;
            }

            std::vector<Frame>& Direction::frames()
            {
                return _frames;
            }

            const std::vector<Frame>& Direction::frames() const
            {
                return _frames;
            }
        }
    }
}
