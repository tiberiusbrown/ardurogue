#include "game.hpp"

static constexpr uint8_t ROOM_TRIES = 64;

struct room_info
{
    uint8_t w;
    uint8_t h;
    uint8_t mask[8];
};

static constexpr uint8_t cbm(uint32_t x)
{
    return
        (((x & 0x00000001) ? 1 : 0) << 7) +
        (((x & 0x00000010) ? 1 : 0) << 6) +
        (((x & 0x00000100) ? 1 : 0) << 5) +
        (((x & 0x00001000) ? 1 : 0) << 4) +
        (((x & 0x00010000) ? 1 : 0) << 3) +
        (((x & 0x00100000) ? 1 : 0) << 2) +
        (((x & 0x01000000) ? 1 : 0) << 1) +
        (((x & 0x10000000) ? 1 : 0) << 0);
}

static constexpr room_info const ROOM_TYPES[] PROGMEM =
{
{
    6, 6, {
    cbm(0x10000111),
    cbm(0x00000011),
    cbm(0x00110011),
    cbm(0x00110011),
    cbm(0x00000011),
    cbm(0x10000111),
    cbm(0x11111111),
    cbm(0x11111111), },
},
{
    8, 8, {
    cbm(0x11000011),
    cbm(0x10000001),
    cbm(0x00000000),
    cbm(0x00000000),
    cbm(0x00000000),
    cbm(0x00000000),
    cbm(0x10000001),
    cbm(0x11000011), },
},
{
    8, 8, {
    cbm(0x11100111),
    cbm(0x11100111),
    cbm(0x11000011),
    cbm(0x00000000),
    cbm(0x00000000),
    cbm(0x11000011),
    cbm(0x11100111),
    cbm(0x11100111), },
},
{
    8, 8, {
    cbm(0x00000000),
    cbm(0x01100110),
    cbm(0x01000010),
    cbm(0x00000000),
    cbm(0x00000000),
    cbm(0x01000010),
    cbm(0x01100110),
    cbm(0x00000000), },
},
};
static constexpr uint8_t NUM_ROOM_TYPES =
sizeof(ROOM_TYPES) / sizeof(ROOM_TYPES[0]);

static void dig_tile(uint8_t x, uint8_t y, bool explored)
{
    uint16_t const i = y / 8 * MAP_W + x;
    uint8_t const m = 1 << (y % 8);
    tmap[i] &= ~m;
    if(explored)
        tfog[i] |= m;
}

static constexpr uint8_t MIN_SIZE = 8;
static constexpr uint8_t MIN_SPLIT_SIZE = MIN_SIZE * 2 + 1;
static constexpr uint8_t SPLIT_TRIES = 8;

struct bsp_data
{
    struct bsp_node { uint8_t x, y, w, h; };
    std::array<bsp_node, MAP_ROOMS> stack;
    uint8_t n;
    uint8_t mapi;
};
static_assert(sizeof(bsp_data) <= sizeof(buf), "");

// r0.x < r1.x
static void join_rooms_horz(uint8_t r0, uint8_t r1, bool explored)
{
    uint8_t x0 = rooms[r0].x + rooms[r0].w;
    uint8_t y0 = rooms[r0].y + rooms[r0].h / 2;
    while(tile_is_solid(x0 - 1, y0)) --x0;
    uint8_t x1 = rooms[r1].x - 1;
    uint8_t y1 = rooms[r1].y + rooms[r1].h / 2;
    while(tile_is_solid(x1 + 1, y1)) ++x1;

    uint8_t x2 = (x0 + x1 + 1) / 2;
    for(uint8_t i = x0; i <= x2; ++i)
        dig_tile(i, y0, explored);
    if(y0 < y1)
        for(uint8_t i = y0; i <= y1; ++i)
            dig_tile(x2, i, explored);
    else
        for(uint8_t i = y1; i <= y0; ++i)
            dig_tile(x2, i, explored);
    for(uint8_t i = x2; i <= x1; ++i)
        dig_tile(i, y1, explored);
}

// r0.y < r1.y
static void join_rooms_vert(uint8_t r0, uint8_t r1, bool explored)
{
    uint8_t x0 = rooms[r0].x + rooms[r0].w / 2;
    uint8_t y0 = rooms[r0].y + rooms[r0].h;
    while(tile_is_solid(x0, y0 - 1)) --y0;
    uint8_t x1 = rooms[r1].x + rooms[r1].w / 2;
    uint8_t y1 = rooms[r1].y - 1;
    while(tile_is_solid(x1, y1 + 1)) ++y1;

    uint8_t y2 = (y0 + y1 + 1) / 2;
    for(uint8_t i = y0; i <= y2; ++i)
        dig_tile(x0, i, explored);
    if(x0 < x1)
        for(uint8_t i = x0; i <= x1; ++i)
            dig_tile(i, y2, explored);
    else
        for(uint8_t i = x1; i <= x0; ++i)
            dig_tile(i, y2, explored);
    for(uint8_t i = y2; i <= y1; ++i)
        dig_tile(x1, i, explored);
}

