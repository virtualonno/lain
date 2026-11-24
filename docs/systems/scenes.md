# The screens the grid runs

Besides the media player, the grid hands the screen to a few scenes of its own, each a loop that runs to its end and
returns: the About screen from the system menu, the upgrade and gate screens when the player opens an upgrade or a
gate node, and the continue prompt after the end roll; the spheres of the end roll's voice track are here too. They
are in `src/about.c`, `src/upgrade.c`, `src/memcard.c` with `src/special_scenes.c`, and `src/continue_prompt.c`. Which
node opens which screen is decided by the node's kind in `RunGridMenu` (`src/grid_menu.c`,
[grid-menu.md](grid-menu.md), [nodes.md](nodes.md)):

| Kind | Nodes | Screen |
| --- | --- | --- |
| `NODE_KIND_UPGRADE` (7) | SSkn01 to SSkn06, and SSkn04# | `RunUpgradeScreen` |
| `NODE_KIND_GATE` (8) | GaTE01 to GaTE04 | `RunGateScreen` |
| `NODE_KIND_P2` (9) | P2-01 to P2-06 | `RunP2Screen` (`src/loading.c`), after setting the node's bit in `gP2NodesOpened` |

After any of the three the node is marked opened and removed from its cell; an upgrade only if it was accepted.

Most of them read their pictures with the loaders in `src/about.c`, which read a napk-packed TIM from a part of
`BIN.BIN` ([formats.md](../formats.md)), wait for it, and upload it: `LoadTimPartAt` to a given place,
`LoadTimPart` where the TIM says, and `LoadTimPartClut`, which uploads only the palette and keeps the image in
memory for the caller.

## About

`RunAboutScreen`, the system menu's About item, scrolls the staff list for 1,776 frames and returns; it reads no
input. It clears the screen, fades the music out, keeps parts 15, 16 and 17 in memory (three pictures of
320 by 512 texels), uploads part 19 to VRAM at (320, 0) and part 18 at (384, 0), and starts music 1.

Each frame after the first `UploadAboutWindow` uploads the 240 lines of the list that are visible to a window at VRAM
(448, 0), one line further down the three pictures than the frame before, with a filler band before the first and
after the last; so the list rises one line a frame, 1,536 lines in all. Behind it, part 19 is drawn as twenty tiles of
64 by 64 that cycle through four frames of animation, and part 18, a 4-bit picture eight texels wide, is stretched
semi-transparently to 320 by 240. At the end the music fades out and music 0 starts again.

## The upgrade screen

`RunUpgradeScreen` loads parts 10, 11 and 12 to VRAM at (320, 0), (384, 0) and (448, 0), with their palettes in
rows 193 to 195 at x 320, and runs through `phase` (`enum UpgradePhase`), with two quads turning about their Y axes
throughout:

| Phase | What it does |
| --- | --- |
| `UPGRADE_PHASE_FADE_IN` (0) | fades sprites and quads in over 33 frames |
| `UPGRADE_PHASE_CHOOSE` (1) | up chooses `UPGRADE_CHOICE_ACCEPT` (0), down `UPGRADE_CHOICE_DECLINE` (1), shown by swapping two palettes; circle accepts (sets result 1 and advances to progress) or declines (skips to fade-out, result 0) |
| `UPGRADE_PHASE_PROGRESS` (2) | runs a progress bar of twenty steps of 18 frames, a six-frame blip moving along it with a trail of five |
| `UPGRADE_PHASE_FADE_OUT` (3) | fades sprites and quads out over 33 frames |
| `UPGRADE_PHASE_DONE` (4) | clears VRAM and returns result |

On 1 the grid menu raises `gUpgradesApplied`, which is saved with the game and shown by the system menu as its
"Application Version" ([system-menu.md](system-menu.md)).

## The gate screen

`RunGateScreen` (`src/memcard.c`) shows a carousel of the gates opened so far and adds one. It loads part 25 to
VRAM at (448, 256) and draws "press ANY button" into VRAM with `UploadTextToVram`. The carousel is built
by `InitGateScene` and drawn each frame by `UpdateGateScene` (`src/special_scenes.c`): models 41 to 44 are the four
gate slabs (`gGateSlabTmds`), a quarter turn apart on a pivot (model 40) at `gGateCenterX`, `gGateCenterY`,
`gGateCenterZ`; as many are shown as `gGatesOpened`, and once one is the carousel turns 22 units of 4096 a frame.
Model 46 is the new slab, placed at a random x and y at z −300, and models 47 and 48 are two planes drawn every frame.
Behind them thirty semi-transparent sprites of `gGateBackdropTim` (`src/tim_data_6.s`) are stacked down the screen,
and each frame the screen fills a 56 by 256 area of VRAM at (384, 256) with 8 by 32 blocks copied at random from six
beside it at (448, 328).

The screen runs in four phases (`enum GatePhase`), ending at `GATE_PHASE_DONE` (4):

