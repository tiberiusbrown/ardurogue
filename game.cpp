#include "game.hpp"

static uint8_t simple_mod(uint8_t n, uint8_t d)
{
    while(n >= d) n -= d;
    return n;
}

globals globals_;

uint8_t u8rand()
{
    rand_seed ^= rand_seed << 7;
    rand_seed ^= rand_seed >> 9;
    rand_seed ^= rand_seed << 8;
    return (uint8_t)rand_seed;
}

uint8_t u8rand(uint8_t m)
{
    return simple_mod(u8rand(), m);
}

bool tile_is_unknown(uint8_t x, uint8_t y)
{
    if(x >= MAP_W || y >= MAP_H) return false;
    uint16_t i = y / 8 * MAP_W + x;
    uint8_t m = 1 << (y % 8);
    return (tfog[i] & m) == 0;
}

bool tile_is_solid_or_unknown(uint8_t x, uint8_t y)
{
    if(x >= MAP_W || y >= MAP_H) return true;
    uint16_t i = y / 8 * MAP_W + x;
    uint8_t m = 1 << (y % 8);
    return ((tmap[i] | ~tfog[i]) & m) != 0;
}

static bool try_move_ent(uint8_t i, uint8_t dx, uint8_t dy)
{
    uint8_t nx = ents[i].x + dx;
    uint8_t ny = ents[i].y + dy;
    if(tile_is_solid(nx, ny))
    {
        door* d = get_door(nx, ny);
        if(d && !d->secret)
        {
            d->open = 1;
            maps[map_index].got_doors.set(index_of_door(*d));
            update_doors();
            update_light();
            return true;
        }
        // TODO: check for other entities to attack
        return false;
    }
    ents[i].x = nx;
    ents[i].y = ny;
    update_light();
    return true;
}

bool tile_is_solid(uint8_t x, uint8_t y)
{
    if(x >= MAP_W || y >= MAP_H) return true;
    uint8_t t = tmap[y / 8 * MAP_W + x];
    return (t & (1 << (y % 8))) != 0;
}

bool tile_is_explored(uint8_t x, uint8_t y)
{
    if(x >= MAP_W || y >= MAP_H) return false;
    uint8_t t = tfog[y / 8 * MAP_W + x];
    return (t & (1 << (y % 8))) != 0;
}

door* get_door(uint8_t x, uint8_t y)
{
    for(uint8_t i = 0; i < num_doors; ++i)
        if(doors[i].x == x && doors[i].y == y)
            return &doors[i];
    return nullptr;
}

uint8_t index_of_door(door const& d)
{
    return uint8_t(ptrdiff_t(&d - &doors.d_[0]));
}

void render()
{
    draw_dungeon(ents[0].x, ents[0].y);
    paint_left();
    draw_info();
    paint_right();
}

void game_setup()
{
    for(uint16_t i = 0; i < sizeof(globals_); ++i)
        ((uint8_t*)&globals_)[i] = 0;

    ents[0].type = entity::PLAYER;
    ents[0].x = 4;
    ents[0].y = 4;

    game_seed = 0xbabe;

    generate_dungeon(0);
    update_light();
    render();
}

void game_loop()
{
    uint8_t b = wait_btn();
    bool rd = false;
    switch(b)
    {
    case BTN_UP   : rd = try_move_ent(0, 0, -1); break;
    case BTN_DOWN : rd = try_move_ent(0, 0, +1); break;
    case BTN_LEFT : rd = try_move_ent(0, -1, 0); break;
    case BTN_RIGHT: rd = try_move_ent(0, +1, 0); break;
    case BTN_A:
        if(++opt.wall_style == NUM_WALL_STYLES) opt.wall_style = 0;
        rd = true;
        break;
    case BTN_B:
        //++game_seed;
        generate_dungeon(0);
        render();
        break;
    default: break;
    }
    if(rd) render();
}
