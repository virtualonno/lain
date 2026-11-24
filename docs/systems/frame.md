# The frame

Serial Experiments Lain has no single frame loop. `main` (`src/main.c`) brings the machine up once and then
alternates between two long-running screens, the start screen and the grid menu; each screen, and every
smaller screen they open, runs its own loop and draws and shows its own frames with the helpers in
`src/graphics.c`. The display buffers and ordering tables themselves are in [graphics.md](graphics.md),
reading from the disc in [files.md](files.md), the start screen in [start-screen.md](start-screen.md) and
the grid menu in [grid-menu.md](grid-menu.md).

## Boot

`main` calls `SetMem(2)` (2 MB of main memory) and `ResetCallback`, then initialises in this order:

1. `OpenHostLog("d:\\usr\\tmp\\psx.log")`: the host log, empty in this build (below).
2. `InitGraphics`: the GPU, the GTE, the two display buffers and the ordering tables ([graphics.md](graphics.md)).
3. The debug font: `FntLoad(960, 256)`, and a print stream of 256 by 200 at (16, 16) opened with `FntOpen`
   and made the dump font with `SetDumpFnt`.
4. `InitCdRequests` (`src/cd.c`), the CD request machine ([files.md](files.md)).
5. `InitClipPlaybackSlots` (`src/lapks_player.c`): the clip decoder, the game's heap, the loading image, and
   the first two clips read from `LAPKS.BIN`.
6. `InitSound(0)` (`src/sound.c`): LIBSND, the sound bank and the two music sequences from `SND.BIN`
   ([sound.md](sound.md)).
7. `InitMemCard` (`src/memcard.c`, [memory-card.md](memory-card.md)) and `InitPads` (`src/pad.c`).
8. `InitText(68, 49)` (`src/text.c`): the font images from `BIN.BIN`, and the caption's position.
9. `LoadMenuTileImage` (`src/system_menu.c`), then the grid: `InitGridScene`, `InitGridState`,
   `BuildNodeGrid`, and `LoadGridEntryClip`, which starts reading the clip that plays as the grid opens.
10. `VSyncCallback(HandleVSync)`.

Once the loading image is in, every load in this sequence waits for its read by calling `DrawLoadingFrame(0)`
(`src/loading.c`) in a loop, so the loading animation runs while the program boots; the vertical-blank handler
is installed only at the end.

`InitProgram` and `InitGraphicsAndFont` repeat this sequence, the whole of it and its steps 2 and 3;
`ResetForStartScreen` repeats the reset `main` makes before it goes back to the start screen. Nothing calls the
three.

## The outer loop

```
for (;;) {
    RunStartScreen                       src/kana_entry.c: load a save or enter a name
    for (;;) {
        wait for PollGridEntryClip       the entry clip's read is done
        LoadAltLoadingImage              the in-game loading animation's image
        EnsureSiteDisc(siteChange)       the right disc is in the drive (files.md)
        switch (RunGridMenu())                           src/grid_menu.c (enum GridMenuResult)
        case GRID_MENU_EXIT_LOAD (1):        save loaded from the system menu     -> back to the grid
        case GRID_MENU_EXIT_SITE_CHANGE (2): Site change (siteChange = 1)         -> back to the grid
        case GRID_MENU_EXIT_CONTINUE (3):    continue after end roll, other disc  -> back to the grid
        default (GRID_MENU_EXIT_START, 0):   leave for the start screen
    }
    LoadLoadingImage, InitGridState, ResetNodeStates, BuildNodeGrid, InitSound(1), LoadGridEntryClip
}
```

