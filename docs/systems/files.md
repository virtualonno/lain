# Files and the discs

Everything the program reads after boot comes from a handful of large files on the disc, read by sector
through LIBCD: the program finds no file by name while it runs, except the two marker files that tell the
discs apart. The files are archives of parts, and the program knows where each part lies from tables compiled
into it. What is inside each archive is in [formats.md](../formats.md); the movies and XA audio, which the
media player streams with its own calls, are in [media-player.md](media-player.md).

## The two discs

Both discs carry the same executable (`SLPS_016.03` on disc 1, `SLPS_016.04` on disc 2) and the same first
files; they differ in their site archive and their movies and XA audio. The program addresses files by their
row in `gDiscFiles` (`src/media_player.c`): 100 rows of `DiscFile`, a first sector and a size in bytes.

| Row | Disc 1 | Disc 2 | Read through |
| --- | --- | --- | --- |
| 0 | `SLPS_016.03` | `SLPS_016.04` | not read |
| 1 | `SYSTEM.CNF` | `SYSTEM.CNF` | not read |
| 2 | `LAIN_1.INF` | `LAIN_2.INF` | not read by row (below) |
| 3 | `BIN.BIN` | `BIN.BIN` | `gBinParts` (44 parts) |
| 4 | `LAPKS.BIN` | `LAPKS.BIN` | `gLapksParts` (59) |
| 5 | `SND.BIN` | `SND.BIN` | `gSndParts` (4) |
| 6 | `SITEA.BIN` | `SITEB.BIN` | `gSiteAParts` (790), `gSiteBParts` (558) |
| 7 | `VOICE.BIN` | `VOICE.BIN` | `gVoiceParts` (639) |
| 8 on | the movies and XA files | the movies and XA files | the media player |

Rows 0 to 6 lie at the same sectors on both discs; the site archives differ in size, so `VOICE.BIN`, which
follows, starts at sector 10,917 on disc 1 and 10,073 on disc 2. The table's sizes of the two small files are
rounded up to a multiple of four (72 and 20 for 69 and 18 bytes).

Each disc has its own table compiled in, `gDisc1Files` (69 rows) and `gDisc2Files` (57).
`EnsureSiteDisc` copies the one for `gCurrentDisc` into `gDiscFiles`, always 100 rows, so the rows past each
table's end take whatever follows it in memory. Until the first copy, `gDiscFiles` holds disc 1's 69 rows,
compiled in, and zeros after them.

## Which disc is in

`gCurrentDisc` is the disc the game wants: 0 for disc 1 (Site A), 1 for disc 2 (Site B). `InitGridState`
sets it to 0, loading a save sets it from the save, and a Site change from the system menu or continuing after
the end roll flips it.

`IdentifyDisc` (`src/disc_id.c`) looks for `\LAIN_1.INF;1` and then `\LAIN_2.INF;1` with `CdSearchFile` and
answers 0, 1, or -1 for neither. `EnsureSiteDisc` (`src/disc_change.c`), which `main` calls before every run of
the grid menu ([frame.md](frame.md)), skips the screen when the answer is `gCurrentDisc`. Otherwise it shows
the disc-change screen until the right disc is in:

```
loop:
    draw the screen, PollDiscChange()        until it answers other than DISC_POLL_WAIT_LID (each draw is a frame)
    while it answers DISC_POLL_CHECKING, draw "checking in progress"
    DISC_POLL_READY: IdentifyDisc() == gCurrentDisc ?  done : answer DISC_POLL_WRONG_DISC ("WRONG DISC")
    DISC_POLL_WRONG_DISC or DISC_POLL_NO_DISC: show "WRONG DISC" or "No DISC" and go round again
```

`PollDiscChange` (`src/disc_id.c`) watches the drive through blocking `CdControlB` commands and returns a
`DiscPollStatus` (`enum DiscPollStatus` in `include/disc_id.h`), using `SeekToDiscStartInline` (`enum DiscSeekResult`:
`DISC_SEEK_TIMEOUT` (0), `DISC_SEEK_ERROR` (1), `DISC_SEEK_OK` (2), `DISC_SEEK_SHELL_OPEN` (`CdlStatShellOpen`)):

| Status | Meaning |
| --- | --- |
| `DISC_POLL_NO_DISC` (-2) | drive stayed without motor for 300 polls, reported errors for 600, or seek returned `DISC_SEEK_TIMEOUT` |
| `DISC_POLL_WRONG_DISC` (-1) | seek returned `DISC_SEEK_ERROR` (result[1] bit 0x40), or `IdentifyDisc` mismatched `gCurrentDisc` |
| `DISC_POLL_WAIT_LID` (0) | waiting for the lid to open and close (also returned on `DISC_SEEK_SHELL_OPEN`) |
| `DISC_POLL_CHECKING` (1) | drive spinning up and reading the table of contents |
| `DISC_POLL_READY` (2) | seek to the disc's start returned `DISC_SEEK_OK` |

