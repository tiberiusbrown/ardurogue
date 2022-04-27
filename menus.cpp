#include "game.hpp"

#include "git.hpp"

static char const HS_MSG_ESCAPED[] PROGMEM = "Escaped with the amulet";
static char const HS_MSG_RETURNED[] PROGMEM = "Returned to the surface";
static char const HS_MSG_ABANDONED[] PROGMEM = "Abandoned the game";
static char const HS_MSG_ENTITY[] PROGMEM = "Death by @M";
static char const HS_MSG_SUICIDE[] PROGMEM = "Committed suicide... somehow";
//static char const HS_MSG_TRAP[] PROGMEM = "";
static char const HS_MSG_STARVED[] PROGMEM = "Starved to death";
static char const* const HS_MSGS[6] PROGMEM =
{
    HS_MSG_ESCAPED,
    HS_MSG_RETURNED,
    HS_MSG_ABANDONED,
    HS_MSG_ENTITY,
    HS_MSG_SUICIDE,
    //HS_MSG_TRAP,
    HS_MSG_STARVED,
};

static void show_high_scores_offset(uint8_t x, uint8_t ti)
{
    draw_text(x + 46, 0, PSTR("High Scores"));
    set_hline(0, 63, 6);
    for(uint8_t i = 0, y = 11; i < NUM_HIGH_SCORES; ++i, y += 7)
    {
        auto h = high_scores[i];
        if(h.type == HS_NONE) continue;
        draw_textf(x, y, pgmptr(&HS_MSGS[h.type - 1]), h.data);
        char tn[6];
        uint8_t n = tsprintf(tn, PSTR("@u"), h.score);
        draw_text_nonprog(x + 127 - n * 4, y, tn);
        if(ti == i)
            inv_rect(0, 63, y - 1, y + 5);
    }
    if(ti < NUM_HIGH_SCORES)
    {
        draw_text(x + 37, 48, PSTR("Save high score?"));
        draw_yesno(x + 39, 56);
    }
    else
    {
#if !ENABLE_DEBUG_MENU
        static char const VERSION[] PROGMEM =
            GIT_COUNT "  " GIT_COMMIT_DAY "  " GIT_DESCRIBE "  V" SAVE_VERSION_STR;
        draw_text(x, 59, VERSION);
#endif
    }
}

void show_high_scores(uint8_t i)
{
    show_high_scores_offset(1, i);
    paint_left();
    show_high_scores_offset(uint8_t(-63), i);
    paint_right();
    for(;;)
    {
        uint8_t b = wait_btn();
        if(i < NUM_HIGH_SCORES && b == BTN_A)
            save();
        if(b & (BTN_A | BTN_B))
            break;
    }
}

static void draw_inventory(
    uint8_t x,
    char const* prompt,
    uint8_t* its,
    uint8_t offi,
    uint8_t seli)
{
    draw_text(x, 0, prompt);
    set_hline(0, 63, 7);
    for(uint8_t i = 0, y = 10; i < 9; ++i, y += 6)
    {
        uint8_t j = i + offi;
        if(j >= INV_ITEMS + 7) continue;
        uint8_t it = its[j];
        if(it == 255) continue;
        if(it < INV_ITEMS)
        {
            if(item_is_equipped(it))
                draw_char(x + 1, y, '#');
            draw_textf(x + 7, y, PSTR("@i"), inv[it]);
            if(j == seli)
                inv_rect(0, 63, y - 1, y + 5); // TODO: width
        }
        else if(x == 0)
            draw_text(0, y, pgmptr(&INV_CATEGORIES[it - INV_ITEMS]));
    }
}