Before going back to the grid each case calls `RestoreGridCursor`, frees the idle clips (`FreeIdleClips`) and
starts the entry clip's read again (`LoadGridEntryClip`). Continuing after the end roll also mutes the music
(`SsSetMVol(0, 0)`), runs the start screen's image loading again (`LoadLoadingImage`, then
`StepTimLoadSequence(1)` until it finishes), restores the volume and reloads the text images (`InitText`).
`siteChange` is 1 only on the pass after a Site change; `EnsureSiteDisc` passes it to the disc-change
screen it draws. Both a Site change and continuing after the end roll switch `gCurrentDisc` before
`RunGridMenu` returns, so the next `EnsureSiteDisc` asks for the other disc. Before and
after each start screen `main` writes the heap's free space to the host log (`GetLeftMemory`), which this
build discards.

`InitSound(1)` does nothing: every step of `InitSound` sits under `reset == 0`.

## One frame

A screen's frame begins with `BeginFrame` and ends with a swap. `BeginFrame` (`src/graphics.c`) sets the view
(`GsSetRefView2(&gView)`), records the buffer being built in `gActiveBuffer` (`GsGetActiveBuff`), points
LIBGS's packet work area at that buffer's half of `gPacketArea`, and clears both of its ordering tables,
`gOt[gActiveBuffer]` (3D) and `gSubOt[gActiveBuffer]` (2D).

The grid menu ends its frames with `EndFrame`:

```
DrawSync(0)                                   the GPU has finished the last table
GsSortOt(&gSubOt[gActiveBuffer], &gOt[gActiveBuffer])   the 2D table goes into the 3D one
DrawSync(0); VSync(0)                         wait for the vertical blank
ResetGraph(1)
GsSwapDispBuff()                              show the other buffer
GsSortClear(0, 0, 0, &gOt[gActiveBuffer])     clear to black first
GsDrawOt(&gOt[gActiveBuffer])                 start the GPU on this frame's table
```

The 2D screens (the loading animation, the continue prompt, the start screen and others) draw only into
`gSubOt` and end the frame with the same sequence on `gSubOt[gActiveBuffer]` alone, written out in each
routine. `GsDrawOt` returns at once: the GPU draws one frame's table while the program builds the next into
the other buffer's table and packet area, and the next frame's first `DrawSync(0)` is where the program waits
for it.

A grid frame is `BeginFrame`, `UpdateGridScene` (the 3D grid, `src/grid_scene.c`), the clip's frame when one
is showing (`DrawClipFrame`), `StepHorizonAndStreaks`, and `EndFrame`; each of `RunGridMenu`'s states
does its own work, often input (`HandleGridInput`), before, among or after these ([grid.md](grid.md),
[grid-menu.md](grid-menu.md)).

## Timing

A frame lasts at least one vertical blank and the program keeps no other clock: every swap waits for the next
blank, so a frame whose work takes longer than one field takes two or more. Some screens wait for two blanks
each frame (the continue prompt calls `VSync(0)` twice). The Japanese console shows about 60 fields a second.

Counts that must last a fixed time are frame counts: the grid menu's idle count (`gIdleFrames`, which after
500 frames without a new pad event plays idle media), the system menu's message timer (`gMenuCounter`,
120), the loading animation's 29 frames. The vertical-blank count `gVSyncCount` (below) times only the clip and movie
decoders' stages and, on the grid, the ring's dip, at most once every 3,600 blanks, and its ripple's decay, a
step every 30 (`src/grid_effects.c`).

## The vertical blank

`HandleVSync` (`src/main.c`) runs at every vertical blank once boot ends:

- `CountVSync` (`src/clip.c`) adds one to `gVSyncCount`.
- `PollXaPlayback` (`src/media_player.c`, [media-player.md](media-player.md)), on every fourth blank
  (`VSync(-1) & 3`) while an XA track plays, asks the drive for its position (`CdlGetlocP`), moves the
  track's progress, and stops the track when the read has left its sectors; a disc error restarts the read.
- `SoundVSync` (`src/sound.c`) is empty.

## The heap

The game allocates from one heap, `sHeapArea` in `src/lapks_player.c`: 868,352 bytes, handed to
`InitHeapArea` (`src/heap.c`) by `InitClipPlaybackSlots` at boot. Clips, read buffers, unpacked images and the
memory card's transfer buffer all come from it.