After any terminal status (`DISC_POLL_NO_DISC`, `DISC_POLL_WRONG_DISC`, or `DISC_POLL_READY`) it waits for the lid
again.

The screen, `DrawDiscChangeScreen`, draws on a white background from `BIN.BIN` part 24 (in video memory at
(768, 256)): "DISC Change Request", or "Change SITE" after a Site change; "Please insert DISC 1" or "2" from
`gCurrentDisc`; and the status line. The labels are in `gDiscChangeSprites`.

## The read queue

`src/cd.c` reads one request at a time; after a read's first command, everything happens in LIBCD's
callbacks. There are 24 request slots,
`sCdRequests`, each a `CdRequest`: busy, done, the first sector, the size in bytes, the destination, and
`ahead`, the number of requests queued before it. `InitCdRequests` (at boot) frees the slots and calls
`CdInit`.

`RequestFilePartRead(file, part, parts, dest)` takes a free slot, sets its sector to the file's first sector
plus the part's (`gDiscFiles[file].sector + parts[part].sector`) and its size to the part's, and returns the
slot's number, or -1 when all 24 are taken. When no read is under way it starts this one; otherwise the request
waits its turn. `PollCdRequest(handle)` answers 0 while the request is pending, 1 once when it is done (which
frees the slot), and -1 for a free slot. Every part the program reads is one of these calls followed by a loop
of polls, most drawing a loading frame each time round ([frame.md](frame.md)).

The read runs as a state machine in `gCdReadState` (`enum CdReadState`), advanced by `HandleCdSync` each time the drive completes a
command:

| State | Waiting for | Then |
| --- | --- | --- |
| `CD_READ_IDLE` (0) | nothing (idle) | started by `RequestFileRead` / `RequestFilePartRead` / `RequestNamedFileRead` |
| `CD_READ_WAIT_SETLOC` (1) | `CdlSetloc` (the first sector) | `CdlSetmode`: double speed, whole sectors with their headers |
| `CD_READ_WAIT_SETMODE` (2) | `CdlSetmode`, or `CdlSetloc` on a retry or the next request | install `HandleCdReady`, `CdlReadN` |
| `CD_READ_WAIT_READ` (3) | `CdlReadN` | `CD_READ_READING` |
| `CD_READ_READING` (4) | the `CdlPause` at the end of the read | the request is done; seek for the next (`CD_READ_WAIT_SETMODE`), or `CD_READ_IDLE` |

`HandleCdReady` runs for each sector as it arrives. It reads the sector's header first and checks the sector's
number: the first must be the one sought, each next one more than the last. It then copies up to 2,048 bytes
straight to the destination; the last sector's copy is rounded up to a whole word, so a read can write up to 3
bytes past its size (inside the block when the destination came from `AllocHeapBlock`, which rounds sizes up the
same way). After the last sector, or on a sector out of order, it removes itself and pauses the drive. In `CD_READ_READING`
the pause's completion ends the request if every byte arrived; otherwise the request starts again from its first
sector and its destination's start. A disc error pauses the drive during a read and is retried the same way,
and before one restarts the request from `CD_READ_WAIT_SETLOC`; `gCdDiskErrorCount` counts them and nothing reads it.

When a request ends, every pending request's `ahead` goes down by one and the one that reaches 0 is next, so
requests are read in the order they were made.

`src/cd.c` has three more ways in that nothing calls. `RequestFileRead(file, dest)` reads a whole file by its
row. `RequestNamedFileRead(name, dest)` finds a file by name with `CdSearchFile`, but stores what it found only
in the slot's unread `file.pos` field, so its seek would go to the sector the slot last held. `FindFileSector` returns
a named file's first sector and size, and `SeekCurrentRequest` restarts the current request.

## Parts

A part is a `FilePart` (`include/cd.h`): its first sector counted from the start of its file, and its size in
bytes. The tables are in `src/file_parts.c`. A part starts on a sector boundary; a read takes whole sectors from
the drive and keeps the part's size.

