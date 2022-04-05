#include "game.hpp"

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

bool direction_menu(uint8_t& d, char const* s)
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
    uint8_t b = wait_btn();
    switch(b)
    {
    case BTN_UP   : return d = 0, true;
    case BTN_DOWN : return d = 1, true;
    case BTN_LEFT : return d = 2, true;
    case BTN_RIGHT: return d = 3, true;
    default: break;
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

static char const* const MEN_ITEMS[] PROGMEM =
{
    MEN_MAP,
    MEN_STATS,
    MEN_SETTINGS,
    MEN_SAVE,
};

static bool act_wait(action& a)
{
    a.type = action::WAIT;
    return true;
}

static bool act_use(action& a)
{
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
    return direction_menu(a.dir, ACT_CLOSE);
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

static void draw_map_offset(uint8_t ox)
{
    for(uint8_t y = 0; y < 64; y += 2)
        for(uint8_t x = 0; x < 64; x += 2)
        {
            set_pixel(x + ((y >> 1) & 1), y);
        }
    for(uint8_t y = 0; y < MAP_H; ++y)
    {
        bool prior = false;
        for(uint8_t tx = 0; tx < 32; ++tx)
        {
            uint8_t x = tx + ox;
            uint8_t px = tx * 2, py = y * 2;
            if(tile_is_solid_or_unknown(x, y))
            {
                uint8_t m =
                    (uint8_t)tile_is_solid_or_unknown(x - 1, y) +
                    (uint8_t)tile_is_solid_or_unknown(x + 1, y) +
                    (uint8_t)tile_is_solid_or_unknown(x, y - 1) +
                    (uint8_t)tile_is_solid_or_unknown(x, y + 1) +
                    (uint8_t)tile_is_solid_or_unknown(x - 1, y - 1) +
                    (uint8_t)tile_is_solid_or_unknown(x - 1, y + 1) +
                    (uint8_t)tile_is_solid_or_unknown(x + 1, y - 1) +
                    (uint8_t)tile_is_solid_or_unknown(x + 1, y + 1);
                if(tile_is_solid(x, y) && m < 8)
                {
                    clear_rect(px, px + 2, py, py + 2);
                    set_rect(px, px + 1, py, py + 1);
                    if(prior) clear_vline(px - 1, py, py + 2);
                    prior = false;
                }
                else
                    prior = true;
            }
            else
            {
                clear_pixel(px + (y & 1), py);
            }
        }
    }

    if(tile_is_explored(xdn, ydn))
    {
        uint8_t tx = (xdn - ox) * 2, ty = ydn * 2;
        set_hline(tx, tx + 1, ty + 1);
        set_pixel(tx, ty);
    }
    if(tile_is_explored(xup, yup))
    {
        uint8_t tx = (xup - ox) * 2, ty = yup * 2;
        set_hline(tx, tx + 1, ty + 1);
        set_pixel(tx + 1, ty);
    }
}

static void men_map()
{
    draw_map_offset(0);
    paint_left();
    draw_map_offset(32);
    paint_right();
    while(wait_btn() != BTN_B)
        ;
}

static void men_info() {}
static void men_settings() {}
static void men_save() {}

using men_method = void(*)(void);
static men_method const MEN_METHODS[] PROGMEM =
{
    men_map,
    men_info,
    men_settings,
    men_save,
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
                n = 3;
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