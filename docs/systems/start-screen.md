# The start screen and the name entry

`main` runs the start screen (`RunStartScreen`) before the grid, at boot and again whenever the grid ends without
going on. It offers two choices: the first opens the kana name entry (`RunNameEntry`) and starts a new game with the
name spelled there, the second loads a save from the memory card. Both are in `src/kana_entry.c`, with the screen's
records in `include/kana_entry.h` and the romaji tables in `src/kana_data.s`; the sixteen fading lines behind the
start screen are `src/line_effect.c`.

## The start screen

`RunStartScreen` keeps the 256 by 32 area of VRAM at (320, 480) aside while it runs and puts it back when it ends.
It first calls `StepTimLoadSequence(0)` (`src/loading.c`) until it finishes, which uploads the screen's pictures
from `BIN.BIN` and keeps part 14 in memory as `gStripTim`. Its sprites are built from `gStartScreenSprites`. The
screen is a state machine in one loop, a frame per pass (`enum StartScreenState`):

| State | Value | What happens |
| --- | ---: | --- |
| `START_OPENING` | 0 | the opening: the line trails, its pictures and a spinning quad, no input |
| `START_SHOW_CHOICES` | 1 | 33 frames: the opening fades and the two choices appear |
| `START_CHOOSE` | 2 | the choice: up for the first, down for the second, circle to take it |
| `START_LOAD_OPEN` | 3 | 33 frames out into the load confirmation |
| `START_LOAD_CONFIRM` | 4 | "Are you sure?" prompt for Load |
| `START_LOAD` | 5 | the memory card load (`enum MemCardLoadStep`) |
| `START_LOAD_CLOSE` | 9 | 33 frames back to `START_CHOOSE` on No or a failed load |
| `START_NAME_ENTRY_OPEN` | 10 | 33 frames out into the name entry |
| `START_NAME_ENTRY` | 11 | runs `RunNameEntry()` |
| `START_NAME_ENTRY_CLOSE` | 12 | 33 frames back to `START_CHOOSE` if the name entry was left empty |

In `START_OPENING`, the picture at (192, 205) keeps its first image (`OPENING_FLASH_BASE` (0) in `enum OpeningFlash`),
and at moments chosen at random, 10 to 49 frames apart, one of two sequences of other images (`OPENING_FLASH_SEQ_A`
(1) or `OPENING_FLASH_SEQ_B` (2)) replaces it for a single frame; when either sequence has flashed five times the
screen goes on. From `START_SHOW_CHOICES` two semi-transparent strips stand slanted across the screen
(`ScrollStripQuads`), showing `gStripTim` scrolling up through VRAM at (448, 256), two lines a frame.

The question of the second choice uses the system menu's texts and panel, uploaded again to the same place in VRAM
([system-menu.md](system-menu.md)), but its answer starts on Yes. Left chooses Yes, right No, circle answers (sound
26 for Yes, 27 for No). Yes runs the card steps of a load ([memory-card.md](memory-card.md)), showing their messages
with `DrawMessageWindow`; a failure shows its message for 60 frames (a refused save for 30) and returns to the
choice. A save that loads
shows the loaded message for 30 frames and ends the start screen: `main` goes into the grid where the save left it.

The name entry ends the start screen when it returns with a name; left empty, it returns to the choice.

## The line trails

`InitLineTrails` sets up sixteen trails, each a point that loops out from an origin near (160, 100) and back. A
loop is the curve origin + t²(1 − t)·c0 + t(1 − t)²·c1 for t from 0 to 1 in 256 steps (`AdvanceTrailPoint`), with the
two control points `sTrailControl` drawn at random for each loop (`SeedTrailCurve`) and the first one mirrored
on every other loop. `StepLineTrails` advances every trail four steps a frame, so a loop takes 64 frames, the
trails' phases a sixteenth of a loop apart. Each trail keeps its last 256 points in a ring and draws the latest
128 as Gouraud lines (`DrawLineTrail`), blue fading to black along the trail. Only the start screen uses them.

## The name entry

