# Guide to ArduRogue

**WARNING: Spoilers ahead!**

Jump to Section:
- [Item Types](#itemtypes)
- [Item Effects](#itemeffects)
   - [Potions](#potions)
   - [Scrolls](#scrolls)
   - [Rings](#rings)
   - [Amulets](#amulets)
   - [Wands](#wands)
- [Enemy Types](#enemytypes)

## Item Types <a name="itemtypes"></a>

| Symbol | Name        | Stackable | Equippable | Notes |
| :-: | :-: | :-: | :-: | - |
| ![](img/food.png)   | food ration | &#9745; |        | Consumable. Staves off hunger. |
| ![](img/potion.png) | potion      | &#9745; |        | Provides a temporary magical effect. Affects the user when drunk, or the target when thrown. |
| ![](img/scroll.png) | scroll      | &#9745; |        | Performs some magical effect when read. |
| ![](img/wand.png)   | wand        |         |        | Performs some directed ranged magical effect when used. |
| ![](img/bow.png)    | bow         |         | Weapon | Must be equipped for ranged combat. |
| ![](img/arrow.png)  | arrow       | &#9745; |        | Required for ranged combat. |
| ![](img/sword.png)  | sword       |         | Weapon | Gives bonus to attack. |
| ![](img/ring.png)   | ring        |         | Hand   | Can wear up to two rings at a time. Provides a constant magical effect. |
| ![](img/amulet.png) | amulet      |         | Neck   | Provides a constant magical effect. |
| ![](img/armor.png)  | armor       |         | Body   | Gives bonus to defense. |
| ![](img/armor.png)  | helm        |         | Head   | Gives bonus to defense. |
| ![](img/armor.png)  | boots       |         | Feet   | Gives bonus to defense. |

## Item Effects <a name="itemeffects"></a>

### Potions <a name="potions"></a>

| Potion of... | Effect |
| - | - |
| healing | Restores the target's strength if weakened. Heals the target for 1/4 to 3/4 of max health. |
| confusion | Temporarily confused the target. |
| poison | Weakens the target. |
| strength | Restores the target's strength if weakened. Otherwise, increases the target's strength by 1. |
| invisibility | Makes the target temporarily invisible. |
| paralysis | Temporarily paralyzes the target. |
| slowing | Temporarily slows the target. |
| experience | Immediately grants a set amount of experience. |

### Scrolls <a name="scrolls"></a>

| Scroll of... | Effect |
| - | - |
| identify | Reveals the type and quality of a chosen item. |
| enchant | Enhances the quality of a chosen item, or recharges a wand. |
| remove curse | Removes the curse on an item, allowing it to be removed. |
| teleport | Teleports the reader. |
| magic mapping | Reveals the full layout of the current dungeon level. |
| fear | Causes all enemies in view to flee temporarily. |
| torment | Immediately halves the health of the reader and all enemies in view. |
| mass confuse | Temporarily confuses the reader and all enemies in view. |
| mass poison | Weakens the reader and all enemies in view. |

### Rings <a name="rings"></a>

| Ring of... | Effect | Cursed Effect |
| - | - | - |
| see invisible | Allows the wearer to see invisible beings. | The wearer has a 50% chance to be unable to see a creature, whether or not it is invisible. |
| strength | Grants a bonus to the user's strength. | Gives a penalty to the user's strength. |
| dexterity | Grants a bonus to the user's dexterity. | Gives a penalty to the user's dexterity. |
| protection | Grants a bonus to the user's defense, as if wearing better armor. | Gives a penalty to the user's defense. |
| invisibility | The user is permanently invisible. | The user cannot become invisible, even temporarily. |
| fire protection | The user is immune to fire. | Fire damages the user twice as much. |
| attack | Grants a bonus to the user's attack, as if holding a stronger weapon. | Gives a penalty to the user's attack. |
| sustenance | Slows down the advance of hunger. | Hastens the advance of hunger. |

### Amulets <a name="amulets"></a>

| Amulet of... | Effect | Cursed Effect |
| - | - | - |
| speed | 
| clarity | 
| conservation |
| regeneration |
| the vampire |
| ironblood |
| vitality |
| wisdom |

### Wands <a name="wands"></a>

## Enemy Types <a name="enemytypes"></a>

| Symbol | Name | Str | Dex | Spd | Def | Health | Notes |
| - | - | -: | -: | -: | -: | -: | - |
| ![](img/bat.png)         | bat              |  1 | 6 | 8 | 0 |   1 | Passive: does not attack until attacked. |
| ![](img/snake.png)       | snake            |  2 | 3 | 3 | 0 |   3 |  |
| ![](img/rattlesnake.png) | rattlesnake      |  3 | 3 | 3 | 0 |   4 | Chance to poison on hit. |
| ![](img/zombie.png)      | zombie           |  4 | 2 | 2 | 0 |   6 |  |
| ![](img/goblin.png)      | goblin           |  5 | 4 | 4 | 1 |  10 |  |
| ![](img/phantom.png)     | phantom          |  6 | 4 | 4 | 1 |  12 | Invisible. |
| ![](img/orc.png)         | orc              |  7 | 4 | 4 | 3 |  16 |  |
| ![](img/tarantula.png)   | tarantula        |  5 | 4 | 4 | 0 |  12 | Chance to paralyze on hit. |
| ![](img/hobgoblin.png)   | hobgoblin        |  8 | 4 | 4 | 2 |  20 |  |
| ![](img/mimic.png)       | mimic            |  7 | 4 | 4 | 3 |  20 | Disguises itself as an item. |
| ![](img/incubus.png)     | incubus          |  9 | 4 | 4 | 3 |  24 | Chance to confuse on hit. |
| ![](img/troll.png)       | troll            | 10 | 3 | 3 | 5 |  32 | Regenerates health. |
| ![](img/griffin.png)     | griffin          |  7 | 6 | 6 | 1 |  24 |  |
| ![](img/dragon.png)      | dragon           | 12 | 4 | 4 | 8 |  48 | Breathes fire. |
| ![](img/angel.png)       | fallen angel     | 10 | 6 | 6 | 3 |  24 | Chance to confuse or paralyze on hit. |
| ![](img/darkness.png)    | Lord of Darkness | 16 | 6 | 8 | 8 | 128 | Regenerates. Chance to poison, confuse, or paralyze on hit. |
