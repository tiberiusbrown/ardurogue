#include "game.hpp"

uint8_t tsprintf(char* b, char const* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    uint8_t r = tvsprintf(b, fmt, ap);
    va_end(ap);
    return r;
}

// does not copy null char
static char* tstrcpy_prog(char* dst, char const* src)
{
    char c;
    while((c = pgm_read_byte(src++)) != '\0')
        *dst++ = c;
    return dst;
}

static char* quantify_item(
    char* dst, uint8_t n, char const* s, char const* s2 = nullptr)
{
    if(n == 0)
    {
        dst = tstrcpy_prog(dst, s);
        if(s2) *dst++ = ' ', dst = tstrcpy_prog(dst, s2);
        return dst;
    }
    ++n;
    uint8_t t = n % 10;
    n /= 10;
    if(n != 0) *dst++ = '0' + n;
    *dst++ = '0' + t;
    *dst++ = ' ';
    dst = tstrcpy_prog(dst, s);
    if(s2) *dst++ = ' ', dst = tstrcpy_prog(dst, s2);
    *dst++ = 's';
    return dst;
}

static char const ITEM_NAME_ARMOR[] PROGMEM = "armor";
static char const ITEM_NAME_HELM [] PROGMEM = "helm";
static char const ITEM_NAME_BOOTS[] PROGMEM = "boots";
static char const* const ITEM_NAME_ARMORS[] PROGMEM =
{
    ITEM_NAME_ARMOR,
    ITEM_NAME_HELM,
    ITEM_NAME_BOOTS,
};

static char* item_name(char* dst, item it)
{
    uint8_t n = it.quant_or_level;
    uint8_t st = it.subtype;
    char pm = it.cursed ? '-' : '+';
    if(it.cursed && it.identified)
        dst = tstrcpy_prog(dst, PSTR("cursed "));
    switch(it.type)
    {
    case item::FOOD:
        dst = quantify_item(dst, n, PSTR("food ration"));
        return dst;
    case item::ARROW:
        dst = quantify_item(dst, n, PSTR("arrow"));
        return dst;
    case item::BOW:
        dst = tstrcpy_prog(dst, PSTR("bow"));
        return dst;
    case item::SWORD:
        dst = tstrcpy_prog(dst, PSTR("sword"));
        return dst;
    case item::ARMOR:
    case item::HELM:
    case item::BOOTS:
        dst = tstrcpy_prog(dst, pgmptr(&ITEM_NAME_ARMORS[it.type - item::ARMOR]));
        if(it.identified)
            dst += tsprintf(dst, PSTR(" [@d def]"), armor_item_defense(it));
        return dst;
    case item::RING:
        if(ring_is_identified(st))
        {
            dst = tstrcpy_prog(dst, PSTR("ring of "));
            dst = tstrcpy_prog(dst, pgmptr(&RNG_NAMES[st]));
            if(it.identified)
            {
                char const* s = PSTR("");
                uint8_t q = it.quant_or_level + 1;
                switch(st)
                {
                case RNG_STRENGTH:
                    s = PSTR(" [@c@u str]");
                    break;
                case RNG_DEXTERITY:
                    s = PSTR(" [@c@u dex]");
                    break;
                case RNG_PROTECTION:
                    s = PSTR(" [@c@u]");
                    break;
                default:
                    break;
                }
                dst += tsprintf(dst, s, pm, q) - 1;
            }
            return dst;
        }
        else
        {
            dst = tstrcpy_prog(dst, pgmptr(&UNID_RNG_AMU_NAMES[perm_rng[st]]));
            dst = tstrcpy_prog(dst, PSTR(" ring"));
            return dst;
        }
    case item::AMULET:
        if(amulet_is_identified(st))
        {
            dst = tstrcpy_prog(dst, PSTR("amulet of "));
            dst = tstrcpy_prog(dst, pgmptr(&AMU_NAMES[st]));
            if(it.identified)
            {
                switch(st)
                {
                case AMU_SPEED:
                    dst += tsprintf(dst, PSTR(" [@c@u spd]"),
                        it.cursed ? '-' : '+',
                        it.quant_or_level / 2 + 1) - 1;
                    break;
                default:
                    break;
                }
            }
            return dst;
        }
        else
        {
            dst = tstrcpy_prog(dst, pgmptr(&UNID_RNG_AMU_NAMES[perm_amu[st]]));
            dst = tstrcpy_prog(dst, PSTR(" amulet"));
            return dst;
        }
    case item::POTION:
        if(potion_is_identified(st))
        {
            dst = quantify_item(dst, n, PSTR("potion"));
            dst = tstrcpy_prog(dst, PSTR(" of "));
            dst = tstrcpy_prog(dst, pgmptr(&POT_NAMES[st]));
            return dst;
        }
        else
        {
            return quantify_item(dst, n,
                pgmptr(&UNID_POT_NAMES[perm_pot[st]]),
                PSTR("potion"));
        }
    case item::SCROLL:
        if(scroll_is_identified(st))
        {
            dst = quantify_item(dst, n, PSTR("scroll"));
            dst = tstrcpy_prog(dst, PSTR(" of "));
            dst = tstrcpy_prog(dst, pgmptr(&SCR_NAMES[st]));
            return dst;
        }
        else
        {
            return quantify_item(dst, n,
                pgmptr(&UNID_SCR_NAMES[perm_scr[st]]),
                PSTR("scroll"));
        }

    default:
        break;
    }
    return dst;
}

