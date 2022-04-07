#include "game.hpp"

#include "git.hpp"

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
        if(j >= INV_ITEMS + 6) continue;
        uint8_t it = its[j];
        if(it == 255) continue;
        if(it < INV_ITEMS)
        {
            draw_textf(x + 4, y, PSTR("@i"), inv[it]);
            if(j == seli)
                inv_rect(0, 63, y - 1, y + 5); // TODO: width
        }
        else if(x == 0)
            draw_text(0, y, pgmptr(&INV_CATEGORIES[it - INV_ITEMS]));

    }
}

uint8_t inventory_menu(char const* prompt)
{
    uint8_t its[INV_ITEMS + 7];
    uint8_t* itp = &its[0];
    uint8_t* tp;

    // TODO: reduce code size in this method?

    *itp++ = INV_ITEMS + 0; // weapons
    tp = itp;
    for(uint8_t i = 0; i < INV_ITEMS; ++i)
        if(inv[i].type == item::SWORD || inv[i].type == item::BOW || inv[i].type == item::ARROW)
            *itp++ = i;
    if(itp == tp) --itp;

    *itp++ = INV_ITEMS + 1; // armor
    tp = itp;
    for(uint8_t i = 0; i < INV_ITEMS; ++i)
        if(inv[i].type == item::ARMOR || inv[i].type == item::HELM || inv[i].type == item::BOOTS)
            *itp++ = i;
    if(itp == tp) --itp;

    *itp++ = INV_ITEMS + 2; // jewelry
    tp = itp;
    for(uint8_t i = 0; i < INV_ITEMS; ++i)
        if(inv[i].type == item::RING || inv[i].type == item::AMULET)
            *itp++ = i;
    if(itp == tp) --itp;

    *itp++ = INV_ITEMS + 3; // potion
    tp = itp;
    for(uint8_t i = 0; i < INV_ITEMS; ++i)
        if(inv[i].type == item::POTION)
            *itp++ = i;
    if(itp == tp) --itp;

    *itp++ = INV_ITEMS + 4; // scroll
    tp = itp;
    for(uint8_t i = 0; i < INV_ITEMS; ++i)
        if(inv[i].type == item::SCROLL)
            *itp++ = i;
    if(itp == tp) --itp;

    *itp++ = INV_ITEMS + 5; // misc
    tp = itp;
    for(uint8_t i = 0; i < INV_ITEMS; ++i)
        if(inv[i].type == item::FOOD)
            *itp++ = i;
    if(itp == tp) --itp;

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

        while(seli > offi + 9) offi += 3;
        while(seli < offi) offi -= 3;
    }
    return 255;
}

bool yesno_menu(char const* fmt, ...)
{
    draw_info_without_status();
    {
        char t[64];
        va_list ap;
        va_start(ap, fmt);
        tvsprintf(t, fmt, ap);
        va_end(ap);
        uint8_t len = tstrlen(t);
        uint8_t x = 1, y = STATUS_START_Y;
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
                draw_text(x, y, &t[n], false);
            }
            else
            {
                draw_text(x, y, &t[n], false);
            }
            x += w + SPACE_WIDTH + 1;
            while(t[n++] != '\0')
                ;
        }
    }
    constexpr uint8_t Y = 55;
    constexpr int8_t OX = -4;
    for(uint8_t i = 0; i < 2; ++i)
    {
        uint8_t x = i * 32;
        set_vline(x + 10 + OX, Y + 2, Y + 6);
        set_vline(x + 18 + OX, Y + 2, Y + 6);
        set_hline(x + 12 + OX, x + 16 + OX, Y);
        set_hline(x + 12 + OX, x + 16 + OX, Y + 8);
        set_pixel(x + 11 + OX, Y + 1);
        set_pixel(x + 17 + OX, Y + 1);
        set_pixel(x + 11 + OX, Y + 7);
        set_pixel(x + 17 + OX, Y + 7);
        draw_char(x + 13 + OX, Y + 2, 'A' + i);
    }
    draw_text(22 + OX, Y + 2, PSTR("Yes"));
    draw_text(54 + OX, Y + 2, PSTR("No"));
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

bool direction_menu(uint8_t& d, char const* s, bool cancel)
{
    draw_info_without_status();
    if(s)
    {
        uint8_t w = text_width(s);
        draw_text(32 - w / 2, STATUS_START_Y, s);
    }
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

static char const ACT_WAIT[] PROGMEM = "Wait / Search";
static char const ACT_USE[] PROGMEM = "Use / [Un]equip";
static char const ACT_SHOOT[] PROGMEM = "Shoot";
static char const ACT_DROP[] PROGMEM = "Drop Item";
static char const ACT_THROW[] PROGMEM = "Throw Item";
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

static char const MEN_MAP[] PROGMEM = "Dungeon Map";
static char const MEN_STATS[] PROGMEM = "Player Info";
static char const MEN_SETTINGS[] PROGMEM = "Settings";
static char const MEN_SAVE[] PROGMEM = "Save Progress";
static char const MEN_DEBUG[] PROGMEM = "Debug Info";

static char const* const MEN_ITEMS[] PROGMEM =
{
    MEN_MAP,
    MEN_STATS,
    MEN_SETTINGS,
    MEN_SAVE,
    MEN_DEBUG,
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
    return false;
}

static bool act_drop(action& a)
{
    return false;
}

static bool act_throw(action& a)
{
    return false;
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

static void men_map()
{
    draw_map_offset(0);
    paint_left();
    draw_map_offset(32);
    paint_right();
    while(wait_btn() != BTN_B)
        (void)0;
}

static void men_info() {}
static void men_settings() {}
static void men_save() {}

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

using men_method = void(*)(void);
static men_method const MEN_METHODS[] PROGMEM =
{
    men_map,
    men_info,
    men_settings,
    men_save,
    men_debug,
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

    for(;;)
    {
        draw_info_without_status();
        for(uint8_t j = 0, y = STATUS_START_Y; j <= n; ++j, y += 6)
            draw_text(8, y, (char const*)pgm_read_ptr(&text[j]));
        inv_rect(1, 63, i * 6 + STATUS_START_Y - 1, i * 6 + STATUS_START_Y + 5);
        paint_right();
        uint8_t b = wait_btn();
        switch(b)
        {
        case BTN_LEFT:
        case BTN_RIGHT:
            if(act)
            {
                text = MEN_ITEMS;
                n = 4;
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
