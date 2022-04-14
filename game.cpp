#include "game.hpp"

#include <stddef.h>

void pgm_memcpy(void* dst, void const* src, uint8_t n)
{
    uint8_t* d = (uint8_t*)dst;
    uint8_t const* s = (uint8_t const*)src;
    for(uint8_t i = 0; i < n; ++i)
        d[i] = pgm_read_byte(&s[i]);
}

void memzero(void* dst, uint16_t n)
{
    uint8_t* d = (uint8_t*)dst;
    for(uint16_t i = 0; i < n; ++i)
        d[i] = 0;
}

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

uint8_t armor_item_defense(item it)
{
    // assumes item is armor, helm, or boots
    uint8_t level = it.quant_or_level;
    if(it.cursed) level = -(level + 2);
    return item::BOOTS - it.type + level + 1;
}

uint8_t weapon_item_attack(item it)
{
    // assumes item is sword or bow
    uint8_t level = it.quant_or_level;
    if(it.cursed) level = -(level + 2);
    uint8_t m = 3;
    if(it.type == item::BOW) m = 1;
    return level + m;
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

bool player_pickup_item(uint8_t i)
{
    uint8_t j = 255;
    auto& it = items[i].it;
    if(it.stackable())
    {
        for(j = 0; j < INV_ITEMS; ++j)
            if(inv[j].type == it.type && inv[j].subtype == it.subtype)
            {
                if(inv[j].quant_or_level >= 63)
                {
                    status(PSTR("You can't hold any more of those."));
                    return false;
                }
                break;
            }
    }
    if(j >= INV_ITEMS)
    {
        for(j = 0; j < INV_ITEMS; ++j)
            if(inv[j].type == item::NONE)
                break;
    }
    if(j >= INV_ITEMS)
    {
        status(PSTR("You can't hold any more items."));
        return false;
    }
    if(inv[j].type != item::NONE)
    {
        // add to stackable
        item tt = it;
        uint8_t tot = inv[j].quant_or_level + it.quant_or_level + 1;
        if(tot > 63)
        {
            // overflow back onto ground
            tot -= 64; // remaining on the ground
            tt.quant_or_level -= (tot + 1);
            it.quant_or_level = tot;
        }
        else
        {
            it.type = item::NONE;
        }
        inv[j].quant_or_level += tt.quant_or_level + 1;
        status(PSTR("You now have @i."), inv[j]);
        return true;
    }
    else
        inv[j] = it;
    it.type = item::NONE;
    status(PSTR("You got the @i."), inv[j]);
    return true;
}

void player_remove_item(uint8_t i)
{
    if(inv[i].stackable() && inv[i].quant_or_level > 0)
    {
        --inv[i].quant_or_level;
        return;
    }
    // shift all items down. nice to retain ordering
    for(uint8_t j = i; j < INV_ITEMS - 1; ++j)
        inv[j] = inv[j + 1];
    inv[INV_ITEMS - 1].type = item::NONE;
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

// advance all entities and effects
static void advance()
{
    uint8_t pspeed = entity_speed(0);
    // apply amulet of regeneration
    if(wearing_uncursed_amulet(AMU_REGENERATION))
    {
        uint8_t i = tmax<uint8_t>(entity_max_health(0), 48);
        if(u8rand() < i)
            entity_heal(0, 1);
    }
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

void end_game()
{

}

void step()
{
    action a{};
    render();

    //
    // checks after the player moved
    //
    if(just_moved)
    {
        uint8_t px = ents[0].x, py = ents[0].y;
        // loop through items in reverse order
        // so that rendered item is first
        for(int8_t i = MAP_ITEMS - 1; i >= 0; --i)
        {
            map_item& mit = items[i];
            if(mit.it.type == item::NONE || mit.x != px || mit.y != py)
                continue;
            if(yesno_menu(PSTR("Pick up the @i?"), mit.it))
            {
                player_pickup_item(i);
                render();
            }
        }

        if(px == xdn && py == ydn &&
            yesno_menu(PSTR("Go down to the next dungeon?")))
        {
            ++map_index;
            generate_dungeon();
            ents[0].x = xup, ents[0].y = yup;
            update_light();
            render();
        }
        else if(px == xup && py == yup &&
            yesno_menu(map_index == 0 ?
                PSTR("Return to the surface?") :
                PSTR("Go back up the stairs?")))
        {
            if(map_index == 0)
            {
                bool have_amulet = false;
                for(auto it : inv)
                    if(it.type == item::AMULET && it.subtype == AMU_YENDOR)
                        have_amulet = true;
                if(have_amulet)
                {
                    hs.type = HS_ESCAPED;
                    status(PSTR("You have escaped with the amulet of Yendor!"));
                }
                else
                {
                    hs.type = HS_RETURNED;
                    status(PSTR("You leave without the amulet of Yendor."));
                }
                ents[0].type = entity::NONE;
                return;
            }
            else
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

    reset_status();
    uint8_t b = wait_btn();
    switch(b)
    {
    case BTN_UP: a.type = action::MOVE; a.data = 0; break;
    case BTN_DOWN: a.type = action::MOVE; a.data = 1; break;
    case BTN_LEFT: a.type = action::MOVE; a.data = 2; break;
    case BTN_RIGHT: a.type = action::MOVE; a.data = 3; break;
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
    just_moved = (px != ents[0].x || py != ents[0].y);
}

static void init_all_perms()
{
    init_perm(perm_pot.data(), (uint8_t)perm_pot.size());
    init_perm(perm_scr.data(), (uint8_t)perm_scr.size());
    init_perm(perm_rng.data(), (uint8_t)perm_rng.size());
    init_perm(perm_amu.data(), (uint8_t)perm_amu.size());
}

static void new_game()
{
    memzero(&globals_.saved, offsetof(saved_data, wall_style));
    seed();
    game_seed = rand_seed;
}

static void load_game()
{
    load();
    rand_seed = game_seed;
}

uint8_t process_high_score()
{
    uint8_t i;
    for(i = 0; i < NUM_HIGH_SCORES; ++i)
    {
        if(hs.score > high_scores[i].score)
            break;
    }
    if(i < NUM_HIGH_SCORES)
    {
        for(uint8_t j = NUM_HIGH_SCORES - 1; j > i; --j)
            high_scores[j] = high_scores[j - 1];
        high_scores[i] = hs;
        save();
    }
    return i;
}

void paint_left(bool clear) { paint_offset(0, clear); }
void paint_right(bool clear) { paint_offset(64, clear); }

void run()
{
    stack_canary_init();

    for(;;)
    {
        paint_left();
        paint_right();
        reset_status();
        memzero(&globals_, sizeof(globals_));

        bool saved = save_valid();
        if(saved) load();
        else wall_style = 2;

        char const* back = PSTR("");
        saved &= (ents[0].type == entity::PLAYER);
        if(saved)
        {
            load_game();
            back = PSTR("back ");
        }
        else
        {
            draw_text(7, 24, PSTR("ArduRogue"));
            set_box(5, 42, 22, 30);
            draw_text(0, 34, PSTR("Press A to play."));
            paint_offset(40);
            while(wait_btn() != BTN_A)
                (void)0;
            new_game();
        }

        init_all_perms();
        generate_dungeon();

        if(!saved)
        {
            generate_items_and_ents();
            // initialize player for new game
            for(auto& i : pinfo.equipped) i = 255;
            pgm_memcpy(&pstats, &MONSTER_INFO[entity::PLAYER], sizeof(pstats));
            new_entity(0, entity::PLAYER, xup, yup);
        }

        statusx = 1;
        statusy = STATUS_START_Y;
        status(PSTR("Welcome @pto ArduRogue."), back);

        update_light();
        render();

        for(;;)
        {
            step();
            if(ents[0].type == entity::NONE)
            {
                status(PSTR("Press B to continue."));
                render();
                if(saved) destroy_save();
                uint8_t hsi = process_high_score();
                // game is over!
                // handle this here: high score list? TODO
                while(wait_btn() != BTN_B)
                    (void)0;
                show_high_scores(hsi);
                break;
            }
        }
    }
}
