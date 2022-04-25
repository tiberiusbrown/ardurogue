#include "game.hpp"

static constexpr uint8_t WAND_RANGE = 6;

static void identify_item(uint8_t i)
{
    auto& it = inv[i];
    uint8_t st = it.subtype;
    if(it.type >= item::WAND)
        it.identified = 1;
    switch(it.type)
    {
    case item::POTION: identify_potion(st); break;
    case item::SCROLL: identify_scroll(st); break;
    case item::RING:   identify_ring(st);   break;
    case item::AMULET: identify_amulet(st); break;
    case item::WAND:   identify_wand(st);   break;
    case item::FOOD:
    case item::ARROW:
        break;
    default:
        break;
    }
}

void wand_effect(uint8_t i, uint8_t d, uint8_t subtype)
{
    scan_result sr;
    if(d < 4)
    {
        scan_dir(i, d, WAND_RANGE, sr);
        if(subtype != WND_DIGGING)
            draw_ray_anim(ents[i].x, ents[i].y, d, sr.n);
    }
    else
    {
        sr.x = ents[i].x;
        sr.y = ents[i].y;
        sr.i = i;
        d = u8rand() % 4;
    }

    entity* te = get_entity(sr.x, sr.y);
    if(subtype == WND_DIGGING)
    {
        uint8_t x = ents[i].x, y = ents[i].y;
        uint8_t dx = pgm_read_byte(&DIRX[d]);
        uint8_t dy = pgm_read_byte(&DIRY[d]);
        for(uint8_t i = 0; i < WAND_RANGE; ++i)
        {
            x += dx;
            y += dy;
            if(x >= MAP_W || y >= MAP_H) break;
            dig_tile(x, y);
            if(door* d = get_door(x, y))
            {
                d->open = 1;
                d->secret = 0;
            }
        }
        return;
    }
    else if(subtype == WND_FIRE)
    {
        static int8_t const FIRE[18] PROGMEM =
        {
            0, 0, -1, -1, 0, -1, 1, -1, 1, 0, 1, 1, 0, 1, -1, 1, -1, 0,
        };
        draw_dungeon_at_player();
        for(uint8_t i = 0; i < 18; i += 2)
        {
            uint8_t tx = sr.x + pgm_read_byte(&FIRE[i]);
            uint8_t ty = sr.y + pgm_read_byte(&FIRE[i + 1]);
            draw_sprite_nonprog_rel_and_wait(0x0eae, tx, ty);
            if(entity* e = get_entity(tx, ty))
                entity_take_fire_damage_from_entity(i, index_of_entity(*e), u8rand(8) + 8);
        }
        paint_left();
        return;
    }
    else if(!te) return;
    switch(subtype)
    {
    case WND_FORCE:
    {
        scan_result tsr;
        scan_dir_pos(te->x, te->y, d, 8, tsr);
        te->x = tsr.px;
        te->y = tsr.py;
        // TODO: if(entity* pe = get_entity(tsr.x, tsr.y))
        //           damage both te and pe
        break;
    }
    case WND_TELEPORT:
        if(sr.i < MAP_ENTITIES)
            teleport_entity(sr.i);
        break;
    case WND_STRIKING:
        entity_take_magic_damage_from_entity(i, sr.i, u8rand(12) + 12);
        break;
    case WND_ICE:
        slow_entity(sr.i);
        entity_take_magic_damage_from_entity(i, sr.i, u8rand(8) + 8);
        break;
    case WND_POLYMORPH:
    {
        uint8_t t = te->type;
        if(t > entity::BAT && t < entity::DARKNESS)
        {
            te->type = u8rand() < 64 ? t + 1 : t - 1;
            healths[index_of_entity(*te)] = entity_max_health(sr.i);
        }
        break;
    }
    default:
        break;
    }

}

