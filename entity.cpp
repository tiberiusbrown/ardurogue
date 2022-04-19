#include "game.hpp"

static constexpr uint8_t ARROW_BREAK_CHANCE = 64;

uint8_t entity_speed(uint8_t i)
{
    uint8_t r;
    if(i == 0)
    {
        r = pstats.speed;
        // amulet of speed
        uint8_t j = pinfo.equipped[SLOT_AMULET];
        if(j < INV_ITEMS && inv[j].subtype == AMU_SPEED)
        {
            item const& it = inv[j];
            uint8_t n = it.quant_or_level / 2 + 1;
            if(it.cursed)
                r -= n;
            else
                r += n;
        }
    }
    else
        r = pgm_read_byte(&MONSTER_INFO[ents[i].type].speed);
    // TODO: factor in slow/speed effect
    if((r & 0x80) || r == 0) r = 1;
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
    {
        // ring bonus is after weaken
        r += ring_bonus(RNG_STRENGTH);
    }
    if((int8_t)r < 0) r = 0;
    return r;
}

uint8_t entity_dexterity(uint8_t i)
{
    uint8_t r;
    if(i == 0)
    {
        r = pstats.dexterity;
        r += ring_bonus(RNG_DEXTERITY);
    }
    else
        r = pgm_read_byte(&MONSTER_INFO[ents[i].type].dexterity);
    if((int8_t)r < 0) r = 0;
    return r;
}

uint8_t entity_attack(uint8_t i)
{
    uint8_t r = entity_strength(i);
    if(i == 0)
    {
        uint8_t j = pinfo.equipped[SLOT_WEAPON];
        if(j < INV_ITEMS)
            r += weapon_item_attack(inv[j]);
    }
    if((int8_t)r < 0) r = 0;
    return r;
}

uint8_t entity_defense(uint8_t i)
{
    uint8_t r;
    if(i == 0)
    {
        r = pstats.defense;
        // ring of protection
        r += ring_bonus(RNG_PROTECTION);
        // armor
        for(uint8_t i = SLOT_ARMOR; i <= SLOT_BOOTS; ++i)
        {
            uint8_t j = pinfo.equipped[i];
            if(j < INV_ITEMS)
                r += armor_item_defense(inv[j]);
        }
    }
    else
        r = pgm_read_byte(&MONSTER_INFO[ents[i].type].defense);
    // TODO: factor in slow/speed effect
    if((int8_t)r < 0) r = 0;
    return r;
}

void advance_entity(uint8_t i)
{
    auto& e = ents[i];
    if(e.type == entity::NONE) return;
    entity_info info;
    entity_get_info(i, info);
    action a;

    // moster ai
    if(i != 0 && !e.paralyzed)
    {
        if(e.confused)
            a.type = action::MOVE, a.data = u8rand() & 3;
        else
            monster_ai(i, a);
        entity_perform_action(i, a);
    }

    // advance confusion/paralysis/invis
    if(e.confused)
    {
        bool end;
        if(i == 0) end = (--pinfo.confuse_rem == 0);
        else end = ((u8rand() & 7) == 0);
        if(end)
        {
            e.confused = 0;
            status(PSTR("@S @A no longer confused."), i, i);
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
            status(PSTR("@S @A no longer paralyzed."), i, i);
        }
    }
    if(e.invis && !info.invis) // temporary invis
    {
        bool end;
        if(i == 0) end = (--pinfo.invis_rem == 0);
        else end = ((u8rand() & 15) == 0);
        if(end)
        {
            e.invis = 0;
            if(i == 0) status(PSTR("You are no longer invisible."));
        }
    }
}

bool test_attack_hit(uint8_t atti, uint8_t defi) // 0 for miss
{
    uint8_t ta = entity_dexterity(atti);
    uint8_t td = entity_dexterity(defi);
    return u8rand(ta * 2 + td) >= td;
}

static uint8_t att_def_mod(uint8_t x)
{
    uint8_t a = ((x + 1) * 3 + 1) / 4;
    uint8_t b = x - a;
    return u8rand(a) + b + 1;
}

uint8_t calculate_hit_damage(uint8_t atti, uint8_t defi) // 0 for block
{
    uint8_t ta = entity_attack(atti);
    uint8_t td = entity_defense(defi);
    ta = att_def_mod(ta);
    td = att_def_mod(td);
    uint8_t dam = (ta * ta + ta / 2 + 1) / (ta + td + 1);
    if(dam == 0) dam = 1;
    return dam;
}

