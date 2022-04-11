#include "game.hpp"

static bool consume_consumable()
{
    uint8_t i = pinfo.equipped[SLOT_AMULET];
    if(i >= INV_ITEMS) return true;
    if(inv[i].subtype != AMU_CONSERVATION) return true;
    // wearing amulet of conservation: 50% chance not to consume
    return u8rand() < 128;
}

static void identify_item(uint8_t i)
{
    auto& it = inv[i];
    it.identified = 1;
    switch(it.type)
    {
    case item::POTION: identify_potion(it.subtype); break;
    case item::SCROLL: identify_scroll(it.subtype); break;
    case item::RING: identify_ring(it.subtype); break;
    case item::AMULET: identify_amulet(it.subtype); break;
    default: break;
    }
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
            identify_item(i);
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
                status(PSTR("The @i glows blue for a moment."), it);
                if(it.quant_or_level < 63)
                    it.quant_or_level += 1;
            }
        }
        else
            status(PSTR("Nothing happens."));
        break;
    case SCR_REMOVE_CURSE:
        i = inventory_menu(PSTR("Uncurse which item?"));
        if(i >= INV_ITEMS)
        {
            auto& it = inv[i];
            if(it.stackable() || !it.cursed)
                status(PSTR("Nothing happens."));
            else
            {
                it.cursed = 0;
                it.quant_or_level = 0;
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
    case item::RING:   return SLOT_RING;
    case item::AMULET: return SLOT_AMULET;
    default: break;
    }
    return 255;
}

bool unequip_item(uint8_t i)
{
    char const* verb = inv[i].type <= item::SWORD ?
        PSTR("stop using") : PSTR("take off");
    if(inv[i].cursed)
    {
        status(PSTR("You are unable to @p the @i."), verb, inv[i]);
        return false;
    }
    for(auto& j : pinfo.equipped)
        if(j == i)
        {
            status(PSTR("You @p the @i."), verb, inv[i]);
            return j = 255, true;
        }
    return false;
}

bool equip_item(uint8_t i)
{
    uint8_t type = inv[i].type;
    uint8_t slot = slot_of_item(type);
    uint8_t& j = pinfo.equipped[slot];
    if(j < INV_ITEMS && !unequip_item(j))
        return false;
    bool weap = type <= item::SWORD;
    identify_item(i);
    status(PSTR("You @p the @i."),
        weap ? PSTR("ready") : PSTR("put on"),
        inv[i]);
    j = i;
    return true;
}

bool use_item(uint8_t i)
{
    item& it = inv[i];
    uint8_t subtype = it.subtype;
    switch(it.type)
    {
    case item::POTION:
        player_remove_item(i);
        entity_apply_potion(0, subtype);
        return true;
    case item::SCROLL:
        player_remove_item(i);
        use_scroll(subtype);
        return true;
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

void entity_apply_potion(uint8_t i, uint8_t subtype)
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
            pinfo.invis_rem = u8rand() % 16 + 12;
        ents[i].invis = 1;
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
