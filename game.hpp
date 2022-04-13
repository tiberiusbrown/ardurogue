#pragma once

#include <stdint.h>
#include <stdarg.h>

// platform functionality
uint8_t wait_btn(); // wait for button press
void seed(); // sets rand_seed
void paint_offset(uint8_t x, bool clear = true);
uint8_t read_persistent(uint16_t addr);
void update_persistent(uint16_t addr, uint8_t data);
void flush_persistent();

// game logic
void run();

#ifdef ARDUINO
#include <Arduino.h>

#if 1
// http://michael-buschbeck.github.io/arduino/2013/10/20/string-merging-pstr/
#undef PSTR
#define PSTR(str) \
  (__extension__({ \
    PGM_P ptr;  \
    asm volatile \
    ( \
      ".pushsection .progmem.data, \"SM\", @progbits, 1" "\n\t" \
      "PSTR%=: .string " #str                            "\n\t" \
      ".popsection"                                      "\n\t" \
      "ldi %A0, lo8(PSTR%=)"                             "\n\t" \
      "ldi %B0, hi8(PSTR%=)"                             "\n\t" \
      : "=d" (ptr) \
    ); \
    ptr; \
  }))
#endif

#else
#define PSTR(str_) str_
#define PROGMEM
inline uint8_t pgm_read_byte(void const* p) { return *(uint8_t*)p; }
inline uint16_t pgm_read_word(void const* p) { return *(uint16_t*)p; }
inline void const* pgm_read_ptr(void const* p) { return *(void const**)p; }
#endif

// useful when T is a pointer type, like function pointer or char const*
template<class T>
inline T pgmptr(T const* p) { return (T)pgm_read_ptr(p); }

template<class T, size_t N>
struct array
{
    T d_[N];
    T* data() { return d_; }
    constexpr T const* data() const { return d_; }
    T& operator[](size_t i) { verify(i); return d_[i]; }
    constexpr T const& operator[](size_t i) const
    {
#if defined(_MSC_VER) && !defined(NDEBUG)
        verify(i);
#endif
        return d_[i];
    }
    constexpr size_t size() const { return N; }
    T* begin() { return d_; }
    constexpr T const* begin() const { return d_; }
    T* end() { return d_ + N; }
    constexpr T const* end() const { return d_ + N; }
private:
    void verify(size_t i) const
    {
        (void)i;
#if defined(_MSC_VER) && !defined(NDEBUG)
        if(i >= N) __debugbreak();
#endif
    }
};

template<size_t N> struct bitset
{
    static constexpr size_t ND = (N + 7) / 8;
    array<uint8_t, ND> d_;
    constexpr bool test(size_t i) const { return (d_[i / 8] >> (i % 8)) & 1; }
    void set(size_t i) { d_[i / 8] |= (1 << (i % 8)); }
    void clear(size_t i) { d_[i / 8] &= ~(1 << (i % 8)); }
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

static void memzero(void* dst, uint16_t n)
{
    uint8_t* d = (uint8_t*)dst;
    for(uint16_t i = 0; i < n; ++i)
        d[i] = 0;
}

static constexpr uint8_t BTN_UP    = 0x80;
static constexpr uint8_t BTN_DOWN  = 0x10;
static constexpr uint8_t BTN_LEFT  = 0x20;
static constexpr uint8_t BTN_RIGHT = 0x40;
static constexpr uint8_t BTN_A     = 0x08;
static constexpr uint8_t BTN_B     = 0x04;

static constexpr uint8_t MAP_W = 64;
static constexpr uint8_t MAP_H = 32;
static constexpr uint8_t MAP_ITEMS = 32;
static constexpr uint8_t MAP_ENTITIES = 32;
static constexpr uint8_t MAP_ROOMS = 32;
static constexpr uint8_t MAP_DOORS = 32;
static constexpr uint8_t INV_ITEMS = 32;
static constexpr uint8_t NUM_MAPS = 16;

struct coord { uint8_t x, y; };

struct entity_info
{
    uint8_t mean     : 1; // whether it can attack
    uint8_t nomove   : 1; // stays put (mon only)
    uint8_t regens   : 1; // regenerates health
    uint8_t invis    : 1; // invisible
    uint8_t poison   : 1; // chance to drain strength (weaken) on hit
    uint8_t vampire  : 1; // chance to drain max hp on hit
    uint8_t confuse  : 1; // chance to confuse on hit
    uint8_t paralyze : 1; // chance to paralyze on hit
    uint8_t fbreath  : 1; // ranged attack: breathe fire
    uint8_t opener   : 1; // whether it can open doors

    uint8_t strength;    // higher is better
    uint8_t dexterity;   // higher is better
    uint8_t speed;       // lower is faster
    uint8_t defense;     // higher is better
    uint8_t max_health;

    uint8_t xp;          // xp granted for killing / current xp (player)
};

// item slots
enum
{
    SLOT_WEAPON,
    SLOT_ARMOR,
    SLOT_HELM,
    SLOT_BOOTS,
    SLOT_RING,
    SLOT_AMULET,
    NUM_SLOTS
};

// info specific to player
struct player_info
{
    uint8_t invis_rem;    // invis cycles remaining
    uint8_t confuse_rem;  // confuse cycles remaining
    uint8_t paralyze_rem; // paralysis cycles remaining
    uint8_t vamp_drain;   // amount of max health drained by vampire
    array<uint8_t, NUM_SLOTS> equipped;
};

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
        PHANTOM,
        ORC,
        TARANTULA,
        HOBGOBLIN,
        MIMIC,
        INCUBUS,
        TROLL,
        GRIFFIN,
        ANGEL,
        DRAGON,
        DARKNESS,
    };
    uint8_t type;
    uint8_t confused  : 1;
    uint8_t paralyzed : 1;
    uint8_t weakened  : 1;
    uint8_t aggro     : 1; // when a non-mean monster is attacked by player
    uint8_t invis     : 1;
    uint8_t health;
    uint8_t x, y;
};

