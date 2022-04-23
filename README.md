# ArduRogue

ArduRogue is a traditional roguelike for the [Arduboy](https://www.arduboy.com/).

Read the guide [here](guide.md).

## Features

- Random dungeon generation
- Save/Load games, save (if it exists) destroyed on death (permadeath)
- High score tracking
- 16 types of enemies, each with their own quirks and abilities
- Variety of items
  - 8 potion effects
  - 9 scroll effects
  - 8 ring types (up to 2 rings can be worn)
  - 8 amulet types (not including the amulet of Yendor)
- Items can be cursed, identified, enchanted
- Throw a potion at an enemy to apply its effect to it
- Hidden doors
- Affects: confusion, invisibility, weaken, slow, etc.

## Wishlist (in order of priority)

If enough prog bytes could be freed up...

- Floor traps
- Spawn new monsters on occasion
- Different map types (cave style?)
- Wands
  - Striking (simple single-target damage)
  - Force (pushes monsters back)
  - Fire (area of effect damage)
  - Lightning (arcs between monsters)
  - Teleportation (teleports target)
  - Frenzy (target attacks any nearest entity)

## Build Notes

Contents of `platform.local.txt`:
```
compiler.c.extra_flags=-mcall-prologues -mstrict-X -fno-inline-small-functions -fno-tree-scev-cprop -fno-move-loop-invariants -fweb -frename-registers -std=c++11
compiler.cpp.extra_flags={compiler.c.extra_flags} 
compiler.c.elf.extra_flags=-Wl,--relax
```
