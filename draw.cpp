#include "game.hpp"

static uint16_t const ENTITY_IMGS[32] PROGMEM =
{
    0x0000, // none
    0x6ff6, // player
};

static void set_pixel(uint8_t x, uint8_t y)
{
    if(!((x | y) & 0xc0)) // if(x < 64 && y < 64)
    {
        uint8_t& b = buf[y / 8 * 64 + x];
        b |= (1 << (y % 8));
    }
}

static void clear_pixel(uint8_t x, uint8_t y)
{
    if(!((x | y) & 0xc0)) // if(x < 64 && y < 64)
    {
        uint8_t& b = buf[y / 8 * 64 + x];
        b &= ~(1 << (y % 8));
    }
}

static void set_hline(uint8_t x0, uint8_t x1, uint8_t y)
{
    for(; x0 <= x1; ++x0)
        set_pixel(x0, y);
}

static void clear_hline(uint8_t x0, uint8_t x1, uint8_t y)
{
    for(; x0 <= x1; ++x0)
        clear_pixel(x0, y);
}

static void set_vline(uint8_t x, uint8_t y0, uint8_t y1)
{
    for(; y0 <= y1; ++y0)
        set_pixel(x, y0);
}

static void set_rect(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1)
{
    for(; y0 <= y1; ++y0)
        set_hline(x0, x1, y0);
}

static void clear_rect(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1)
{
    for(; y0 <= y1; ++y0)
        clear_hline(x0, x1, y0);
}

static void set_box(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1)
{
    set_hline(x0, x1, y0);
    set_hline(x0, x1, y1);
    set_vline(x0, y0, y1);
    set_vline(x1, y0, y1);
}

static void draw_box_pretty(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1)
{
    clear_rect(x0 - 1, x1 + 2, y0 - 1, y1 + 2);
    set_box(x0, x1, y0, y1);
    set_hline(x0 + 1, x1 + 1, y1 + 1);
    set_vline(x1 + 1, y0 + 1, y1 + 1);
}

static void draw_img(
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

static void draw_sprite(
    uint16_t const* p, // PROGMEM
    uint8_t x, uint8_t y)
{
    uint16_t tp = pgm_read_word(p);
    uint8_t tt[4];
    tt[0] = (tp >> 12) & 0xf;
    tt[1] = (tp >> 8) & 0xf;
    tt[2] = (tp >> 4) & 0xf;
    tt[3] = (tp >> 0) & 0xf;
    x *= 5;
    y *= 5;
    clear_img(tt, 4, x - 1, y - 1);
    clear_img(tt, 4, x + 0, y - 1);
    clear_img(tt, 4, x + 1, y - 1);
    draw_img(tt, 4, x, y);
}

static void draw_dungeon_minimap()
{
    static constexpr uint8_t OX = 3, OY = 2;
    for(uint8_t y = 0; y < MAP_H/2; ++y)
        for(uint8_t x = 0; x < MAP_W/2; ++x)
            if(2 <=
                (uint8_t)(!tile_is_solid_or_unknown(x * 2 + 0, y * 2 + 0)) +
                (uint8_t)(!tile_is_solid_or_unknown(x * 2 + 1, y * 2 + 0)) +
                (uint8_t)(!tile_is_solid_or_unknown(x * 2 + 0, y * 2 + 1)) +
                (uint8_t)(!tile_is_solid_or_unknown(x * 2 + 1, y * 2 + 1)))
                set_pixel(x + OX, y + OY);
}

void draw_info()
{
    draw_box_pretty(1, MAP_W / 2 + 5, 0, MAP_H / 2 + 4);
    draw_dungeon_minimap();
    draw_text(1, 40, PSTR("THE QUICK BROWN"));
    draw_text(1, 46, PSTR("FOX JUMPS OVER"));
    draw_text(1, 52, PSTR("THE LAZY DOG."));
    draw_text(1, 58, PSTR("0123456789?!"));
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

    uint8_t r2 = light_radius2();
    uint8_t ex = ents[0].x;
    uint8_t ey = ents[0].y;

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
                uint8_t dx = u8abs(ex - tx);
                uint8_t dy = u8abs(ey - ty);
                if(dx * dx + dy * dy < r2 && path_clear(ex, ey, tx, ty))
                    set_pixel(px + 2, py + 3);
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

            uint8_t i = 0;

            i |= (uint8_t)tile_is_solid_or_unknown(tx + 0, ty - 1) << 0;
            i |= (uint8_t)tile_is_solid_or_unknown(tx + 0, ty + 1) << 1;
            i |= (uint8_t)tile_is_solid_or_unknown(tx - 1, ty + 0) << 2;
            i |= (uint8_t)tile_is_solid_or_unknown(tx + 1, ty + 0) << 3;

            i |= (uint8_t)tile_is_solid_or_unknown(tx + 1, ty + 1) << 4;
            i |= (uint8_t)tile_is_solid_or_unknown(tx - 1, ty + 1) << 5;
            i |= (uint8_t)tile_is_solid_or_unknown(tx + 1, ty - 1) << 6;
            i |= (uint8_t)tile_is_solid_or_unknown(tx - 1, ty - 1) << 7;

            // sides
            if(!(i & 0x1)) draw_tile(&TILES[30], px, py);
            if(!(i & 0x2)) draw_tile(&TILES[20], px, py);
            if(!(i & 0x4)) draw_tile(&TILES[50], px, py);
            if(!(i & 0x8)) draw_tile(&TILES[40], px, py);

            // corners
            if((i & 0x1a) == 0x0a && !tile_is_unknown(tx, ty - 1))
                draw_tile(&TILES[60], px, py); // NW
            if((i & 0x26) == 0x06 && !tile_is_unknown(tx, ty - 1))
                draw_tile(&TILES[70], px, py); // NE
            if((i & 0x49) == 0x09 && !tile_is_unknown(tx + 1, ty))
                draw_tile(&TILES[80], px, py); // SW
            if((i & 0x85) == 0x05) draw_tile(&TILES[90], px, py); // SE

            // corrections
            if(!(i & 0x6)) clear_pixel(px + 0, py + 4);
            if(!(i & 0xa)) clear_pixel(px + 3, py + 4);
        }
    }

    // draw entities
    for(auto const& e : ents)
    {
        if(e.type == entity::NONE) continue;
        uint8_t ex = e.x - mx;
        uint8_t ey = e.y - my;
        if(ex >= 13 || ey >= 13) continue;
        draw_sprite(&ENTITY_IMGS[e.type], ex, ey);
    }
}

