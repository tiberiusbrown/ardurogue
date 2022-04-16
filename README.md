# ArduRogue

ArduRogue is a traditional roguelike for the [Arduboy](https://www.arduboy.com/).

## TODO (in order of priority)

- Floor traps
- Spawn new monsters on occasion
- Allow wearing up to two rings
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
compiler.c.extra_flags=-mcall-prologues -mstrict-X -fno-inline-small-functions -fno-tree-scev-cprop  -fno-move-loop-invariants
compiler.cpp.extra_flags=-mcall-prologues -mstrict-X -fno-inline-small-functions -fno-tree-scev-cprop  -fno-move-loop-invariants
compiler.c.elf.extra_flags=-Wl,--relax
```
