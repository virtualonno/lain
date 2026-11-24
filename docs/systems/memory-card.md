# The memory card

The game keeps one save file, `BISLPS-01603LAINdata`, on the card in the first port. Both discs write the same
name, so a save made on either disc is found on the other. The calls to Sony's memory card library (`libmcrd`)
and the packing and checking of the save are in `src/memcard.c`; the steps that drive them a frame at a time are
in the system menu (`UpdateSystemMenu`, `src/system_menu.c`, [system-menu.md](system-menu.md)) and the start
screen (`RunStartScreen`, `src/kana_entry.c`, [start-screen.md](start-screen.md)). The save's records are in
`include/card.h`; the byte layout of the file is also in [formats.md](../formats.md).

`src/memcard.c` also holds the gate screen (`RunGateScreen`, [scenes.md](scenes.md)) and the wide digits the save's
title is written with.

## The library calls

`InitMemCard`, called once by `main`, starts the library (`MemCardInit(0)`, `MemCardStart`). The other wrappers
work on channel 0, and those that answer return the library's error code negated, so a caller switches on
`-result` and the cases read as the error values of `libmcrd.h`: 1 `McErrCardNotExist`, 2 `McErrCardInvalid`,
3 `McErrNewCard`, 4 `McErrNotFormat`, 5 `McErrFileNotExist` and 7 `McErrBlockFull`.

| Wrapper | Library call | Its answer |
| --- | --- | --- |
| `AcceptMemCard` | `MemCardAccept(0)` | polled |
| `OpenMemCardFile(forWrite)` | `MemCardOpen` of the save, mode 1 to read or 2 to write | returned |
| `CreateMemCardFile` | `MemCardCreateFile` of the save, one block | returned |
| `FormatMemCard` | `MemCardFormat(0)` | returned |
| `ReadMemCardFile`, `WriteMemCardFile` | `MemCardReadData`, `MemCardWriteData` from offset 0 | polled |
| `CloseMemCardFile` | `MemCardClose` | none |
| `PollMemCardSync` | `MemCardSync(1, ...)` | |

`PollMemCardSync` returns 0 while the last polled call is still running, 1 when it finished without error, and
otherwise the negated error. So the accept, the read and the write each take one step to start and then a step
per frame polling until the answer is not 0; the open, create and format answer when they return. `UnformatMemCard` and `AcceptMemCardAndWait` (an accept that
spins on `MemCardSync` until it finishes) are defined and called by nothing.

## What is saved

The save is a `CardSaveBlock`: 8 KB, one card block, a 512-byte `CardHeader` followed by `CardSaveData`. Only its
first 28 pages of 128 bytes (3,584 bytes) are written and read; the rest of the block is never touched.

The header starts as a copy of `gCardHeaderTemplate`: the card file's "SC" mark, 0x13 (three icon frames), one
block, and from byte 96 the icon's 16-colour palette and its three 16 by 16 frames. `PackCardSaveBlock` then writes
the title over it from byte 4, in Shift-JIS:

- the prefix `gCardTitlePrefix`, 連続ゲイム小説［ｌａｉｎ］　第, or, once in 70 saves at random,
  `gCardTitlePrefixRare`, 連続ゲノム小説［ｌａｉｎ］　第;
- the number of the coming load (`gLoadCount` + 1) in full-width digits, then 回：;
- one of four counters (`enum CardTitleStat`) chosen at random: `CARD_STAT_TOKIMEKI` (0) ときめき度 N％
  (`gTokimekiPercent`), `CARD_STAT_DOKUDENPA` (1) 毒電波 N つ (`gDokudenpaCount`, falling through to the next if it is
  0), `CARD_STAT_HARUMAGE` (2) はるまげ度 N％ (`gHarumagePercent`) or `CARD_STAT_GAKKURI` (3) がっくり度 N％
  (`gGakkuriPercent`).

The four counters are counted by the grid menu during play and cleared each time the grid is entered
(`RestoreGridCursor`, or `InitGridState` before a new game); they appear only in the title and are not saved.
`AppendWideNumber` writes the digits from `gWideDigits`: it caps the value at 9999 and tests each digit for zero
alone, so a zero inside a number is dropped (the 101st load reads 第１１回) or, where the counters ask for padding,
written as a full-width space, as leading zeros are (100％ reads １　０％).

`CardSaveData` follows at byte 512:

| Field | From | Notes |
| --- | --- | --- |
| `signature` | `CARD_SAVE_SIGNATURE` | 0x4C41494E, the bytes "NIAL" |
| `name` | `gPlayerName` | the first 20 bytes of the romaji name ([start-screen.md](start-screen.md)) |
| `loadCount` | `gLoadCount` | |
| `currentDisc` | `gCurrentDisc` | 0 or 1, the site being played |
| `gatesOpened`, `p2NodesOpened`, `upgradesApplied`, `endRollCount` | the globals of those names | |
| `cursorColumn`, `cursorRow` | `gCursorColumn`, `gCursorRow` | the grid cursor on the current disc |
| `otherDiscCursorColumn`, `otherDiscCursorRow` | the other disc's cursor | |
| `nodes[716]` | `gNodes` | four bytes of each node |

The four bytes of a node are those at offset 34 of its `Node` (`include/nodes.h`): the upper half of `flags`
(its upgrade count, kind, shown and opened bits and end roll threshold) and `unlockedBy`. Nothing writes
`unlockedBy` during play, so loading it puts back the values it already had.

## How it is checked

`PackCardSaveBlock` clears the block, fills it, and then gives each of the first 27 pages (the header's four and
the 23 that `CardSaveData` spans) a check byte, the XOR of its 128 bytes, stored in order from byte 3,456, in the
28th page. It returns 28, the page count, which the callers shift left by 7 for the byte count to transfer.

