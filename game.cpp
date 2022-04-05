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
    return uint8_t(ptrdiff_t(&d - &doors[0]));
}

uint8_t index_of_entity(entity const& e)
{
    return uint8_t(ptrdiff_t(&e - &ents[0]));
}

uint8_t xp_for_level()
{
    return tmin<uint8_t>(plevel, 15) * 16 + 10;
}

void player_gain_xp(uint8_t xp)
{
    uint8_t txp = xp_for_level();
    if(txp - xp < pstats.xp)
    {
        ++plevel;
        status(PSTR("You advance to level @u!"), plevel + 1);
        pstats.max_health += 4;
        if(plevel <= 19)
        {
            if(plevel % 4 == 2)
            {
                status(PSTR("You feel stronger."));
                pstats.strength += 1;
            }
            if(plevel % 4 == 0)
            {
                status(PSTR("You feel quicker."));
                pstats.dexterity += 1;
            }
        }
        ents[0].health = entity_max_health(0);
        pstats.xp += (xp - txp);
    }
    else
        pstats.xp += xp;
}

void render()
{
    draw_dungeon(ents[0].x, ents[0].y);
    paint_left();
    draw_info();
    paint_right();
}

static void init_perm(uint8_t* p, uint8_t n)
{
    for(uint8_t i = 0; i < n; ++i)
        p[i] = i;
    for(uint8_t i = 0; i < n - 1; ++i)
        swap(p[i], p[u8rand(n - i)]);
}

void game_setup()
{
    stack_canary_init();
    memzero(&globals_, sizeof(globals_));

    game_seed = 0xbabe;
    opt.wall_style = 2;

    rand_seed = game_seed;
    init_perm(perm_pot.data(), perm_pot.size());
    init_perm(perm_scr.data(), perm_scr.size());
    init_perm(perm_rng.data(), perm_rng.size());
    init_perm(perm_amu.data(), perm_amu.size());

    map_index = 0;
    generate_dungeon();

    pinfo = {};
    pgm_memcpy(&pstats, &MONSTER_INFO[entity::PLAYER], sizeof(pstats));
    new_entity(0, entity::PLAYER, xup, yup);

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
    statusn = 0;
    statusx = 1;
    statusy = STATUS_START_Y;
    action a{};
    uint8_t b = wait_btn();
    switch(b)
    {
    case BTN_UP   : a.type = action::MOVE; a.dir = 0; break;
    case BTN_DOWN : a.type = action::MOVE; a.dir = 1; break;
    case BTN_LEFT : a.type = action::MOVE; a.dir = 2; break;
    case BTN_RIGHT: a.type = action::MOVE; a.dir = 3; break;
    case BTN_A:
        //if(++opt.wall_style == NUM_WALL_STYLES) opt.wall_style = 0;
        repeat_action(a);
        break;
    case BTN_B:
        if(!action_menu(a))
        {
            render();
            return;
        }
        break;
    default: break;
    }
    uint8_t px = ents[0].x, py = ents[0].y;
    if(entity_perform_action(0, a))
    {
        update_doors();
        update_light();
        advance();
    }
    render();
    
    // checks after the player moved
    if(px != ents[0].x || py != ents[0].y)
    {
        if(ents[0].x == xdn && ents[0].y == ydn &&
            yesno_menu(PSTR("Go down to the next dungeon?")))
        {
            ++map_index;
            generate_dungeon();
            ents[0].x = xup, ents[0].y = yup;
            update_light();
            render();
        }
        else if(ents[0].x == xup && ents[0].y == yup &&
            yesno_menu(PSTR("Go back up the stairs?")))
        {
            --map_index;
            // TODO: test for returning to surface
            generate_dungeon();
            ents[0].x = xdn, ents[0].y = ydn;
            update_light();
            render();
        }
    }
}
