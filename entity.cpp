#include "game.hpp"

uint8_t entity_speed(uint8_t i)
{
    uint8_t r;
    if(i == 0)
    {
        // TODO: factor in item modifiers
        r = pstats.speed;
    }
    else
        r = pgm_read_byte(&MONSTER_INFO[ents[i].type].speed);
    // TODO: factor in slow/speed effect
    return r;
}

uint8_t entity_max_health(uint8_t i)
{
    uint8_t r;
    if(i == 0)
    {
        // TODO: factor in item modifiers
        r = pstats.max_health;
    }
    else
        r = pgm_read_byte(&MONSTER_INFO[ents[i].type].max_health);
    return r;
}

void advance_entity(uint8_t i)
{
    auto& e = ents[i];
    if(e.type == entity::NONE) return;
    action a;

    // moster ai
    if(i != 0 && !e.paralyzed)
    {
        if(e.confused)
            a.type = action::MOVE, a.dir = u8rand() & 3;
        else
            monster_ai(i, a);
        entity_perform_action(i, a);
    }

    // advance confusion/paralysis
    if(e.confused)
    {
        bool end;
        if(i == 0) end = (--pinfo.confuse_rem == 0);
        else end = ((u8rand() & 7) == 0);
        if(end)
        {
            e.confused = 0;
            status(PSTR("%S %I no longer confused."), e.type, e.type);
        }
    }
    if(e.paralyzed)
    {
        bool end;
        if(i == 0) end = (--pinfo.paralyze_rem == 0);
        else end = ((u8rand() & 7) == 0);
        if(end)
        {
            e.paralyzed = 0;
            status(PSTR("%S %I no longer paralyzed."), e.type, e.type);
        }
    }
    
    // advance invis for player
    if(i == 0 && pinfo.invis_rem > 0 && --pinfo.invis_rem == 0)
    {
        pstats.invis = 0;
        status(PSTR("You are no longer invisible."));
    }

    render();
}

bool entity_perform_action(uint8_t i, action const& a)
{
    auto& e = ents[i];
    uint8_t dir = a.dir;
    if(a.type != action::WAIT)
    {
        if(e.paralyzed)
        {
            if(i == 0)
                status(PSTR("You are paralyzed!"));
            return true; // absorb action
        }
        if(e.confused)
        {
            dir = u8rand() & 3;
            if(i == 0)
                status(PSTR("You are confused!"));
        }
    }
    int8_t dx = (int8_t)pgm_read_byte(&DIRX[dir & 3]);
    int8_t dy = (int8_t)pgm_read_byte(&DIRY[dir & 3]);
    uint8_t nx = e.x + dx;
    uint8_t ny = e.y + dy;
    auto info = entity_get_info(i);
    switch(a.type)
    {
    case action::CLOSE:
    {
        door* d = get_door(nx, ny);
        if(d && !d->secret)
        {
            if(!d->open)
                status(PSTR("The door is already closed."));
            else if(get_entity(nx, ny))
                status(PSTR("The door is blocked."));
            else
            {
                d->open = 0;
                return true;
            }
        }
        else
            status(PSTR("You see no door there."));
        return false;
    }
    case action::MOVE:
    {
        door* d = get_door(nx, ny);
        if(d && !d->open && !d->secret && info.opener)
        {
            d->open = 1;
            maps[map_index].got_doors.set(index_of_door(*d));
            return true;
        }
        if(tile_is_solid(nx, ny))
            return false;
        entity* te = get_entity(nx, ny);
        if(te)
        {
            if(!info.mean && !e.confused) return true;
            bool msg = player_can_see(e.x, e.y);
            if(msg)
                status(PSTR("%S %V %O."), e.type, e.type, PSTR("hit"), te->type);
            // TODO: damage etc
            return true;
        }
        e.x = nx;
        e.y = ny;
        return true;
    }
    case action::WAIT:
        return true;
    default:
        break;
    }
    return false;
}
