# ArduRogue

ArduRogue is a traditional roguelike for the [Arduboy](https://www.arduboy.com/).

## Introduction

This is you: ![](img/player.png)

## Monsters

In roughly increasing difficulty:

| Symbol                   | Name        | Notes |
| ------------------------ | ----------- | ----- |
| ![](img/bat.png)         | bat         | non-aggressive, fast, cannot open doors |
| ![](img/snake.png)       | snake       | slow, cannot open doors |
| ![](img/rattlesnake.png) | rattlesnake | slow, poisonous, cannot open doors |
| ![](img/zombie.png)      | zombie      | slow |
| ![](img/goblin.png)      | goblin      | |
| ![](img/orc.png)         | orc         | |
| ![](img/hobgoblin.png)   | hobgoblin   | |
| ![](img/mimic.png)       | mimic       | looks exactly like a closed door, stays still until attacked | 
| ![](img/troll.png)       | troll       | slow, regenerates health |
| ![](img/griffin.png)     | griffin     | fast, cannot open doors |
| ![](img/dragon.png)      | dragon      | cannot open doors |

## Items

### Potions

Symbol: ![](img/potion.png)

### Scrolls

Symbol: ![](img/scroll.png)

### Rings

Symbol: ![](img/ring.png)

You are able to wear one ring at a time.

### Amulets

Symbol: ![](img/amulet.png)

## Effects

## Tips

### Use Doors to Escape

Not all monsters are able to open doors (as a rule, only humanoid monsters can).
Use this to your advantage by closing a door behind you to block off a dangerous monster's pursuit.
Note that you will need to have some distance, as you cannot close a door while the monster is standing in the doorframe.

### Hidden Doors

If you're stuck in a room with no doors, or can't find the staircase to the next dungeon, try searching for hidden doors.

![](img/hiddendoor1.png)

Press B to bring up the action menu.

![](img/hiddendoor2.png)

Select "Wait / Search". This action spends a turn doing nothing, but will search all adjacent squares for secrets.
Then walk next to the walls of the room and press A while standing next to each section of wall (pressing A will repeat the previously selected action).

![](img/hiddendoor3.png)

## TODO

- Wands
  - Striking (simple single-target damage)
  - Force (pushes monsters back)
  - Fire (area of effect damage)
  - Lightning (arcs between monsters)
  - Teleportation (teleports target)
  - Frenzy (target attacks any nearest entity)
- Floor traps
- Dragon breathes fire
- Allow wearing up to two rings
- Different map types (cave style?)
- High Score List
