#include "game.hpp"

static constexpr uint8_t ROOM_BIG_CHANCE = 64;

// chances are out of 256
// chance to place a door
static constexpr uint8_t DOOR_CHANCE = 196;
// chance for any door to be secret
static constexpr uint8_t DOOR_SECRET_CHANCE = 48;
// chance for isolated door (only door leading to a room) to be secret
static constexpr uint8_t DOOR_ISOLATED_SECRET_CHANCE = 0;

static constexpr uint8_t RANDOM_DOOR_SPACE = 4;

struct map_gen_info
{
    uint8_t monster_types[6];
    item guaranteed_item;
};

static map_gen_info const MAP_GEN_INFOS[NUM_MAPS] PROGMEM =
{
{ // level 1
    { entity::BAT, entity::SNAKE, entity::SNAKE, },
    { 0, 0, 0, item::FOOD }, // extra food
},
{ // level 2
    { entity::SNAKE, entity::SNAKE, entity::SNAKE, entity::SNAKE, entity::RATTLESNAKE, entity::RATTLESNAKE, },
    { 0, 0, 0, item::SCROLL, SCR_REMOVE_CURSE },
},
{ // level 3
    { entity::ZOMBIE, entity::ZOMBIE, entity::ZOMBIE, entity::GOBLIN, entity::GOBLIN, entity::PHANTOM },
    { 0, 0, 0, item::SCROLL, SCR_ENCHANT },
},
{ // level 4
    { entity::ZOMBIE, entity::GOBLIN, entity::GOBLIN, entity::PHANTOM, entity::ORC, },
    { 0, 0, 0, item::SCROLL, SCR_REMOVE_CURSE },
},
{ // level 5
    { entity::PHANTOM, entity::PHANTOM, entity::PHANTOM, entity::PHANTOM, entity::PHANTOM, entity::PHANTOM, },
    { 0, 0, 0, item::POTION, POT_STRENGTH }
},
{ // level 6
    { entity::GOBLIN, entity::GOBLIN, entity::GOBLIN, entity::ORC, entity::HOBGOBLIN, },
    { 0, 0, 0, item::SCROLL, SCR_ENCHANT },
},
{ // level 7
    { entity::ORC, entity::ORC, entity::HOBGOBLIN, entity::TARANTULA, entity::MIMIC },
    { 0, 0, 0, item::POTION, POT_POISON },
},
{ // level 8
    { entity::ORC, entity::HOBGOBLIN, entity::TARANTULA, entity::TARANTULA, entity::TARANTULA, entity::MIMIC },
    { 0, 0, 0, item::SCROLL, SCR_REMOVE_CURSE },
},
{ // level 9
    { entity::HOBGOBLIN, entity::HOBGOBLIN, entity::HOBGOBLIN, entity::TARANTULA, entity::MIMIC, entity::INCUBUS },
    { 0, 0, 0, item::SCROLL, SCR_ENCHANT },
},
{ // level 10
    { entity::MIMIC, entity::MIMIC, entity::MIMIC, entity::MIMIC, entity::TARANTULA, entity::HOBGOBLIN, },
    { 0, 0, 0, item::POTION, POT_STRENGTH }
},
{ // level 11
    { entity::TARANTULA, entity::HOBGOBLIN, entity::MIMIC, entity::INCUBUS, entity::INCUBUS, entity::TROLL, },
    { 0, 0, 0, item::POTION, POT_CONFUSION },
},
{ // level 12
    { entity::HOBGOBLIN, entity::MIMIC, entity::INCUBUS, entity::TROLL, entity::TROLL, entity::GRIFFIN, },
    { 0, 0, 0, item::SCROLL, SCR_ENCHANT },
},
{ // level 13
    { entity::MIMIC, entity::INCUBUS, entity::TROLL, entity::GRIFFIN, entity::GRIFFIN, entity::DRAGON, },
    { 0, 0, 0, item::POTION, POT_CONFUSION },
},
{ // level 14
    { entity::INCUBUS, entity::TROLL, entity::GRIFFIN, entity::DRAGON, entity::DRAGON, entity::DRAGON, },
    { 0, 0, 0, item::POTION, POT_INVIS },
},
{ // level 15
    { entity::INCUBUS, entity::ANGEL, entity::ANGEL, entity::DRAGON, entity::DRAGON, entity::DRAGON, },
    { 0, 0, 0, item::SCROLL, SCR_ENCHANT },
},
{ // level 16
    { entity::ANGEL, entity::ANGEL, entity::ANGEL, entity::ANGEL, entity::ANGEL, entity::ANGEL, },
    { 0, 0, 0, item::POTION, POT_CONFUSION },
},
};

