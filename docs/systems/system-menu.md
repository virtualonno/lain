# The system menu

Triangle (or select) on the grid opens the system menu over it: a 7 by 7 square of tiles that flip over to spell
one of five items, Load, About, Change, Save and Exit. It lives in `src/system_menu.c` with its records in
`include/system_menu.h`; the grid menu drives it (`RunGridMenu`, `src/grid_menu.c`,
[grid-menu.md](grid-menu.md)), and the memory card steps it runs for Save and Load are described in
[memory-card.md](memory-card.md). The same unit holds the message window the card steps draw, the tile grid behind
the media player, and the two routines that draw a `SpriteQuad`, which most screens use.

## Drawing a SpriteQuad

A `SpriteQuad` (`include/sprite_quad.h`) is a `POLY_FT4` with the fields to build it: position and size, texture
page, palette and texel corner, a colour, a scale per axis (4096 is 1) and three angles (4096 to a turn).
`attribute` bit 22 mirrors it vertically, bit 23 horizontally, and bit 30 makes it semi-transparent.

- `SortSpriteQuad` fills the polygon as an upright rectangle and adds it to the ordering table at the given entry.
- `SortRotatedSpriteQuad` scales the four corners, rotates them about the centre by the three angles, and divides
  them by depth, with the eye 350 units in front of the nearest corner and the centre's depth drawn at its own
  size, so a tile turned about its Y axis narrows like a card being flipped. The rotation goes through the
  library's matrix routines (`RotMatrix_gte`, `ApplyRotMatrixLV`).

## The tiles

`LoadMenuTileImage`, called once at boot, reads part 2 of `BIN.BIN`, a napk-packed TIM
([formats.md](../formats.md)), into VRAM at (640, 456) with its palette at (0, 483). The tiles' faces are 16 by 16
cells of it: `gTileNumbers` gives each of the 49 tiles one of 16 cells, and `gTileMirror` mirrors some, so that
the square's pattern is symmetric about both axes. Each tile has a back, a 14 by 14 square of the same image,
spaced 21 pixels apart where the faces are 24.

`StepSystemMenuOpen` allocates a block of 128 `SpriteQuad`s (`gMenuQuads`) from the heap, uploads the menu's texts
(`RenderMenuText`) and sets up the quads (`InitMenuQuads`):

| Quads | What |
| --- | --- |
| 0, 1 and 2 | "Are you sure?" and the two halves of the panel behind it |
| 3, 4 and 5 | "Yes", "No" and the highlight over the answer chosen |
| 6 and 7 | a bar and "Application Version N.0" below the square |
| 8 and 9 | "Permission " and "denied" |
| 15 to 22 | the letters of a label that run past the square's right edge, two banks of four |
| 23 to 71 | the 49 tiles' faces |
| 72 to 120 | their backs |

`RenderMenuText` draws the labels into VRAM with `UploadTextToVram` (`src/text.c`): "Site", "Save" and "Exit" in a
column at x 704 and "Load", "About" and "Change" at x 728, 24 lines apart. N in the version line is one more than
`gUpgradesApplied`, so it rises with each upgrade ([scenes.md](scenes.md)). No tile ever shows "Site".

## The items

`gMenuItem` selects one of the five items (`enum MenuItemId`, with `MENU_ITEM_NONE` (-1) when none is excluded or
selected). `gMenuItems` gives each item the tile its label starts on, its length and where its label lies in VRAM:

| Item | Label starts at | Letters past the edge |
| --- | --- | --- |
| `MENU_ITEM_LOAD` (0) Load | row 1, column 1 | none |
| `MENU_ITEM_ABOUT` (1) About | row 2, column 5 | 3 |
| `MENU_ITEM_CHANGE` (2) Change | row 3, column 3 | 2 |
| `MENU_ITEM_SAVE` (3) Save | row 5, column 1 | none |
| `MENU_ITEM_EXIT` (4) Exit | row 6, column 5 | 2 |

A label is one letter to a tile, each tile showing a 16-pixel slice of the text; letters past column 6 go into the
overflow quads, which stand in a row to the right of the square. `FlipItemToLabel` turns an item's tiles over to its
label and `FlipItemToTiles` back, in 17 frames: about the Y axis to half a turn and back while spinning once about X,
the image changing at frame 9. While one item turns to tiles and the next to its label, each uses its own overflow
bank (`gOverflowBank`).

## Opening and closing

`StepSystemMenuOpen` is called once a frame until it returns 1, counting in `gMenuFrame`:

- frames 0 to 16: the tiles and their backs spread from the centre of the screen into the square, spinning;
- frames 17 to 81: the tiles flip over a diagonal at a time, from the bottom left corner to the top right, as
  `gTileFlipDelay` staggers them four frames apart; halfway through its flip the first tile of each item takes the
  first letter of its label;
- frames 82 to 98: Change turns to its label, and the menu starts with it chosen.

`StepSystemMenuClose` turns the chosen item back to tiles, then spreads the square outwards for 16 frames while the
tiles spin, and frees the block.

## The state machine

