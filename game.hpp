#pragma once

#include <stdint.h>

#ifdef ARDUINO
#include <avr_stl.h>
#else
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

static constexpr uint8_t MAP_W = 128;
static constexpr uint8_t MAP_H = 64;
static constexpr uint8_t MAP_ITEMS = 48;
static constexpr uint8_t MAP_ENTITIES = 32;
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

struct saved_data
{
    std::bitset<size_t(NUM_MAPS) * MAP_ITEMS> got_items;
    std::bitset<size_t(NUM_MAPS) * MAP_ENTITIES> got_ents;
    std::array<item, INV_ITEMS> inv;
    uint16_t seed0, seed1;
};

struct globals
{
    std::array<uint8_t, 64 * 64 / 8> buf;
    std::array<uint8_t, size_t(MAP_W) * MAP_H / 8> tmap;
    std::array<entity, MAP_ENTITIES> ents;
    std::array<item, MAP_ITEMS> items;
    saved_data saved;
};

extern globals globals_;

// breakout items from struct
static constexpr auto& buf = globals_.buf;
static constexpr auto& tmap = globals_.tmap;
static constexpr auto& ents = globals_.ents;
static constexpr auto& items = globals_.items;
static constexpr auto& got_items = globals_.saved.got_items;
static constexpr auto& got_ents = globals_.saved.got_ents;
static constexpr auto& inv = globals_.saved.inv;
static constexpr auto& seed0 = globals_.saved.seed0;
static constexpr auto& seed1 = globals_.saved.seed1;