Each block starts with a 16-byte `HeapBlock` header: a tag, the size after the header, and the next and
previous blocks in address order. The tag is the ASCII of "Free" (0x46726565) or "Use!" (0x55736521).

- `AllocHeapBlock(size)` rounds the size up to a multiple of 4 and takes the first free block large enough,
  starting at `gHeapSearchStart`. If more than 31 bytes would be left over it splits the block, the remainder
  becoming a free block after it; otherwise the whole block is used. It returns the address after the header,
  or 0 when the size is 0, nothing fits, the heap is not set up, or a tag is neither word.
- `FreeHeapBlock(address)` marks the block free and merges it with a free block before and after it, moving
  `gHeapSearchStart` back when the freed block lies before it.
- `gHeapUsed` adds header and size when a block is allocated; freeing subtracts the size, and a header for each
  merge. `GetLeftMemory` returns `gHeapSize - gHeapUsed`; `gHeapPeakUsed` keeps the highest `gHeapUsed`.

`CheckHeapBlock`, which checks a block's tags and its neighbours', looks for the header 8 bytes before the
address it is given, not 16; nothing calls it, nor `GetHeapPeakUsed`. `LoadMenuTileImage`
(`src/system_menu.c`) hands its read buffer, taken from `AllocHeapBlock`, to the C library's `free`, so the
game heap never sees that block freed.

When the heap is full, a clip poller whose loader got no buffer (`PollGridEntryClip` and the others) prints
"MEMORY TOO FEW" to the debug font, and `PurgeIdle` prints "PurgeIdle". Nothing calls `FntFlush`, so the debug font's text is never drawn.

## The host log

`src/host_log.c` holds `OpenHostLog`, `HostLogPrintf` and two more routines, all empty: the calls to them from
`main`, `InitProgram` and the heap's corruption checks do nothing. `AllocHeapBlock` and `CheckHeapBlock`
report through the C library's `printf` instead.

## Controllers

`InitPads` (`src/pad.c`) gives LIBPAD two 36-byte receive buffers (`PadInitDirect`, `PadStartCom`), one a port;
the game reads only the first. Once a frame, a screen that takes input calls `ReadPadEvent`, which turns the
first controller's state into one number in `gPadEvent`, the pad event (`enum PadEvent` in
`include/pad.h`):

- Nothing is read unless LIBPAD has found the controller (`PadGetState(0)`) and it is a digital pad (type 4)
  or an analog one (type 7).
- One event a frame, the first that holds in this order: circle, cross, start, triangle or select, square,
  L1, L2, R1, R2; then on an analog pad the right stick (X before Y) and the left stick; then the
  direction buttons (`ReadPadDirection`). A stick counts as pushed outside 0x50 to 0xB0, its rest being 0x80.
- An event the same as the last one is reported as `PAD_EVENT_NONE` (0), so holding a button gives one event; it gives another
  only after something else has been reported, release included. There is no repeat.

`gSceneCommand` (`enum SceneCommand`), cleared here, is not input: the grid menu writes it to tell the grid scene what to animate
(`src/grid_scene.c`). `gCursorCell` and `gCursorLevel` are also defined in `src/pad.c`, but belong to the
grid.

## Where to look

| What | Where |
| --- | --- |
| Boot and the outer loop, the vertical-blank handler | `main`, `HandleVSync` (`src/main.c`) |
| Beginning and ending a frame | `BeginFrame`, `EndFrame` (`src/graphics.c`) |
| The loading animation's frame | `DrawLoadingFrame` (`src/loading.c`) |
| The grid menu's states and their frames | `RunGridMenu` (`src/grid_menu.c`) |
| The heap | `src/heap.c`; the area is `sHeapArea` (`src/lapks_player.c`) |
| Controller input | `ReadPadEvent` (`src/pad.c`), the codes in `include/pad.h` |
