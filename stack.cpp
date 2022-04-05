#include "game.hpp"

#ifdef ARDUINO
extern char* __bss_end;
extern uint8_t __stack;
#endif

static constexpr uint8_t CANARY_VAL = 0xa5;

void stack_canary_init()
{
#ifdef ARDUINO
    uint8_t* p = (uint8_t*)&__bss_end;
    while(p <= SP)
        *p++ = CANARY_VAL;
#endif
}

uint8_t unused_stack()
{
#ifdef ARDUINO
    uint8_t* p = (uint8_t*)&__bss_end;
    uint8_t n = 0;
    while(p <= SP)
    {
        if(*p++ != CANARY_VAL) break;
        if(++n == 255) break;
    }
    return n;
#else
    return 0;
#endif
}
