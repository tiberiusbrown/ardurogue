#include "game.hpp"

static uint16_t const ENTITY_IMGS[] PROGMEM =
{
    0x0000, // none
    0x6ff6, // player
    0x0fa4, // bat
    0x0bd0, // snake
    0x0f5a, // rattlesnake
    0x9db9, // zombie
    0x0bf0, // goblin
    0x0f9f, // orc
    0x0f2c, // hobgoblin
    0xefbe, // mimic (looks like a closed door)
    0x01f1, // troll
    0x069d, // griffin
    0xf996, // dragon
};

static uint16_t const ITEM_IMGS[] PROGMEM =
{
    0x0000, // none
    0x9429, // food
    0x00b0, // potion
    0x01b3, // scroll
    0x8421, // arrow
    0x0960, // bow
    0x04f4, // sword
    0x0aaa, // ring
    0x0606, // amulet
    0x0f90, // armor
    0x0f90, // helm
    0x0f90, // boots
};

static uint16_t const DOOR_IMGS[2] PROGMEM = { 0xefbe, 0xe11e };

void set_pixel(uint8_t x, uint8_t y)
{
    if(!((x | y) & 0xc0)) // if(x < 64 && y < 64)
    {
        uint8_t& b = buf[y / 8 * 64 + x];
        b |= (1 << (y % 8));
    }
}

void clear_pixel(uint8_t x, uint8_t y)
{
    if(!((x | y) & 0xc0)) // if(x < 64 && y < 64)
    {
        uint8_t& b = buf[y / 8 * 64 + x];
        b &= ~(1 << (y % 8));
    }
}

void inv_pixel(uint8_t x, uint8_t y)
{
    if(!((x | y) & 0xc0)) // if(x < 64 && y < 64)
    {
        uint8_t& b = buf[y / 8 * 64 + x];
        b ^= (1 << (y % 8));
    }
}

void set_hline(uint8_t x0, uint8_t x1, uint8_t y)
{
    for(; x0 <= x1; ++x0)
        set_pixel(x0, y);
}

void clear_hline(uint8_t x0, uint8_t x1, uint8_t y)
{
    for(; x0 <= x1; ++x0)
        clear_pixel(x0, y);
}

void inv_hline(uint8_t x0, uint8_t x1, uint8_t y)
{
    for(; x0 <= x1; ++x0)
        inv_pixel(x0, y);
}

void set_vline(uint8_t x, uint8_t y0, uint8_t y1)
{
    for(; y0 <= y1; ++y0)
        set_pixel(x, y0);
}

void clear_vline(uint8_t x, uint8_t y0, uint8_t y1)
{
    for(; y0 <= y1; ++y0)
        clear_pixel(x, y0);
}

void set_rect(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1)
{
    for(; y0 <= y1; ++y0)
        set_hline(x0, x1, y0);
}

void clear_rect(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1)
{
    for(; y0 <= y1; ++y0)
        clear_hline(x0, x1, y0);
}

void inv_rect(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1)
{
    for(; y0 <= y1; ++y0)
        inv_hline(x0, x1, y0);
}

void set_box(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1)
{
    set_hline(x0, x1, y0);
    set_hline(x0, x1, y1);
    set_vline(x0, y0, y1);
    set_vline(x1, y0, y1);
}

void draw_box_pretty(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1)
{
    clear_rect(x0 - 1, x1 + 2, y0 - 1, y1 + 2);
    set_box(x0, x1, y0, y1);
    set_hline(x0 + 1, x1 + 1, y1 + 1);
    set_vline(x1 + 1, y0 + 1, y1 + 1);
}

static void set_img(
    uint8_t const* p, // NOT PROGMEM
    uint8_t w,
    uint8_t x, uint8_t y)
{
    for(uint8_t i = 0; i < w; ++i, ++x)
    {
        for(uint8_t tp = p[i], ty = y; tp; tp >>= 1, ++ty)
            if(tp & 1) set_pixel(x, ty);
    }
}

void set_img_prog(uint8_t const* p, uint8_t w, uint8_t x, uint8_t y)
{
    uint8_t tp[8];
    for(uint8_t i = 0; i < w; ++i)
        tp[i] = pgm_read_byte(&p[i]);
    set_img(tp, w, x, y);
}

