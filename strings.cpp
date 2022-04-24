#include "game.hpp"

static char const MON_BAT        [] PROGMEM = "bat";
static char const MON_SNAKE      [] PROGMEM = "snake";
static char const MON_RATTLESNAKE[] PROGMEM = "rattlesnake";
static char const MON_ZOMBIE     [] PROGMEM = "zombie";
static char const MON_GOBLIN     [] PROGMEM = "goblin";
static char const MON_PHANTOM    [] PROGMEM = "phantom";
static char const MON_ORC        [] PROGMEM = "orc";
static char const MON_TARANTULA  [] PROGMEM = "tarantula";
static char const MON_HOBGOBLIN  [] PROGMEM = "hobgoblin";
static char const MON_MIMIC      [] PROGMEM = "mimic";
static char const MON_INCUBUS    [] PROGMEM = "incubus";
static char const MON_TROLL      [] PROGMEM = "troll";
static char const MON_GRIFFIN    [] PROGMEM = "griffin";
static char const MON_DRAGON     [] PROGMEM = "dragon";
static char const MON_ANGEL      [] PROGMEM = "fallen angel";
static char const MON_DARKNESS   [] PROGMEM = "Lord of Darkness";

char const* const MONSTER_NAMES[] PROGMEM =
{
    nullptr, // none
    nullptr, // player
    MON_BAT,
    MON_SNAKE,
    MON_RATTLESNAKE,
    MON_ZOMBIE,
    MON_GOBLIN,
    MON_PHANTOM,
    MON_ORC,
    MON_TARANTULA,
    MON_HOBGOBLIN,
    MON_MIMIC,
    MON_INCUBUS,
    MON_TROLL,
    MON_GRIFFIN,
    MON_DRAGON,
    MON_ANGEL,
    MON_DARKNESS,
};

#if 0
static char const ITM_NONE   [] PROGMEM = "";
static char const ITM_FOOD   [] PROGMEM = "food ration";
static char const ITM_POTION [] PROGMEM = "potion";
static char const ITM_SCROLL [] PROGMEM = "scroll";
static char const ITM_ARROW  [] PROGMEM = "arrow";
static char const ITM_BOW    [] PROGMEM = "bow";
static char const ITM_SWORD  [] PROGMEM = "sword";
static char const ITM_RING   [] PROGMEM = "ring";
static char const ITM_AMULET [] PROGMEM = "amulet";
static char const ITM_ARMOR  [] PROGMEM = "armor";
static char const ITM_HELM   [] PROGMEM = "helm";
static char const ITM_BOOTS  [] PROGMEM = "boots";

char const* const ITEM_NAMES[] PROGMEM =
{
    ITM_NONE  ,
    ITM_FOOD  ,
    ITM_POTION,
    ITM_SCROLL,
    ITM_ARROW ,
    ITM_BOW   ,
    ITM_SWORD ,
    ITM_RING  ,
    ITM_AMULET,
    ITM_ARMOR ,
    ITM_HELM  ,
    ITM_BOOTS ,
};
#endif

static char const NPOT_HEALING[] PROGMEM = "healing";
static char const NPOT_CONFUSION[] PROGMEM = "confusion";
static char const NPOT_POISON[] PROGMEM = "poison";
static char const NPOT_STRENGTH[] PROGMEM = "strength";
static char const NPOT_INVIS[] PROGMEM = "invisibility";
static char const NPOT_PARALYSIS[] PROGMEM = "paralysis";
static char const NPOT_SLOWING[] PROGMEM = "slowing";
static char const NPOT_EXPERIENCE[] PROGMEM = "experience";

char const* const POT_NAMES[NUM_POT] PROGMEM =
{
    NPOT_HEALING,
    NPOT_CONFUSION,
    NPOT_POISON,
    NPOT_STRENGTH,
    NPOT_INVIS,
    NPOT_PARALYSIS,
    NPOT_SLOWING,
    NPOT_EXPERIENCE,
};

