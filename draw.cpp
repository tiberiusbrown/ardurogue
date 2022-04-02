#include "game.hpp"

static uint16_t const ENTITY_IMGS[32] PROGMEM =
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
    0x01f1, // troll
    0x069d, // griffin
    0xf996, // dragon
};
static uint16_t const DOOR_IMGS[2] PROGMEM = { 0xefbe, 0xe11e };

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
    draw_img(tt, 4, x, y);
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

void draw_info()
{
    //draw_box_pretty(1, MAP_W / 2 + 5, 0, MAP_H / 2 + 4);
    draw_box_pretty(1, MAP_W + 5, 0, MAP_H + 4);
    draw_dungeon_minimap();
    //draw_text(1, 34, PSTR("the lazy dog."));
    //draw_text(1, 40, PSTR("THE QUICK BROWN"));
    //draw_text(1, 46, PSTR("FOX JUMPS OVER"));
    //draw_text(1, 52, PSTR("THE LAZY DOG."));
    //draw_text(1, 58, PSTR("0123456789!?%"));
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
        if(dx <= 13 && dy < 13 && tile_is_explored(xdn, ydn))
            draw_sprite_nonprog(0x8cef, dx, dy);
    }

    // draw items

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
}

static constexpr uint16_t fd(
    uint8_t a = 0, uint8_t b = 0, uint8_t c = 0)
{
    return
        (uint16_t(a & 0x1f) <<  0) |
        (uint16_t(b & 0x1f) <<  5) |
        (uint16_t(c & 0x1f) << 10);
}

static uint16_t const FONT_DATA[] PROGMEM =
{

    fd(0x17            ), /* ! */
    fd(0x03, 0x00, 0x03), /* " */
    fd(                ), /* # */
    fd(                ), /* $ */
    fd(0x19, 0x04, 0x13), /* % */
    fd(                ), /* & */
    fd(0x03            ), /* ' */
    fd(0x0e, 0x11      ), /* ( */
    fd(0x11, 0x03      ), /* ) */
    fd(                ), /* * */
    fd(0x04, 0x0e, 0x04), /* + */
    fd(0x10, 0x08      ), /* , */
    fd(0x04, 0x04, 0x04), /* - */
    fd(0x10            ), /* . */
    fd(0x18, 0x04, 0x03), /* / */

    fd(0x0e, 0x11, 0x0e), /* 0 */
    fd(0x12, 0x1f, 0x10), /* 1 */
    fd(0x19, 0x15, 0x12), /* 2 */
    fd(0x11, 0x15, 0x0a), /* 3 */
    fd(0x07, 0x04, 0x1f), /* 4 */
    fd(0x17, 0x15, 0x09), /* 5 */
    fd(0x0e, 0x15, 0x08), /* 6 */
    fd(0x01, 0x19, 0x07), /* 7 */
    fd(0x0a, 0x15, 0x0a), /* 8 */
    fd(0x02, 0x15, 0x0e), /* 9 */

    fd(0x0a            ), /* : */
    fd(0x10, 0x0a      ), /* ; */
    fd(0x04, 0x04, 0x0a), /* < */
    fd(0x0a, 0x0a, 0x0a), /* = */
    fd(0x0a, 0x04, 0x04), /* > */
    fd(0x01, 0x15, 0x02), /* ? */
    fd(                ), /* @ */

    fd(0x1e, 0x05, 0x1e), /* A */
    fd(0x1f, 0x15, 0x0a), /* B */
    fd(0x0e, 0x11, 0x11), /* C */
    fd(0x1f, 0x11, 0x0e), /* D */
    fd(0x1f, 0x15, 0x11), /* E */
    fd(0x1f, 0x05, 0x01), /* F */
    fd(0x0e, 0x11, 0x19), /* G */
    fd(0x1f, 0x04, 0x1f), /* H */
    fd(0x11, 0x1f, 0x11), /* I */
    fd(0x11, 0x0f      ), /* J */
    fd(0x1f, 0x04, 0x1b), /* K */
    fd(0x1f, 0x10, 0x10), /* L */
    fd(0x1f, 0x02, 0x1f), /* M */
    fd(0x1f, 0x01, 0x1e), /* N */
    fd(0x1f, 0x11, 0x1f), /* O */
    fd(0x1f, 0x05, 0x02), /* P */
    fd(0x0e, 0x11, 0x1e), /* Q */
    fd(0x1f, 0x05, 0x1a), /* R */
    fd(0x12, 0x15, 0x09), /* S */
    fd(0x01, 0x1f, 0x01), /* T */
    fd(0x1f, 0x10, 0x1f), /* U */
    fd(0x07, 0x18, 0x07), /* V */
    fd(0x1f, 0x08, 0x1f), /* W */
    fd(0x1b, 0x04, 0x1b), /* X */
    fd(0x03, 0x14, 0x0f), /* Y */
    fd(0x19, 0x15, 0x13), /* Z */

    fd(0x1f, 0x11      ), /* [ */
    fd(0x03, 0x04, 0x18), /* \ */
    fd(0x11, 0x1f      ), /* ] */
    fd(0x02, 0x01, 0x02), /* ^ */
    fd(0x10, 0x10, 0x10), /* _ */
    fd(0x01, 0x02      ), /* ` */

    fd(0x0c, 0x12, 0x1e), /* a */
    fd(0x1f, 0x12, 0x0c), /* b */
    fd(0x0c, 0x12, 0x12), /* c */
    fd(0x0c, 0x12, 0x1f), /* d */
    fd(0x0c, 0x1a, 0x14), /* e */
    fd(0x1e, 0x05      ), /* f */
    fd(0x16, 0x1e      ), /* g */
    fd(0x1f, 0x02, 0x1c), /* h */
    fd(0x1d            ), /* i */
    fd(0x10, 0x0d      ), /* j */
    fd(0x1f, 0x04, 0x1a), /* k */
    fd(0x1f            ), /* l */
    fd(0x1e, 0x04, 0x1e), /* m */
    fd(0x1e, 0x02, 0x1c), /* n */
    fd(0x0c, 0x12, 0x0c), /* o */
    fd(0x1e, 0x0a, 0x04), /* p */
    fd(0x04, 0x0a, 0x1e), /* q */
    fd(0x1e, 0x04, 0x02), /* r */
    fd(0x16, 0x1a      ), /* s */
    fd(0x0f, 0x12      ), /* t */
    fd(0x0e, 0x10, 0x1e), /* u */
    fd(0x06, 0x18, 0x06), /* v */
    fd(0x1e, 0x08, 0x1e), /* w */
    fd(0x12, 0x0c, 0x12), /* x */
    fd(0x02, 0x14, 0x0e), /* y */
    fd(0x1a, 0x16      ), /* z */

};

void draw_text(uint8_t x, uint8_t y, char const* t, bool prog)
{
    for(;;)
    {
        char c = prog ? pgm_read_byte(t++) : *t++;
        if(c < 32) return;
        if(c == ' ')
        {
            x += 2;
            continue;
        }
        uint16_t p = pgm_read_word(FONT_DATA + c - 33);
        uint8_t d[3], n = 3;
        d[0] = uint8_t(p) & 0x1f;
        d[1] = uint8_t(p >> 5) & 0x1f;
        d[2] = uint8_t(p >> 10) & 0x1f;
        if(d[2] == 0) n = (d[1] == 0 ? 1 : 2);
        for(uint8_t i = 0; i < n; ++i, ++x)
        {
            uint8_t t = d[i];
            for(uint8_t j = y; t; ++j, t >>= 1)
                if(t & 1) set_pixel(x, j);
        }
        ++x;
    }
}