| Phase | What it does |
| --- | --- |
| `GATE_PHASE_ASSEMBLE` (0) | 44 tiles of 8 by 8 appear one every other frame, in random order, each from a random place on the screen, and reach a 4 by 11 block around `gGateFlightStart` in four steps while copying random 8 by 8 texel blocks into a 32 by 88 picture at VRAM (320, 0) |
| `GATE_PHASE_UPLOAD_SLAB` (1) | copies that 32 by 88 picture to VRAM (896, 88) and starts the slab's flight (`gGateFlightActive`) |
| `GATE_PHASE_FLIGHT` (2) | `UpdateGateScene` flies the slab over 60 frames along a Bézier curve (`EvalBezier`) from its start through the origin to the carousel's centre, aligns to `gGateAlignYaws`, hides the slab and shows that slot's gate; `RunGateScreen` counts `gGatesOpened` and plays sound 30 on the fourth |
| `GATE_PHASE_WAIT_INPUT` (3) | blinks "press ANY button" (20 frames in 40) and shows the fourth-gate pictures when `gGatesOpened >= 4`, until any pad event moves to `GATE_PHASE_DONE` |

While the carousel turns, `RotateTimColumnsRight` turns the slabs' picture one column to the right each time bit 5
of the carousel's angle comes on, with three `MoveImage`s through the area beside it. Four gates are what the system
menu's Change asks for.

## The end roll's spheres

After the end roll's movie the media player runs the voice screen on track 733, which calls `InitEndRollSpheres`
(`src/special_scenes.c`), which loads parts 30 and 26 to 29 and builds a large sphere (model 41,
`gLargeSphereTmd`) and five small ones around it (models 42 to 46, `gSmallSphereTmd`, at `gSmallSpherePositions`),
all on a pivot at the origin. `StepAndSortEndRollSpheres(delay)` draws them, and every delay + 1 frames turns the
large sphere one way and the small ones the other, 22 units each.

`StepEndRollSpheresCollapse` then draws them in, a step per call through `gSphereCollapseStep` (`enum SphereCollapseStep`), until it sets `gSphereCollapseDone`:

| Step | What it does |
| --- | --- |
| `SPHERE_COLLAPSE_INIT_SMALL` (0) | saves the point-delta state and starts a 30-step collapse of the five small spheres' shared 84-vertex TMD (`gModels[42]`) toward the origin |
| `SPHERE_COLLAPSE_SMALL` (1) | steps `ApplyPointDeltas` on `gModels[42]`, then hides `gModels[42..46]` and restores the saved point-delta state |
| `SPHERE_COLLAPSE_INIT_LARGE` (2) | saves the point-delta state and starts a 60-step collapse of the large sphere's 80-vertex TMD (`gModels[41]`) toward the origin |
| `SPHERE_COLLAPSE_LARGE` (3) | steps `ApplyPointDeltas` on `gModels[41]`, then hides `gModels[41]` and restores the saved point-delta state |
| `SPHERE_COLLAPSE_FINISH` (4) | sets `gSphereCollapseDone` and advances to `SPHERE_COLLAPSE_DONE` (0xff) |

The five small spheres share one TMD, so moving model 42's vertices draws in all five. The steps use the
grid's point interpolator (`InitPointDeltas`, `ApplyPointDeltas`, `POINT_DELTA_SPHERE_COLLAPSE`), whose state is saved before each
collapse and put back after. The voice screen itself is in [speech.md](speech.md).

## The continue prompt

After the end roll, the grid menu counts it in `gEndRollCount` (up to 9999), marks the node opened and runs
`RunContinuePrompt` (`src/continue_prompt.c`). It loads part 31 to VRAM at (768, 408), with its palettes in row 488,
and offers `CONTINUE_CHOICE_END` (0, "end") and `CONTINUE_CHOICE_CONTINUE` (1, "continue") (`enum ContinueChoice`): up
chooses the first, down the second, circle takes it (sound 0). The screen runs at one frame in two:

- a cursor of eight frames (`gContinuePromptSprites` 0 to 7) under the chosen word, and a quad spinning in the
  middle (`sSpinningQuad`);
- two of four frames of static side by side across the middle, changing every frame;
- once in 500 frames on average, a pair of eyes over the static for 180 frames, fading in over 16 steps (17 frames) and out
  over the last 16.

`CONTINUE_CHOICE_END` returns `GRID_MENU_EXIT_START` (0): the grid ends, and `main` resets the game and goes back to
the start screen ([start-screen.md](start-screen.md)), so what was not saved is lost. `CONTINUE_CHOICE_CONTINUE`
returns `GRID_MENU_EXIT_CONTINUE` (3): the grid menu switches `gCurrentDisc` and swaps the two discs' cursors, then
sets the cursor to column 6, row 10 and the other disc's to column 5, row 0, which overrides the swap; `main` reloads
the grid's pictures and goes into the grid, asking for the other disc first ([files.md](files.md)).

The grid's backdrop of six vertical bars, `gVerticalBarsTmd` in `src/vertical_bars.c`, is part of the grid scene
([grid.md](grid.md)).
