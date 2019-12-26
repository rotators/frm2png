#pragma once

#include <cstdint>
#include <vector>
#include "../Frm/Frame.h"

namespace Falltergeist
{
    namespace Format
    {
        namespace Frm
        {
            class Direction
            {
                public:
                    Direction() = default;
                    Direction(Direction&&) = default;
                    Direction(const Direction&) = delete;
                    Direction& operator= (const Direction&) = delete;
                    Direction& operator= (Direction&&) = default;
                    ~Direction() = default;

                    int16_t shiftX() const;
                    void setShiftX(int16_t value);

                    int16_t shiftY() const;
                    void setShiftY(int16_t value);

                    uint32_t dataOffset() const;
                    void setDataOffset(uint32_t value);

                    uint16_t maxFrameWidth() const;
                    uint16_t maxFrameHeight() const;

                    std::vector<Frame>& frames();
                    const std::vector<Frame>& frames() const;

                protected:
                    int16_t _shiftX = 0;
                    int16_t _shiftY = 0;
                    uint32_t _dataOffset = 0;
                    std::vector<Frame> _frames;
            };
        }
    }
}