static void use_wand(uint8_t subtype)
{
    draw_dungeon_at_player();
    paint_left();
    identify_wand(subtype);
    uint8_t d = 255;
    direction_menu(d); // canceling will mean target self
    wand_effect(0, d, subtype);
}

static void use_scroll(uint8_t subtype)
{
    uint8_t i;
    identify_scroll(subtype);
    switch(subtype)
    {
    case SCR_IDENTIFY:
        i = inventory_menu(PSTR("Identify which item?"));
        if(i < INV_ITEMS)
        {
            identify_item(i);
            status(PSTR("You identified: @i."), inv[i]);
        }
        else
            status(PSTR("Nothing happens."));
        break;
    case SCR_ENCHANT:
        i = inventory_menu(PSTR("Enchant which item?"));
        if(i < INV_ITEMS)
        {
            auto& it = inv[i];
            if(it.stackable())
                status(PSTR("Nothing happens."));
            else
            {
                uint8_t n = 1;
                if(it.type == item::WAND) n = 4;
                it.level += n;
                if(it.level < ENCHANT_LEVEL_MAX)
                    it.level = ENCHANT_LEVEL_MAX;
                status(PSTR("The @i glows blue for a moment."), it);
            }
        }
        else
            status(PSTR("Nothing happens."));
        break;
    case SCR_REMOVE_CURSE:
        i = inventory_menu(PSTR("Uncurse which item?"));
        if(i < INV_ITEMS)
        {
            auto& it = inv[i];
            if(it.type <= item::WAND || !it.cursed)
                status(PSTR("Nothing happens."));
            else
            {
                it.cursed = 0;
                status(PSTR("The @i glows white for a moment."), it);
            }
        }
        else
            status(PSTR("Nothing happens."));
        break;
    case SCR_TELEPORT:
        teleport_entity(0);
        break;
    case SCR_MAPPING:
        for(auto& t : tfog)
            t = 0xff;
        status(PSTR("You become aware of your surroundings."));
        break;
    case SCR_FEAR:
    case SCR_TORMENT:
    case SCR_MASS_CONFUSE:
    case SCR_MASS_POISON:
        for(uint8_t i = 0; i < MAP_ENTITIES; ++i)
        {
            auto& e = ents[i];
            if(e.type == entity::NONE) continue;
            if(!player_can_see_entity(i)) continue;
            aggro_monster(i);
            if(subtype == SCR_FEAR)
            {
                e.scared = 1;
                if(i != 0)
                    status(PSTR("@S flees!"), i);
            }
            else if(subtype == SCR_TORMENT)
            {
                healths[i] /= 2;
                status(PSTR("@U stricken!"), i);
            }
            else if(subtype == SCR_MASS_CONFUSE)
                confuse_entity(i);
            else if(subtype == SCR_MASS_POISON)
                poison_entity(i);
        }
    default:
        break;
    }
}

uint8_t slot_of_item(uint8_t type)
{
    switch(type)
    {
    case item::BOW:
    case item::SWORD:  return SLOT_WEAPON;
    case item::HELM:   return SLOT_HELM;
    case item::ARMOR:  return SLOT_ARMOR;
    case item::BOOTS:  return SLOT_BOOTS;
    case item::RING:
    {
        uint8_t j = pinfo.equipped[SLOT_RING1];
        // if slot 1 empty, return slot 1
        if(j >= INV_ITEMS) return SLOT_RING1;
        // if slot 2 cursed, return slot 1
        j = pinfo.equipped[SLOT_RING2];
        if(j < INV_ITEMS && inv[j].cursed) return SLOT_RING1;
        // otherwise return slot 2
        return SLOT_RING2;
    }
    case item::AMULET: return SLOT_AMULET;
    default: break;
    }
    return 255;
}

static void status_pi(char const* s, char const* p, item i)
{
    status(s, p, i);
}

