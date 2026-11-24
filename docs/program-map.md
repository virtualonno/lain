# Program map

Serial Experiments Lain is one PlayStation executable, the same on both discs (`SLPS_016.03` on disc 1,
`SLPS_016.04` on disc 2), that stays resident for the whole game: no code is loaded later, and every other file
on the discs is data ([formats.md](formats.md)). The player enters a name or loads a save, then moves through
the grid: a cylinder of levels whose cells hold nodes, most of them a movie or an XA track for the media player
or the voice screen, the rest an upgrade, a gate or a P2 screen. Disc 1 holds Site A and disc 2 Site B, and the program asks for the
other disc when the player changes Site. Routine and global names are the source's; a unit is a file of `src/`.

## Systems

Each document under `systems/` says how one part of the program works and which routines and records to read.

The engine:

- [The frame](systems/frame.md): boot, the outer loop between the start screen and the grid, how a frame is
  begun and shown, timing, the vertical-blank handler, the heap, the controllers and the pad event.
- [Graphics](systems/graphics.md): the display buffers, ordering tables and packet areas, the model record,
  image upload and palette fades, the views and lights, text drawn into video memory, the video-memory dump.
- [Files and the discs](systems/files.md): the file tables of the two discs, telling them apart and asking
  for the other, the read queue and its callbacks, archive parts, napk unpacking, the loading sequence, the P2
  screen.
- [Sound](systems/sound.md): the `SND.BIN` bank and its two sequences through LIBSND, sound effects, the grid
  and About music.
- [Speech](systems/speech.md): the `VOICE.BIN` syllable archive, building and speaking the player's name, the
  voice screen.
- [The media player](systems/media-player.md): a media node's tracks, the player screen and its Play/Exit cube,
  XA streaming, movies through the MDEC, idle media, the end roll.
- [Clips](systems/clips.md): the `LAPKS.BIN` clips, their NITR-BS frames and masks and the decoder, the idle
  and sequence slots, which clip plays when.

The game:

- [The grid](systems/grid.md): the Site's 3D scene: cells on the cylinder, the camera, levels and travel, the
  ring, bands, level labels and effects, and how a frame of the grid is built and drawn.
- [The grid menu](systems/grid-menu.md): `RunGridMenu`'s states: input, the cursor, turning, tilting, moving
  between levels, Jump To, opening a node and what follows, the idle clips.
- [Nodes](systems/nodes.md): the node table field by field, placement on the grid, visibility and unlocking,
  the kinds of node and what each opens, the progress counters.
- [The start screen](systems/start-screen.md): a new game or a save, its fading lines, and the kana name entry
  that spells the player's name.
- [The system menu](systems/system-menu.md): the tile menu the grid opens (Load, About, Change, Save, Exit), its
  prompt, the message window, the save and load steps.
- [The memory card](systems/memory-card.md): the card calls, the save file, what is saved and how it is checked.
- [The screens the grid runs](systems/scenes.md): About, the upgrade screen, the gate screen, the end roll's
  spheres, the continue prompt.

## Where to find things

| To find | Look at |
| --- | --- |
| Boot and the outer loop | `main` (`src/main.c`) |
| A frame's beginning and end | `BeginFrame`, `EndFrame` (`src/graphics.c`) |
| Controller input | `ReadPadEvent` and `gPadEvent` (`src/pad.c`), the codes in `include/pad.h` |
| The grid menu, one state a frame | `RunGridMenu` and `gGridMenuState` (`src/grid_menu.c`) |
| The grid's 3D scene | `UpdateGridScene` (`src/grid_scene.c`), its effects in `src/grid_effects.c` |
| The nodes | `gNodes` (`src/nodes.c`, `Node` in `include/nodes.h`); placement in `src/grid_nodes.c` |
| Which disc the game wants, and asking for it | `gCurrentDisc`; `EnsureSiteDisc` (`src/disc_change.c`) |
| Reading from the disc | `RequestFilePartRead` and `PollCdRequest` (`src/cd.c`), the part tables in `src/file_parts.c` |
| Memory | `AllocHeapBlock` and `FreeHeapBlock` (`src/heap.c`) |
| Playing a media node | `RunMediaPlayer` (`src/media_player.c`) |
| The clips on the grid | `src/lapks_player.c` (slots and reads), `src/clip.c` (decoding) |
| The player's name and its speech | `gPlayerName` (`src/kana_entry.c`); `src/speech.c`, `src/voice_screen.c` |
| Saving and loading | `PackCardSaveBlock`, `UnpackCardSaveBlock` (`src/memcard.c`); the steps in `src/system_menu.c` |
| Text on screen | `UploadTextToVram` (`src/text.c`); node titles and words in `src/string_tables.c` |
| Music and sound effects | `src/sound.c` |

## Layout in memory

| | Address |
| --- | --- |
| Loaded at | 0x80010000 |
| Code (`.text`) | 0x80013074 to 0x80070F4C |
| Sony's library code begins | 0x8003E864 |
| Entry point (the startup) | 0x8001307C |
| Global pointer (`$gp`): small data begins | 0x800A5EE4 |
| Uninitialized data | 0x800A67D8 to 0x801FD498 |

