#pragma once

#include <stdint.h>

#ifdef ARDUINO
#include <avr_stl.h>
#include <Arduino.h>
#else
#define PSTR(str_) str_
#define PROGMEM
inline uint8_t pgm_read_byte(void const* p) { return *(uint8_t*)p; }
inline uint16_t pgm_read_word(void const* p) { return *(uint16_t*)p; }
#endif
#include <array>
#include <bitset>

// platform functionality
uint8_t wait_btn(); // wait for button press
void seed();        // initialize seed0 and seed1
void paint_left();  // draw to left half screen
void paint_right(); // draw to right half screen

static constexpr uint8_t BTN_UP    = 0x80;
static constexpr uint8_t BTN_DOWN  = 0x10;
static constexpr uint8_t BTN_LEFT  = 0x20;
static constexpr uint8_t BTN_RIGHT = 0x40;
static constexpr uint8_t BTN_A     = 0x08;
static constexpr uint8_t BTN_B     = 0x04;

// game logic
void game_setup();
void game_loop();

static constexpr uint8_t MAP_W = 48;
static constexpr uint8_t MAP_H = 48;
static constexpr uint8_t MAP_ITEMS = 32;
static constexpr uint8_t MAP_ENTITIES = 32;
static constexpr uint8_t MAP_ROOMS = 32;
static constexpr uint8_t INV_ITEMS = 32;
static constexpr uint8_t NUM_MAPS = 16;

struct entity
{
    enum
    {
        NONE,
        PLAYER,
    };
    uint8_t type : 5;
    uint8_t confused : 1;
    uint8_t paralyzed : 1;
    uint8_t weakened : 1;
    uint8_t health;
    uint8_t x, y;
};

struct item
{
    uint8_t type : 4;
    uint8_t subtype : 4;
    uint8_t identified : 1;
    uint8_t cursed : 1;
    uint8_t enchant_level : 4;
};

struct map_item
{
    item it;
    uint8_t x, y;
};

struct map_info
{
    std::bitset<MAP_ITEMS> got_items;   // picked-up items
    std::bitset<MAP_ENTITIES> got_ents; // defeated monsters
    std::bitset<MAP_ROOMS> got_rooms;   // explored rooms
};

struct room
{
    uint8_t x, y, w, h;
    bool inside(uint8_t tx, uint8_t ty) const
    {
        return uint8_t(tx - x) < w && uint8_t(ty - y) < h;
    }
};

struct saved_data
{
    std::array<map_info, NUM_MAPS> maps;
    std::array<item, INV_ITEMS> inv;
    std::array<entity, MAP_ENTITIES> ents;
    std::array<item, MAP_ITEMS> items;
    std::array<room, MAP_ROOMS> rooms;
    uint8_t num_rooms;
    uint16_t game_seed;
};

static constexpr uint8_t NUM_WALL_STYLES = 4;
struct options
{
    uint8_t wall_style;
};

struct globals
{
    std::array<uint8_t, 64 * 64 / 8> buf;
    std::array<uint8_t, size_t(MAP_W) * MAP_H / 8> tmap; // 1: wall/door
    std::array<uint8_t, size_t(MAP_W) * MAP_H / 8> tfog; // 1: explored
    saved_data saved;
    options opt;
    uint16_t rand_seed;
};

extern globals globals_;

// breakout items from struct
static constexpr auto& buf = globals_.buf;
static constexpr auto& tmap = globals_.tmap;
static constexpr auto& tfog = globals_.tfog;
static constexpr auto& ents = globals_.saved.ents;
static constexpr auto& items = globals_.saved.items;
static constexpr auto& maps = globals_.saved.maps;
static constexpr auto& rooms = globals_.saved.rooms;
static constexpr auto& num_rooms = globals_.saved.num_rooms;
static constexpr auto& inv = globals_.saved.inv;
static constexpr auto& game_seed = globals_.saved.game_seed;
static constexpr auto& rand_seed = globals_.rand_seed;
static constexpr auto& opt = globals_.opt;

static constexpr uint16_t SAVE_FILE_BYTES = sizeof(saved_data);
static constexpr uint16_t GAME_DATA_BYTES = sizeof(globals);

template<class T>
inline T const& min(T const& a, T const& b)
{
    return a < b ? a : b;
}
template<class T>
inline T const& max(T const& a, T const& b)
{
    return a < b ? b : a;
}

inline uint8_t u8abs(uint8_t x)
{
    return (x & 0x80) ? -x : x;
}

inline uint8_t light_radius() { return 5; }
inline uint8_t light_radius2()
{
    uint8_t r = light_radius();
    return r * r + r / 2;
}

// game.cpp
uint8_t u8rand();
uint8_t u8rand(uint8_t m);
bool tile_is_solid(uint8_t x, uint8_t y);
bool tile_is_explored(uint8_t x, uint8_t y);
bool tile_is_unknown(uint8_t x, uint8_t y);
bool tile_is_solid_or_unknown(uint8_t x, uint8_t y);
void render();

// draw.cpp
void draw_info();
void draw_dungeon(uint8_t mx, uint8_t my);
void draw_text(uint8_t x, uint8_t y, const char* p, bool prog = true);

// generate.cpp
void generate_dungeon(uint8_t mapi);

// light.cpp
bool path_clear(
    uint8_t x0, uint8_t y0,
    uint8_t x1, uint8_t y1);
void update_light();
