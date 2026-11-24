# Names

The executable keeps no symbols, so almost every name in the game's own code was chosen here. A name in this tree
is one of three things:

- **Sony's,** for everything the development kit defines: the library routines and globals as the kit's archive
  objects spell them, and the types, macros and constants of its headers (`GsSPRITE`, `CdlLOC`, `POLY_FT4`).
- **Spelled by the image,** where the executable's own bytes give the name (below).
- **A descriptive name chosen here,** which says what the routine, global, record or field is or does, as far as
  that is known. It makes no claim about what the authors called it. Where all that is known is that nothing
  reads a thing, the name says so: a field `unused`, a global `gUnusedNapkWord1` or `gCdUnreadWord`.

The order of the uninitialized globals in the executable follows the authors' names for them
([building.md](building.md)), but no name of the game's code is recovered from it.

## What the image spells

A few strings in the executable name the code that prints them, and the tree keeps or follows them:

- **`PurgeIdle`**: the clip player's routine that frees idle clips to make room prints its own name
  (`FntPrint("PurgeIdle\n")`, `src/lapks_player.c`), and keeps it.
- **`malloc::` and `free::`**: the heap's messages (`src/heap.c`) begin with these. The game also links LIBC2,
  which defines `malloc` and `free` (`src/lib/c2/malloc.s`), so the tree's routines are `AllocHeapBlock` and
  `FreeHeapBlock`. The log line
  `left memory %d` (`src/main.c`) gives the heap's `GetLeftMemory`.
- **The host log**: `main` passes `OpenHostLog` (`src/host_log.c`, where every routine is empty) the path
  `d:\usr\tmp\psx.log`, on the development PC, and the system
  menu's version line is `Application Version %d.0` (`RenderMenuText`).
- **The game's data names itself**: every node of `gNodes` (`src/nodes.c`) carries a name the game shows
  (`Cou001`, `Dc1001`, `GaTE01`, ...), the save file is `BISLPS-01603LAINdata`, and the disc's file names
  (`LAPKS.BIN`, `SITEA.BIN`, ...) give the routines and tables that read them their words (`gLapksParts`,
  `gSiteAParts`).

## Conventions of the descriptive names

- Routines are `PascalCase`, verb first: `RunGridMenu`, `UnpackNapk`, `LoadSpeechClip`, `PollCdRequest`. A
  routine that runs a screen until it ends is `Run...`; one that does a frame's work is `Update...` or
  `Step...`; one that tests a request is `Poll...`.
- Globals are `g` followed by `PascalCase` (`gNodes`, `gTrackIndex`), every one of the game's 626; file-local
  statics are `s` followed by `PascalCase` (`sCdRequests`, `sHeapArea`), every one of its 59.
- Records are `PascalCase` typedefs (`Node`, `Track`, `DiscFile`, `FilePart`, `CardSaveData`); fields and locals
  are `lowerCamelCase`; constants and macros are `UPPER_SNAKE_CASE` (`CD_REQUEST_COUNT`, `VOICE_NAME_COUNT`,
  `CARD_SAVE_SIGNATURE`).
- An array of the game's own data is named for what it holds, with its format last: `gHorizonTim`,
  `gLabelBandTmd`, `gLargeSphereTmd`. A table of a disc file's parts is `g<File>Parts`.
- A file of `src/` is named for what it holds (`media_player.c`, `memcard.c`, `band_tmd.c`); a file whose
  object holds only images or models no one screen owns is numbered, in the original's object order
  (`tim_data.s`, `tim_data_3.s` to `tim_data_6.s`, `tmd_data_5.c`).
- "previous" is spelled out in every name, never shortened.

## The program's words

The game's own words, as the code uses them. Where a word could be confused with another, both are given.