static uint8_t inventory_menu_ex(char const* prompt, uint8_t const* cats)
{
    uint8_t its[INV_ITEMS + 8];
    uint8_t* itp = &its[0];

    for(uint8_t j = 0; j < 7; ++j)
    {
        *itp++ = INV_ITEMS + j;
        uint8_t* tp = itp;
        for(uint8_t i = 0; i < INV_ITEMS; ++i)
        {
            if(pgm_read_byte(&cats[inv[i].type]) == j)
                *itp++ = i;
        }
        if(itp == tp) --itp;
    }

    uint8_t n = (uint8_t)(ptrdiff_t)(itp - &its[0]);
    while(itp < &its[0] + sizeof(its))
        *itp++ = 255;

    uint8_t seli = 1;
    uint8_t offi = 0;
    for(;;)
    {
        draw_inventory(0           , prompt, its, offi, seli);
        paint_left();
        draw_inventory(uint8_t(-64), prompt, its, offi, seli);
        paint_right();

        uint8_t b = wait_btn();
        if(b == BTN_B) break;
        if(b == BTN_A) return its[seli];
        if(b == BTN_DOWN)
        {
            if(seli < n - 1)
            {
                if(its[++seli] >= INV_ITEMS)
                    ++seli;
            }
            else seli = 1;
        }
        if(b == BTN_UP)
        {
            if(seli > 1)
            {
                if(its[--seli] >= INV_ITEMS)
                    --seli;
            }
            else seli = n - 1;
        }

        while(seli > offi + 8) offi += 3;
        while(seli < offi) offi -= 3;
    }
    return 255;
}

static uint8_t const DEFAULT_ITEM_CATS[item::NUM_ITEM_TYPES] PROGMEM =
{
    255, 6, 3, 4, 6, 2, 0, 0, 5, 5, 1, 1, 1,
};

uint8_t inventory_menu(char const* prompt)
{
    return inventory_menu_ex(prompt, DEFAULT_ITEM_CATS);
}

void draw_yesno(uint8_t x, uint8_t y)
{
    static uint8_t const CIRCLE[7] PROGMEM =
    {
        0x1c, 0x3e, 0x7f, 0x7f, 0x7f, 0x3e, 0x1c,
    };
    set_img_prog(CIRCLE, 7, x, y);
    set_img_prog(CIRCLE, 7, x + 32, y);
    draw_char(x + 2, y + 1, 'A');
    draw_char(x + 34, y + 1, 'B');
    draw_text(x + 11, y + 1, PSTR("Yes"));
    draw_text(x + 43, y + 1, PSTR("No"));
}

bool yesno_menu(char const* s)
{
    item undefined_item;
#if defined(__GNUC__) && defined(__AVR_ARCH__)
    // trick gcc into thinking "undefined_item" is defined
    // this prevents inserting additional instructions to initialize regs
    asm("" : "=r"(undefined_item));
#else
    // who cares about non avr
    undefined_item = {};
#endif
    return yesno_menui(s, undefined_item);
}

bool yesno_menui(char const* fmt, item it)
{
    draw_info_without_status();
    uint8_t y = STATUS_START_Y;
    {
        char t[128];
        tsprintf(t, fmt, it);
        uint8_t len = tstrlen(t);
        uint8_t x = 1;
        uint8_t n = 0;
        while(n < len) n = advance_white(t, n);
        n = 0;
        while(n < len)
        {
            uint8_t w = text_width(&t[n], false);
            if(x + w > 64)
            {
                x = 1;
                y += 6;
            }
            draw_text_nonprog(x, y, &t[n]);
            x += w + SPACE_WIDTH + 1;
            while(t[n++] != '\0')
                ;
        }
    }

    draw_yesno(7, y + 8);
    paint_right();
    for(;;)
    {
        uint8_t b = wait_btn();
        if(b & (BTN_A | BTN_B))
        {
            render();
            return b == BTN_A;
        }
    }
    return false;
}

static bool direction_menu_ex(uint8_t& d, char const* s, bool cancel)
{
    draw_info_without_status();
    draw_text(5, STATUS_START_Y, s);
    draw_text(5, STATUS_START_Y + 6, PSTR("Which direction?"));
    constexpr uint8_t X = 24;
    constexpr uint8_t Y = STATUS_START_Y + 20;
    for(uint8_t i = 0; i < 4; ++i)
    {
        set_vline(X - i, Y + i, Y + 7 - i);
        set_vline(X + 15 + i, Y + i, Y + 7 - i);
        set_hline(X + 4 + i, X + 11 - i, Y - 3 - i);
        set_hline(X + 4 + i, X + 11 - i, Y + 10 + i);
    }
    paint_right();
    for(;;)
    {
        uint8_t b = wait_btn();
        switch(b)
        {
        case BTN_UP: return d = 0, true;
        case BTN_DOWN: return d = 1, true;
        case BTN_LEFT: return d = 2, true;
        case BTN_RIGHT: return d = 3, true;
        default: break;
        }
        if(cancel) break;
    }
    return false;
}