static void clear_img(
    uint8_t const* p, // NOT PROGMEM
    uint8_t w,
    uint8_t x, uint8_t y)
{
    for(uint8_t i = 0; i < w; ++i, ++x)
    {
        for(uint8_t tp = p[i], ty = y; tp; tp >>= 1, ++ty)
            if(tp & 1) clear_pixel(x, ty);
    }
}

static void draw_tile(
    uint8_t const* p, // PROGMEM, 5 set + 5 clear
    uint8_t x, uint8_t y)
{
    for(uint8_t i = 0; i < 5; ++i, ++x)
    {
        uint8_t t = pgm_read_byte(&p[i]);
        for(uint8_t ty = y; t; t >>= 1, ++ty)
            if(t & 1) set_pixel(x, ty);
        t = pgm_read_byte(&p[i + 5]);
        for(uint8_t ty = y; t; t >>= 1, ++ty)
            if(t & 1) clear_pixel(x, ty);
    }
}

static void draw_sprite_precise_nonprog(
    uint16_t tp,
    uint8_t x, uint8_t y)
{
    uint8_t tt[4];
    tt[0] = (tp >> 12) & 0xf;
    tt[1] = (tp >> 8) & 0xf;
    tt[2] = (tp >> 4) & 0xf;
    tt[3] = (tp >> 0) & 0xf;
    clear_img(tt, 4, x - 1, y - 1);
    clear_img(tt, 4, x + 0, y - 1);
    clear_img(tt, 4, x + 1, y - 1);
    clear_img(tt, 4, x - 1, y);
    clear_img(tt, 4, x + 1, y);
    set_img(tt, 4, x, y);
}

static void draw_sprite_precise(
    uint16_t const* p, // PROGMEM
    uint8_t x, uint8_t y)
{
    uint16_t tp = pgm_read_word(p);
    draw_sprite_precise_nonprog(tp, x, y);
}

static void draw_sprite_nonprog(
    uint16_t tp,
    uint8_t x, uint8_t y)
{
    draw_sprite_precise_nonprog(tp, x * 5, y * 5);
}

static void draw_sprite(
    uint16_t const* p, // PROGMEM
    uint8_t x, uint8_t y)
{
    draw_sprite_precise(p, x * 5, y * 5);
}

static void draw_dungeon_minimap()
{
    static constexpr uint8_t OX = 3, OY = 2;
#if 1
    for(uint8_t y = 0; y < MAP_H; ++y)
        for(uint8_t x = 0; x < MAP_W; ++x)
            if(!tile_is_solid_or_unknown(x, y))
                set_pixel(x + OX, y + OY);
#else
    for(uint8_t y = 0; y < MAP_H/2; ++y)
        for(uint8_t x = 0; x < MAP_W/2; ++x)
            if(2 <=
                (uint8_t)(!tile_is_solid_or_unknown(x * 2 + 0, y * 2 + 0)) +
                (uint8_t)(!tile_is_solid_or_unknown(x * 2 + 1, y * 2 + 0)) +
                (uint8_t)(!tile_is_solid_or_unknown(x * 2 + 0, y * 2 + 1)) +
                (uint8_t)(!tile_is_solid_or_unknown(x * 2 + 1, y * 2 + 1)))
                set_pixel(x + OX, y + OY);
#endif
}

static void draw_progress_bar(
    uint8_t x, uint8_t y,
    uint8_t a, uint8_t b)
{
    constexpr uint8_t W = 16;
    set_box(x, x + W + 1, y, y + 2);
    uint8_t f = b == 0 ? 0 : uint8_t(uint16_t(a * W + W / 2) / b);
    set_hline(x + 1, x + f, y + 1);
}

void draw_info()
{
    draw_info_without_status();
    draw_status();
}

void draw_info_without_status()
{
    auto const& e = ents[0];
    draw_textf(1, 0, PSTR("Dungeon Level @u"), map_index + 1);
    draw_textf(1, 6, PSTR("HP"));
    draw_textf(33, 6, PSTR("XP"));
    draw_progress_bar(9, 7, ents[0].health, entity_max_health(0));
    draw_progress_bar(41, 7, pstats.xp, xp_for_level());
}

static int8_t const DDIRX[8] PROGMEM =
{
    -1, +1, -1, +1, +1, -1, 0, 0,
};
static int8_t const DDIRY[8] PROGMEM =
{
    -1, -1, +1, +1, 0, 0, +1, -1,
};

