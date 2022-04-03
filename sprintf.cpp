#include "game.hpp"

void tsprintf(char* b, char const* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    tvsprintf(b, fmt, ap);
    va_end(ap);
}

// does not copy null char
char* tstrcpy_prog(char* dst, char const* src)
{
    char c;
    while((c = pgm_read_byte(src++)) != '\0')
        *dst++ = c;
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
        if(c != '%')
        {
            *b++ = c;
            if(c == '\0') return;
            continue;
        }
        c = (char)pgm_read_byte(fmt++);
        switch(c)
        {
        case '%': // escaped '%'
            *b++ = '%';
            break;
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
            u = (uint8_t)va_arg(ap, int);
            if(u == entity::PLAYER)
            {
                *b++ = (c == 'S' ? 'Y' : 'y');
                *b++ = 'o';
                *b++ = 'u';
            }
            else
            {
                *b++ = (c == 'S' ? 'T' : 't');
                *b++ = 'h';
                *b++ = 'e';
                *b++ = ' ';
                b = tstrcpy_prog(b, (char const*)pgm_read_ptr(&MONSTER_NAMES[u]));
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
        case 'I': // is/are
            u = (uint8_t)va_arg(ap, int);
            b = tstrcpy_prog(b, u == entity::PLAYER ? PSTR("are") : PSTR("is"));
            break;
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
