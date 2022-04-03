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

entity* get_entity(uint8_t x, uint8_t y)
{
    for(uint8_t i = 0; i < MAP_ENTITIES; ++i)
        if(ents[i].type != entity::NONE && ents[i].x == x && ents[i].y == y)
            return &ents[i];
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

    game_seed = 0xbabe;
    map_index = 0;
    generate_dungeon();

    pinfo = {};
    pgm_memcpy(&pstats, &MONSTER_INFO[entity::PLAYER], sizeof(pstats));
    new_entity(0, entity::PLAYER, xup, yup);

    for(uint8_t i = 1; i < 24; ++i)
    {
        auto c = find_unoccupied();
        new_entity(i, entity::BAT, c.x, c.y);
    }

    statusn = 0;
    statusx = 1;
    statusy = STATUS_START_Y;
    status(PSTR("Welcome to ArduRogue."));

    update_light();
    render();
}

// advance all entities and effects
static void advance()
{
    uint8_t pspeed = entity_speed(0);
    for(uint8_t i = 0; i < MAP_ENTITIES; ++i)
    {
        uint8_t espeed = entity_speed(i);
        while(espeed >= pspeed)
        {
            advance_entity(i);
            espeed -= pspeed;
        }
        if(u8rand(pspeed) < espeed)
            advance_entity(i);
    }
}

void game_loop()
{
    uint8_t b = wait_btn();
    statusn = 0;
    statusx = 1;
    statusy = STATUS_START_Y;
    action a{};
    switch(b)
    {
    case BTN_UP   : a.type = action::MOVE; a.dir = 0; break;
    case BTN_DOWN : a.type = action::MOVE; a.dir = 1; break;
    case BTN_LEFT : a.type = action::MOVE; a.dir = 2; break;
    case BTN_RIGHT: a.type = action::MOVE; a.dir = 3; break;
    case BTN_A:
        if(++opt.wall_style == NUM_WALL_STYLES) opt.wall_style = 0;
        break;
    case BTN_B:
        //++game_seed;
        generate_dungeon();
        break;
    default: break;
    }
    if(entity_perform_action(0, a))
    {
        update_doors();
        update_light();
        render();
        advance();
    }
}