struct action
{
    enum
    {
        WAIT,
        USE,
        SHOOT,
        DROP,
        THROW,
        CLOSE,
        MOVE,
    };
    uint8_t type;
    uint8_t data; // dir or item
};

// potion types
enum
{
    POT_HEALING,
    POT_CONFUSION,
    POT_POISON,
    POT_STRENGTH,
    POT_INVIS,
    NUM_POT,
};

// scroll types
enum
{
    SCR_IDENTIFY,
    SCR_ENCHANT,
    SCR_REMOVE_CURSE,
    SCR_TELEPORT,
    SCR_MAPPING,
    NUM_SCR,
};

// ring types
enum
{
    RNG_SEE_INVIS,
    RNG_STRENGTH,
    RNG_DEXTERITY,
    RNG_PROTECTION,
    NUM_RNG,
};

// amulet types
enum
{
    AMU_SPEED,
    AMU_CLARITY,      // prevents becoming confused
    AMU_CONSERVATION, // chance to prevent consuming potions/scrolls
    AMU_REGENERATION, // (chance to) heal each turn
    AMU_YENDOR,
    NUM_AMU = AMU_YENDOR,
};

static constexpr uint8_t NUM_IDENT =
    NUM_POT + NUM_SCR + NUM_RNG + NUM_AMU;

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
        NUM_ITEM_TYPES,
    };
    uint8_t type           : 4;
    uint8_t subtype        : 4;
    uint8_t quant_or_level : 6;
    uint8_t identified     : 1;
    uint8_t cursed         : 1;
    bool stackable() { return type <= ARROW; }
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

enum
{
    HS_NONE,
    HS_ESCAPED,   // escaped with the amulet
    HS_RETURNED,  // returned to the surface without the amulet
    HS_ABANDONED, // chose to end the game
    HS_ENTITY,    // killed by entity
    HS_TRAP,      // killed by trap
};
struct high_score
{
    uint16_t score;
    uint8_t type;
    uint8_t data;     // type of entity/trap
};

static constexpr uint8_t NUM_WALL_STYLES = 4;

struct saved_data
{
    uint16_t                    game_seed;
    high_score                  hs;
    array<map_info, NUM_MAPS>   maps;
    array<item, INV_ITEMS>      inv;
    array<entity, MAP_ENTITIES> ents;
    array<map_item, MAP_ITEMS>  items;
    array<room, MAP_ROOMS>      rooms;
    array<door, MAP_DOORS>      doors;
    uint8_t                     num_rooms;
    uint8_t                     num_doors;
    uint8_t                     map_index;
    entity_info                 pstats;
    player_info                 pinfo;
    bitset<NUM_IDENT>           identified;
    uint8_t                     prev_action;
    uint8_t                     plevel;

    // options, high score table, etc
    uint8_t                     wall_style;
    array<high_score, 4>        high_scores;
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
    char statusbuf[128];
    uint8_t statusn, statusx, statusy;
    array<uint8_t, NUM_POT> perm_pot;
    array<uint8_t, NUM_SCR> perm_scr;
    array<uint8_t, NUM_RNG> perm_rng;
    array<uint8_t, NUM_AMU> perm_amu;
    bool just_moved;
};

extern globals globals_;

