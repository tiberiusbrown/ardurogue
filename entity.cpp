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
        r = pstats.max_health;
        if(pinfo.vamp_drain >= r)
            return 0;
    }
    else
        r = pgm_read_byte(&MONSTER_INFO[ents[i].type].max_health);
    return r;
}

uint8_t entity_strength(uint8_t i)
{
    uint8_t r;
    if(i == 0)
        r = pstats.strength;
    else
        r = pgm_read_byte(&MONSTER_INFO[ents[i].type].strength);
    if(ents[i].weakened)
        r = (r + 1) / 2;
    if(i == 0)
        ; // TODO: item modifiers
    return r;
}

uint8_t entity_dexterity(uint8_t i)
{
    uint8_t r;
    if(i == 0)
    {
        // TODO: factor in item modifiers
        r = pstats.dexterity;
    }
    else
        r = pgm_read_byte(&MONSTER_INFO[ents[i].type].dexterity);
    return r;
}

uint8_t entity_attack(uint8_t i)
{
    uint8_t r = entity_strength(i);
    // TODO: factor in wielded item
    return r;
}

uint8_t entity_defense(uint8_t i)
{
    uint8_t r;
    if(i == 0)
    {
        // TODO: factor in item modifiers
        r = pstats.defense;
    }
    else
        r = pgm_read_byte(&MONSTER_INFO[ents[i].type].defense);
    // TODO: factor in slow/speed effect
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
            status(PSTR("@S @A no longer confused."), e.type, e.type);
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
            status(PSTR("@S @A no longer paralyzed."), e.type, e.type);
        }
    }
    
    // advance invis for player
    if(i == 0 && pinfo.invis_rem > 0 && --pinfo.invis_rem == 0)
    {
        pstats.invis = 0;
        status(PSTR("You are no longer invisible."));
    }
}

bool test_attack_hit(uint8_t atti, uint8_t defi) // 0 for miss
{
    uint8_t ta = entity_dexterity(atti);
    uint8_t td = entity_dexterity(defi);
    return u8rand(ta * 2 + td) >= td;
}

uint8_t calculate_hit_damage(uint8_t atti, uint8_t defi) // 0 for block
{
    uint8_t ta = entity_attack(atti);
    uint8_t td = entity_defense(defi);
    ta = u8rand(ta + td) + 1;
    if(ta < td)
        return 0;
    return ta - td;
}

void entity_heal(uint8_t i, uint8_t amount, bool cansee)
{

}

void entity_take_damage(uint8_t atti, uint8_t defi, uint8_t dam, bool cansee)
{
    auto const& e = ents[atti];
    auto info = entity_get_info(atti);
    auto& te = ents[defi];
    if(dam > te.health)
    {
        if(cansee)
            status(PSTR("@S @V!"), te.type, te.type, PSTR("die"));
        else
            status(PSTR("You hear the sound of death."));
        uint8_t tt = te.type;
        te.type = entity::NONE;
        maps[map_index].got_ents.set(defi);
        if(atti == 0)
            player_gain_xp(pgm_read_byte(&MONSTER_INFO[tt].xp));
    }
    else
    {
        if(defi == 0 && info.vampire && u8rand() < 40)
        {
            status(PSTR("@S drains your health!"), e.type);
            pinfo.vamp_drain += 3;
            ents[0].health = tmin(ents[0].health, entity_max_health(0));
            if(ents[0].health == 0)
            {
                status(PSTR("@S @V!"), te.type, te.type, PSTR("die"));
                return;
            }
        }

        if(info.confuse && u8rand() < 40)
        {
            if(cansee)
                status(PSTR("@S @V confused!"), te.type, te.type, PSTR("become"));
            if(defi == 0)
                pinfo.confuse_rem = u8rand() % 4 + 4;
            te.confused = 1;
        }

        if(info.poison && !te.weakened && u8rand() < 20)
        {
            if(cansee)
                status(PSTR("@S @A weakened!"), te.type, te.type);
            te.weakened = 1;
        }

        te.health -= dam;
    }
}

static void entity_attack_entity(uint8_t atti, uint8_t defi, bool cansee)
{
    bool hit = test_attack_hit(atti, defi);
    auto const& e = ents[atti];
    auto& te = ents[defi];
    if(!hit && cansee)
        status(PSTR("@S @v @O."), e.type, e.type, PSTR("miss"), te.type);
    if(atti == 0)
        te.aggro = 1;
    if(hit)
    {
        uint8_t dam = calculate_hit_damage(atti, defi);
        if(cansee)
        {
            if(dam > 0)
                status(PSTR("@S @V @O."), e.type, e.type, PSTR("hit"), te.type);
            else
                status(PSTR("@S @V @P attack."), te.type, te.type, PSTR("block"), e.type);
        }
        entity_take_damage(atti, defi, dam, cansee);
    }
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
            bool cansee = player_can_see(e.x, e.y);
            uint8_t ti = index_of_entity(*te);
            entity_attack_entity(i, ti, cansee);
            return true;
        }
        e.x = nx;
        e.y = ny;
        return true;
    }
    case action::WAIT:
        if(i == 0)
        {
            // search
            for(uint8_t j = 0; j < 4; ++j)
            {
                uint8_t tx = e.x + (int8_t)pgm_read_byte(&DIRX[j]);
                uint8_t ty = e.y + (int8_t)pgm_read_byte(&DIRY[j]);
                door* d = get_door(tx, ty);
                if(!d) continue;
                if(d->secret)
                {
                    status(PSTR("You found a hidden door!"));
                    d->secret = 0;
                }
            }
        }
        return true;
    default:
        break;
    }
    return false;
}