static uint8_t ddir_mask(uint8_t tx, uint8_t ty)
{
    uint8_t i = 0;
    for(uint8_t j = 0; j < 8; ++j)
    {
        i <<= 1;
        int8_t dx = (int8_t)pgm_read_byte(&DDIRX[j]);
        int8_t dy = (int8_t)pgm_read_byte(&DDIRY[j]);
        i |= (uint8_t)tile_is_solid_or_unknown(tx + dx, ty + dy);
    }
    return i;
}

void draw_map_offset(uint8_t ox)
{
    for(uint8_t y = 0; y < 64; y += 2)
        for(uint8_t x = 0; x < 64; x += 2)
        {
            set_pixel(x + ((y >> 1) & 1), y);
        }

    //dig_nonsecret_door_tiles();

    for(uint8_t y = 0; y < MAP_H; ++y)
    {
        bool prior = false;
        for(uint8_t tx = 0; tx < 32; ++tx)
        {
            uint8_t x = tx + ox;
            uint8_t px = tx * 2, py = y * 2;
            if(tile_is_solid_or_unknown(x, y))
            {
                uint8_t m = ddir_mask(x, y);
                if(tile_is_solid(x, y) && m != 0xff)
                {
                    clear_rect(px, px + 2, py, py + 2);
                    set_rect(px, px + 1, py, py + 1);

                    // thinner walls
                    if(!(m & 0x1)) clear_hline(px, px + 1, py);
                    if(!(m & 0x4)) clear_vline(px, py, py + 1);
                    if((m & 0x85) == 0x05) clear_pixel(px, py);

                    if(prior) clear_vline(px - 1, py, py + 2);
                    prior = false;
                }
                else
                    prior = true;
            }
            else
            {
                clear_pixel(px + (y & 1), py);
            }
        }
    }

    // draw doors
    for(uint8_t i = 0; i < num_doors; ++i)
    {
        auto const& d = doors[i];
        if(d.secret || d.open) continue;
        if(!tile_is_explored(d.x, d.y)) continue;
        uint8_t px = (d.x - ox) * 2, py = d.y * 2;
        clear_rect(px, px + 2, py, py + 2);
        set_pixel(px + 1, py + 1);
    }

    //update_doors();
    
    // draw stairs
    if(tile_is_explored(xdn, ydn))
    {
        uint8_t tx = (xdn - ox) * 2, ty = ydn * 2;
        set_hline(tx, tx + 1, ty + 1);
        set_pixel(tx, ty);
    }
    if(tile_is_explored(xup, yup))
    {
        uint8_t tx = (xup - ox) * 2, ty = yup * 2;
        set_hline(tx, tx + 1, ty + 1);
        set_pixel(tx + 1, ty);
    }
}

