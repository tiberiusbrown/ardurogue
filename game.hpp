#pragma once

#include <stdint.h>

#ifdef ARDUINO
#include <Arduino.h>
#else
#define PSTR(str_) str_
#define PROGMEM
inline uint8_t pgm_read_byte(void const* p) { return *(uint8_t*)p; }
inline uint16_t pgm_read_word(void const* p) { return *(uint16_t*)p; }
#endif

template<class T, size_t N>
struct array
{
    T d_[N];
    T* data() { return d_; }
    T const* data() const { return d_; }
    T& operator[](size_t i) { return d_[i]; }
    T const& operator[](size_t i) const { return d_[i]; }
    size_t size() { return N; }
    T* begin() { return d_; }
    T const* begin() const { return d_; }
    T* end() { return d_ + N; }
    T const* end() const { return d_ + N; }
};

template<size_t N> struct bitset
{
    static constexpr size_t ND = (N + 7) / 8;
    array<uint8_t, ND> d_;
    bool test(size_t i) const
    {
        return (d_[i / 8] >> (i % 8)) & 1;
    }
    void set(size_t i)
    {
        d_[i / 8] |= (1 << (i % 8));
    }
    void clear(size_t i)
    {
        d_[i / 8] &= ~(1 << (i % 8));
    }
};

template<class T> void swap(T& a, T& b)
{
    T c = a;
    a = b;
    b = c;
}

static void pgm_memcpy(void* dst, void const* src, uint8_t n)
{
    uint8_t* d = (uint8_t*)dst;
    uint8_t const* s = (uint8_t const*)src;
    for(uint8_t i = 0; i < n; ++i)
        d[i] = pgm_read_byte(&s[i]);
}

static void memzero(void* dst, uint8_t n)
{
    uint8_t* d = (uint8_t*)dst;
    for(uint8_t i = 0; i < n; ++i)
        d[i] = 0;
}

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
static constexpr uint8_t MAP_DOORS = 32;
static constexpr uint8_t INV_ITEMS = 32;
static constexpr uint8_t NUM_MAPS = 16;

struct coord { uint8_t x, y; };

struct entity_info
{
    uint8_t strength;
    uint8_t dexterity;
    uint8_t speed;
};

struct monster_info
{
    uint8_t mean    : 1; // whether it attacks on sight
    uint8_t regens  : 1; // regeneratess health
    uint8_t invis   : 1; // invisible
    uint8_t nomove  : 1; // stays put
    uint8_t poison  : 1; // chance to drain strength on hit
    uint8_t vampire : 1; // chance to drain max hp on hit
    uint8_t confuse : 1; // chance to confuse on hit

    entity_info i;
};

extern monster_info const MONSTER_INFO[32] PROGMEM;

struct entity
{
    enum
    {
        NONE,
        PLAYER,
        BAT,
        SNAKE,
        RATTLESNAKE,
        ZOMBIE,
        GOBLIN,
        ORC,
        HOBGOBLIN,
        TROLL,
        GRIFFIN,
        DRAGON,
    };
    uint8_t type      : 5;
    uint8_t confused  : 1;
    uint8_t paralyzed : 1;
    uint8_t weakened  : 1;
    uint8_t health;
    uint8_t x, y;
};

// potion types
enum
{
    POT_HEALING,
};

// scroll types
enum
{
    SCR_TELEPORT,
};

struct item
{
    enum
    {
        NONE,
        FOOD,   // quantity
        POTION, // quantity
        SCROLL, // quantity
        ARROW,  // quantity
        BOW,    // level
        SWORD,  // level
        RING,   // level
        AMULET, // level
        ARMOR,  // level
        HELM,   // level
        BOOTS,  // level
    };
    uint8_t type           : 4;
    uint8_t subtype        : 4;
    uint8_t identified     : 1;
    uint8_t cursed         : 1;
    uint8_t quant_or_level : 6;
};

struct map_item
{
    item it;
    uint8_t x, y;
};

struct map_info
{
    bitset<MAP_ITEMS>    got_items; // picked-up items
    bitset<MAP_ENTITIES> got_ents;  // defeated monsters
    bitset<MAP_ROOMS>    got_rooms; // explored rooms
    bitset<MAP_DOORS>    got_doors; // opened doors
};