The executable loads at 0x80010000: read-only data first, then the startup (`2MBYTE.OBJ`, which clears the
uninitialised data, sets up the stack and calls `main`), the game's code from 0x80013138 to 0x8003E864, and
Sony's library code after it. The game reads every file into heap blocks or its own static buffers; the heap is
one 868,352-byte array, `sHeapArea` ([frame.md](systems/frame.md#the-heap)).

## The source by subsystem

The units are linked in the order `config/lain/SLPS.LNK` lists them, which reproduces the original's layout:
`main.c` first and `loading.c` last, with Sony's libraries after them.

### Boot and the frame

| Unit | Holds |
| --- | --- |
| `main.c` | `main`, the vertical-blank handler `HandleVSync`, and three boot helpers nothing calls |
| `heap.c` | The game's heap: first-fit blocks behind tagged headers |
| `host_log.c` | The host log's routines, all empty |
| `pad.c` | LIBPAD's set-up and the per-frame read of the first controller into one pad event |

### Graphics and text

| Unit | Holds |
| --- | --- |
| `graphics.c` | The display, the ordering tables and packet areas, a frame's start and end, the `Model` pools and helpers, TIM upload, views, lights, palette fades, Bezier curves, morphs; the grid's fixed views, paths and model tables |
| `text.c` | The two font faces, text written into video memory, the cursor's caption |
| `string_tables.c` | The titles and words that nodes index |
| `vram_dump.c` | Writing a rectangle of video memory to the development host as a TIM; nothing calls it |

### Files and the discs

| Unit | Holds |
| --- | --- |
| `cd.c` | The 24-slot read queue and its LIBCD callbacks |
| `file_parts.c` | The part tables of `LAPKS.BIN`, `BIN.BIN`, `SND.BIN`, `VOICE.BIN`, `SITEA.BIN` and `SITEB.BIN` |
| `napk.c` | Unpacking napk data |
| `disc_id.c` | Watching the drive's lid and telling the discs apart by `LAIN_1.INF` and `LAIN_2.INF` |
| `disc_change.c` | The disc-change screen and the switch of the file table |
| `loading.c` | The shared images' loading sequence, the loading animation, the P2 screen |

### The grid

| Unit | Holds |
| --- | --- |
| `grid_menu.c` | `RunGridMenu`'s 30 states: input, the cursor, Jump To, opening each kind of node; the progress counters |
| `grid_scene.c` | The 3D cylinder of levels: building its cells, the camera paths, the cursor highlight, cells in flight |
| `grid_effects.c` | The ring and its burst, the orbiting quad, the horizon and its streaks |
| `grid_nodes.c` | Placing the 716 nodes on the levels, their opened flags, nodes that share a word |
| `nodes.c` | The node table `gNodes`, and the grid's heading table |

### Media, sound and speech

| Unit | Holds |
| --- | --- |
| `media_player.c` | The media player: movies and XA tracks, the word ring, track images, the end roll; the disc file tables |
| `media_cube.c` | The media player's Play/Exit cube |
| `clip.c` | Decoding a `LAPKS.BIN` clip's frames through the MDEC, with their masks; the vertical-blank count |
| `lapks_player.c` | The clip slots and their reads, and the heap's area |
| `sound.c` | LIBSND: the bank, the two sequences, sound effects and music |
| `speech.c` | Joining `VOICE.BIN` syllables into the player's spoken name |
| `voice_screen.c` | The voice screen: the spoken name, the XA track and the track image's tiles |

### Menus and screens

| Unit | Holds |
| --- | --- |
| `kana_entry.c` | The start screen and the kana name entry |
| `line_effect.c` | The start screen's sixteen fading lines |
| `system_menu.c` | The system menu, its prompt and message window, the save and load steps, the sprite-quad sorters other screens use |
| `memcard.c` | The memory card calls, packing and unpacking the save; the gate screen |
| `about.c` | The About screen, and loaders of `BIN.BIN` images other units use |
| `upgrade.c` | The upgrade screen |
| `continue_prompt.c` | The prompt after the end roll |
| `special_scenes.c` | The spheres after the end roll and the gate scene |

### Data

| Unit | Holds |
| --- | --- |
| `tim_data.s`, `tim_data_3.s` to `tim_data_6.s` | TIM images linked into the executable: the horizon, the ring, level labels and digits, cell icons, the cursor highlight, the cube's pieces, the gate's slabs and backdrop |
| `band_tmd.c`, `level_label_tmd.s`, `ring_tmd.c`, `vertical_bars.c`, `media_cube_tmd.c`, `tmd_data_5.c` | TMD models linked into the executable: bands, level labels, the ring, the background bars, the cube's pieces, the cells and their pieces, the spheres, the gate scene's slabs |
| `kana_data.s` | The kana-to-romaji tables |

Sony's Runtime Library is under `src/lib/`, one directory per library ([building.md](building.md)).

## Game terms

The program's own words, used across the source and these documents:

- **Site:** one disc's grid, Site A on disc 1 and Site B on disc 2 (`gCurrentDisc` 0 or 1).
- **Level:** one layer of a Site, three rows of eight cells; Site A has 22, Site B 13 (`GetGridLevelCount`).
- **Node:** one entry of `gNodes[716]`, placed on a cell, opened from the grid, its state saved.
- **Gate:** one of the four kind-8 nodes; opening all four permits a Site change.
- **Clip:** a short masked animation from `LAPKS.BIN`, decoded by `src/clip.c`. A **movie** is a streamed `.STR`
  file from `MOVIE/` or `MOVIE2/`.
- **Media player:** the screen that plays a media node's movie or XA track.
- **Pad event:** the one code `ReadPadEvent` leaves in `gPadEvent` each frame.
- **Music:** one of the two LIBSND sequences in `SND.BIN`.
