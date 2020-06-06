#pragma once

#include <cstdint>

class Player {
public:
    uint16_t id;
    int16_t posX;
    int16_t posY;
    bool moved;
};