static char const HEX_CHARS[] PROGMEM = "0123456789ABCDEF";

uint8_t tvsprintf(char* b, char const* fmt, va_list ap)
{
    char c, ct;
    uint8_t u;
    uint8_t dec[2];
    char const* s;
    char* b_orig = b;
    for(;;)
    {
        c = (char)pgm_read_byte(fmt++);
        if(c != '@')
        {
            *b++ = c;
            if(c == '\0') goto end;
            continue;
        }
        c = (char)pgm_read_byte(fmt++);
        switch(c)
        {
        case 'c': // char
            *b++ = (char)va_arg(ap, int);
            break;
        case 'x': // hex byte
            u = (uint8_t)va_arg(ap, int);
            *b++ = pgm_read_byte(&HEX_CHARS[u >> 4]);
            *b++ = pgm_read_byte(&HEX_CHARS[u & 15]);
            break;
        case 's': // ram string
            s = va_arg(ap, char const*);
            while((c = *s++) != '\0')
                *b++ = c;
            break;
        case 'p': // progmem string
            s = va_arg(ap, char const*);
            b = tstrcpy_prog(b, s);
            break;
        case 'S': // subject
        case 'O': // object
        case 'T': // subject possessive
        case 'P': // object possessive
            dec[0] = (uint8_t)va_arg(ap, int); // index
            u = ents[dec[0]].type;
            if(dec[0] == 0) // player
            {
                *b++ = (c >= 'S' ? 'Y' : 'y');
                *b++ = 'o';
                *b++ = 'u';
                if(!(c & 1)) *b++ = 'r'; // 'T' or 'P'
            }
            else
            {
                if(player_can_see_entity(dec[0]))
                {
                    *b++ = (c >= 'S' ? 'T' : 't');
                    *b++ = 'h';
                    *b++ = 'e';
                    *b++ = ' ';
                    b = tstrcpy_prog(b, pgmptr(&MONSTER_NAMES[u]));
                }
                else
                {
                    *b++ = (c >= 'S' ? 'S' : 's');
                    b = tstrcpy_prog(b, PSTR("omething"));
                }
                if(!(c & 1)) // 'T' or 'P'
                {
                    *b++ = '\'';
                    *b++ = 's';
                }
            }
            break;
        case 'V': // verb
        case 'v': // verb whose plural needs +es
            u = (uint8_t)va_arg(ap, int); // index
            u = ents[u].type;
            s = va_arg(ap, char const*);
            b = tstrcpy_prog(b, s);
            if(u != entity::PLAYER)
            {
                if(c == 'v') *b++ = 'e';
                *b++ = 's';
            }
            break;
        case 'A': // is/are
            // TODO: combine this with S?
            u = (uint8_t)va_arg(ap, int); // index
            u = ents[u].type;
            b = tstrcpy_prog(b, u == entity::PLAYER ? PSTR("are") : PSTR("is"));
            break;
        case 'i': // item
        {
            item it = va_arg(ap, item);
            b = item_name(b, it);
            break;
        }
        case 'd':
        case 'u': // uint8_t
            u = (uint8_t)va_arg(ap, int);
            if(c == 'd')
            {
                *b++ = (u & 0x80 ? '-' : '+');
                u &= 0x7f;
            }
            dec[0] = u % 10;
            u /= 10;
            if(u != 0)
            {
                dec[1] = u % 10;
                u /= 10;
                if(u != 0)
                    *b++ = '0' + u;
                *b++ = '0' + dec[1];
            }
            *b++ = '0' + dec[0];
            break;
        default:
            break;
        }
    }
end:
    return (uint8_t)(ptrdiff_t)(b - b_orig);
}

uint8_t tstrlen(char const* s)
{
    uint8_t r = 0;
    while(*s++ != '\0') ++r;
    return r;
}