// breakout stuff from struct for ease of coding
inline constexpr auto& buf = globals_.buf;
inline constexpr auto& tmap = globals_.tmap;
inline constexpr auto& tfog = globals_.tfog;
inline constexpr auto& xdn = globals_.xdn;
inline constexpr auto& ydn = globals_.ydn;
inline constexpr auto& xup = globals_.xup;
inline constexpr auto& yup = globals_.yup;
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
inline constexpr auto& pstats = globals_.saved.pstats;
inline constexpr auto& pinfo = globals_.saved.pinfo;
inline constexpr auto& rand_seed = globals_.rand_seed;
inline constexpr auto& wall_style = globals_.saved.wall_style;
inline constexpr auto& statusbuf = globals_.statusbuf;
inline constexpr auto& statusn = globals_.statusn;
inline constexpr auto& statusx = globals_.statusx;
inline constexpr auto& statusy = globals_.statusy;
inline constexpr auto& perm_pot = globals_.perm_pot;
inline constexpr auto& perm_scr = globals_.perm_scr;
inline constexpr auto& perm_rng = globals_.perm_rng;
inline constexpr auto& perm_amu = globals_.perm_amu;
inline constexpr auto& identified = globals_.saved.identified;
inline constexpr auto& prev_action = globals_.saved.prev_action;
inline constexpr auto& plevel = globals_.saved.plevel;
inline constexpr auto& just_moved = globals_.just_moved;
inline constexpr auto& hs = globals_.saved.hs;
inline constexpr auto& high_scores = globals_.saved.high_scores;

static constexpr uint8_t NUM_HIGH_SCORES = high_scores.size();

inline bool potion_is_identified(uint8_t subtype)
{
    return identified.test(subtype);
}
inline bool scroll_is_identified(uint8_t subtype)
{
    return identified.test(NUM_POT + subtype);
}
inline bool ring_is_identified(uint8_t subtype)
{
    return identified.test(NUM_POT + NUM_SCR + subtype);
}
inline bool amulet_is_identified(uint8_t subtype)
{
    return identified.test(NUM_POT + NUM_SCR + NUM_RNG + subtype);
}

inline void identify_potion(uint8_t subtype)
{
    identified.set(subtype);
}
inline void identify_scroll(uint8_t subtype)
{
    identified.set(NUM_POT + subtype);
}
inline void identify_ring(uint8_t subtype)
{
    identified.set(NUM_POT + NUM_SCR + subtype);
}
inline void identify_amulet(uint8_t subtype)
{
    identified.set(NUM_POT + NUM_SCR + NUM_RNG + subtype);
}

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
inline uint8_t u8max(uint8_t a, uint8_t b)
{
    return a < b ? b : a;
}

inline uint8_t light_radius() { return 5; }
inline uint8_t light_radius2()
{
    uint8_t r = light_radius();
    return r * r + r / 2;
}

inline bool player_is_dead() { return ents[0].type == entity::NONE; }

// strings.cpp
extern char const* const MONSTER_NAMES[] PROGMEM;
//extern char const* const ITEM_NAMES[] PROGMEM;
extern char const* const POT_NAMES[] PROGMEM;
extern char const* const SCR_NAMES[] PROGMEM;
extern char const* const RNG_NAMES[] PROGMEM;
extern char const* const AMU_NAMES[] PROGMEM;
extern char const* const UNID_POT_NAMES[] PROGMEM;
extern char const* const UNID_SCR_NAMES[] PROGMEM;
extern char const* const UNID_RNG_AMU_NAMES[] PROGMEM;
extern char const* const INV_CATEGORIES[] PROGMEM;

// game.cpp
void paint_left(bool clear = true);  // draw to left half screen
void paint_right(bool clear = true); // draw to right half screen
uint8_t u8rand();
uint8_t u8rand(uint8_t m);
bool tile_is_solid(uint8_t x, uint8_t y);
bool tile_is_explored(uint8_t x, uint8_t y);
bool tile_is_unknown(uint8_t x, uint8_t y);
bool tile_is_solid_or_unknown(uint8_t x, uint8_t y);
door* get_door(uint8_t x, uint8_t y); 
entity* get_entity(uint8_t x, uint8_t y);
uint8_t index_of_door(door const& d);
uint8_t index_of_entity(entity const& e);
uint8_t xp_for_level();
uint8_t armor_item_defense(item it);
uint8_t weapon_item_attack(item it);
void player_gain_xp(uint8_t xp);
bool player_pickup_item(uint8_t i); // map item
void player_remove_item(uint8_t i); // inv item
void render();
uint8_t process_high_score();

// font.cpp
uint8_t draw_char(uint8_t x, uint8_t y, char c); // returns width of char
void draw_text(uint8_t x, uint8_t y, const char* p, bool prog = true);
// fmt is PROGMEM
void draw_textf(uint8_t x, uint8_t y, const char* fmt, ...); // max 64
uint8_t char_width(char c);
uint8_t text_width(char const* s, bool prog = true);