bool direction_menu_nocancel(uint8_t& d, char const* s)
{
    return direction_menu_ex(d, s, false);
}

bool direction_menu(uint8_t& d, char const* s)
{
    return direction_menu_ex(d, s, true);
}

bool direction_menu(uint8_t& d)
{
    return direction_menu_ex(d, PSTR(""), true);
}

static char const ACT_WAIT[] PROGMEM = "Wait / Search";
static char const ACT_USE[] PROGMEM = "Use / [Un]equip";
static char const ACT_SHOOT[] PROGMEM = "Shoot";
static char const ACT_DROP[] PROGMEM = "Drop Item";
static char const ACT_THROW[] PROGMEM = "Throw Potion";
static char const ACT_CLOSE[] PROGMEM = "Close Door";

static char const* const ACT_ITEMS[] PROGMEM =
{
    ACT_WAIT,
    ACT_USE,
    ACT_SHOOT,
    ACT_DROP,
    ACT_THROW,
    ACT_CLOSE,
};

#if ENABLE_MINIMAP
static char const MEN_MAP[] PROGMEM = "View Minimap";
#endif
#if ENABLE_DUNGEON_SCROLL
static char const MEN_SCROLL[] PROGMEM = "Scroll Dungeon";
#endif
static char const MEN_INV[] PROGMEM = "Inventory";
static char const MEN_STATS[] PROGMEM = "Player Info";
static char const MEN_HS[] PROGMEM = "High Scores";
static char const MEN_SETTINGS[] PROGMEM = "Settings";
static char const MEN_ABANDON[] PROGMEM = "Abandon Game";
static char const MEN_SAVE[] PROGMEM = "Save Progress";
#if ENABLE_DEBUG_MENU
static char const MEN_DEBUG[] PROGMEM = "Debug Info";
#endif

static char const* const MEN_ITEMS[] PROGMEM =
{
#if ENABLE_MINIMAP
    MEN_MAP,
#endif
#if ENABLE_DUNGEON_SCROLL
    MEN_SCROLL,
#endif
    MEN_INV,
    MEN_STATS,
    MEN_HS,
    MEN_SETTINGS,
    MEN_ABANDON,
    MEN_SAVE,
#if ENABLE_DEBUG_MENU
    MEN_DEBUG,
#endif
};

static bool act_wait(action& a)
{
    a.type = action::WAIT;
    return true;
}

static bool act_use(action& a)
{
    uint8_t i = inventory_menu(PSTR("Use which item?"));
    if(i < INV_ITEMS)
    {
        a.type = action::USE;
        a.data = i;
        return true;
    }
    return false;
}

static bool act_shoot(action& a)
{
    uint8_t i = pinfo.equipped[SLOT_WEAPON];
    if(!(i < INV_ITEMS && inv[i].is_type(item::BOW)))
    {
        status_simple(PSTR("You're not holding a bow."));
        return false;
    }
    bool have_arrow = false;
    for(uint8_t j = 0; j < INV_ITEMS; ++j)
        have_arrow |= inv[j].is_type(item::ARROW);
    if(!have_arrow)
    {
        status_simple(PSTR("You have no arrows."));
        return false;
    }
    if(!direction_menu(a.data))
        return false;
    a.type = action::SHOOT;
    return true;
}

static bool act_drop(action& a)
{
    uint8_t i = inventory_menu(PSTR("Drop which item?"));
    if(i < INV_ITEMS)
    {
        a.type = action::DROP;
        a.data = i;
        return true;
    }
    return false;
}

static uint8_t const POTION_ITEM_CATS[item::NUM_ITEM_TYPES] PROGMEM =
{
    255, 255, 3, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
};