uint8_t calculate_arrow_damage(uint8_t defi)
{
    uint8_t ta = entity_dexterity(0);
    uint8_t td = entity_dexterity(defi);
    if(u8rand(ta * 3 + td) < td)
        return 0;
    uint8_t dam = u8rand(ta) + ta / 2 + entity_strength(0) / 4 + 1;
    td = entity_defense(defi);
    dam -= u8rand(td / 2 + 1);
    if(dam == 0) dam = 1;
    return dam;
}

void entity_restore_strength(uint8_t i)
{
    if(ents[i].weakened)
    {
        ents[i].weakened = 0;
        if(i == 0) status(PSTR("Your strength returns."));
    }
}

void entity_heal(uint8_t i, uint8_t amount)
{
    auto& e = ents[i];
    uint8_t mhp = entity_max_health(i);
    if(e.health >= mhp) return;
    char const* s = (amount < 3 ? PSTR("slightly ") : PSTR(""));
    if(i == 0)
        status(PSTR("You feel @pbetter."), s);
    else if(player_can_see_entity(i))
        status(PSTR("@S looks @pbetter."), i, s);
    if(mhp - e.health <= amount)
        e.health = mhp;
    else
        e.health += amount;
}

void entity_take_damage(uint8_t i, uint8_t dam)
{
    auto& te = ents[i];
    //bool cansee = player_can_see_entity(i);
    if(dam >= te.health)
    {
        //if(cansee)
            status(PSTR("@S @V!"), i, i, PSTR("die"));
        //else
        //    status(PSTR("You hear the sound of death."));
        uint8_t tt = te.type;
        te.health = 0;
        te.type = entity::NONE;
        maps[map_index].got_ents.set(i);            
    }
    else
    {
        te.health -= dam;
    }
}

void entity_take_damage_from_entity(uint8_t atti, uint8_t defi, uint8_t dam)
{
    auto const& e = ents[atti];
    entity_info info;
    entity_get_info(atti, info);
    auto& te = ents[defi];
    uint8_t tetype = te.type;

    if(defi == 0)
    {
        hs.type = HS_ENTITY;
        hs.data = e.type;
    }
    entity_take_damage(defi, dam);
    if(te.type == entity::NONE) // entity was killed
    {
        if(atti == 0)
        {
            uint8_t xp = pgm_read_byte(&MONSTER_INFO[tetype].xp);
            player_gain_xp(xp);
            hs.score += xp;
        }
    }
    else // entity was damaged but not killed: on hit effects
    {
        if(defi == 0 && info.vampire && u8rand() < 40)
        {
            status(PSTR("@S drains your health!"), atti);
            pinfo.vamp_drain += 3;
            ents[0].health = tmin(ents[0].health, entity_max_health(0));
            if(ents[0].health == 0)
            {
                status(PSTR("@S @V!"), defi, defi, PSTR("die"));
                return;
            }
        }

        if(info.confuse && u8rand() < 40)
            confuse_entity(defi);

        if(info.poison && u8rand() < 40)
            poison_entity(defi);
    }
}

void teleport_entity(uint8_t i)
{
    find_unoccupied_guaranteed(ents[i].x, ents[i].y);
    if(i == 0)
        status(PSTR("You find yourself in another location."));
    confuse_entity(i);
}

static bool is_uncursed_subtype(uint8_t subtype, uint8_t slot)
{
    uint8_t j = pinfo.equipped[slot];
    return j < INV_ITEMS && inv[j].subtype == subtype && !inv[j].cursed;
}

bool wearing_uncursed_amulet(uint8_t subtype)
{
    return is_uncursed_subtype(subtype, SLOT_AMULET);
}

static uint8_t ring_bonus_slot(uint8_t slot, uint8_t subtype)
{
    uint8_t j = pinfo.equipped[slot];
    if(j >= INV_ITEMS) return 0;
    item it = inv[j];
    if(it.subtype != subtype) return 0;
    int8_t r = it.quant_or_level + 1;
    if(it.cursed) r = -r;
    return uint8_t(r);
}

int8_t ring_bonus(uint8_t subtype)
{
    return
        ring_bonus_slot(SLOT_RING1, subtype) +
        ring_bonus_slot(SLOT_RING2, subtype);
}

bool wearing_uncursed_ring(uint8_t subtype)
{
    return
        is_uncursed_subtype(subtype, SLOT_RING1) ||
        is_uncursed_subtype(subtype, SLOT_RING2);;
}

void confuse_entity(uint8_t i)
{
    if(i == 0 && wearing_uncursed_amulet(AMU_CLARITY))
        return;
    auto& te = ents[i];
    if(player_can_see_entity(i))
        status(PSTR("@S @V confused!"), i, i, PSTR("become"));
    if(i == 0)
        pinfo.confuse_rem = u8rand() % 4 + 4;
    te.confused = 1;
}