The buffer a part is read into is the caller's, mostly a heap block of the part's size
([frame.md](frame.md#the-heap)). One is not: `InitText` reads `BIN.BIN` part 1, the font (17,568 bytes), into
`sFontTim` (`src/text.c`), which is 17,536 bytes long, so the read writes 32 bytes past its end.

Many parts are packed. `UnpackNapk(packed, packedSize)` (`src/napk.c`) checks for the tag "napk", allocates a
heap block of the unpacked size that follows it, and unpacks into it: flag bytes, literal bytes and two-byte
back-references ([formats.md](../formats.md)). It returns 0 for untagged data or when the heap is full. The
caller frees the packed buffer and, when done, the unpacked one.

## The loading sequence

`StepTimLoadSequence(discardPart14)` (`src/loading.c`) loads the images the screens share, one step per call in
`gTimLoadStep` (`enum TimLoadStep`): `TIM_LOAD_START` (0) advances without drawing a loading frame,
`TIM_LOAD_DRAW_FIRST` (1) draws the first loading frame, and each image takes two steps (`TIM_LOAD_REQUEST_*`
then `TIM_LOAD_UPLOAD_*`, which polls the read, unpacks with `UnpackNapk`, and uploads with `OpenTIM`, `ReadTIM`,
`LoadClut` and `LoadImage`). Every call but the first and the last draws a loading frame, and the last resets
`gTimLoadStep` to `TIM_LOAD_START` and returns 1. `RunStartScreen` runs it before the start screen with 0, and
`main` again after the end roll with 1. The `BIN.BIN` parts it loads:

| Steps | Part | Image at | Palette at |
| --- | --- | --- | --- |
| `TIM_LOAD_REQUEST_START_CHOICES` (2), `TIM_LOAD_UPLOAD_START_CHOICES` (3) | 13, start screen choices | (320, 409) | (320, 489) |
| `TIM_LOAD_REQUEST_KANA_CHART_MID` (4), `TIM_LOAD_UPLOAD_KANA_CHART_MID` (5) | 6, kana chart rows 5 to 9 | (384, 0) | (320, 241) |
| `TIM_LOAD_REQUEST_KANA_CHART_END` (6), `TIM_LOAD_UPLOAD_KANA_CHART_END` (7) | 7, kana chart rows 10 to 12 | (344, 0) | (320, 241) |
| `TIM_LOAD_REQUEST_KANA_CHART_TOP` (8), `TIM_LOAD_UPLOAD_KANA_CHART_TOP` (9) | 5, kana chart rows 0 to 4 | (448, 0) | (320, 241) |
| `TIM_LOAD_REQUEST_KANA_UI` (10), `TIM_LOAD_UPLOAD_KANA_UI` (11) | 4, kana entry interface | (384, 256) | (320, 242) |
| `TIM_LOAD_REQUEST_KANA_GLYPHS` (12), `TIM_LOAD_UPLOAD_KANA_GLYPHS` (13) | 8, entered kana glyphs | (384, 321) | (320, 243) |
| `TIM_LOAD_REQUEST_MESSAGE_WINDOW` (14), `TIM_LOAD_UPLOAD_MESSAGE_WINDOW` (15) | 22, message window | (896, 256) | (0, 485) |
| `TIM_LOAD_REQUEST_JUMP_TO` (16), `TIM_LOAD_UPLOAD_JUMP_TO` (17) | 23, Jump To panel | (832, 256) | (0, 486) |
| `TIM_LOAD_REQUEST_DISC_CHANGE` (18), `TIM_LOAD_UPLOAD_DISC_CHANGE` (19) | 24, disc-change screen | (768, 256) | (0, 487) |
| `TIM_LOAD_REQUEST_CONTINUE_PROMPT` (20), `TIM_LOAD_UPLOAD_CONTINUE_PROMPT` (21) | 31, continue prompt | (768, 408) | (0, 488) |
| `TIM_LOAD_REQUEST_START_STRIP` (22), `TIM_LOAD_UPLOAD_START_STRIP` (23) | 14, start screen strip | kept unpacked in main memory, `gStripTim` | (320, 244) |
| `TIM_LOAD_REQUEST_START_OPENING` (24), `TIM_LOAD_UPLOAD_START_OPENING` (25) | 3, start screen opening | (320, 256) | (320, 408) |

With `discardPart14` set, part 14 is read and dropped: the start screen frees `gStripTim` when it ends, and is
not run again on that path.

The loading animation itself is `BIN.BIN` part 42 at boot, before the start screen comes back and after the end
roll (`LoadLoadingImage`, with its palette at (640, 376)), and part 43 before each run of the grid and as the voice
screen ends (`LoadAltLoadingImage`, which keeps the palette already loaded). Both
go to (640, 256) as 29 frames of 24 by 40 in rows of ten; `DrawLoadingFrame` shows the next frame each call.

`src/loading.c` also holds the P2 screen (`RunP2Screen`, entered from the grid menu): `BIN.BIN` parts 32 to
41 placed by `LoadTimPartAt`, the pieces shown for the bits set in `gP2NodesOpened`, and "press ANY button"
blinking once 30 frames have passed; any pad event leaves.

## Where to look

| What | Where |
| --- | --- |
| The file tables | `gDiscFiles`, `gDisc1Files`, `gDisc2Files` (`src/media_player.c`) |
| Which disc is in | `IdentifyDisc`, `PollDiscChange` (`src/disc_id.c`) |
| The disc-change screen | `EnsureSiteDisc`, `DrawDiscChangeScreen` (`src/disc_change.c`) |
| Reading | `RequestFilePartRead`, `PollCdRequest`, `HandleCdSync`, `HandleCdReady` (`src/cd.c`) |
| The part tables | `src/file_parts.c` |
| Unpacking | `UnpackNapk` (`src/napk.c`) |
| The shared images, the loading animation | `StepTimLoadSequence`, `DrawLoadingFrame` (`src/loading.c`) |