`UnpackCardSaveBlock` recomputes the 27 check bytes and refuses the save (returns 0) at the first that differs,
then refuses it unless the signature is right. A save it accepts is copied back into the name, the counts, the two
cursors and the nodes; then `gLoadCount` is raised by one (while below 9999) and `gShowAllTitles` is cleared. The
title is not checked beyond its page bytes, and nothing else in the save is range-checked.

To read, both callers call `PackCardSaveBlock` on the empty buffer first, only for its page count, and read over
what it packed.

## Save

The system menu's Save item, once its confirm prompt is answered yes, runs the save steps in `gMemCardStep`
(`enum MemCardSaveStep`). A failure plays sound 28, shows a message for 120 frames (`gMenuCounter`) and returns to
`MENU_CHOOSE`; which `MessageId` each answer shows is in [system-menu.md](system-menu.md).

| Step | What it does |
| --- | --- |
| `MEMCARD_SAVE_ACCEPT` (0), `MEMCARD_SAVE_POLL_ACCEPT` (1) | accept the card and poll; `McErrNewCard` restarts, `McErrNotFormat` goes to the format check |
| `MEMCARD_SAVE_OPEN` (2) | open the save for writing: `McErrNone` asks to overwrite, `McErrFileNotExist` creates it |
| `MEMCARD_SAVE_SHOW_OVERWRITE` (3), `MEMCARD_SAVE_CONFIRM_OVERWRITE` (4) | show the overwrite prompt while polling card presence; Start confirms, any other button cancels |
| `MEMCARD_SAVE_START_WRITE` (5), `MEMCARD_SAVE_POLL_WRITE` (6), `MEMCARD_SAVE_DONE` (7) | allocate the 24 KB `gMemCardBuffer`, pack and write 28 pages, free and close, then show saved for 120 frames |
| `MEMCARD_SAVE_WAIT_OVERWRITE_SYNC` (8), `MEMCARD_SAVE_RETRY_OVERWRITE_ACCEPT` (9) | finish a pending accept after Start, or wait 5 frames and re-accept after a non-success poll |
| `MEMCARD_SAVE_ERR_NO_CARD` (10), `MEMCARD_SAVE_ERR_INVALID` (20), `MEMCARD_SAVE_ERR_FULL` (50) | show no card, invalid card, or no free block for 120 frames |
| `MEMCARD_SAVE_SHOW_FORMAT` (30), `MEMCARD_SAVE_CONFIRM_FORMAT` (31), `MEMCARD_SAVE_FORMAT` (32) | ask to format (Start confirms, any other cancels), run `FormatMemCard`, then create |
| `MEMCARD_SAVE_FORMAT_CHECK_INIT` (35), `MEMCARD_SAVE_FORMAT_CHECK_ACCEPT` (36), `MEMCARD_SAVE_FORMAT_CHECK_POLL` (37) | accept again until `McErrNotFormat` is answered five times before prompting |
| `MEMCARD_SAVE_WAIT_FORMAT_SYNC` (38), `MEMCARD_SAVE_RETRY_FORMAT_ACCEPT` (39) | finish a pending accept after Start on the format prompt, or wait 5 frames and re-accept |
| `MEMCARD_SAVE_CREATE` (40), `MEMCARD_SAVE_OPEN_CREATED` (41) | create the one-block save (`McErrBlockFull` fails), open it for writing and jump to `MEMCARD_SAVE_START_WRITE` |
| `MEMCARD_SAVE_CANCEL_SYNC` (90) | wait for a pending accept to finish after cancel, close the file and return to `MENU_CHOOSE` |

## Load

The system menu's Load item, once confirmed, and the start screen's load run the same steps (`enum MemCardLoadStep` in
`include/memcard.h`), each in its own copy (`gMemCardStep` and `gMenuCounter` in the system menu, `sub` and `waitCount`
in `RunStartScreen`):

| Step | What it does |
| --- | --- |
| `MEMCARD_LOAD_ACCEPT` (0), `MEMCARD_LOAD_POLL_ACCEPT` (1) | accept the card and poll; `McErrNewCard` restarts, success jumps to `MEMCARD_LOAD_OPEN` |
| `MEMCARD_LOAD_OPEN` (100), `MEMCARD_LOAD_START_READ` (101) | open the save for reading (`gMenuCounter = 10`), allocate the 24 KB buffer and start reading 28 pages |
| `MEMCARD_LOAD_POLL_READ` (2) | poll the read and unpack: accepted advances to `MEMCARD_LOAD_DONE`, refused to `MEMCARD_LOAD_ERR_CORRUPT`; `McErrCardNotExist` counts down from 10 and restarts from `MEMCARD_LOAD_ACCEPT` (never running out because open resets the count to 10) |
| `MEMCARD_LOAD_DONE` (3), `MEMCARD_LOAD_ERR_CORRUPT` (4) | show loaded (ending the load) or corrupt save, and free the buffer |
| `MEMCARD_LOAD_ERR_NO_CARD` (10), `MEMCARD_LOAD_ERR_INVALID` (20), `MEMCARD_LOAD_ERR_UNFORMATTED` (30), `MEMCARD_LOAD_ERR_NO_FILE` (40) | show the matching error message and return |

After a load from the system menu, `UpdateSystemMenu` returns `MENU_RESULT_LOAD` and the grid menu returns
`GRID_MENU_EXIT_LOAD` to `main`, which enters the grid again at the loaded cursor; if the disc in the drive is not the
loaded `currentDisc`, `EnsureSiteDisc` asks for the other one first ([files.md](files.md)). After a load from the start
screen, `RunStartScreen` ends and `main` goes on into the grid the same way.
