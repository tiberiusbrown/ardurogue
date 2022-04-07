#include "game.hpp"

bool use_item(uint8_t i)
{
    item& it = inv[i];
    inv[i].identified = 1;
    switch(it.type)
    {
    case item::POTION:
        entity_apply_potion(0, it.subtype);
        player_remove_item(i);
        return true;
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
        break;
    }
    case POT_CONFUSION:
        confuse_entity(i);
        break;
    case POT_POISON:
        poison_entity(i);
        break;
    default:
        // TODO
        status(PSTR("NOT IMPLEMENTED"));
        break;
    }
}
