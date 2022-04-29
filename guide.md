# Guide to ArduRogue

**WARNING: Spoilers ahead!**

Jump to Section:
- [How to Play](#howtoplay)
    - [Controls](#controls)
    - [Action Menu](#actionmenu)
    - [Game Menu](#gamemenu)
    - [Saving and Loading](#saving)
- [Game Reference](#gamereference)
    - [Items](#itemtypes)
        - [Potions](#potions)
        - [Scrolls](#scrolls)
        - [Rings](#rings)
        - [Amulets](#amulets)
        - [Wands](#wands)
    - [Enemies](#enemytypes)
    - [Combat Formulas](#combatformulas)

## How to Play <a name="howtoplay"></a>

### Controls <a name="controls"></a>

Use the arrow buttons to move in the cardinal directions.
Unlike most roguelikes, ArduRogue does not support diagonal movement or actions.
- Moving into an [enemy](#enemytypes) will perform a melee attack on that enemy.
- Moving onto an [item](#itemtypes) will produce a prompt asking whether to pick up the item.
- Moving onto stairs will produce a prompt asking whether to descend or ascend the stairs.

When presented with a prompt, press A to accept or B to decline.

### Action Menu <a name="actionmenu"></a>

During normal gameplay, press B to access the action menu.
The action menu allows you to perform non-movement actions, such as shooting an arrow or drinking a potion.
While in the action menu, use UP or DOWN to cycle the menu options, and LEFT or RIGHT to switch to the game menu.
Press A to select an action to perform, or B to leave the menu with no selection.

During normal gameplay, press A to repeat the previously performed action from the action menu.
This is useful to perform the same action multiple times without having to access the action menu each time, such as when
- searching for a hidden door,
- using a bow and arrow for ranged combat,
- dropping several items to clean out your inventory, or
- using items multiple times in a row, like a wand.

### Game Menu <a name="gamemenu"></a>

While in the action menu, press LEFT or RIGHT to switch to the game menu.
The game menu provides access to additional information, such as a map of the current dungeon level or player stats.
It also provides the option to [save your game](#saving).

### Saving and Loading <a name="saving"></a>

After pressing A at the title screen, ArduRogue will either continue from a saved game, if it exists, or begin a new game.

**Beware! ArduRogue enforces permadeath by deleting the saved game after it is loaded. Be sure to save every time when you're finished playing.**
Saving a game returns you to the title screen.

ArduRogue will only write to EEPROM when
- you select "Save and Exit" from the game menu,
- you continue playing from an existing saved game (it is then deleted), or
- you consent to saving a high score after the end of a game.

Settings are also stored with the saved game, but are not deleted when loading.

The EEPROM is not written to when beginning a new game. If you never save your progress or save a high score then ArduRogue will never touch EEPROM.

## Game Reference <a name="gamereference"></a>

### Items <a name="itemtypes"></a>

| Symbol              | Name        | Stackable | Equippable | Notes |
| :-----------------: | :---------: | :-------: | :--------: | ----- |
| ![](img/food.png)   | food ration | &#9745;   |            | Consumable. Staves off hunger. |
| ![](img/potion.png) | potion      | &#9745;   |            | Provides a temporary magical effect. Affects the user when drunk, or the target when thrown. |
| ![](img/scroll.png) | scroll      | &#9745;   |            | Performs some magical effect when read. |
| ![](img/wand.png)   | wand        |           |            | Performs some directed ranged magical effect when used. |
| ![](img/bow.png)    | bow         |           | Weapon     | Must be equipped for ranged combat. |
| ![](img/arrow.png)  | arrow       | &#9745;   |            | Required for ranged combat. |
| ![](img/sword.png)  | sword       |           | Weapon     | Gives bonus to attack. |
| ![](img/ring.png)   | ring        |           | Hand       | Can wear up to two rings at a time. Provides a constant magical effect. |
| ![](img/amulet.png) | amulet      |           | Neck       | Provides a constant magical effect. |
| ![](img/armor.png)  | armor       |           | Body       | Gives bonus to defense. |
| ![](img/armor.png)  | helm        |           | Head       | Gives bonus to defense. |
| ![](img/armor.png)  | boots       |           | Feet       | Gives bonus to defense. |

#### Potions <a name="potions"></a>

| Potion of... | Effect |
| ------------ | ------ |
| healing      | Restores the target's strength if weakened. Heals the target for 1/4 to 3/4 of max health. |
| confusion    | Temporarily confused the target. |
| poison       | Weakens the target. |
| strength     | Restores the target's strength if weakened. Otherwise, increases the target's strength by 1. |
| invisibility | Makes the target temporarily invisible. |
| paralysis    | Temporarily paralyzes the target. |
| slowing      | Temporarily slows the target. |
| experience   | Immediately grants a set amount of experience. |

#### Scrolls <a name="scrolls"></a>

| Scroll of...  | Effect |
| ------------- | ------ |
| identify      | Reveals the type and quality of a chosen item. |
| enchant       | Enhances the quality of a chosen item, or recharges a wand. |
| remove curse  | Removes the curse on an item, allowing it to be removed. |
| teleport      | Teleports the reader. |
| magic mapping | Reveals the full layout of the current dungeon level. |
| fear          | Causes all enemies in view to flee temporarily. |
| torment       | Immediately halves the health of the reader and all enemies in view. |
| mass confuse  | Temporarily confuses the reader and all enemies in view. |
| mass poison   | Weakens the reader and all enemies in view. |

#### Rings <a name="rings"></a>

| Ring of...      | Effect | Cursed Effect |
| --------------- | ------ | ------------- |
| see invisible   | Allows the wearer to see invisible beings. | The wearer has a 50% chance to be unable to see an enemy, whether or not it is invisible. |
| strength        | Grants a bonus to the wearer's strength. | Gives a penalty to the wearer's strength. |
| dexterity       | Grants a bonus to the wearer's dexterity. | Gives a penalty to the wearer's dexterity. |
| protection      | Grants a bonus to the wearer's defense, as if wearing better armor. | Gives a penalty to the wearer's defense. |
| fire protection | The wearer is immune to fire. | Fire damages the wearer twice as much. |
| attack          | Grants a bonus to the wearer's attack, as if holding a stronger weapon. | Gives a penalty to the wearer's attack. |
| sustenance      | Slows down the advance of hunger. | Hastens the advance of hunger. |
| invisibility    | The wearer is permanently invisible. | The wearer cannot become invisible, even temporarily. |

#### Amulets <a name="amulets"></a>

| Amulet of... | Effect | Cursed Effect |
| ------------ | ------ | ------------- |
| speed        | Grants a bonus to the wearer's speed. | Gives a penalty to the wearer's speed. |
| clarity      | The wearer cannot be confused. | The wearer occasionally becomes confused. |
| conservation | Chance for the wearer to not consume the item when using scrolls or potions. | Chance for the wearer to consume two items. |
| regeneration | The wearer slowly heals. | Halves all healing the wearer receives. |
| the vampire  | The wearer heals for a small amount on each melee hit. | Nothing. |
| ironblood    | The wearer cannot be paralyzed. | The wearer occasionally becomes paralyzed. |
| vitality     | Grants a bonus to the wearer's maximum health. | Gives a penalty to the wearer's maximum health. |
| wisdom       | The wearer gains experience 50% more quickly. | The wearer gains experience 50% more slowly. |

#### Wands <a name="wands"></a>

### Enemies <a name="enemytypes"></a>

| Symbol                   | Name             | Str | Dex | Spd | Def | Health | Notes |
| ------------------------ | ---------------- | --: | --: | --: | --: | -----: | ----- |
| ![](img/bat.png)         | bat              |   1 |   6 |   8 |   0 |      1 | Passive: does not attack until attacked. |
| ![](img/snake.png)       | snake            |   2 |   3 |   3 |   0 |      3 |  |
| ![](img/rattlesnake.png) | rattlesnake      |   3 |   3 |   3 |   0 |      4 | Chance to poison on hit. |
| ![](img/zombie.png)      | zombie           |   4 |   2 |   2 |   0 |      6 |  |
| ![](img/goblin.png)      | goblin           |   5 |   4 |   4 |   1 |     10 |  |
| ![](img/phantom.png)     | phantom          |   6 |   4 |   4 |   1 |     12 | Invisible. |
| ![](img/orc.png)         | orc              |   7 |   4 |   4 |   3 |     16 |  |
| ![](img/tarantula.png)   | tarantula        |   5 |   4 |   4 |   0 |     12 | Chance to paralyze on hit. |
| ![](img/hobgoblin.png)   | hobgoblin        |   8 |   4 |   4 |   2 |     20 |  |
| ![](img/mimic.png)       | mimic            |   7 |   4 |   4 |   3 |     20 | Disguises itself as an item. |
| ![](img/incubus.png)     | incubus          |   9 |   4 |   4 |   3 |     24 | Chance to confuse on hit. |
| ![](img/troll.png)       | troll            |  10 |   3 |   3 |   5 |     32 | Regenerates health. |
| ![](img/griffin.png)     | griffin          |   7 |   6 |   6 |   1 |     24 |  |
| ![](img/dragon.png)      | dragon           |  12 |   4 |   4 |   8 |     48 | Breathes fire. |
| ![](img/angel.png)       | fallen angel     |  10 |   6 |   6 |   3 |     24 | Chance to confuse or paralyze on hit. |
| ![](img/darkness.png)    | Lord of Darkness |  16 |   6 |   8 |   8 |    128 | Regenerates. Chance to poison, confuse, or paralyze on hit. |

## Combat Formulas <a name="combatformulas"></a>

When a melee attack successfully hits, the attacker's attack stat and the defender's defense stat are each used as the base for random rolls.
For a base stat value N, the roll will be in the inclusive range from floor(N/4) to N.
For example, if you have an attack stat of 13, the attack roll will be a random value from 3 to 13.

After the attack roll and defense roll are determined, the damage is calculated by mixing the two values.
The following table shows the final damage dealt for different combinations of attack and defense rolls.

<table>
<tr><td /><td colspan="21" style="text-align:center"><b>Defense</b></td></tr>
<tr style="text-align:right"><td><b>Attack</b></td><td><b>0</b></td><td><b>1</b></td><td><b>2</b></td><td><b>3</b></td><td><b>4</b></td><td><b>5</b></td><td><b>6</b></td><td><b>7</b></td><td><b>8</b></td><td><b>9</b></td><td><b>10</b></td><td><b>11</b></td><td><b>12</b></td><td><b>13</b></td><td><b>14</b></td><td><b>15</b></td><td><b>16</b></td><td><b>17</b></td><td><b>18</b></td><td><b>19</b></td><td><b>20</b></td></tr>
<tr style="text-align:right"><td><b>0</b></td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td></tr>
<tr style="text-align:right"><td><b>1</b></td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td></tr>
<tr style="text-align:right"><td><b>2</b></td><td>2</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td></tr>
<tr style="text-align:right"><td><b>3</b></td><td>3</td><td>2</td><td>2</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td></tr>
<tr style="text-align:right"><td><b>4</b></td><td>4</td><td>3</td><td>3</td><td>2</td><td>2</td><td>2</td><td>2</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td></tr>
<tr style="text-align:right"><td><b>5</b></td><td>5</td><td>4</td><td>4</td><td>3</td><td>3</td><td>2</td><td>2</td><td>2</td><td>2</td><td>2</td><td>2</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td></tr>
<tr style="text-align:right"><td><b>6</b></td><td>6</td><td>5</td><td>5</td><td>4</td><td>4</td><td>3</td><td>3</td><td>3</td><td>3</td><td>2</td><td>2</td><td>2</td><td>2</td><td>2</td><td>2</td><td>2</td><td>1</td><td>1</td><td>1</td><td>1</td><td>1</td></tr>
<tr style="text-align:right"><td><b>7</b></td><td>7</td><td>6</td><td>5</td><td>5</td><td>4</td><td>4</td><td>4</td><td>3</td><td>3</td><td>3</td><td>3</td><td>3</td><td>2</td><td>2</td><td>2</td><td>2</td><td>2</td><td>2</td><td>2</td><td>2</td><td>2</td></tr>
<tr style="text-align:right"><td><b>8</b></td><td>8</td><td>7</td><td>6</td><td>6</td><td>5</td><td>5</td><td>5</td><td>4</td><td>4</td><td>4</td><td>4</td><td>3</td><td>3</td><td>3</td><td>3</td><td>3</td><td>3</td><td>2</td><td>2</td><td>2</td><td>2</td></tr>
<tr style="text-align:right"><td><b>9</b></td><td>9</td><td>8</td><td>7</td><td>7</td><td>6</td><td>6</td><td>5</td><td>5</td><td>5</td><td>4</td><td>4</td><td>4</td><td>4</td><td>4</td><td>3</td><td>3</td><td>3</td><td>3</td><td>3</td><td>3</td><td>3</td></tr>
<tr style="text-align:right"><td><b>10</b></td><td>10</td><td>9</td><td>8</td><td>8</td><td>7</td><td>7</td><td>6</td><td>6</td><td>6</td><td>5</td><td>5</td><td>5</td><td>5</td><td>4</td><td>4</td><td>4</td><td>4</td><td>4</td><td>3</td><td>3</td><td>3</td></tr>
<tr style="text-align:right"><td><b>11</b></td><td>11</td><td>10</td><td>9</td><td>9</td><td>8</td><td>8</td><td>7</td><td>7</td><td>6</td><td>6</td><td>6</td><td>5</td><td>5</td><td>5</td><td>5</td><td>5</td><td>4</td><td>4</td><td>4</td><td>4</td><td>4</td></tr>
<tr style="text-align:right"><td><b>12</b></td><td>12</td><td>11</td><td>10</td><td>10</td><td>9</td><td>9</td><td>8</td><td>8</td><td>7</td><td>7</td><td>7</td><td>6</td><td>6</td><td>6</td><td>6</td><td>5</td><td>5</td><td>5</td><td>5</td><td>5</td><td>4</td></tr>
<tr style="text-align:right"><td><b>13</b></td><td>13</td><td>12</td><td>11</td><td>11</td><td>10</td><td>9</td><td>9</td><td>8</td><td>8</td><td>8</td><td>7</td><td>7</td><td>7</td><td>6</td><td>6</td><td>6</td><td>6</td><td>6</td><td>5</td><td>5</td><td>5</td></tr>
<tr style="text-align:right"><td><b>14</b></td><td>14</td><td>13</td><td>12</td><td>12</td><td>11</td><td>10</td><td>10</td><td>9</td><td>9</td><td>9</td><td>8</td><td>8</td><td>8</td><td>7</td><td>7</td><td>7</td><td>7</td><td>6</td><td>6</td><td>6</td><td>6</td></tr>
<tr style="text-align:right"><td><b>15</b></td><td>15</td><td>14</td><td>13</td><td>13</td><td>12</td><td>11</td><td>11</td><td>10</td><td>10</td><td>9</td><td>9</td><td>9</td><td>8</td><td>8</td><td>8</td><td>7</td><td>7</td><td>7</td><td>7</td><td>7</td><td>6</td></tr>
<tr style="text-align:right"><td><b>16</b></td><td>16</td><td>15</td><td>14</td><td>14</td><td>13</td><td>12</td><td>12</td><td>11</td><td>11</td><td>10</td><td>10</td><td>10</td><td>9</td><td>9</td><td>9</td><td>8</td><td>8</td><td>8</td><td>8</td><td>7</td><td>7</td></tr>
<tr style="text-align:right"><td><b>17</b></td><td>17</td><td>16</td><td>15</td><td>14</td><td>14</td><td>13</td><td>13</td><td>12</td><td>12</td><td>11</td><td>11</td><td>10</td><td>10</td><td>10</td><td>9</td><td>9</td><td>9</td><td>8</td><td>8</td><td>8</td><td>8</td></tr>
<tr style="text-align:right"><td><b>18</b></td><td>18</td><td>17</td><td>16</td><td>15</td><td>15</td><td>14</td><td>14</td><td>13</td><td>13</td><td>12</td><td>12</td><td>11</td><td>11</td><td>10</td><td>10</td><td>10</td><td>10</td><td>9</td><td>9</td><td>9</td><td>9</td></tr>
<tr style="text-align:right"><td><b>19</b></td><td>19</td><td>18</td><td>17</td><td>16</td><td>16</td><td>15</td><td>14</td><td>14</td><td>13</td><td>13</td><td>12</td><td>12</td><td>12</td><td>11</td><td>11</td><td>11</td><td>10</td><td>10</td><td>10</td><td>9</td><td>9</td></tr>
<tr style="text-align:right"><td><b>20</b></td><td>20</td><td>19</td><td>18</td><td>17</td><td>17</td><td>16</td><td>15</td><td>15</td><td>14</td><td>14</td><td>13</td><td>13</td><td>13</td><td>12</td><td>12</td><td>11</td><td>11</td><td>11</td><td>11</td><td>10</td><td>10</td></tr>
</table>