static bool act_throw(action& a)
{
    uint8_t i = inventory_menu_ex(PSTR("Throw what?"), POTION_ITEM_CATS);
    if(i >= INV_ITEMS) return false;
    a.type = action::THROW;
    a.data = i;
    render();
    return true;
}

static bool act_close(action& a)
{
    a.type = action::CLOSE;
    return direction_menu(a.data, ACT_CLOSE);
}

using act_method = bool(*)(action&);
static act_method const ACT_METHODS[] PROGMEM =
{
    act_wait,
    act_use,
    act_shoot,
    act_drop,
    act_throw,
    act_close,
};

#if ENABLE_MINIMAP
static void men_map()
{
    draw_map_offset(0);
    paint_left();
    draw_map_offset(32);
    paint_right();
    while(wait_btn() != BTN_B)
        (void)0;
}
#endif

static void men_inv()
{
    (void)inventory_menu(PSTR("Inventory"));
}

static void draw_info_bonus(uint8_t x, uint8_t y, uint8_t val, uint8_t base)
{
    char const* s = PSTR("@u (@d)");
    if(val == base) s = PSTR("@u");
    draw_textf(x, y, s, val, val - base);
}
static void draw_player_info(uint8_t x)
{
    draw_text(x + 30, 0, PSTR("Player Information"));
    set_hline(0, 63, 6);

    static char const T0[] PROGMEM = "Level:";
    static char const T1[] PROGMEM = "Max Health:";
    static char const T2[] PROGMEM = "Strength:";
    static char const T3[] PROGMEM = "Dexterity:";
    static char const T4[] PROGMEM = "Attack:";
    static char const T5[] PROGMEM = "Defense:";
    static char const T6[] PROGMEM = "Speed:";
    static char const* const TS[] PROGMEM =
    {
        T0, T1, T2, T3, T4, T5, T6,
    };

    {
        uint8_t ta[14];
        uint8_t* const tb = &ta[7];

        ta[0] = tb[0] = plevel + 1;
        ta[1] = entity_max_health(0), tb[1] = pstats.max_health;
        ta[2] = entity_strength(0),   tb[2] = pstats.strength;
        ta[3] = entity_dexterity(0),  tb[3] = pstats.dexterity;
        ta[4] = entity_attack(0),     tb[4] = ta[2];
        ta[5] = entity_defense(0),    tb[5] = pstats.defense;
        ta[6] = entity_speed(0),      tb[6] = pstats.speed;

        for(uint8_t i = 0, y = 9; i < 7; ++i, y += 6)
        {
            draw_text(x, y, pgmptr(&TS[i]));
            draw_info_bonus(x + 40, y, ta[i], tb[i]);
        }
    }

    {
        char const* ts[6];
        char const** tp = &ts[0];
        uint8_t y = 9;
        draw_text(x + 70, y, PSTR("Effects:"));
        if(ents[0].confused)  *tp++ = PSTR("confused");
        if(ents[0].paralyzed) *tp++ = PSTR("paralyzed");
        if(ents[0].weakened)  *tp++ = PSTR("weakened");
        if(ents[0].slowed)    *tp++ = PSTR("slowed");
        if(ents[0].invis)     *tp++ = PSTR("invisible");
        if(hunger == 255)     *tp++ = PSTR("starving");
        x += 75;
        while(tp > &ts[0])
            draw_text(x, y += 6, *(--tp));
        if(y == 9) draw_text(x, 15, PSTR("none"));
    }
}
static void men_info()
{
    draw_player_info(0);
    paint_left();
    draw_player_info(uint8_t(-64));
    paint_right();
    while(wait_btn() != BTN_B)
        (void)0;
}

static void men_settings()
{
    uint8_t ws = wall_style;
    for(;;)
    {
        wall_style &= 3;
        draw_dungeon_at_player();
        paint_left();
        draw_text(20, 1, MEN_SETTINGS);
        draw_textf(3, 9, PSTR("Wall Style: { @u }"), wall_style);
        paint_right();
        uint8_t b = wait_btn();
        if(b == BTN_LEFT ) --wall_style;
        if(b == BTN_RIGHT) ++wall_style;
        if(b & (BTN_B | BTN_A))
        {
            if(b == BTN_B) wall_style = ws;
            return;
        }
    }
}

