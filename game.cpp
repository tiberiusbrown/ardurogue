#include "game.hpp"

#include <stddef.h>

static constexpr uint8_t MAX_ITEM_QUANT = 98;

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

void dig_tile(uint8_t x, uint8_t y)
{
    uint16_t i = (uint16_t(y * (MAP_W / 8)) & 0xffc0) | x;
    tmap[i] &= ~ymask(y);
}

void fill_tile(uint8_t x, uint8_t y)
{
    uint16_t i = (uint16_t(y * (MAP_W / 8)) & 0xffc0) | x;
    tmap[i] |= ymask(y);
}

bool tile_is_unknown(uint8_t x, uint8_t y)
{
    if(x >= MAP_W || y >= MAP_H) return false;
    uint16_t i = (uint16_t(y * (MAP_W / 8)) & 0xffc0) | x;
    return (tfog[i] & ymask(y)) == 0;
}

bool tile_is_solid_or_unknown(uint8_t x, uint8_t y)
{
    if(x >= MAP_W || y >= MAP_H) return true;
    uint16_t i = (uint16_t(y * (MAP_W / 8)) & 0xffc0) | x;
    return ((tmap[i] | ~tfog[i]) & ymask(y)) != 0;
}

bool tile_is_solid(uint8_t x, uint8_t y)
{
    if(x >= MAP_W || y >= MAP_H) return true;
    uint16_t i = (uint16_t(y * (MAP_W / 8)) & 0xffc0) | x;
    return (tmap[i] & ymask(y)) != 0;
}

bool tile_is_explored(uint8_t x, uint8_t y)
{
    if(x >= MAP_W || y >= MAP_H) return false;
    uint16_t i = (uint16_t(y * (MAP_W / 8)) & 0xffc0) | x;
    return (tfog[i] & ymask(y)) != 0;
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
    uint8_t level = it.level - ENCHANT_LEVEL_ZERO;
    return item::BOOTS - it.type + level;
}

uint8_t weapon_item_attack(item it)
{
    // assumes item is sword or bow
    uint8_t level = it.level - ENCHANT_LEVEL_ZERO;
    return level + (uint8_t(it.type - item::BOW) * 8) - 5;
}

void add_to_score(uint8_t amount)
{
    hs.score += amount;
    if(hs.score >= 0xff00)
        hs.score = 0xff00;
}

void player_gain_xp(uint8_t xp)
{
    add_to_score(xp);
    if(plevel == 49) return;
    uint8_t txp = xp_for_level();
    if(wearing_uncursed_amulet(AMU_WISDOM))
        xp += (xp + 1) / 2;
    if(txp - xp < pstats.xp)
    {
        ++plevel;
        status_u(PSTR("You advance to level @u!"), plevel + 1);
        pstats.max_health += 4;
        if(plevel % 4 == 2)
        {
            status_simple(STR_YOU_FEEL_STRONGER);
            pstats.strength += 1;
        }
        if(plevel % 4 == 0)
        {
            status_simple(STR_YOU_FEEL_MORE_AGILE);
            pstats.dexterity += 1;
        }
        healths[0] = entity_max_health(0);
        pstats.xp = 0;
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
            if(inv[j].is_same_type_as(it))
            {
                if(inv[j].quant >= MAX_ITEM_QUANT)
                {
                    status_simple(PSTR("You can't hold any more of those."));
                    return false;
                }
                break;
            }
    }
    if(j >= INV_ITEMS)
    {
        for(j = 0; j < INV_ITEMS; ++j)
            if(inv[j].is_nothing())
                break;
    }
    if(j >= INV_ITEMS)
    {
        status_simple(PSTR("You can't hold any more items."));
        return false;
    }
    if(!inv[j].is_nothing())
    {
        // add to stackable
        item tt = it;
        uint8_t tot = inv[j].quant + it.quant + 1;
        if(tot > MAX_ITEM_QUANT)
        {
            // overflow back onto ground
            tot -= (MAX_ITEM_QUANT + 1); // remaining on the ground
            tt.quant -= (tot + 1);
            it.quant = tot;
        }
        else
        {
            it.reset();
        }
        inv[j].quant += tt.quant + 1;
        status_i(PSTR("You now have @i."), inv[j]);
        return true;
    }
    else
        inv[j] = it;
    it.reset();
    status_i(PSTR("You got the @i."), inv[j]);
    return true;
}