bool unequip_item(uint8_t i)
{
    auto it = inv[i];
    char const* verb = it.type <= item::SWORD ?
        PSTR("stop using") : PSTR("take off");
    if(it.cursed)
    {
        status_pi(PSTR("You are unable to @p the @i."), verb, it);
        return false;
    }
    for(auto& j : pinfo.equipped)
        if(j == i)
        {
            status_pi(PSTR("You @p the @i."), verb, it);
            return j = 255, true;
        }
    return false;
}

bool equip_item(uint8_t i)
{
    item it = inv[i];
    uint8_t type = it.type;
    uint8_t slot = slot_of_item(type);
    uint8_t& j = pinfo.equipped[slot];
    if(j < INV_ITEMS && !unequip_item(j))
        return false;
    bool weap = type <= item::SWORD;
    identify_item(i);
    status(PSTR("You @p the @i."),
        weap ? PSTR("ready") : PSTR("put on"),
        inv[i]);
    if(it.is_type(item::AMULET, AMU_IRONBLOOD))
    {
        entity_restore_strength(0);
        end_paralysis(0);
    }
    if(it.is_type(item::AMULET, AMU_CLARITY))
        end_confusion(0);
    adjust_health_to_max_health(0);
    j = i;
    return true;
}

bool use_item(uint8_t i)
{
    item it = inv[i];
    uint8_t subtype = it.subtype;
    {
        if(it.stackable())
        {
            int8_t cb = amulet_bonus(AMU_CONSERVATION);
            if(cb <= 0 || u8rand() < 192)
                player_remove_item(i);
            if(cb < 0 && u8rand() < 64)
                player_remove_item(i);
        }
    }
    switch(it.type)
    {
    case item::FOOD:
    {
        hunger = 0;
        status(PSTR("You feel full."));
        return true;
    }
    case item::POTION:
        entity_apply_potion(subtype, 0);
        return true;
    case item::SCROLL:
        use_scroll(subtype);
        return true;
    case item::WAND:
    {
        uint8_t n = it.quant;
        if(n <= 1)
        {
            player_remove_item(i);
            status(PSTR("The @i crumbles to dust."), it);
        }
        else
            inv[i].quant = n - 1;
        use_wand(subtype);
        return true;
    }
    case item::BOW:
    case item::SWORD:
    case item::HELM:
    case item::ARMOR:
    case item::BOOTS:
    case item::RING:
    case item::AMULET:
        if(item_is_equipped(i))
            return unequip_item(i);
        return equip_item(i);
    default:
        break;
    }
    return false;
}

void entity_apply_potion(uint8_t subtype, uint8_t i)
{
    identify_potion(subtype);
    switch(subtype)
    {
    case POT_HEALING:
    {
        uint8_t mhp = entity_max_health(i);
        entity_heal(i, u8rand(mhp / 2) + mhp / 4);
        entity_restore_strength(i);
        break;
    }
    case POT_CONFUSION:
        confuse_entity(i);
        break;
    case POT_POISON:
        poison_entity(i);
        break;
    case POT_STRENGTH:
        if(ents[i].weakened)
            entity_restore_strength(i);
        else if(i == 0)
        {
            status(PSTR("You feel stronger!"));
            pstats.strength += 1;
        }
        break;
    case POT_INVIS:
        if(i == 0)
        {
            if(ring_bonus(RNG_INVIS) < 0)
                break;
            pinfo.invis_rem = u8rand() % 16 + 12;
            status(PSTR("You turn invisible."));
        }
        ents[i].invis = 1;
        break;
    case POT_PARALYSIS:
        paralyze_entity(i);
        break;
    case POT_SLOWING:
        slow_entity(i);
        break;
    case POT_EXPERIENCE:
        if(i == 0)
            player_gain_xp(50);
        break;
    default:
        // TODO
        //status(PSTR("NOT IMPLEMENTED"));
        break;
    }
}

bool item_is_equipped(uint8_t i)
{
    for(auto j : pinfo.equipped)
        if(i == j) return true;
    return false;
}
