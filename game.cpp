#include "game.hpp"

#include <string.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

static uint16_t const ENTITY_IMGS[32] PROGMEM =
{
    0x0000,
    0x6ff6,
};

globals globals_;

static uint16_t rand_seed0, rand_seed1;

static uint8_t u8rand()
{
    const uint16_t temp = (rand_seed0 ^ (rand_seed0 << 5));
    rand_seed0 = rand_seed1;
    rand_seed1 = (rand_seed1 ^ (rand_seed1 >> 1)) ^ (temp ^ (temp >> 3));
    return (uint8_t)rand_seed1;
}

static void set_pixel(uint8_t x, uint8_t y)
{
    if(x < 64 && y < 64)
    {
        uint8_t& b = buf[y / 8 * 64 + x];
        b |= (1 << (y % 8));
    }
}

static void clear_pixel(uint8_t x, uint8_t y)
{
    if(x < 64 && y < 64)
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

static bool tile_is_solid(uint8_t x, uint8_t y)
{
    if(x >= MAP_W || y >= MAP_H) return true;
    uint8_t t = tmap[y / 8 * 128 + x];
    return (t & (1 << (y % 8))) != 0;
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
    tt[1] = (tp >>  8) & 0xf;
    tt[2] = (tp >>  4) & 0xf;
    tt[3] = (tp >>  0) & 0xf;
    x *= 5;
    y *= 5;
    clear_img(tt, 4, x - 1, y - 1);
    clear_img(tt, 4, x + 0, y - 1);
    clear_img(tt, 4, x + 1, y - 1);
    draw_img(tt, 4, x, y);
}

static void draw_dungeon(uint8_t mx, uint8_t my)
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

    for(uint8_t y = 0; y < 13; ++y)
    {
        for(uint8_t x = 0; x < 13; ++x)
        {
            uint8_t tx = x + mx;
            uint8_t ty = y + my;
            uint8_t px = x * 5;
            uint8_t py = y * 5;

            if(false /* tile is unexplored */)
            {
                draw_tile(&TILES[0], px, py);
                continue;
            }
            if(!tile_is_solid(tx, ty))
            {
                if(tile_is_solid(tx, ty - 1))
                {
                    uint8_t a = px - 1;
                    uint8_t b = px + 4;
                    if(!tile_is_solid(tx - 1, ty - 1)) ++a;
                    if(!tile_is_solid(tx + 1, ty - 1)) --b;
                    for(uint8_t i = 0; i < opt.wall_style; ++i)
                        set_hline(a, b, py - 1 + i);
                }
                continue;
            }
            draw_tile(&TILES[0], px, py);

            uint8_t i = 0;

            i |= (uint8_t)tile_is_solid(tx + 0, ty - 1) << 0;
            i |= (uint8_t)tile_is_solid(tx + 0, ty + 1) << 1;
            i |= (uint8_t)tile_is_solid(tx - 1, ty + 0) << 2;
            i |= (uint8_t)tile_is_solid(tx + 1, ty + 0) << 3;

            i |= (uint8_t)tile_is_solid(tx + 1, ty + 1) << 4;
            i |= (uint8_t)tile_is_solid(tx - 1, ty + 1) << 5;
            i |= (uint8_t)tile_is_solid(tx + 1, ty - 1) << 6;
            i |= (uint8_t)tile_is_solid(tx - 1, ty - 1) << 7;

            // sides
            if(!(i & 0x1)) draw_tile(&TILES[30], px, py);
            if(!(i & 0x2)) draw_tile(&TILES[20], px, py);
            if(!(i & 0x4)) draw_tile(&TILES[50], px, py);
            if(!(i & 0x8)) draw_tile(&TILES[40], px, py);

            // corners
            if((i & 0x1a) == 0x0a) draw_tile(&TILES[60], px, py); // NW
            if((i & 0x26) == 0x06) draw_tile(&TILES[70], px, py); // NE
            if((i & 0x49) == 0x09) draw_tile(&TILES[80], px, py); // SW
            if((i & 0x85) == 0x05) draw_tile(&TILES[90], px, py); // SE

            // corrections
            if((i & 0x6) == 0) clear_pixel(px + 0, py + 4);
            if((i & 0xa) == 0) clear_pixel(px + 3, py + 4);
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

static void dig_tile(uint8_t x, uint8_t y)
{
    uint8_t& t = tmap[y / 8 * 128 + x];
    t &= ~(1 << (y % 8));
}

static void generate_dungeon(uint8_t mapi)
{
    rand_seed0 = seed0;
    rand_seed1 = seed1 + mapi;
    for(auto& t : tmap) t = 0xff;

    for(uint8_t y = 0; y < 3; ++y)
        for(uint8_t x = 0; x < 3; ++x)
            dig_tile(x, y);
    dig_tile(3, 2);
    dig_tile(5, 5);
    dig_tile(5, 4);
    dig_tile(4, 5);
    dig_tile(1, 3);

    for(uint8_t i = 0; i < 5; ++i)
        dig_tile(i, 4);

    dig_tile(7, 5);
    dig_tile(7, 4);
}

static bool try_move_ent(uint8_t i, uint8_t dx, uint8_t dy)
{
    uint8_t nx = ents[i].x + dx;
    uint8_t ny = ents[i].y + dy;
    if(tile_is_solid(nx, ny))
    {
        // TODO: check for other entities to attack
        return false;
    }
    ents[i].x = nx;
    ents[i].y = ny;
    return true;
}

void game_setup()
{
    memset(&globals_, 0, sizeof(globals_));

    ents[0].type = entity::PLAYER;

    seed0 = 0x1234;
    seed1 = 0xcafe;

    generate_dungeon(0);

    draw_dungeon(ents[0].x, ents[0].y);
    paint_left();
    paint_right();
}

void game_loop()
{
    uint8_t b = wait_btn();
    bool rd = false;
    switch(b)
    {
    case BTN_UP   : rd = try_move_ent(0, 0, -1); break;
    case BTN_DOWN : rd = try_move_ent(0, 0, +1); break;
    case BTN_LEFT : rd = try_move_ent(0, -1, 0); break;
    case BTN_RIGHT: rd = try_move_ent(0, +1, 0); break;
    case BTN_A:
        if(++opt.wall_style == NUM_WALL_STYLES) opt.wall_style = 0;
        rd = true;
        break;
    default: break;
    }
    if(rd)
    {
        draw_dungeon(ents[0].x, ents[0].y);
        paint_left();
    }
}
