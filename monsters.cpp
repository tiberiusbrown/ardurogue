#include "game.hpp"

entity_info const MONSTER_INFO[] PROGMEM =
{
    /*
    * mean        poison      opener            defense
    *    nomove      vampire     strength           max_health
    *       regens      confuse       dexterity          xp
    *          invis       paralyze        speed
    */
    { },                                                        // none
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,   4,   4,  16,  0,  10,   0 }, // player
    { 0, 0, 0, 0, 0, 0, 0, 0, 0,   1,   6,  32,  0,   1,   1 }, // bat
    { 1, 0, 0, 0, 0, 0, 0, 0, 0,   2,   3,  12,  0,   2,   2 }, // snake
    { 1, 0, 0, 0, 1, 0, 0, 0, 0,   3,   3,  12,  0,   3,   3 }, // rattlesnake
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,   4,   2,   8,  0,   6,   5 }, // zombie
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,   5,   4,  16,  1,  10,   6 }, // goblin
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,   7,   4,  16,  2,  16,   7 }, // orc
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,   8,   4,  16,  3,  20,   8 }, // hobgoblin
    { 1, 0, 1, 0, 0, 0, 0, 0, 1,  12,   3,  12,  5,  32,  11 }, // troll
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,   7,   6,  24,  1,  24,  12 }, // griffin
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,  20,   4,  16,  8,  48,  20 }, // dragon
};

entity_info entity_get_info(uint8_t i)
{
    if(i == 0) return pstats;
    entity_info r;
    pgm_memcpy(&r, &MONSTER_INFO[ents[i].type], sizeof(r));
    return r;
}

static uint8_t dist_to_player(uint8_t x, uint8_t y)
{
    uint8_t dx = u8abs(x - ents[0].x);
    uint8_t dy = u8abs(y - ents[0].y);
    return dx + dy; // manhatten distance
}

void monster_ai(uint8_t i, action& a)
{
    auto& e = ents[i];
    auto info = entity_get_info(i);
    a.type = action::WAIT;
    if(info.nomove || player_is_dead())
        return;
    uint8_t dp = dist_to_player(e.x, e.y);
    if(!info.mean || pstats.invis || dp >= 10)
    {
        a.dir = u8rand() & 3;
        uint8_t nx = e.x + (int8_t)pgm_read_byte(&DIRX[a.dir]);
        uint8_t ny = e.y + (int8_t)pgm_read_byte(&DIRY[a.dir]);
        entity* e = get_entity(nx, ny);
        if(e && (!info.mean || e->type != entity::PLAYER))
            return;
        a.type = action::MOVE;
        return;
    }
    for(uint8_t i = 0; i < 4; ++i)
    {
        uint8_t nx = e.x + (int8_t)pgm_read_byte(&DIRX[i]);
        uint8_t ny = e.y + (int8_t)pgm_read_byte(&DIRY[i]);
        if(tile_is_solid(nx, ny)) continue;
        if(entity* e = get_entity(nx, ny))
            if(e->type != entity::PLAYER)
                continue;
        uint8_t td = dist_to_player(nx, ny);
        if(td < dp || (td == dp && (u8rand() & 1)))
        {
            a.type = action::MOVE;
            dp = td, a.dir = i;
        }
    }
}