void player_remove_item(uint8_t i)
{
    if(inv[i].stackable() && inv[i].quant > 0)
    {
        inv[i].quant -= 1;
        return;
    }
    inv[i].reset();
}

void put_item_on_ground(uint8_t x, uint8_t y, item it)
{
    // try to combine with existing stackable item on ground
    if(it.stackable())
    {
        for(auto& t : items)
        {
            if(!(t.x == x && t.y == y)) continue;
            if(t.it.is_same_type_as(it))
            {
                uint8_t n = t.it.quant;
                if(n < MAX_ITEM_QUANT) ++n;
                t.it.quant = n;
                return;
            }
        }
    }
    for(auto& t : items)
        if(t.it.is_nothing())
        {
            t.x = x;
            t.y = y;
            t.it = it;
            return;
        }
    status_i(PSTR("The @i breaks."), it);
}

void render()
{
    draw_dungeon_at_player();
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

void advance_hunger()
{
    int8_t rs = ring_bonus(RNG_SUSTENANCE);
    if(rs > 0 && u8rand() % 2 == 0)
        return;
    if(hunger < 255)
    {
        ++hunger;
        if(hunger < 255 && rs < 0)
            ++hunger;
    }
    else if(healths[0] > 0)
    {
        status_simple(PSTR("You are starving!"));
        hs.type = HS_STARVED;
        entity_take_damage(0, u8rand() % 2 + 1);
    }
}

// advance all entities and effects
static void advance()
{
    uint8_t pspeed = entity_speed(0);
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

    if(u8rand() < 128)
        advance_hunger();

    if(u8rand() % 64 == 0)
    {
        if(!ents[0].confused && amulet_bonus(AMU_CLARITY) < 0)
        {
            status_cursed_amulet();
            confuse_entity(0);
        }
        if(!ents[0].weakened && amulet_bonus(AMU_IRONBLOOD) < 0)
        {
            status_cursed_amulet();
            paralyze_entity(0);
        }
    }

}

void scan_dir(uint8_t i, uint8_t d, uint8_t n, scan_result& r)
{
    scan_dir_pos(ents[i].x, ents[i].y, d, n, r);
}

void scan_dir_pos(uint8_t x, uint8_t y, uint8_t d, uint8_t n, scan_result& r)
{
    auto c = dircoord(d);
    uint8_t k = 0;
    r.i = 255;
    while(k < n)
    {
        if(tile_is_solid(x + c.x, y + c.y))
            break;
        x += c.x, y += c.y;
        ++k;
        if(entity* e = get_entity(x, y))
        {
            r.i = index_of_entity(*e);
            break;
        }
    }
    r.n = k;
    r.x = x;
    r.y = y;
    r.px = x - c.x;
    r.py = y - c.y;
}

void step()
{
    action a{};
    render();

    //
    // checks after the player moved
    //
    if(gflags & GFLAG_JUST_MOVED)
    {
        // spawn monster
        if(u8rand() < 8)
            try_spawn_monster();

        uint8_t px = ents[0].x, py = ents[0].y;
        // loop through items in reverse order
        // so that rendered item is first
        for(int8_t i = MAP_ITEMS - 1; i >= 0; --i)
        {
            map_item& mit = items[i];
            if(mit.it.is_nothing() || mit.x != px || mit.y != py)
                continue;
            if(yesno_menui(PSTR("Pick up the @i?"), mit.it))
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
                    if(it.is_type(item::AMULET, AMU_YENDOR))
                        have_amulet = true;
                if(have_amulet)
                {
                    hs.type = HS_ESCAPED;
                    status_simple(PSTR("You have escaped with the amulet of Yendor!"));
                }
                else
                {
                    hs.type = HS_RETURNED;
                    status_simple(PSTR("You have left without the amulet of Yendor."));
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
    uint8_t px = ents[0].x, py = ents[0].y;
    switch(b)
    {
    case BTN_UP: a.type = action::MOVE; a.data = 0; break;
    case BTN_DOWN: a.type = action::MOVE; a.data = 1; break;
    case BTN_LEFT: a.type = action::MOVE; a.data = 2; break;
    case BTN_RIGHT: a.type = action::MOVE; a.data = 3; break;
    case BTN_A:
        if(!repeat_action(a))
        {
            render();
            goto end;
        }
        break;
    case BTN_B:
        if(!action_menu(a))
        {
            render();
            goto end;
        }
        break;
    default: break;
    }
    if(entity_perform_action(0, a))
    {
        update_doors();
        update_light();
        advance();
    }
end:
    gflags &= ~GFLAG_JUST_MOVED;
    if(px != ents[0].x || py != ents[0].y)
        gflags |= GFLAG_JUST_MOVED;
}

static void init_all_perms()
{
    init_perm(perm_pot.data(), (uint8_t)perm_pot.size());
    init_perm(perm_scr.data(), (uint8_t)perm_scr.size());
    init_perm(perm_rng.data(), (uint8_t)perm_rng.size());
    init_perm(perm_amu.data(), (uint8_t)perm_amu.size());
    init_perm(perm_wnd.data(), (uint8_t)perm_wnd.size());
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
        if(hs.score >= high_scores[i].score)
            break;
    }
    if(i < NUM_HIGH_SCORES)
    {
        for(uint8_t j = NUM_HIGH_SCORES - 1; j > i; --j)
            high_scores[j] = high_scores[j - 1];
        high_scores[i] = hs;
    }
    return i;
}

void paint_left() { paint_offset(0, true); }
void paint_right() { paint_offset(64, true); }
void paint_left_no_clear() { paint_offset(0, false); }
void paint_right_no_clear() { paint_offset(64, false); }

void run()
{
    stack_canary_init();

    for(;;)
    {
        paint_left();
        paint_right();
        reset_status();
        memzero(&globals_, sizeof(globals_));

        draw_text(7, 24, PSTR("ArduRogue"));
        set_box(5, 42, 22, 30);
        draw_text(0, 34, PSTR("Press A to play."));
        paint_offset(40);
        while(wait_btn() != BTN_A)
            (void)0;

        bool saved = save_valid();
        if(saved) load();
        saved &= (ents[0].type == entity::PLAYER);
        char const* back = PSTR("");
        if(saved)
        {
            rand_seed = game_seed;
            back = PSTR("back ");
            init_all_perms();
            generate_dungeon();
            load();         // need another load to overwrite current map entities
            destroy_save(); // NOW destroy save
            status_simple(PSTR(
                "Welcome back to ArduRogue. "
                "The save file has been removed. "
                "Don't forget to save again when you're done!"));
        }
        else
        {
            wall_style = 2;
            new_game();
            init_all_perms();
            generate_dungeon();
            // initialize player for new game
            for(auto& i : pinfo.equipped) i = 255;
            pgm_memcpy(&pstats, &MONSTER_INFO[entity::PLAYER], sizeof(pstats));
            new_entity(0, entity::PLAYER, xup, yup);
            status_simple(PSTR("Welcome to ArduRogue."));
        }

        update_light();

        for(;;)
        {
            step();
            if(gflags & GFLAG_JUST_SAVED)
            {
                // go back to title screen
                break;
            }
            else if(ents[0].type == entity::NONE)
            {
                status_simple(PSTR("Press B to continue."));
                render();
                uint8_t hsi = process_high_score();
                while(wait_btn() != BTN_B)
                    (void)0;
                show_high_scores(hsi);
                break;
            }
        }
    }
}
