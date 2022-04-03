#include "game.hpp"

static constexpr uint8_t MORE_WIDTH =
	2 + 3 + 4 + 4 + 3 + 4 + 2;

static inline bool is_white(char c)
{
	return c <= ' ';
}

static uint8_t advance_white(char* b, uint8_t i)
{
	while(!is_white(b[i])) ++i;
	b[i] = '\0';
	return i + 1;
}

void draw_status()
{
    uint8_t sx = 1, sy = STATUS_START_Y;
    uint8_t n = 0;
    while(n < statusn)
    {
        uint8_t w = text_width(&statusbuf[n], false);
        if(sx + w > 64)
        {
            sx = 1, sy += 6;
            draw_text(sx, sy, &statusbuf[n], false);
        }
        else
        {
            draw_text(sx, sy, &statusbuf[n], false);
            sx += w + SPACE_WIDTH + 1;
        }
        while(statusbuf[n++] != '\0')
            ;
    }
}

static void more_status()
{
    draw_info();
    draw_text(65 - MORE_WIDTH, 59, PSTR("[more]"));
    paint_right();
    while(wait_btn() != BTN_A)
        ;
    statusn = 0;
    statusx = 1;
    statusy = STATUS_START_Y;
}

void status(char const* fmt, ...)
{
	char buf[64];
	va_list ap;
	va_start(ap, fmt);
	tvsprintf(buf, fmt, ap);
	va_end(ap);
	uint8_t len = tstrlen(buf);
	uint8_t a = 0, b = advance_white(buf, 0);
	for(;;)
	{
        if(uint8_t(statusn + b - a) > sizeof(statusbuf))
            more_status();
        for(uint8_t i = a; i < b; ++i)
            statusbuf[statusn++] = buf[i];
		uint8_t n = text_width(&buf[a], false);
		if(statusy >= 59 && statusx + n > 64 - MORE_WIDTH)
		{
            more_status();
			continue;
		}
		if(statusx + n > 64)
		{
			statusx = 1;
			statusy += 6;
			continue;
		}
		statusx += n + SPACE_WIDTH + 1;
		if(b >= len) break;
		a = b;
		b = advance_white(buf, a);
	}
}