static uint8_t const FONT_DATA[] PROGMEM =
{
    0x00, 0xff, 0xff, 0xff, /* [space] */

    0x17, 0xff, 0xff, 0xff, /* ! */
    0x03, 0x00, 0x03, 0xff, /* " */
    0xff, 0xff, 0xff, 0xff, /* # */
    0xff, 0xff, 0xff, 0xff, /* $ */
    0x19, 0x04, 0x13, 0xff, /* % */
    0xff, 0xff, 0xff, 0xff, /* & */
    0x03, 0xff, 0xff, 0xff, /* ' */
    0x0e, 0x11, 0xff, 0xff, /* ( */
    0x11, 0x0e, 0xff, 0xff, /* ) */
    0xff, 0xff, 0xff, 0xff, /* * */
    0x04, 0x0e, 0x04, 0xff, /* + */
    0x10, 0x08, 0xff, 0xff, /* , */
    0x04, 0x04, 0x04, 0xff, /* - */
    0x10, 0xff, 0xff, 0xff, /* . */
    0x18, 0x04, 0x03, 0xff, /* / */

    0x0e, 0x11, 0x0e, 0xff, /* 0 */
    0x12, 0x1f, 0x10, 0xff, /* 1 */
    0x19, 0x15, 0x12, 0xff, /* 2 */
    0x11, 0x15, 0x0a, 0xff, /* 3 */
    0x07, 0x04, 0x1f, 0xff, /* 4 */
    0x17, 0x15, 0x09, 0xff, /* 5 */
    0x0e, 0x15, 0x08, 0xff, /* 6 */
    0x01, 0x19, 0x07, 0xff, /* 7 */
    0x0a, 0x15, 0x0a, 0xff, /* 8 */
    0x02, 0x15, 0x0e, 0xff, /* 9 */

    0x0a, 0xff, 0xff, 0xff, /* : */
    0x10, 0x08, 0xff, 0xff, /* , */
    0xff, 0xff, 0xff, 0xff, /* < */
    0x0a, 0x0a, 0x0a, 0xff, /* = */
    0xff, 0xff, 0xff, 0xff, /* > */
    0x01, 0x15, 0x02, 0xff, /* ? */
    0xff, 0xff, 0xff, 0xff, /* @ */

    0x1e, 0x05, 0x1e, 0xff, /* A */
    0x1f, 0x15, 0x0a, 0xff, /* B */
    0x0e, 0x11, 0x11, 0xff, /* C */
    0x1f, 0x11, 0x0e, 0xff, /* D */
    0x1f, 0x15, 0x11, 0xff, /* E */
    0x1f, 0x05, 0x01, 0xff, /* F */
    0x0e, 0x11, 0x19, 0xff, /* G */
    0x1f, 0x04, 0x1f, 0xff, /* H */
    0x1f, 0xff, 0xff, 0xff, /* I */
    0x11, 0x0f, 0xff, 0xff, /* J */
    0x1f, 0x04, 0x1b, 0xff, /* K */
    0x1f, 0x10, 0x10, 0xff, /* L */
    0x1f, 0x02, 0x02, 0x1f, /* M */
    0x1f, 0x06, 0x0c, 0x1f, /* N */
    0x0e, 0x11, 0x11, 0x0e, /* O */
    0x1f, 0x05, 0x02, 0xff, /* P */
    0x0e, 0x11, 0x1e, 0xff, /* Q */
    0x1f, 0x05, 0x1a, 0xff, /* R */
    0x12, 0x15, 0x09, 0xff, /* S */
    0x01, 0x1f, 0x01, 0xff, /* T */
    0x0f, 0x10, 0x10, 0x0f, /* U */
    0x07, 0x18, 0x07, 0xff, /* V */
    0x1f, 0x08, 0x08, 0x1f, /* W */
    0x1b, 0x04, 0x1b, 0xff, /* X */
    0x03, 0x14, 0x0f, 0xff, /* Y */
    0x19, 0x15, 0x13, 0xff, /* Z */
};

void draw_text(uint8_t x, uint8_t y, char const* t, bool prog)
{
    for(;;)
    {
        char c = prog ? pgm_read_byte(t++) : *t++;
        if(c == '\0') return;
        uint8_t const* p = FONT_DATA + ((c - ' ') * 4);
        for(uint8_t i = 0; i < 4; ++i)
        {
            uint8_t t = pgm_read_byte(p++);
            if(!(t & 0x80))
            {
                for(uint8_t j = y; t; ++j, t >>= 1)
                    if(t & 1) set_pixel(x, j);
                ++x;
            }
        }
        ++x;
    }
}