static void men_abandon()
{
    if(yesno_menu(PSTR("Are you sure you want to end this game?")))
    {
        status_simple(PSTR("You have abandoned this game."));
        hs.type = HS_ABANDONED;
        ents[0].type = entity::NONE;
    }
}

static void men_save()
{
    save();
}

#if ENABLE_DEBUG_MENU
static void men_debug_offset(uint8_t x)
{
    static char const GIT_STUFF[] PROGMEM =
        GIT_BRANCH " " GIT_DESCRIBE " " GIT_COMMIT_DAY;
    draw_text(x, 0, GIT_STUFF);
    draw_textf(x, 6, PSTR("Rand: @x@x/@x@x"),
        game_seed >> 8, game_seed & 0xff,
        rand_seed >> 8, rand_seed & 0xff);
    draw_textf(x, 12, PSTR("Stack: @u"), unused_stack());
}
static void men_debug()
{
    men_debug_offset(0);
    paint_left();
    men_debug_offset(uint8_t(-64));
    paint_right();
    while(wait_btn() != BTN_B)
        (void)0;
}
#endif

#if ENABLE_DUNGEON_SCROLL
static void men_scroll()
{
    uint8_t x = ents[0].x, y = ents[0].y;
    for(;;)
    {
        draw_dungeon(x, y);
        paint_left();
        uint8_t d;
        if(!direction_menu(d, PSTR("Scroll Dungeon")))
            break;
        auto c = dircoord(d);
        x += (int8_t)c.x * 4;
        y += (int8_t)c.y * 4;
        if(x & 0x80) x = 0;
        if(y & 0x80) y = 0;
        if(x >= MAP_W) x = MAP_W - 1;
        if(y >= MAP_H) y = MAP_H - 1;
    }
}
#endif

static void men_high_scores()
{
    show_high_scores(255);
}

using men_method = void(*)(void);
static men_method const MEN_METHODS[] PROGMEM =
{
#if ENABLE_MINIMAP
    men_map,
#endif
#if ENABLE_DUNGEON_SCROLL
    men_scroll,
#endif
    men_inv,
    men_info,
    men_high_scores,
    men_settings,
    men_abandon,
    men_save,
#if ENABLE_DEBUG_MENU
    men_debug,
#endif
};

bool repeat_action(action& a)
{
    return pgmptr(&ACT_METHODS[prev_action])(a);
}

bool action_menu(action& a)
{
    uint8_t i = prev_action;
    uint8_t n = 5;
    char const* const* text = ACT_ITEMS;
    bool act = true;
    constexpr uint8_t START_Y = 9;

    for(;;)
    {
        draw_text(10, 0, act ? PSTR("{ Actions }") : PSTR("{ Game Menu }"));
        set_hline(1, 63, 6);
        for(uint8_t j = 0, y = START_Y; j <= n; ++j, y += 7)
            draw_text(8, y, (char const*)pgm_read_ptr(&text[j]));
        inv_rect(1, 63, i * 7 + START_Y - 1, i * 7 + START_Y + 5);
        paint_right();
        uint8_t b = wait_btn();
        switch(b)
        {
        case BTN_LEFT:
        case BTN_RIGHT:
            if(act)
            {
                text = MEN_ITEMS;
                n = 5 + ENABLE_MINIMAP + ENABLE_DUNGEON_SCROLL + ENABLE_DEBUG_MENU;
                i = 0;
            }
            else
            {
                text = ACT_ITEMS;
                n = 5;
                i = prev_action;
            }
            act = !act;
            break;
        case BTN_UP:
            i = (i == 0 ? n : i - 1);
            break;
        case BTN_DOWN:
            i = (i == n ? 0 : i + 1);
            break;
        case BTN_A:
            if(act && pgmptr(&ACT_METHODS[i])(a))
            {
                prev_action = i;
                return true;
            }
            else if(!act)
                pgmptr(&MEN_METHODS[i])();
            return false;
        case BTN_B:
            return false;
        default:
            break;
        }
    }

    return false;
}