struct room_info
{
    uint8_t w;
    uint8_t h;
    uint8_t mask[8];
};

struct big_room_info
{
    uint8_t w;
    uint8_t h;
    uint8_t mask[32];
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

static constexpr big_room_info const BIG_ROOM_TYPES[] PROGMEM =
{
{
    15, 15, {
    cbm(0x11111000), cbm(0x00111111),
    cbm(0x11100000), cbm(0x00001111),
    cbm(0x11000000), cbm(0x00000111),
    cbm(0x10000000), cbm(0x00000011),
    cbm(0x10000000), cbm(0x00000011),
    cbm(0x00000111), cbm(0x11000001),
    cbm(0x00000111), cbm(0x11000001),
    cbm(0x00000110), cbm(0x11000001),
    cbm(0x00000110), cbm(0x11000001),
    cbm(0x00000110), cbm(0x11000001),
    cbm(0x10000000), cbm(0x00000011),
    cbm(0x10000000), cbm(0x00000011),
    cbm(0x11000000), cbm(0x00000111),
    cbm(0x11100000), cbm(0x00001111),
    cbm(0x11111000), cbm(0x00111111),
    cbm(0x11111111), cbm(0x11111111), }
},
{
    16, 16, {
    cbm(0x11111000), cbm(0x00011111),
    cbm(0x11100000), cbm(0x00000111),
    cbm(0x11000000), cbm(0x00000011),
    cbm(0x10000000), cbm(0x00000001),
    cbm(0x10001100), cbm(0x00110001),
    cbm(0x00001100), cbm(0x00110000),
    cbm(0x00000000), cbm(0x00000000),
    cbm(0x00000000), cbm(0x00000000),
    cbm(0x00000000), cbm(0x00000000),
    cbm(0x00000000), cbm(0x00000000),
    cbm(0x00001100), cbm(0x00110000),
    cbm(0x10001100), cbm(0x00110001),
    cbm(0x10000000), cbm(0x00000001),
    cbm(0x11000000), cbm(0x00000011),
    cbm(0x11100000), cbm(0x00000111),
    cbm(0x11111000), cbm(0x00011111), }
},
{
    13, 13, {
    cbm(0x11111101), cbm(0x11111111),
    cbm(0x11111000), cbm(0x11111111),
    cbm(0x11110000), cbm(0x01111111),
    cbm(0x11100000), cbm(0x00111111),
    cbm(0x11000000), cbm(0x00011111),
    cbm(0x10000000), cbm(0x00001111),
    cbm(0x00000000), cbm(0x00000111),
    cbm(0x10000000), cbm(0x00001111),
    cbm(0x11000000), cbm(0x00011111),
    cbm(0x11100000), cbm(0x00111111),
    cbm(0x11110000), cbm(0x01111111),
    cbm(0x11111000), cbm(0x11111111),
    cbm(0x11111101), cbm(0x11111111),
    cbm(0x11111111), cbm(0x11111111),
    cbm(0x11111111), cbm(0x11111111),
    cbm(0x11111111), cbm(0x11111111), }
},
{
    12, 12, {
    cbm(0x11000000), cbm(0x00111111),
    cbm(0x10000000), cbm(0x00011111),
    cbm(0x00000000), cbm(0x00001111),
    cbm(0x00000000), cbm(0x00001111),
    cbm(0x00000000), cbm(0x00001111),
    cbm(0x00000000), cbm(0x00001111),
    cbm(0x00000000), cbm(0x00001111),
    cbm(0x00000000), cbm(0x00001111),
    cbm(0x00000000), cbm(0x00001111),
    cbm(0x00000000), cbm(0x00001111),
    cbm(0x10000000), cbm(0x00011111),
    cbm(0x11000000), cbm(0x00111111),
    cbm(0x11111111), cbm(0x11111111),
    cbm(0x11111111), cbm(0x11111111),
    cbm(0x11111111), cbm(0x11111111),
    cbm(0x11111111), cbm(0x11111111), }
},
};
static constexpr uint8_t NUM_BIG_ROOM_TYPES =
sizeof(BIG_ROOM_TYPES) / sizeof(BIG_ROOM_TYPES[0]);

static constexpr room_info const ROOM_TYPES[] PROGMEM =
{
{
    4, 4, {
    cbm(0x00001111),
    cbm(0x00001111),
    cbm(0x00001111),
    cbm(0x00001111),
    cbm(0x11111111),
    cbm(0x11111111),
    cbm(0x11111111),
    cbm(0x11111111), },
},
{
    5, 5, {
    cbm(0x00000111),
    cbm(0x00000111),
    cbm(0x00000111),
    cbm(0x00000111),
    cbm(0x00000111),
    cbm(0x11111111),
    cbm(0x11111111),
    cbm(0x11111111), },
},
{
    6, 6, {
    cbm(0x00000011),
    cbm(0x00000011),
    cbm(0x00000011),
    cbm(0x00000011),
    cbm(0x00000011),
    cbm(0x00000011),
    cbm(0x11111111),
    cbm(0x11111111), },
},
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
    6, 6, {
    cbm(0x10000111),
    cbm(0x00000011),
    cbm(0x00000011),
    cbm(0x00000011),
    cbm(0x00000011),
    cbm(0x10000111),
    cbm(0x11111111),
    cbm(0x11111111), },
},
{
    7, 7, {
    cbm(0x00000001),
    cbm(0x00000001),
    cbm(0x00000001),
    cbm(0x00000001),
    cbm(0x00000001),
    cbm(0x00000001),
    cbm(0x00000001),
    cbm(0x11111111), },
},
{
    7, 7, {
    cbm(0x11101111),
    cbm(0x11000111),
    cbm(0x10000011),
    cbm(0x00000001),
    cbm(0x10000011),
    cbm(0x11000111),
    cbm(0x11101111),
    cbm(0x11111111), },
},
{
    7, 7, {
    cbm(0x11000111),
    cbm(0x11000111),
    cbm(0x00000001),
    cbm(0x00000001),
    cbm(0x00000001),
    cbm(0x11000111),
    cbm(0x11000111),
    cbm(0x11111111), },
},
{
    7, 7, {
    cbm(0x11000111),
    cbm(0x11000111),
    cbm(0x00010001),
    cbm(0x00111001),
    cbm(0x00010001),
    cbm(0x11000111),
    cbm(0x11000111),
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

static uint8_t random_room_type()
{
    if(u8rand() < ROOM_BIG_CHANCE)
        return 0x80 + u8rand(NUM_BIG_ROOM_TYPES);
    return u8rand(NUM_ROOM_TYPES);
}

uint8_t room::w() const
{
    if(type & 0x80) return pgm_read_byte(&BIG_ROOM_TYPES[type & 0x7f].w);
    return pgm_read_byte(&ROOM_TYPES[type].w);
}

uint8_t room::h() const
{
    if(type & 0x80) return pgm_read_byte(&BIG_ROOM_TYPES[type & 0x7f].h);
    return pgm_read_byte(&ROOM_TYPES[type].h);
}

bool room::solid(uint8_t rx, uint8_t ry) const
{
    if(type & 0x80)
    {
        uint8_t i = ry * 2 + rx / 8;
        uint8_t m = pgm_read_byte(&BIG_ROOM_TYPES[type & 0x7f].mask[i]);
        return (m & (1 << (rx & 7))) != 0;
    }
    uint8_t m = pgm_read_byte(&ROOM_TYPES[type].mask[ry]);
    return (m & (1 << rx)) != 0;
}

bool room::inside(uint8_t tx, uint8_t ty) const
{
    uint8_t rx = uint8_t(tx - x);
    uint8_t ry = uint8_t(ty - y);
    if(!(rx < w() && ry < h()))
        return false;
    return !solid(rx, ry);
}

bool room::inside_bb(uint8_t tx, uint8_t ty) const
{
    uint8_t rx = uint8_t(tx - x);
    uint8_t ry = uint8_t(ty - y);
    return (rx < w() && ry < h());
}

static void dig_tile(uint8_t x, uint8_t y)
{
    uint16_t const i = y / 8 * MAP_W + x;
    uint8_t const m = 1 << (y % 8);
    tmap[i] &= ~m;
}

static void fill_tile(uint8_t x, uint8_t y)
{
    uint16_t const i = y / 8 * MAP_W + x;
    uint8_t const m = 1 << (y % 8);
    tmap[i] |= m;
}

void dig_nonsecret_door_tiles()
{
    for(int i = 0; i < num_doors; ++i)
    {
        auto const& d = doors[i];
        if(!d.secret)
            dig_tile(d.x, d.y);
    }
}

void update_doors()
{
    for(int i = 0; i < num_doors; ++i)
    {
        auto const& d = doors[i];
        if(d.open)
            dig_tile(d.x, d.y);
        else
            fill_tile(d.x, d.y);
    }
}

static bool dig_room(
    uint8_t type, uint8_t x, uint8_t y)
{
    auto& r = rooms[num_rooms];
    r.x = x;
    r.y = y;
    r.type = type;

    if(x >= MAP_W) return false;
    if(y >= MAP_H) return false;
    uint8_t bx = x + r.w();
    uint8_t by = y + r.h();
    if(bx > MAP_W) return false;
    if(by > MAP_H) return false;

    // ensure room can fit here
    for(uint8_t ty = y; ty < by; ++ty)
        for(uint8_t tx = x; tx < bx; ++tx)
        {
            if(!r.inside(tx, ty))
                continue;
            for(uint8_t i = 0; i < 8; ++i)
            {
                if(!tile_is_solid(
                    tx + pgm_read_byte(&DDIRX[i]),
                    ty + pgm_read_byte(&DDIRY[i])))
                    return false;
            }
        }

    bool explored = maps[map_index].got_rooms.test(num_rooms);
    for(uint8_t iy = y; iy < by; ++iy)
        for(uint8_t ix = x; ix < bx; ++ix)
            if(r.inside(ix, iy))
            {
                dig_tile(ix, iy);
                if(explored)
                    set_tile_explored(ix, iy);
            }
    ++num_rooms;
    return true;
}

static void dig_initial_room()
{
    uint8_t t = random_room_type();
    if(map_index == NUM_MAPS - 1)
        t = 0x80;
    dig_room(t, MAP_W / 2 - 3, MAP_H / 2 - 3);
}

static void add_door(uint8_t x, uint8_t y)
{
    dig_tile(x, y);
    if(u8rand() >= DOOR_CHANCE) return;
    if(num_doors >= MAP_DOORS) return;
    if(get_door(x, y) != nullptr) return;
    auto& d = doors[num_doors];
    uint8_t t = u8rand();
    if(maps[map_index].got_doors.test(num_doors))
    {
        set_tile_explored(x, y);
        d.open = 1;
    }
    else if(t < DOOR_SECRET_CHANCE)
    {
        d.secret = 1;
        set_tile_explored(x, y);
    }
    d.x = x;
    d.y = y;
    ++num_doors;
}

static void random_room_edge(uint8_t d, uint8_t t, uint8_t& rx, uint8_t& ry)
{
    room r = { 0, 0, t };
    for(;;)
    {
        r.x = u8rand(r.w());
        r.y = u8rand(r.h());
        switch(d)
        {
        case 0: r.y = 0;         break;
        case 1: r.y = r.h() - 1; break;
        case 2: r.x = 0;         break;
        case 3: r.x = r.w() - 1; break;
        default: break;
        }
        if(!r.solid(r.x, r.y)) break;
    }
    rx = r.x, ry = r.y;
}

int8_t const DIRX[8] PROGMEM =
{
    0, 0, -1, 1, -1, 1, 0, 0
};

static bool try_generate_room()
{
    if(num_rooms >= MAP_ROOMS) return false;
    room pr = rooms[u8rand(num_rooms)];
    uint8_t d = u8rand() % 4; // direction to place new room
    uint8_t t = random_room_type(); // new room type
    uint8_t x0, y0; // current room edge (rel)
    uint8_t x1, y1; // new room edge (rel)
    uint8_t xd, yd; // door pos
    uint8_t xr, yr; // new room
    random_room_edge(d, pr.type, x0, y0);
    random_room_edge(d ^ 1, t, x1, y1);
    int8_t dx = (int8_t)pgm_read_byte(&DIRX[d]);
    int8_t dy = (int8_t)pgm_read_byte(&DIRY[d]);
    xd = pr.x + x0 + dx;
    yd = pr.y + y0 + dy;
    xr = xd - x1 + dx;
    yr = yd - y1 + dy;
    if(!dig_room(t, xr, yr))
        return false;
    add_door(xd, yd);
    return true;
}

static void try_add_random_door()
{
    auto r = rooms[u8rand(num_rooms)];
    uint8_t d = u8rand() % 4;
    uint8_t x, y;
    random_room_edge(d, r.type, x, y);
    x += r.x + (int8_t)pgm_read_byte(&DIRX[d]);
    y += r.y + (int8_t)pgm_read_byte(&DIRY[d]);
    if(!tile_is_solid(x, y)) return;
    for(uint8_t i = 0; i < num_rooms; ++i)
        if(rooms[i].inside_bb(x, y)) return;
    if(d < 2) // north/south
    {
        if(tile_is_solid(x, y - 1) || tile_is_solid(x, y + 1))
            return;
        for(uint8_t i = 0; i < RANDOM_DOOR_SPACE; ++i)
            if(!tile_is_solid(x - i, y) || !tile_is_solid(x + i, y))
                return;
    }
    else // west/east
    {
        if(tile_is_solid(x - 1, y) || tile_is_solid(x + 1, y))
            return;
        for(uint8_t i = 0; i < RANDOM_DOOR_SPACE; ++i)
            if(!tile_is_solid(x, y - i) || !tile_is_solid(x, y + i))
                return;
    }
    // verify not adjacent to another door
    for(uint8_t i = 0; i < 4; ++i)
    {
        if(get_door(x + pgm_read_byte(&DIRX[i]), y + pgm_read_byte(&DIRY[i])))
            return;
    }
    add_door(x, y);
}

bool occupied(uint8_t x, uint8_t y)
{
    if(tile_is_solid(x, y)) return true;
    if(x == xdn && y == ydn) return true;
    if(x == xup && y == yup) return true;
    for(auto const& e : ents)
        if(e.type != entity::NONE && e.x == x && e.y == y)
            return true; 
    for(auto const& i : items)
        if(i.it.type != item::NONE && i.x == x && i.y == y)
            return true;
    for(uint8_t i = 0; i < num_doors; ++i)
        if(doors[i].x == x && doors[i].y == y)
            return true;
    return false;
}

bool find_unoccupied(uint8_t& rx, uint8_t& ry)
{
    for(uint16_t tries = 0; tries < 1024; ++tries)
    {
        uint8_t x = u8rand(MAP_W);
        uint8_t y = u8rand(MAP_H);
        if(!occupied(x, y))
            return rx = x, ry = y, true;
    }
    return false;
}

void find_unoccupied_guaranteed(uint8_t& x, uint8_t& y)
{
    while(!find_unoccupied(x, y))
        (void)0;
}

coord find_unoccupied_guaranteed()
{
    coord c;
    find_unoccupied_guaranteed(c.x, c.y);
    return c;
}

static void generate_item(uint8_t i, item it)
{
    if(maps[map_index].got_items.test(i))
        return;
    auto& mit = items[i];
    find_unoccupied_guaranteed(mit.x, mit.y);
    if(maps[map_index].got_items.test(i))
        it.type = item::NONE;
    mit.it = it;
}

static void generate_random_item(uint8_t i)
{
    item it{};
    uint8_t type = u8rand(item::NUM_ITEM_TYPES);
    it.type = type;
    it.subtype = 0;
    bool cursed = (u8rand() < 32);
    uint8_t enchant = (u8rand() < 64);
    switch(type)
    {
    case item::ARROW:
        it.quant_or_level = u8rand() % 4 + 3;
        break;
    //case item::FOOD:
    //    it.cursed = 0;
    //    break;
    case item::POTION:
        it.subtype = u8rand(NUM_POT);
        break;
    case item::SCROLL:
        it.subtype = u8rand(NUM_SCR);
        break;
    case item::RING:
        it.cursed = cursed;
        it.quant_or_level = enchant;
        it.subtype = u8rand(NUM_RNG);
        break;
    case item::AMULET:
        it.subtype = u8rand(NUM_AMU);
        [[fallthrough]];
    case item::SWORD:
    case item::BOW:
    case item::HELM:
    case item::ARMOR:
    case item::BOOTS:
        it.cursed = cursed;
        it.quant_or_level = enchant;
        break;
    default:
        break;
    }
    generate_item(i, it);
}

void generate_dungeon()
{
    rand_seed = game_seed;
    for(uint8_t i = 0; i < map_index; ++i)
        for(uint8_t j = 0; j < 217; ++j)
            u8rand();

    for(auto& t : tmap) t = 0xff;
    memzero(&tfog, sizeof(tfog));
    memzero(&items, sizeof(items));
    memzero(&rooms, sizeof(rooms));
    memzero(&doors, sizeof(doors));
    num_rooms = 0;
    num_doors = 0;

    render(); // show "Loading..."

    dig_initial_room();
    for(uint16_t i = 0; i < 4096; ++i)
        try_generate_room();

    for(uint16_t i = 0; i < 1024; ++i)
        try_add_random_door();

    // set all solid tiles to explored (makes walls fill out better)
    for(uint16_t i = 0; i < tfog.size(); ++i)
        tfog[i] |= tmap[i];
    update_doors();

    if(map_index < NUM_MAPS - 1)
        find_unoccupied_guaranteed(xdn, ydn);
    else
        xdn = ydn = 255;
    find_unoccupied_guaranteed(xup, yup);

    if(map_index == NUM_MAPS - 1)
    {
        uint8_t x = rooms[0].x + 7;
        uint8_t y = rooms[0].y + 7;
        {
            auto& it = items[MAP_ITEMS - 1];
            it.it = { 0, 1, 0, item::AMULET, AMU_YENDOR };
            it.x = x;
            it.y = y;
        }
        {
            auto& e = ents[1];
            e.type = entity::DARKNESS;
            e.x = x;
            e.y = y + 1;
        }
        {
            uint8_t i = num_doors;
            if(i == MAP_DOORS) --i;
            else ++num_doors;
            auto& d = doors[i];
            d.secret = 0;
            d.x = x;
            d.y = y + 2;
        }
    }

    //
    // add monsters
    //

    if(map_index < 15)
    for(uint8_t i = 1; i < MAP_ENTITIES; ++i)
    {
        auto& e = ents[i];
        e = {};
        if(!find_unoccupied(e.x, e.y))
            continue;
        for(;;)
        {
            uint8_t j = u8rand(6);
            j = pgm_read_byte(&MAP_GEN_INFOS[map_index].monster_types[j]);
            if(j != 0)
            {
                e.type = j;
                if(j == entity::MIMIC)
                    e.health = u8rand(item::NUM_ITEM_TYPES - 1) + 1;
                if(maps[map_index].got_ents.test(i))
                    e.type = entity::NONE;
                entity_info info;
                entity_get_info(i, info);
                e.invis = info.invis;
                break;
            }
        }
    }
}

void generate_items()
{
    //
    // add items
    //

    {
        uint8_t i = 0;
        generate_item(i++, { item::POTION, POT_HEALING });
        generate_item(i++, { item::FOOD });
        {
            union { uint16_t a; item b; } u = {
                pgm_read_word(&MAP_GEN_INFOS[map_index].guaranteed_item)
            };
            if(u.b.type != item::NONE)
                generate_item(i++, u.b);
        }
        
        for(; i < MAP_ITEMS - 4; ++i)
            generate_random_item(i);
    }
}

void new_entity(uint8_t i, uint8_t type, uint8_t x, uint8_t y)
{
    auto& e = ents[i];
    e = {};
    e.type = type;
    e.health = entity_max_health(i);
    e.x = x;
    e.y = y;
}