static void join_rooms(uint8_t r0, uint8_t r1, bool explored)
{
    if(rooms[r0].x > rooms[r1].x)
        std::swap(r0, r1);
    if(rooms[r0].x + MIN_SIZE < rooms[r1].x)
        join_rooms_horz(r0, r1, explored);
    else
    {
        if(rooms[r0].y > rooms[r1].y)
            std::swap(r0, r1);
        join_rooms_vert(r0, r1, explored);
    }
}

static bool room_is_explored(uint8_t mapi, uint8_t r)
{
    return r < MAP_ROOMS && maps[mapi].got_rooms.test(r);
}

static uint8_t generate_recurse()
{
    auto& data = *(bsp_data*)&buf;
    auto& stack = data.stack;
    auto& n = data.n;

    if(num_rooms >= MAP_ROOMS)
        return -1;

    // check stop condition
    if(stack[n].w <= MIN_SPLIT_SIZE && stack[n].h <= MIN_SPLIT_SIZE)
    {
        //rooms[nr].secret = ...;

        uint8_t x, y, w, h;
        bool const explored = maps[data.mapi].got_rooms.test(num_rooms);

        if(u8rand() < 64)
        {
            // special room
            uint8_t t = u8rand(NUM_ROOM_TYPES);
            w = pgm_read_byte(&ROOM_TYPES[t].w);
            h = pgm_read_byte(&ROOM_TYPES[t].h);
            x = stack[n].x;
            y = stack[n].y;
            if(stack[n].w > w) x = stack[n].x + u8rand(stack[n].w - w);
            if(stack[n].h > h) y = stack[n].y + u8rand(stack[n].h - h);
            for(uint8_t iy = 0; iy < h; ++iy)
            {
                uint8_t m = pgm_read_byte(&ROOM_TYPES[t].mask[iy]);
                for(uint8_t ix = 0; ix < w; ++ix, m >>= 1)
                    if(!(m & 1)) dig_tile(x + ix, y + iy, explored);
            }
        }
        else
        {
            w = stack[n].w - u8rand(4) - 1;
            h = stack[n].h - u8rand(4) - 1;
            x = stack[n].x + u8rand(stack[n].w - w);
            y = stack[n].y + u8rand(stack[n].h - h);
            for(uint8_t iy = 0; iy < h; ++iy)
                for(uint8_t ix = 0; ix < w; ++ix)
                    dig_tile(x + ix, y + iy, explored);
        }

        rooms[num_rooms] = { x, y, w, h };

        return num_rooms++;
    }

    uint8_t r = -1;
    
    for(uint8_t i = 0; i < SPLIT_TRIES; ++i)
    {
        if((u8rand() & 1) && stack[n].h > MIN_SPLIT_SIZE)
        {
            // split north-south
            uint8_t y = stack[n].y + u8rand(stack[n].h - MIN_SPLIT_SIZE) + MIN_SIZE;
            ++n;
            stack[n] = stack[n - 1];
            stack[n].h = y - stack[n].y;
            uint8_t r0 = generate_recurse();
            stack[n].y = y + 1;
            stack[n].h = stack[n - 1].h - stack[n].h - 1;
            uint8_t r1 = generate_recurse();
            --n;

            bool explored =
                room_is_explored(data.mapi, r0) ||
                room_is_explored(data.mapi, r1);

            // join with vertical corridor
            if(r0 < MAP_ROOMS && r1 < MAP_ROOMS)
                join_rooms(r0, r1, explored);

            r = (u8rand() & 1) ? r0 : r1;
            break;
        }
        else if(stack[n].w > MIN_SPLIT_SIZE)
        {
            // split west-east
            uint8_t x = stack[n].x + u8rand(stack[n].w - MIN_SPLIT_SIZE) + MIN_SIZE;
            ++n;
            stack[n] = stack[n - 1];
            stack[n].w = x - stack[n].x;
            uint8_t r0 = generate_recurse();
            stack[n].x = x + 1;
            stack[n].w = stack[n - 1].w - stack[n].w - 1;
            uint8_t r1 = generate_recurse();
            --n;

            bool explored =
                room_is_explored(data.mapi, r0) ||
                room_is_explored(data.mapi, r1);

            // join with horizontal corridor
            if(r0 < MAP_ROOMS && r1 < MAP_ROOMS)
                join_rooms(r0, r1, explored);

            r = (u8rand() & 1) ? r0 : r1;
            break;
        }
    }

    return r;
}

void generate_dungeon(uint8_t mapi)
{
    rand_seed = game_seed;
    for(uint8_t i = 0; i < mapi; ++i)
        for(uint8_t j = 0; j < 217; ++j)
            u8rand();

    for(auto& t : tmap) t = 0xff;
    for(auto& t : tfog) t = 0;

    auto& data = *(bsp_data*)&buf;
    data.mapi = mapi;
    data.stack[0] = { 0, 0, MAP_W, MAP_H };
    num_rooms = 0;
    generate_recurse();

    // clear buf
    for(auto& b : buf) b = 0;
}