struct room
{
    uint8_t x, y;
    uint8_t type; // bit 7 set if big
    uint8_t w() const;
    uint8_t h() const;
    bool solid(uint8_t rx, uint8_t ry) const;
    bool inside(uint8_t tx, uint8_t ty) const;
    bool inside_bb(uint8_t tx, uint8_t ty) const;
};

struct door
{
    uint8_t x : 7;
    uint8_t open : 1;
    uint8_t y : 7;
    uint8_t secret : 1; // must be zero if open
};

struct saved_data
{
    uint16_t                    game_seed;
    array<map_info, NUM_MAPS>   maps;
    array<item, INV_ITEMS>      inv;
    array<entity, MAP_ENTITIES> ents;
    array<map_item, MAP_ITEMS>  items;
    array<room, MAP_ROOMS>      rooms;
    array<door, MAP_DOORS>      doors;
    uint8_t                     num_ents;  // only used for generation
    uint8_t                     num_items; // only used for generation
    uint8_t                     num_rooms;
    uint8_t                     num_doors;
    uint8_t                     map_index;
    entity_info                 pstats;
};

static constexpr uint8_t NUM_WALL_STYLES = 4;
struct options
{
    uint8_t wall_style;
};

struct globals
{
    uint16_t rand_seed;
    array<uint8_t, 64 * 64 / 8> buf;
    array<uint8_t, size_t(MAP_W) * MAP_H / 8> tmap; // 1: wall/door
    array<uint8_t, size_t(MAP_W) * MAP_H / 8> tfog; // 1: explored
    uint8_t xdn, ydn; // coords of down stairs
    uint8_t xup, yup; // coords of up stairs
    saved_data saved;
    options opt;
};

extern globals globals_;

// breakout items from struct
inline constexpr auto& buf = globals_.buf;
inline constexpr auto& tmap = globals_.tmap;
inline constexpr auto& tfog = globals_.tfog;
inline constexpr auto& xdn = globals_.xdn;
inline constexpr auto& ydn = globals_.ydn;
inline constexpr auto& xup = globals_.xup;
inline constexpr auto& yup = globals_.yup;
inline constexpr auto& num_ents  = globals_.saved.num_ents;
inline constexpr auto& num_items = globals_.saved.num_items;
inline constexpr auto& num_rooms = globals_.saved.num_rooms;
inline constexpr auto& num_doors = globals_.saved.num_doors;
inline constexpr auto& ents = globals_.saved.ents;
inline constexpr auto& items = globals_.saved.items;
inline constexpr auto& maps = globals_.saved.maps;
inline constexpr auto& rooms = globals_.saved.rooms;
inline constexpr auto& doors = globals_.saved.doors;
inline constexpr auto& map_index = globals_.saved.map_index;
inline constexpr auto& inv = globals_.saved.inv;
inline constexpr auto& game_seed = globals_.saved.game_seed;
inline constexpr auto& rand_seed = globals_.rand_seed;
inline constexpr auto& opt = globals_.opt;

template<class T>
inline T const& tmin(T const& a, T const& b)
{
    return a < b ? a : b;
}
template<class T>
inline T const& tmax(T const& a, T const& b)
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
door* get_door(uint8_t x, uint8_t y);
uint8_t index_of_door(door const& d);
void render();

// draw.cpp
void draw_info();
void draw_dungeon(uint8_t mx, uint8_t my);
void draw_text(uint8_t x, uint8_t y, const char* p, bool prog = true);

// generate.cpp
void dig_nonsecret_door_tiles();
void update_doors();   // set tile to solid for closed doors
void generate_dungeon();
bool occupied(uint8_t x, uint8_t y); // door/stairs/item/entitity
extern int8_t const DIRX[4] PROGMEM;
extern int8_t const DIRY[4] PROGMEM;

// light.cpp
bool player_can_see(uint8_t x, uint8_t y);
bool path_clear(
    uint8_t x0, uint8_t y0,
    uint8_t x1, uint8_t y1);
void set_tile_explored(uint8_t x, uint8_t y);
void update_light();

static constexpr uint16_t SAVE_FILE_BYTES = sizeof(saved_data);
static constexpr uint16_t GAME_DATA_BYTES = sizeof(globals);
