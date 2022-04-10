#include "game.hpp"

entity_info const MONSTER_INFO[] PROGMEM =
{
    /*
    * mean        poison      opener         defense
    *    nomove      vampire     strength        max_health
    *       regens      confuse      dexterity        xp
    *          invis       paralyze      speed
    */
    { },                                                        // none
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,  4,  4,  4,  0,  10,  0 }, // player
    { 0, 0, 0, 0, 0, 0, 0, 0, 0,  1,  6,  8,  0,   1,  1 }, // bat
    { 1, 0, 0, 0, 0, 0, 0, 0, 0,  2,  3,  3,  0,   2,  2 }, // snake
    { 1, 0, 0, 0, 1, 0, 0, 0, 0,  3,  3,  3,  0,   3,  3 }, // rattlesnake
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,  4,  2,  2,  0,   6,  5 }, // zombie
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,  5,  4,  4,  1,  10,  6 }, // goblin
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,  7,  4,  4,  2,  16,  7 }, // orc
    { 1, 0, 0, 0, 0, 0, 0, 0, 1,  8,  4,  4,  3,  20,  8 }, // hobgoblin
    { 1, 1, 0, 0, 0, 0, 0, 0, 1,  8,  4,  4,  3,  20,  8 }, // mimic
    { 1, 0, 1, 0, 0, 0, 0, 0, 1, 12,  3,  3,  5,  32, 11 }, // troll
    { 1, 0, 0, 0, 0, 0, 0, 0, 0,  7,  6,  6,  1,  24, 12 }, // griffin
    { 1, 0, 0, 0, 0, 0, 0, 0, 0, 20,  4,  4,  8,  48, 20 }, // dragon
};

void entity_get_info(uint8_t i, entity_info& info)
{
    if(i == 0)
        info = pstats;
    else
        pgm_memcpy(&info, &MONSTER_INFO[ents[i].type], sizeof(info));
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
    entity_info info;
    entity_get_info(i, info);
    a.type = action::WAIT;
    if((info.nomove && !e.aggro) || player_is_dead())
        return;
    uint8_t dp = dist_to_player(e.x, e.y);
    bool mean = info.mean || e.aggro;
    if(!mean || pstats.invis || dp >= 10)
    {
        a.data = u8rand() & 3;
        uint8_t nx = e.x + (int8_t)pgm_read_byte(&DIRX[a.data]);
        uint8_t ny = e.y + (int8_t)pgm_read_byte(&DIRY[a.data]);
        entity* e = get_entity(nx, ny);
        if(e && (!mean || e->type != entity::PLAYER))
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
            dp = td, a.data = i;
        }
    }
}
