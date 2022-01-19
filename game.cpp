#include "game.hpp"

#include <string.h>

#ifdef ARDUINO
#include <Arduino.h>
#endif

#define THICK_WALLS 1

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
#if 0
    uint16_t i = y / 8 * 64 + x;
    uint8_t f0 = y & 0x7;
    uint8_t f1 = 8 - f0;
    //uint8_t m0 = m << f0;
    //uint8_t m1 = m >> f1;
    if(x + w > 64)
        w = 64 - x;
    while(w-- > 0)
    {
        uint8_t ip = *p++;
        uint8_t ip0 = ip << f0;
        uint8_t ip1 = ip >> f1;
        if(i >= 512) return;
        //buf[i] = (buf[i] & ~m0) | ip0;
        buf[i] |= ip0;
        if(i + 64 < 512)
            //buf[i + 64] = (buf[i + 64] & ~m1) | ip1;
            buf[i + 64] |= ip1;
        ++i;
    }
#endif
}

static void draw_tile(
    uint16_t const* p, // PROGMEM
    uint8_t x, uint8_t y)
{
    uint16_t tp = pgm_read_word(p);
    uint8_t tt[4];
    tt[0] = (tp >> 12) & 0xf;
    tt[1] = (tp >>  8) & 0xf;
    tt[2] = (tp >>  4) & 0xf;
    tt[3] = (tp >>  0) & 0xf;
    draw_img(tt, 4, x * 5, y * 5);
}

static void draw_dungeon(uint8_t mx, uint8_t my)
{
    mx -= 6;
    my -= 6;

    static uint16_t const TILES[16] PROGMEM =
    {
        0xfddf, // 0000 <none>
        0xfccf, // 0001 N
        0xf11f, // 0010 S
        0xf00f, // 0011 NS
        0xdddf, // 0100 W
        0xcccf, // 0101 NW
        0x111f, // 0110 SW
        0x000f, // 0111 NSW
        0xfddd, // 1000 E
        0xfccc, // 1001 NE
        0xf111, // 1010 SE
        0xf000, // 1011 NSE
        0xdddd, // 1100 WE
        0xcccc, // 1101 NWE
        0x1111, // 1110 SWE
        0x0000, // 1111 NSWE
    };

    static uint8_t const BGTILE[5] PROGMEM =
    {
        0x02, 0x08, 0x01, 0x04, 0x10,
    };
    uint8_t bgtile[5];
    for(uint8_t i = 0; i < 5; ++i)
        bgtile[i] = pgm_read_byte(&BGTILE[i]);

    for(uint8_t y = 0; y < 13; ++y)
    {
        for(uint8_t x = 0; x < 13; ++x)
        {
            uint8_t tx = x + mx;
            uint8_t ty = y + my;
            if(!tile_is_solid(tx + 0, ty + 0))
                continue;

            uint8_t px = x * 5;
            uint8_t py = y * 5;

            // draw main tile
            uint8_t i = 0;
            i |= (uint8_t)tile_is_solid(tx + 0, ty - 1) << 0;
            i |= (uint8_t)tile_is_solid(tx + 0, ty + 1) << 1;
            i |= (uint8_t)tile_is_solid(tx - 1, ty + 0) << 2;
            i |= (uint8_t)tile_is_solid(tx + 1, ty + 0) << 3;

            if(i == 0xf)
                draw_img(bgtile, 5, px, py);
            draw_tile(&TILES[i], x, y);

            // draw inside corners and extensions
            if(i & 0x1)
            {
                if((i & 0x4) && !tile_is_solid(tx - 1, ty - 1))
                    set_vline(px + 0, py - 1, py + 0);
                if((i & 0x8) && !tile_is_solid(tx + 1, ty - 1))
                {
                    set_vline(px + 3, py - 1, py + 0);
                    set_pixel(px + 4, py + 0);
                }
                if(!(i & 0x4)) set_pixel(px + 0, py - 1);
                if(!(i & 0x8)) set_pixel(px + 3, py - 1);
            }
            if(i & 0x2)
            {
                if((i & 0x4) && !tile_is_solid(tx - 1, ty + 1))
                    set_vline(px + 0, py + 2, py + 3);
                if((i & 0x8) && !tile_is_solid(tx + 1, ty + 1))
                {
                    set_vline(px + 3, py + 2, py + 3);
                    set_vline(px + 4, py + 2, py + 3);
#if !THICK_WALLS
                    clear_pixel(px + 4, py + 4);
#endif
                }
            }
            else
            {
#if THICK_WALLS
                set_hline(px + 0, px + 3, py + 4);
                if(i & 0x8) set_pixel(px + 4, py + 4);
#else
                clear_pixel(px + 4, py + 4);
#endif
            }
            if(i & 0x8)
            {
                if(!(i & 0x1)) set_pixel(px + 4, py + 0);
                if(!(i & 0x2)) set_vline(px + 4, py + 2, py + 3);
            }
            else
            {
                clear_pixel(px + 4, py + 4);
            }
        }
    }

    // draw entities
    for(auto const& e : ents)
    {
        if(e.type == entity::NONE) continue;
        uint8_t ex = e.x - mx;
        uint8_t ey = e.y - my;
        if(ex >= 13 || ey >= 13) continue;
        draw_tile(&ENTITY_IMGS[e.type], ex, ey);
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
    bool act = false;
    switch(b)
    {
    case BTN_UP   : act = try_move_ent(0, 0, -1); break;
    case BTN_DOWN : act = try_move_ent(0, 0, +1); break;
    case BTN_LEFT : act = try_move_ent(0, -1, 0); break;
    case BTN_RIGHT: act = try_move_ent(0, +1, 0); break;
    default: break;
    }
    if(act)
    {
        draw_dungeon(ents[0].x, ents[0].y);
        paint_left();
    }
}
