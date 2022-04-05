#include "game.hpp"

void tsprintf(char* b, char const* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    tvsprintf(b, fmt, ap);
    va_end(ap);
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
    if(n == 1)
    {
        dst = tstrcpy_prog(dst, s);
        if(s2) dst = tstrcpy_prog(dst, s2);
        return dst;
    }
    uint8_t t = n % 10;
    n /= 10;
    if(n != 0) *dst++ = '0' + n;
    *dst++ = '0' + t;
    *dst++ = ' ';
    dst = tstrcpy_prog(dst, s);
    if(s2) dst = tstrcpy_prog(dst, s2);
    *dst++ = 's';
    return dst;
}

static char* item_name(char* dst, item it)
{
    uint8_t n = it.quant_or_level;
    uint8_t st = it.subtype;
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
    case item::BOOTS:
        dst = tstrcpy_prog(dst, PSTR("boots"));
        return dst;
    case item::HELM:
        dst = tstrcpy_prog(dst, PSTR("helm"));
        return dst;
    case item::ARMOR:
        dst = tstrcpy_prog(dst, PSTR("armor"));
        return dst;
    case item::RING:
        if(ring_is_identified(st))
        {
            dst = tstrcpy_prog(dst, PSTR("ring of "));
            dst = tstrcpy_prog(dst, pgmptr(&RNG_NAMES[st]));
            return dst;
        }
        else
        {
            dst = tstrcpy_prog(dst, pgmptr(&UNID_RNG_AMU_NAMES[perm_rng[st]]));
            dst = tstrcpy_prog(dst, PSTR("ring"));
            return dst;
        }
    case item::AMULET:
        if(amulet_is_identified(st))
        {
            dst = tstrcpy_prog(dst, PSTR("amulet of "));
            dst = tstrcpy_prog(dst, pgmptr(&AMU_NAMES[st]));
            return dst;
        }
        else
        {
            dst = tstrcpy_prog(dst, pgmptr(&UNID_RNG_AMU_NAMES[perm_amu[st]]));
            dst = tstrcpy_prog(dst, PSTR("amulet"));
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
                pgmptr(&UNID_SCR_NAMES[perm_scr[it.subtype]]),
                PSTR("scroll"));
        }

    default:
        break;
    }
    return dst;
}

void tvsprintf(char* b, char const* fmt, va_list ap)
{
    char c, ct;
    uint8_t u;
    uint8_t dec[2];
    char const* s;
    for(;;)
    {
        c = (char)pgm_read_byte(fmt++);
        if(c != '@')
        {
            *b++ = c;
            if(c == '\0') return;
            continue;
        }
        c = (char)pgm_read_byte(fmt++);
        switch(c)
        {
        case 'c': // char
            *b++ = va_arg(ap, char);
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
            u = (uint8_t)va_arg(ap, int);
            if(u == entity::PLAYER)
            {
                *b++ = (c >= 'S' ? 'Y' : 'y');
                *b++ = 'o';
                *b++ = 'u';
                if(!(c & 1)) *b++ = 'r'; // 'T' or 'P'
            }
            else
            {
                *b++ = (c >= 'S' ? 'T' : 't');
                *b++ = 'h';
                *b++ = 'e';
                *b++ = ' ';
                b = tstrcpy_prog(b, (char const*)pgm_read_ptr(&MONSTER_NAMES[u]));
                if(!(c & 1)) // 'T' or 'P'
                {
                    *b++ = '\'';
                    *b++ = 's';
                }
            }
            break;
        case 'V': // verb
        case 'v': // verb whose plural needs +es
            u = (uint8_t)va_arg(ap, int);
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
            u = (uint8_t)va_arg(ap, int);
            b = tstrcpy_prog(b, u == entity::PLAYER ? PSTR("are") : PSTR("is"));
            break;
        case 'i': // item
        {
            item it = va_arg(ap, item);
            b = item_name(b, it);
            break;
        }
        case 'u': // uint8_t
            u = (uint8_t)va_arg(ap, int);
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
}

uint8_t tstrlen(char const* s)
{
    uint8_t r = 0;
    while(*s++ != '\0') ++r;
    return r;
}
