#include "game.hpp"

bool player_can_see(uint8_t x, uint8_t y)
{
    uint8_t px = ents[0].x;
    uint8_t py = ents[0].y;
    int8_t dx = px - x;
    int8_t dy = py - y;
    return
        dx * dx + dy * dy < light_radius2() &&
        path_clear(px, py, x, y);
}

bool path_clear(
    uint8_t x0, uint8_t y0,
    uint8_t x1, uint8_t y1)
{
    int8_t xdir = x0 < x1 ? 1 : x0 == x1 ? 0 : -1;
    int8_t ydir = y0 < y1 ? 1 : y0 == y1 ? 0 : -1;

    //if(tile_is_solid(x0, y0))
    //    x0 += xdir, y0 += ydir;

    uint8_t xdist = u8abs(x0 - x1);
    uint8_t ydist = u8abs(y0 - y1);
    int8_t xi = ydist / 2;
    int8_t yi = xdist / 2;

    while(x0 != x1 || y0 != y1)
    {
        if(tile_is_solid(x0, y0))
            return false;
        xi += xdist;
        yi += ydist;
        if(xi >= ydist)
            x0 += xdir, xi -= ydist;
        if(yi >= xdist)
            y0 += ydir, yi -= xdist;
    }

    return true;
}

static void set_tile_explored(uint8_t x, uint8_t y)
{
    uint16_t const i = y / 8 * MAP_W + x;
    uint8_t const m = 1 << (y % 8);
    tfog[i] |= m;
}

void update_light()
{
    uint8_t px = ents[0].x;
    uint8_t py = ents[0].y;
    uint8_t r = light_radius();
    uint8_t r2 = light_radius2();
    uint8_t ay = py < r ? 0 : py - r;
    uint8_t ax = px < r ? 0 : px - r;
    uint8_t by = py + r;
    uint8_t bx = px + r;
    if(by >= MAP_H) by = MAP_H - 1;
    if(bx >= MAP_W) bx = MAP_W - 1;
    for(uint8_t y = ay; y <= by; ++y)
        for(uint8_t x = ax; x <= bx; ++x)
        {
            uint8_t dx = u8abs(x - px);
            uint8_t dy = u8abs(y - py);
            if(dx * dx + dy * dy < r2 && path_clear(px, py, x, y))
                set_tile_explored(x, y);
        }

    // post process
    for(uint8_t y = ay; y <= by; ++y)
        for(uint8_t x = ax; x <= bx; ++x)
        {
            if(tile_is_solid(x, y) && (
                tile_is_explored(x - 1, y) ||
                tile_is_explored(x + 1, y) ||
                tile_is_explored(x, y - 1) ||
                tile_is_explored(x, y + 1)))
            {
                set_tile_explored(x, y);
            }
        }
}
