# Alley Cat

A modern C / SDL2 implementation of the IBM PC game *Alley Cat* (Bill Williams,
SynSoft / IBM, 1984).

The game renders as the original does: CGA mode 4, palette 1, high intensity —
black, light cyan, light magenta and white, and nothing else. Sprites are the
original 2-bits-per-pixel data, drawn through the same three blit modes the
1984 code uses.

## Building

Requires CMake 3.20+ and a C11 compiler.

**Windows (MSVC)** — SDL2 is downloaded automatically:

```
build.bat
```

or, equivalently:

```
cmake -B build -S .
cmake --build build --config Release
```

The executable and `SDL2.dll` land in `build/Release/`.

**Linux, macOS, MinGW** — install SDL2 development headers first, then:

```
cmake -B build -S .
cmake --build build
```

## Controls

| Key | Action |
| --- | --- |
| Left / Right, or A / D | Walk |
| Up / W | Climb into a window (stand at a drainpipe first) |
| Space or Ctrl | Jump, and confirm on menus |
| Escape | Leave a room, return to the title, or quit from the title |
| Alt+Enter | Toggle fullscreen |

## Playing

Walk the alley and climb a drainpipe into any lit window. Each of the six rooms
asks for something different — collect the fish, dodge the dog, hop the
platforms — and each is on a timer. Clearing a room shuts its window. Clear all
six and the level advances, the windows reopen, and everything gets faster.
Nine lives, and running out ends the run.

## Credits and licence

Released under the GNU General Public License v3.0 — see [LICENSE](LICENSE).

The sprite tables in `src/sprites.c` and the note and music tables in
`src/sound.c` are taken from [rhuizer/alleycat](https://github.com/rhuizer/alleycat),
Ronald Huizer's reverse-engineered reconstruction of the original game data,
which is GPL-3.0. That project is also the reference for how the original draws
sprites and decodes its music, and this implementation follows it on both
counts:

- The cat is blitted with **white as the colour key**, matching
  `SDL_SetColorKey(cat->surface, SDL_SRCCOLORKEY, white)` in its `cat.c`.
  Static art is copied opaquely instead, and a third mode ANDs a sprite into
  the framebuffer the way `alleycat_draw_overlay` does.
- The intro tune is **one byte per BIOS timer tick** (18.2 Hz), where the byte
  is twice the index into `tones[]`. Those entries are **PIT divisors**, so the
  sounding pitch is `1193180 / tones[i]` and the table runs high to low as the
  index rises. The reference settles it: `alleycat.c` pushes
  `1193180 / tones[i]` into `PCS_Push(short freq)`, that value reaches
  `SPK_Sound()`, and `speaker.c` turns it into a phase increment with
  `osc.k = (freq << 16) / audiospec.freq`. Decoded this way the opening phrase
  is C4 C4 B3 B3 A3 B3 A3 G3, spanning 194 Hz to 3107 Hz.

*Alley Cat* is a trademark of its respective owners. This is a fan
reimplementation, not affiliated with IBM or the original authors.
