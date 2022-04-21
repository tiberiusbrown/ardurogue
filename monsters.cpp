#include "game.hpp"

static constexpr entity_info decl_ent(
    uint8_t mean       = 0,
    uint8_t nomove     = 0,
    uint8_t regens     = 0,
    uint8_t invis      = 0,
    uint8_t poison     = 0,
    uint8_t vampire    = 0,
    uint8_t confuse    = 0,
    uint8_t paralyze   = 0,
    uint8_t fbreath    = 0,
    uint8_t opener     = 0,
    uint8_t strength   = 0,
    uint8_t dexterity  = 0,
    uint8_t speed      = 0,
    uint8_t defense    = 0,
    uint8_t max_health = 0,
    uint8_t xp         = 0
)
{
#if USE_CUSTOM_BITFIELDS
    entity_info r{};
    r.mean       = mean      ;
    r.nomove     = nomove    ;
    r.regens     = regens    ;
    r.invis      = invis     ;
    r.poison     = poison    ;
    r.vampire    = vampire   ;
    r.confuse    = confuse   ;
    r.paralyze   = paralyze  ;
    r.fbreath    = fbreath   ;
    r.opener     = opener    ;
    r.strength   = strength  ;
    r.dexterity  = dexterity ;
    r.speed      = speed     ;
    r.defense    = defense   ;
    r.max_health = max_health;
    r.xp         = xp        ;
    return r;
#else
    return
    {
        mean,
        nomove,
        regens,
        invis,
        poison,
        vampire,
        confuse,
        paralyze,
        fbreath,
        opener,
        strength,
        dexterity,
        speed,
        defense,
        max_health,
        xp,
    };
#endif
}

entity_info const MONSTER_INFO[] PROGMEM =
{
    /*
    *        mean        poison      fbreath       speed
    *           nomove      vampire     opener        defense
    *              regens      confuse     strength        max_health
    *                 invis       paralyze     dexterity        xp
    */
    decl_ent(),                                                      // none
    decl_ent(1, 0, 0, 0, 0, 0, 0, 0, 0, 1,  4,  4,  4,  0,  10,  0), // player
    decl_ent(0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  1,  6,  8,  0,   1,  1), // bat
    decl_ent(1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  2,  3,  3,  0,   3,  2), // snake
    decl_ent(1, 0, 0, 0, 1, 0, 0, 0, 0, 0,  3,  3,  3,  0,   4,  3), // rattlesnake
    decl_ent(1, 0, 0, 0, 0, 0, 0, 0, 0, 1,  4,  2,  2,  0,   6,  5), // zombie
    decl_ent(1, 0, 0, 0, 0, 0, 0, 0, 0, 1,  5,  4,  4,  1,  10,  6), // goblin
    decl_ent(1, 0, 0, 1, 0, 0, 0, 0, 0, 1,  6,  4,  4,  1,  12,  7), // phantom
    decl_ent(1, 0, 0, 0, 0, 0, 0, 0, 0, 1,  7,  4,  4,  3,  16,  8), // orc
    decl_ent(1, 0, 0, 0, 0, 0, 0, 1, 0, 0,  5,  4,  4,  0,  12,  9), // tarantula
    decl_ent(1, 0, 0, 0, 0, 0, 0, 0, 0, 1,  8,  4,  4,  2,  20, 11), // hobgoblin
    decl_ent(1, 1, 0, 0, 0, 0, 0, 0, 0, 0,  7,  4,  4,  3,  20, 11), // mimic
    decl_ent(1, 0, 0, 0, 0, 0, 1, 0, 0, 1,  9,  4,  4,  3,  24, 14), // incubus
    decl_ent(1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 10,  3,  3,  5,  32, 18), // troll
    decl_ent(1, 0, 0, 0, 0, 0, 0, 0, 0, 0,  7,  6,  6,  1,  24, 18), // griffin
    decl_ent(1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 12,  4,  4,  8,  48, 25), // dragon
    decl_ent(1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 10,  6,  6,  3,  24, 35), // fallen angel
    decl_ent(1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 16,  6,  8,  8,  64, 90), // lord of darkness
};

void entity_get_info(uint8_t i, entity_info& info)
{
    if(i == 0)
        info = pstats;
    else
        pgm_memcpy(&info, &MONSTER_INFO[ents[i].type], sizeof(info));
}

static uint8_t dist_to_player(uint8_t x, uint8_t y)
{
    uint8_t dx = u8abs(x - ents[0].x);
    uint8_t dy = u8abs(y - ents[0].y);
    return dx + dy; // manhatten distance
}

void monster_ai(uint8_t i, action& a)
{
    auto& e = ents[i];
    uint8_t ex = e.x, ey = e.y;
    entity_info info;
    entity_get_info(i, info);
    a.type = action::WAIT;
    if((info.nomove && !e.aggro) || player_is_dead())
        return;
    uint8_t dp = dist_to_player(e.x, e.y);
    bool mean = info.mean | e.aggro;

    if(!mean || player_is_invisible() || dp >= 10)
    {
        a.data = u8rand() & 3;
        uint8_t nx = ex + pgm_read_byte(&DIRX[a.data]);
        uint8_t ny = ey + pgm_read_byte(&DIRY[a.data]);
        entity* e = get_entity(nx, ny);
        if(e && (!mean || e->type != entity::PLAYER))
            return;
        a.type = action::MOVE;
        return;
    }

    // fire breathing
    if(info.fbreath)
    {
        uint8_t px = ents[0].x, py = ents[0].y;
        scan_result sr;
        uint8_t dir;
        for(dir = 0; dir < 4; ++dir)
        {
            scan_dir(i, dir, 5, sr);
            if(sr.i == 0)
                break;
        }
        if(sr.i == 0 && u8rand() % 2)
        {
            status(PSTR("@S breathes fire!"), i);
            render();
            draw_ray_anim(ex - px + 6, ey - py + 6, dir, sr.n);
            if(!wearing_uncursed_ring(RNG_FIRE_PROTECT))
                entity_take_damage_from_entity(i, 0, u8rand(8) + 8);
            return;
        }
    }

    for(uint8_t i = 0; i < 4; ++i)
    {
        uint8_t nx = ex + pgm_read_byte(&DIRX[i]);
        uint8_t ny = ey + pgm_read_byte(&DIRY[i]);
        if(tile_is_solid(nx, ny)) continue;
        if(entity* te = get_entity(nx, ny))
            if(te->type != entity::PLAYER)
                continue;
        uint8_t td = dist_to_player(nx, ny);
        if(e.scared)
        {
            if(td > dp)
            {
                a.type = action::MOVE;
                dp = td, a.data = i;
            }
        }
        else if(td < dp || (td == dp && (u8rand() & 1)))
        {
            a.type = action::MOVE;
            dp = td, a.data = i;
        }
    }
}