static char const UNID_POT_0[] PROGMEM = "red";
static char const UNID_POT_1[] PROGMEM = "clear";
static char const UNID_POT_2[] PROGMEM = "orange";
static char const UNID_POT_3[] PROGMEM = "green";
static char const UNID_POT_4[] PROGMEM = "blue";
static char const UNID_POT_5[] PROGMEM = "white";
static char const UNID_POT_6[] PROGMEM = "yellow";
static char const UNID_POT_7[] PROGMEM = "violet";

char const* const UNID_POT_NAMES[NUM_POT] PROGMEM =
{
    UNID_POT_0,
    UNID_POT_1,
    UNID_POT_2,
    UNID_POT_3,
    UNID_POT_4,
    UNID_POT_5,
    UNID_POT_6,
    UNID_POT_7,
};

static char const NSCR_IDENTIFY[] PROGMEM = "identify";
static char const NSCR_ENCHANT[] PROGMEM = "enchanting";
static char const NSCR_REMOVE_CURSE[] PROGMEM = "remove curse";
static char const NSCR_TELEPORT[] PROGMEM = "teleportation";
static char const NSCR_MAPPING[] PROGMEM = "magic mapping";
static char const NSCR_FEAR[] PROGMEM = "fear";
static char const NSCR_TORMENT[] PROGMEM = "torment";
static char const NSCR_MASS_CONFUSE[] PROGMEM = "mass confusion";
static char const NSCR_MASS_POISON[] PROGMEM = "mass poison";

char const* const SCR_NAMES[NUM_SCR] PROGMEM =
{
    NSCR_IDENTIFY,
    NSCR_ENCHANT,
    NSCR_REMOVE_CURSE,
    NSCR_TELEPORT,
    NSCR_MAPPING,
    NSCR_FEAR,
    NSCR_TORMENT,
    NSCR_MASS_CONFUSE,
    NSCR_MASS_POISON,
};

static char const UNID_SCR_0[] PROGMEM = "faded";
static char const UNID_SCR_1[] PROGMEM = "yellowed";
static char const UNID_SCR_2[] PROGMEM = "tattered";
static char const UNID_SCR_3[] PROGMEM = "glowing";
static char const UNID_SCR_4[] PROGMEM = "shimmering";
static char const UNID_SCR_5[] PROGMEM = "humming";
static char const UNID_SCR_6[] PROGMEM = "dark";
static char const UNID_SCR_7[] PROGMEM = "bright";
static char const UNID_SCR_8[] PROGMEM = "brilliant";

char const* const UNID_SCR_NAMES[NUM_SCR] PROGMEM =
{
    UNID_SCR_0,
    UNID_SCR_1,
    UNID_SCR_2,
    UNID_SCR_3,
    UNID_SCR_4,
    UNID_SCR_5,
    UNID_SCR_6,
    UNID_SCR_7,
    UNID_SCR_8,
};

static char const NRNG_SEE_INVIS[] PROGMEM = "see invisible";
static char const NRNG_STRENGTH[] PROGMEM = "strength";
static char const NRNG_DEXTERITY[] PROGMEM = "dexterity";
static char const NRNG_PROTECTION[] PROGMEM = "protection";
static char const NRNG_INVIS[] PROGMEM = "invisibility";
static char const NRNG_FIRE_PROTECT[] PROGMEM = "fire protection";
static char const NRNG_ATTACK[] PROGMEM = "attack";
static char const NRNG_SUSTENANCE[] PROGMEM = "sustenance";

char const* const RNG_NAMES[NUM_RNG] PROGMEM =
{
    NRNG_SEE_INVIS,
    NRNG_STRENGTH,
    NRNG_DEXTERITY,
    NRNG_PROTECTION,
    NRNG_INVIS,
    NRNG_FIRE_PROTECT,
    NRNG_ATTACK,
    NRNG_SUSTENANCE,
};

static char const NAMU_SPEED[] PROGMEM = "speed";
static char const NAMU_CLARITY[] PROGMEM = "clarity";
static char const NAMU_CONSERVATION[] PROGMEM = "conservation";
static char const NAMU_REGENERATION[] PROGMEM = "regeneration";
static char const NAMU_VAMPIRE[] PROGMEM = "the vampire";
static char const NAMU_IRONBLOOD[] PROGMEM = "ironblood";
static char const NAMU_VITALITY[] PROGMEM = "vitality";
static char const NAMU_WISDOM[] PROGMEM = "wisdom";
static char const NAMU_YENDOR[] PROGMEM = "Yendor";