`RunNameEntry` keeps the entry's state, a `KanaEntry`, on its stack: `InitKanaEntry` sets it up, `RunKanaEntryMenu`
runs it until it returns, and `RomanizeKanaText` writes the result into `gPlayerName` whichever way it ended.

### The chart

The kana chart has 13 rows of up to five kana, the cursor at `row` and `col`. Row and column decide a kana's code,
which is what `text` holds:

| Row | Kana | Columns | Codes |
| --- | --- | --- | --- |
| 0 to 6 | あ, か, さ, た, な, は and ま rows | all five | row × 5 + column, 0 to 34 |
| 7 | や ゆ よ | 0, 2, 4 | 35 to 37 |
| 8 | ら row | all five | 38 to 42 |
| 9 | わ and ん | 0 and 4 | 43, 44 |
| 10 | the small vowels | all five | 75 to 79 |
| 11 | the small ゃ ゅ ょ | 0, 2, 4 | 46 to 48 |
| 12 | four signs | 0, 2, 3, 4 | see below |

On screen the rows run from right to left, as in a printed kana chart, and the five kana of a row from top to bottom;
so up and down move between the kana of a row, left goes to the next row and right to the one before, and the
diagonals do both. The moves skip the columns rows 7, 9, 11 and 12 lack (`IncrementKanaRow` and the moves in
`RunKanaEntryMenu`).

`DrawKanaChart` draws a nine by nine window of the chart around the cursor, the cursor in the window's fourth row
and fifth column. The cells are quadrilaterals between the points of a 10 by 10 lattice of screen positions
(`gKanaGridX`, `gKanaGridY`), which curves the chart across the screen, and they dim with distance from the cursor
(`gKanaCellShades`). The cursor's kana is drawn in a larger fixed quadrilateral, the highlight; the cell above it is
not drawn. The chart's picture is three texture pages, 5, 6 and 7, five rows to a page, each row a column 48 texels
wide with its kana 32 texels apart.

### Entering kana

Circle adds the kana under the cursor (`ApplySelectedKana`, sound 0); cross removes the last one (sound 27), or,
with none entered, leaves the entry; Start, with at least one entered, plays sound 30 and accepts the name. At most
eight kana are taken. `ApplySelectedKana` refuses what cannot stand where it would go:

- ん, the small kana and the signs of row 12 cannot come first.
- A small vowel follows only ふ (any of the five), で (only ぃ) and ど (only ぅ).
- A small ゃ, ゅ or ょ follows only き, し, ち, に, ひ, み, り, ぎ, じ, び and ぴ. The list also names ぢ, which cannot
  be entered.
- Row 12's signs change or follow the last kana: column 2 voices it (the か and さ rows, た, て and と, and the は
  row; not ち or つ), column 3 makes the は row ぱ, column 0 adds a sign spelled `#` (not twice in a row), and column 4
  one spelled `-` (not after ん, `#` or itself).

`DrawKanaText` draws the kana entered as 16 by 16 glyphs from texture page 22, ending at x 120 with a cursor glyph
after them, or at x 136 with no cursor once eight are entered. Eight tiles ring the highlight (`gKanaTilePositions`);
the one at position 7 − count is left out, so the gap moves round the ring as kana are entered and closes at eight.

## The name it produces

`RomanizeKanaText` spells the codes in capital romaji, letters taken from the four tables of `src/kana_data.s`,
followed by a zero byte:

- Spellings follow the system of し SI, ち TI, つ TU, ふ HU and じ ZI; ん is NN; an i-column kana followed by a small
  ゃ, ゅ or ょ becomes one syllable (KYA, SYU, RYO, ZYA).
- ふ, で and ど followed by a small vowel are spelled F with the vowel, DI and DU.
- The voiced codes are spelled with G, Z, D, B and P, but the range it tests ends at ぱ: ぴ, ぷ, ぺ and ぽ, and a small
  ゃ, ゅ or ょ after ぴ, add nothing to the name.
- The two signs of row 12 are spelled `#` and `-`.

Eight kana make at most 16 letters. The name is read in two places: the card save keeps its first 20 bytes
([memory-card.md](memory-card.md)), and the voice screen builds speech from its romaji (`BuildSpeechPcm`,
[speech.md](speech.md)).