`UpdateSystemMenu`, called once a frame between the two, reads the pad event (`gPadEvent`, `enum PadEvent`) and acts
on `gMenuState` (`enum MenuState`). It returns a `MenuResult`: `MENU_RESULT_STAY` (0) while the menu stays,
`MENU_RESULT_EXIT` (1) for Exit, `MENU_RESULT_LOAD` (-1) after a load and `MENU_RESULT_CHANGE` (-2) for Change; the
grid menu then closes the menu for Exit, or leaves the grid for the other two ([grid-menu.md](grid-menu.md)).

| State | What it does |
| --- | --- |
| `MENU_CHOOSE` (0) | up and down move to the item before or after; circle chooses |
| `MENU_CONFIRM` (1) | the question: left chooses Yes, right No, circle answers |
| `MENU_MOVE` (2) | the old item flips to tiles and the new one to its label; then back to choosing |
| `MENU_CONFIRM_OPEN` (3), `MENU_CONFIRM_CLOSE` (4) | nine frames with the question drawn, then confirm or choose |
| `MENU_SAVE` (5), `MENU_LOAD` (6) | the card steps, counted in `gMemCardStep` (`enum MemCardSaveStep`, `enum MemCardLoadStep`) |
| `MENU_CHANGE` (7) | returns `MENU_RESULT_CHANGE` |
| `MENU_DENIED` (8) | "Permission denied" for 60 frames, then choosing |

Circle on an item:

- **Load** and **Save** ask "Are you sure?", the answer starting on No. Yes goes to the card steps, No closes the
  question.
- **About** runs the About screen (`RunAboutScreen`, [scenes.md](scenes.md)) at once, without a question, and
  returns to the menu.
- **Change** needs all four gates opened (`gGatesOpened`); with fewer it shows "Permission denied", otherwise it asks
  as Load does. Yes returns `MENU_RESULT_CHANGE` (-2): the grid menu swaps the grid cursor with the other disc's, switches `gCurrentDisc`
  and leaves the grid, and `main` asks for the other disc ([files.md](files.md)).
- **Exit** closes the menu.

`StepConfirmOpen` and `StepConfirmClose` are the same routine twice: neither animates the question. A move plays
sound 1 twice in the same frame, once when the press is read and once when the move starts. `gMenuAnimating` is
set and cleared around the animations, and nothing reads it.

Leaving by Load or Change returns without `StepSystemMenuClose`, so the block of quads is not freed;
`FreeMenuQuads`, which would free it, is called by nothing.

## The message window

`DrawMessageWindow(n)` draws message `n` (`enum MessageId`, 1-based into `gMessageImages`) centred across the screen
with its top at y 156, over as many rows of window as the message says, each row a 144-pixel piece and its mirror
image (`gMessageWindowQuads`). A message is a picture of text in texture page 30 with its palette at (0, 485), not a
string; its first call sets the window up (`InitMessageWindow`). The card steps of the system menu and of the start
screen show:

| Message | Shown |
| --- | --- |
| `MESSAGE_NO_CARD` (1) | no card, or the card taken out |
| `MESSAGE_FORMAT_PROMPT` (3) | the format question |
| `MESSAGE_FORMATTING` (4) | when Start answers it, before the format |
| `MESSAGE_FORMATTED` (5) | one frame, after a successful format |
| `MESSAGE_INVALID_CARD` (6) | an invalid card |
| `MESSAGE_NO_FREE_BLOCK` (7) | no free block for the save |
| `MESSAGE_OVERWRITE_PROMPT_OPEN` (8), then `MESSAGE_OVERWRITE_PROMPT` (9) | the overwrite question: 8 for one frame, 9 while it waits |
| `MESSAGE_SAVING` (10) | while the save is written |
| `MESSAGE_SAVED` (11) | saved, for 120 frames |
| `MESSAGE_NO_SAVE` (13) | no save on the card |
| `MESSAGE_SAVE_CORRUPT` (14) | a save the checks refuse |
| `MESSAGE_LOADING` (15) | while the save is read |
| `MESSAGE_LOADED` (16) | loaded |
| `MESSAGE_UNFORMATTED` (17) | an unformatted card, on loading |

`MESSAGE_UNFORMATTED` is the first line of `MESSAGE_FORMAT_PROMPT`, and `MESSAGE_FORMATTING` the left part of
`MESSAGE_FORMATTED`'s line. Messages 2 and 12 are never shown.

In `MENU_SAVE`, `gMemCardStep` runs `enum MemCardSaveStep` (`MEMCARD_SAVE_ACCEPT` through `MEMCARD_SAVE_CANCEL_SYNC`);
in `MENU_LOAD`, it runs `enum MemCardLoadStep` (`MEMCARD_LOAD_ACCEPT` through `MEMCARD_LOAD_START_READ`), shared with
the start screen's load. Both step tables are given in [memory-card.md](memory-card.md). `gMenuCounter` times the
messages and counts the retries.

## The tile grid of the media player

`InitTileGrid`, `DrawTileGrid` and `FreeTileGrid` set up the same 49 tiles and backs as a block of 98 quads for the
media player ([media-player.md](media-player.md)), drawn into its ordering table at the depth given. The grid menu
sets them up before it plays a node, and the media player before it plays idle audio.
`DrawTileGrid` spreads them with `gMenuFrame` as the menu's opening does, but `InitTileGrid` starts that count at
0x8000 and `DrawTileGrid` keeps the bit set, so the factor it spreads them by is large and negative.