void poison_entity(uint8_t i)
{
    auto& te = ents[i];
    if(te.weakened) return;
    if(player_can_see_entity(i))
        status(PSTR("@S @A weakened!"), i, i);
    te.weakened = 1;
}

static void entity_attack_entity(uint8_t atti, uint8_t defi)
{
    bool hit = test_attack_hit(atti, defi);
    auto const& e = ents[atti];
    auto& te = ents[defi];
    bool cansee = player_can_see_entity(atti) || player_can_see_entity(defi);
    if(!hit && cansee)
        status(PSTR("@S @v @O."), atti, atti, PSTR("miss"), defi);
    if(atti == 0)
        aggro_monster(defi);
    if(hit)
    {
        uint8_t dam = calculate_hit_damage(atti, defi);
        if(cansee)
        {
            if(dam > 0)
                status(PSTR("@S @V @O."), atti, atti, PSTR("hit"), defi);
            else
                status(PSTR("@S @V @P attack."), defi, defi, PSTR("block"), atti);
        }
        entity_take_damage_from_entity(atti, defi, dam);
    }
}

bool entity_perform_action(uint8_t i, action const& a)
{
    auto& e = ents[i];
    uint8_t dir = a.data & 3;
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
    int8_t dx = (int8_t)pgm_read_byte(&DIRX[dir]);
    int8_t dy = (int8_t)pgm_read_byte(&DIRY[dir]);
    uint8_t nx = e.x + dx;
    uint8_t ny = e.y + dy;
    entity* te = get_entity(nx, ny);
    entity_info info;
    entity_get_info(i, info);
    switch(a.type)
    {
    case action::USE:
        return use_item(a.data);
    case action::CLOSE:
    {
        door* d = get_door(nx, ny);
        if(d && !d->secret)
        {
            if(!d->open)
                status(PSTR("The door is already closed."));
            else if(te)
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
        if(te)
        {
            uint8_t ti = index_of_entity(*te);
            entity_attack_entity(i, ti);
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
    case action::DROP:
    {
        item it = inv[a.data];
        if(it.type == item::AMULET && it.subtype == AMU_YENDOR)
        {
            status(PSTR("You are unable to drop the @i."), it);
            return false;
        }
        else if(item_is_equipped(a.data))
        {
            if(!unequip_item(a.data))
                return false;
        }
        status(PSTR("You drop the @i."), it);
        player_remove_item(a.data);
        put_item_on_ground(e.x, e.y, it);
        inv[a.data].type = item::NONE;
        return true;
    }
    case action::THROW:
    {
        if(!direction_menu(dir))
            return false;
        dx = (int8_t)pgm_read_byte(&DIRX[dir]);
        dy = (int8_t)pgm_read_byte(&DIRY[dir]);
        item it = inv[a.data];
        player_remove_item(a.data);
        it.quant_or_level = 0;
        for(uint8_t i = 0, x = e.x, y = e.y; i < 8; ++i)
        {
            x += dx;
            y += dy;
            if(tile_is_solid(x, y)) break;
            if(entity* te = get_entity(x, y))
            {
                uint8_t ti = index_of_entity(*te);
                aggro_monster(ti);
                status(PSTR("The @i hits @O!"), it, ti);
                entity_apply_potion(ti, it.subtype);
            }
        }
        status(PSTR("The @i shatters."), it);
        return true;
    }
    case action::SHOOT:
    {
        uint8_t arrow = 0;
        for(; arrow < INV_ITEMS; ++arrow)
            if(inv[arrow].type == item::ARROW)
                break;
        uint8_t q = inv[arrow].quant_or_level;
        if(q == 0)
            inv[arrow].type = item::NONE;
        else
            inv[arrow].quant_or_level = q - 1;
        scan_result sr;
        scan_dir(0, a.data, 8, sr);
        if(sr.i < MAP_ENTITIES)
        {
            uint8_t dam = calculate_arrow_damage(sr.i);
            status(PSTR("Your arrow @ps @O."),
                dam == 0 ? PSTR("misse") : PSTR("hit"), sr.i);
            entity_take_damage_from_entity(0, sr.i, dam);
        }
        if(u8rand() >= ARROW_BREAK_CHANCE)
        {
            item t{};
            t.type = item::ARROW;
            put_item_on_ground(sr.x, sr.y, t);
        }
        return true;
    }
    default:
        break;
    }
    return false;
}

void aggro_monster(uint8_t i)
{
    if(!ents[i].aggro)
    {
        ents[i].health = entity_max_health(i);
        ents[i].aggro = 1;
    }
}