// draw.cpp
static constexpr uint8_t SPACE_WIDTH = 1;
void set_img_prog(uint8_t const* p, uint8_t w, uint8_t x, uint8_t y);
void set_pixel(uint8_t x, uint8_t y);
void clear_pixel(uint8_t x, uint8_t y);
void inv_pixel(uint8_t x, uint8_t y);
void set_hline(uint8_t x0, uint8_t x1, uint8_t y);
void clear_hline(uint8_t x0, uint8_t x1, uint8_t y);
void inv_hline(uint8_t x0, uint8_t x1, uint8_t y);
void set_vline(uint8_t x, uint8_t y0, uint8_t y1);
void clear_vline(uint8_t x, uint8_t y0, uint8_t y1);
void set_rect(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1);
void clear_rect(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1);
void inv_rect(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1);
void set_box(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1);
void draw_box_pretty(uint8_t x0, uint8_t x1, uint8_t y0, uint8_t y1);
void draw_info();
void draw_info_without_status();
void draw_map_offset(uint8_t ox);
void draw_dungeon(uint8_t mx, uint8_t my);

// generate.cpp
void new_entity(uint8_t i, uint8_t type, uint8_t x, uint8_t y);
void dig_nonsecret_door_tiles();
void update_doors();   // set tile to solid for closed doors
void generate_dungeon();
void generate_items_and_ents();
bool occupied(uint8_t x, uint8_t y); // door/stairs/item/entitity
bool find_unoccupied(uint8_t& x, uint8_t& y); // returns true if found
void find_unoccupied_guaranteed(uint8_t& x, uint8_t& y);
coord find_unoccupied_guaranteed();
extern int8_t const DIRX[8] PROGMEM;
inline constexpr int8_t const* DIRY = &DIRX[4];

// light.cpp
bool player_can_see(uint8_t x, uint8_t y);
bool player_can_see_entity(uint8_t i); // takes into account invis
bool path_clear(
    uint8_t x0, uint8_t y0,
    uint8_t x1, uint8_t y1);
void set_tile_explored(uint8_t x, uint8_t y);
void update_light();

// sprintf.cpp: expects fmt to be PROGMEM
uint8_t tsprintf(char* b, char const* fmt, ...);
uint8_t tvsprintf(char* b, char const* fmt, va_list ap);
uint8_t tstrlen(char const* s); // s not progmem

// status.cpp: fmt is PROGMEM
static constexpr uint8_t NUM_STATUS_ROWS = 6;
static constexpr uint8_t STATUS_START_Y = 65 - 6 * NUM_STATUS_ROWS;
uint8_t advance_white(char* b, uint8_t i);
void draw_status();
void status(char const* fmt, ...);
void status_more();
void reset_status();

// entity.cpp
uint8_t entity_speed(uint8_t i);
uint8_t entity_max_health(uint8_t i);
uint8_t entity_strength(uint8_t i);
uint8_t entity_dexterity(uint8_t i);
uint8_t entity_attack(uint8_t i);
uint8_t entity_defense(uint8_t i);
bool test_attack_hit(uint8_t atti, uint8_t defi); // 0 for miss
uint8_t calculate_hit_damage(uint8_t atti, uint8_t defi); // 0 for block
void entity_restore_strength(uint8_t i);
void entity_heal(uint8_t i, uint8_t amount);
void entity_take_damage(uint8_t i, uint8_t dam);
void entity_take_damage_from_entity(uint8_t atti, uint8_t defi, uint8_t dam);
void teleport_entity(uint8_t i);
void confuse_entity(uint8_t i);
void poison_entity(uint8_t i);
void advance_entity(uint8_t i);
bool entity_perform_action(uint8_t i, action const& a);

// monsters.cpp
extern entity_info const MONSTER_INFO[] PROGMEM;
void entity_get_info(uint8_t i, entity_info& info);
void monster_ai(uint8_t i, action& a);

// menus.cpp
uint8_t inventory_menu(char const* s);
void show_high_scores(uint8_t i);
bool yesno_menu(char const* fmt, ...);
bool direction_menu(uint8_t& d, char const* s = nullptr, bool cancel = true);
bool repeat_action(action& a);
bool action_menu(action& a);

// stack.cpp
void stack_canary_init();
uint8_t unused_stack();

// use.cpp
uint8_t slot_of_item(uint8_t type);
bool item_is_equipped(uint8_t i);
bool use_item(uint8_t i);
void entity_apply_potion(uint8_t i, uint8_t subtype);

// save.cpp
void save();
void destroy_save();
bool save_valid();
void load();

static constexpr uint16_t SAVE_FILE_BYTES = sizeof(saved_data);
static constexpr uint16_t GAME_DATA_BYTES = sizeof(globals);
