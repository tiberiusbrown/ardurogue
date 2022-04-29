#include "../game.hpp"

#include <stdio.h>

// dummy platform
void wait() {}
uint8_t wait_btn() { return 0; }
void seed() {}
void paint_offset(uint8_t, bool) {}
uint8_t read_persistent(uint16_t) { return 0; }
void update_persistent(uint16_t, uint8_t) {}
void flush_persistent() {}

int main()
{
    {
        int max_att = 20;
        int max_def = 20;
        printf("<table>\n");
        printf("<tr><td /><td colspan=\"%d\" style=\"text-align:center\"><b>Defense</b></td></tr>\n", max_def + 1);
        printf("<tr style=\"text-align:right\"><td><b>Attack</b></td>");
        for(int def = 0; def <= max_def; ++def)
            printf("<td><b>%d</b></td>", def);
        printf("</tr>\n");
        for(int att = 0; att <= max_att; ++att)
        {
            printf("<tr style=\"text-align:right\"><td><b>%d</b></td>", att);
            for(int def = 0; def <= max_def; ++def)
            {
                int dam = calc_hit_dam_post_mod((uint8_t)att, (uint8_t)def);
                printf("<td>%d</td>", dam);
            }
            printf("</tr>\n");
        }
        printf("</table>\n");
    }

    return 0;
}
