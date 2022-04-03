#include "game.hpp"

static char const MON_BAT        [] PROGMEM = "bat";
static char const MON_SNAKE      [] PROGMEM = "snake";
static char const MON_RATTLESNAKE[] PROGMEM = "rattlesnake";
static char const MON_ZOMBIE     [] PROGMEM = "zombie";
static char const MON_GOBLIN     [] PROGMEM = "goblin";
static char const MON_ORC        [] PROGMEM = "orc";
static char const MON_HOBGOBLIN  [] PROGMEM = "hobgoblin";
static char const MON_TROLL      [] PROGMEM = "troll";
static char const MON_GRIFFIN    [] PROGMEM = "griffin";
static char const MON_DRAGON     [] PROGMEM = "dragon";

char const* const MONSTER_NAMES[] PROGMEM =
{
    nullptr, // none
    nullptr, // player
    MON_BAT,
    MON_SNAKE,
    MON_RATTLESNAKE,
    MON_ZOMBIE,
    MON_GOBLIN,
    MON_ORC,
    MON_HOBGOBLIN,
    MON_TROLL,
    MON_GRIFFIN,
    MON_DRAGON,
};