char const* const AMU_NAMES[NUM_AMU + 1] PROGMEM =
{
    NAMU_SPEED,
    NAMU_CLARITY,
    NAMU_CONSERVATION,
    NAMU_REGENERATION,
    NAMU_VAMPIRE,
    NAMU_IRONBLOOD,
    NAMU_VITALITY,
    NAMU_WISDOM,
    NAMU_YENDOR,
};

static char const UNID_RNG_AMU_0[] PROGMEM = "diamond";
static char const UNID_RNG_AMU_1[] PROGMEM = "ruby";
static char const UNID_RNG_AMU_2[] PROGMEM = "emerald";
static char const UNID_RNG_AMU_3[] PROGMEM = "topaz";
static char const UNID_RNG_AMU_4[] PROGMEM = "gold";
static char const UNID_RNG_AMU_5[] PROGMEM = "silver";
static char const UNID_RNG_AMU_6[] PROGMEM = "platinum";
static char const UNID_RNG_AMU_7[] PROGMEM = "iron";

char const* const UNID_RNG_AMU_NAMES[NUM_RNG < NUM_AMU ? NUM_AMU : NUM_RNG] PROGMEM =
{
    UNID_RNG_AMU_0,
    UNID_RNG_AMU_1,
    UNID_RNG_AMU_2,
    UNID_RNG_AMU_3,
    UNID_RNG_AMU_4,
    UNID_RNG_AMU_5,
    UNID_RNG_AMU_6,
    UNID_RNG_AMU_7,
};

static char const  NWND_FORCE[] PROGMEM = "force";
static char const  NWND_DIGGING[] PROGMEM = "digging";
static char const  NWND_FIRE[] PROGMEM = "fire";
static char const  NWND_STRIKING[] PROGMEM = "striking";
static char const  NWND_ICE[] PROGMEM = "ice";
static char const  NWND_POLYMORPH[] PROGMEM = "polymorph";

char const* const WND_NAMES[NUM_WND] PROGMEM =
{
    NWND_FORCE,
    NSCR_TELEPORT, // reuse string from scroll
    NWND_DIGGING,
    NWND_FIRE,
    NWND_STRIKING,
    NWND_ICE,
    NWND_POLYMORPH,
};

static char const UNID_WND_0[] PROGMEM = "long";
static char const UNID_WND_1[] PROGMEM = "short";
static char const UNID_WND_2[] PROGMEM = "slender";
static char const UNID_WND_3[] PROGMEM = "thick";
static char const UNID_WND_4[] PROGMEM = "twisted";
static char const UNID_WND_5[] PROGMEM = "curved";
static char const UNID_WND_6[] PROGMEM = "glossy";

char const* const UNID_WND_NAMES[NUM_WND] PROGMEM =
{
    UNID_WND_0,
    UNID_WND_1,
    UNID_WND_2,
    UNID_WND_3,
    UNID_WND_4,
    UNID_WND_5,
    UNID_WND_6,
};

static char const INV_CAT_WEAPON[] PROGMEM = "Weapons";
static char const INV_CAT_ARMOR[] PROGMEM = "Armor";
static char const INV_CAT_WAND[] PROGMEM = "Wands";
static char const INV_CAT_POTION[] PROGMEM = "Potions";
static char const INV_CAT_SCROLL[] PROGMEM = "Scrolls";
static char const INV_CAT_JEWELRY[] PROGMEM = "Jewelry";
static char const INV_CAT_MISC[] PROGMEM = "Misc";

char const* const INV_CATEGORIES[] PROGMEM =
{
    INV_CAT_WEAPON,
    INV_CAT_ARMOR,
    INV_CAT_WAND,
    INV_CAT_POTION,
    INV_CAT_SCROLL,
    INV_CAT_JEWELRY,
    INV_CAT_MISC,
};