| Word | Means | Read |
| --- | --- | --- |
| site | One half of the game: Site A is disc 1, Site B disc 2 (`LAIN_1.INF`, `LAIN_2.INF`). Each has its own grid, its own images (`SITEA.BIN`, `SITEB.BIN`) and its own movies. | `EnsureSiteDisc`, `gSiteParts` |
| level | One layer of a site's grid: 22 on Site A and 13 on Site B (`GetGridLevelCount`), each of three rows of eight cells, shown as "Site A level." on screen. Not a floor or a page. | `gGridCells`, `gSiteALevelTim` |
| cell | One place of a level, holding at most one node. | `GridCell`, `gGridCells` |
| node | One of the 716 entries of `gNodes`: its name, four titles, three words, three image parts, its track, its place on the grid and its state. | `Node`, `include/nodes.h` |
| word | One of the 417 strings of `gWords`; a node has up to three (-1 for none). | `gWords`, `RunGridMenu` |
| title | One of the 51 strings of `gTitles`; a node has four, drawn on its caption. | `gTitles`, `SetCellCaption` |
| caption | The plate of titles drawn for the cell under the cursor. | `SetCellCaption`, `DrawCaption` |
| gate | One of the four nodes `GaTE01` to `GaTE04` (kind 8); the system menu's site change is refused until all four are opened. | `RunGateScreen`, `gGatesOpened` |
| upgrade | A step of `gUpgradesApplied`; a node's flags carry an upgrade count compared with it. | `RunUpgradeScreen` |
| track | One of the 736 rows of `gTracks`: a movie or an XA channel. A node's track below 518 plays in the media player and one from 518 to 698 on the voice screen; 699 to 732 are idle media, 733 follows the end roll on the voice screen and 735 is the end roll. | `Track`, `gTrackIndex` |
| media player | The screen that plays a node's track below 518 (`src/media_player.c`): the movie or audio, the word ring, the track images and the Play and Exit controls. | `RunMediaPlayer`, `PlayMediaTrack` |
| voice screen | The screen for tracks 518 to 698 and for 733 after the end roll (`src/voice_screen.c`): it plays the XA track and animates the track image, and below 639 and on 733 it first speaks the player's name with the voice clips. | `RunVoiceScreen` |
| end roll | The closing movie (`MOVIE2/ENDROLL1.STR`), counted in `gEndRollCount` (up to 9,999). | `PlayEndRoll` |
| end roll spheres | The 3D scene of one large and five small spheres on track 733. | `InitEndRollSpheres`, `src/special_scenes.c` |
| movie | A streamed STR video from `MOVIE/` or `MOVIE2/`. Not a clip. | `MovieDecoder`, `src/media_player.c` |
| clip | One animation of `LAPKS.BIN`, masked and decoded a frame at a time. Not a movie. | `src/clip.c`, `ClipSlot` |
| sequence | A batch of one to six clips played back to back. Not music. | `PlaySequenceClips`, `sSequenceSlots` |
| music | One of the two sequences of `SND.BIN`: the grid's and the About screen's. | `gGridMusicSeq`, `gAboutMusicSeq`, `PlayMusic` |
| part | One piece of a disc archive, by its row in the file's part table. | `FilePart`, `src/file_parts.c` |
| speech | PCM built from the voice clips for a romaji string. | `BuildSpeechPcm` |
| kana entry | The screen where the player spells a name in kana, kept in romaji in `gPlayerName`. | `RunKanaEntryMenu` |
| system menu | The menu of Load, About, Site (Change), Save and Exit. | `UpdateSystemMenu`, `RenderMenuText` |
| About screen | The system menu's About item: scrolling images with their own music. | `RunAboutScreen`, `src/about.c` |
| continue prompt | The end-or-continue screen after the end roll. Not the disc change prompt. | `RunContinuePrompt` |
| disc change | The prompt to insert the other site's disc, and the swap of the file table. | `src/disc_change.c` |
| pad event | The code `ReadPadEvent` leaves in `gPadEvent` for a new press: directions 1 to 8, then the shoulder, stick and face buttons. | `ReadPadEvent` |
| MemCard / CardSave | `MemCard...` is the card input and output (after LIBMCRD's prefix); `Card...` is the save block's layout. | `include/card.h`, `src/memcard.c` |
| tpage | A GPU texture page, as LIBGPU's `GetTPage` returns it. Not a level. | `gPlayerTPage` |
| SpriteQuad | The rotatable textured quad record. Not the grid's single orbiting quad effect (`gTexturedQuad`). | `include/sprite_quad.h` |

## Sony's names

- **Library routines and globals** are those the Runtime Library 4.3 objects export, and its static routines
  and data keep the names the objects carry. Static data, inlined helpers and records that no object names, and
  the types, macros and constants the kit's headers do not define, have names chosen here, in the library's
  own style; they are declared in internal headers (`*_int.h`). The kit's headers are included, never
  copied.
- **LIBPAD's** records `PadState` (a controller's state, one per port) and `SerialRegisters` (the controller
  port's registers as LIBPAD reads them) are named here, in `src/lib/libpad/pad_int.h`.
- **NITRBS**, the NITR-BS decoder, is the game's own library and no kit object names it: `UnpackNitrBsFrame`,
  `BuildSignedRunLevelCodes` and its tables are named here.
- **The startup**, `src/lib/obj/2MBYTE.s`, exports the names the kit's `2MBYTE.OBJ` exports
  (`__SN_ENTRY_POINT`, `__main`, `stup0` to `stup2`).
