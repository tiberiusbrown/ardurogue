#include "ardurogue.hpp"

globals globals_;

static uint8_t u8rand()
{
    const uint16_t temp = (seed0 ^ (seed0 << 5));
    seed0 = seed1;
    seed1 = (seed1 ^ (seed1 >> 1)) ^ (temp ^ (temp >> 3));
    return (uint8_t)seed1;
}

void setup()
{
    for(auto& b : buf) b = 0x0a;
    paint_left();
    paint_right();
}

void loop()
{
    wait_btn();
}