void draw_dungeon(uint8_t mx, uint8_t my)
{
    mx -= 6;
    my -= 6;

    static uint8_t const TILES[] PROGMEM =
    {
        0x01, 0x04, 0x10, 0x02, 0x08, // unexplored
        0x00, 0x00, 0x00, 0x00, 0x00,

        0x02, 0x02, 0x02, 0x02, 0x00, // center
        0x1d, 0x1d, 0x1d, 0x1d, 0x1d,

        // sides
        0x08, 0x08, 0x08, 0x08, 0x08, // N
        0x14, 0x14, 0x14, 0x14, 0x14,
        0x01, 0x01, 0x01, 0x01, 0x01, // S
        0x02, 0x02, 0x02, 0x02, 0x02,
        0x00, 0x00, 0x00, 0x1f, 0x00, // W
        0x00, 0x00, 0x16, 0x00, 0x1f,
        0x1f, 0x00, 0x00, 0x00, 0x00, // E
        0x00, 0x06, 0x00, 0x00, 0x00,

        // corners
        0x00, 0x00, 0x00, 0x18, 0x08, // NW
        0x00, 0x00, 0x1c, 0x04, 0x14,
        0x18, 0x00, 0x00, 0x00, 0x00, // NE
        0x04, 0x1c, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x01, 0x01, // SW
        0x00, 0x00, 0x03, 0x02, 0x02,
        0x01, 0x00, 0x00, 0x00, 0x00, // SE
        0x02, 0x03, 0x00, 0x00, 0x00,
    };

    dig_nonsecret_door_tiles();

    for(uint8_t y = 0; y < 13; ++y)
    {
        for(uint8_t x = 0; x < 13; ++x)
        {
            uint8_t tx = x + mx;
            uint8_t ty = y + my;
            uint8_t px = x * 5;
            uint8_t py = y * 5;

            if(tile_is_unknown(tx, ty))
            {
                draw_tile(&TILES[0], px, py);
                continue;
            }
            if(!tile_is_solid_or_unknown(tx, ty))
            {
                if(tile_is_solid(tx, ty - 1))
                {
                    uint8_t a = px - 1;
                    uint8_t b = px + 4;
                    if(x == 0 || !tile_is_solid(tx - 1, ty - 1)) ++a;
                    if(!tile_is_solid(tx + 1, ty - 1)) --b;
                    for(uint8_t i = 0; i < opt.wall_style; ++i)
                        set_hline(a, b, py - 1 + i);
                }
                continue;
            }

            draw_tile(&TILES[0], px, py);
            uint8_t i = ddir_mask(tx, ty);

            // sides
            if(!(i & 0x1)) draw_tile(&TILES[30], px, py); // N
            if(!(i & 0x2)) draw_tile(&TILES[20], px, py); // S
            if(!(i & 0x4)) draw_tile(&TILES[50], px, py); // W
            if(!(i & 0x8)) draw_tile(&TILES[40], px, py); // E

            // corners
            if((i & 0x1a) == 0x0a && tile_is_solid(tx, ty + 1))
                draw_tile(&TILES[60], px, py); // NW
            if((i & 0x26) == 0x06 && tile_is_solid(tx, ty + 1))
                draw_tile(&TILES[70], px, py); // NE
            if((i & 0x49) == 0x09 && tile_is_solid(tx + 1, ty))
                draw_tile(&TILES[80], px, py); // SW
            if((i & 0x85) == 0x05)
                draw_tile(&TILES[90], px, py); // SE

            // corrections
            if(!(i & 0x6)) clear_pixel(px + 0, py + 4);
            if(!(i & 0xa)) clear_pixel(px + 3, py + 4);
        }
    }

    update_doors();

    // draw lit tiles
    for(uint8_t y = 0; y < 13; ++y)
    {
        for(uint8_t x = 0; x < 13; ++x)
        {
            uint8_t tx = x + mx;
            uint8_t ty = y + my;
            if(!tile_is_solid_or_unknown(tx, ty))
            {
                if(player_can_see(tx, ty))
                    set_pixel(x * 5 + 2, y * 5 + 2 + (opt.wall_style == 3));
            }
        }
    }

    // draw doors
    for(uint8_t i = 0; i < num_doors; ++i)
    {
        auto const& d = doors[i];
        if(d.secret) continue;
        uint8_t dx = d.x - mx;
        uint8_t dy = d.y - my;
        if(dx >= 13 || dy >= 13) continue;
        if(tile_is_explored(d.x, d.y))
        {
            uint8_t oy = 0;
            uint8_t px = dx * 5;
            uint8_t py = dy * 5;
            bool ns = !tile_is_solid_or_unknown(d.x, d.y + 1);
            if(ns) oy = opt.wall_style;
            clear_rect(px - 1, px + 4, py, py + 4 + oy);
            draw_sprite_precise(&DOOR_IMGS[d.open], px, py);
        }
    }

    // draw stairs
    {
        uint8_t dx, dy;
        dx = xdn - mx;
        dy = ydn - my;
        if(dx <= 13 && dy < 13 && tile_is_explored(xdn, ydn))
            draw_sprite_nonprog(0xfec8, dx, dy);
        dx = xup - mx;
        dy = yup - my;
        if(dx <= 13 && dy < 13 && tile_is_explored(xup, yup))
            draw_sprite_nonprog(0x8cef, dx, dy);
    }

    // draw items
    for(auto const& i : items)
    {
        if(i.it.type == entity::NONE) continue;
        uint8_t ix = i.x - mx;
        uint8_t iy = i.y - my;
        if(ix >= 13 || iy >= 13) continue;
        if(player_can_see(i.x, i.y))
            draw_sprite(&ITEM_IMGS[i.it.type], ix, iy);
    }

    // draw entities
    for(auto const& e : ents)
    {
        if(e.type == entity::NONE) continue;
        uint8_t ex = e.x - mx;
        uint8_t ey = e.y - my;
        if(ex >= 13 || ey >= 13) continue;
        if(player_can_see(e.x, e.y))
            draw_sprite(&ENTITY_IMGS[e.type], ex, ey);
    }

    // draw generate message
    if(xup == 255)
    {
        draw_box_pretty(10, 52, 24, 40);
        draw_text(17, 30, PSTR("Loading..."));
    }
}
