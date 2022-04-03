#include "game.hpp"

entity_info const MONSTER_INFO[] PROGMEM =
{
    /*
    * mean        poison      opener           max_health
    *    nomove      vampire     strength
    *       regens      confuse       dexterity
    *          invis       paralyze        speed
    */
    { },                                         // none
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,  16,  16,  16,  10 }, // player
    { 0, 0, 0, 0, 0, 0, 0, 0, 0,   4,  24,  32,   1 }, // bat
    { 1, 0, 0, 0, 0, 0, 0, 0, 0,   6,  12,  12,   2 }, // snake
    { 1, 0, 0, 0, 1, 0, 0, 0, 0,   6,  12,  12,   3 }, // rattlesnake
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,  12,   8,   8,   6 }, // zombie
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,  16,  16,  16,  10 }, // goblin
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,  20,  16,  16,  16 }, // orc
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,  24,  16,  16,  20 }, // hobgoblin
    { 1, 0, 1, 0, 0, 0, 0, 0, 1,  32,  12,  12,  32 }, // troll
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,  24,  24,  24,  24 }, // griffin
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,  48,  16,  16,  48 }, // dragon
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
    if(info.nomove)
    {
        a.type = action::WAIT;
        return;
    }
    a.type = action::MOVE;
    uint8_t dp = dist_to_player(e.x, e.y);
    if(!info.mean || dp >= 10)
    {
        a.dir = u8rand() & 3;
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
            dp = td, a.dir = i;
    }
}
