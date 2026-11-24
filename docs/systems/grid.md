# The grid

The site is a 3D scene: a see-through cylinder of cells, one level of it in view, with a rippling ring floating
inside, two bands circling each level with its number, a horizon and streaks of light behind, and Lain's clip in a
frame over it all. `src/grid_scene.c` builds and moves the cylinder and the camera; `src/grid_effects.c` owns the
ring, its burst, the orbiting quad, the horizon and the streaks. The menu that drives it is in
[grid-menu.md](grid-menu.md); which node stands in which cell is in [nodes.md](nodes.md). Models, TIM upload, the
view and the ordering tables are in [graphics.md](graphics.md).

## The cylinder

Everything that turns with the grid hangs from one root model, `gModels[5]`, which in turn hangs from the view's
root, `gModels[1]` (both carry `gPointTmd`, a model of one point). A level is three rows of eight cells. Cell *c*
of a level stands at `gCellPositions[c]`: the eight columns on a circle of radius about 700 round the Y axis,
columns 0 and 7 nearest the viewer, 3 and 4 at the far side, and the three rows at Y 99, -99 and -299 (negative Y
is up). Each cell is turned 22.5 + 45 × column degrees about Y. Levels are 900 units apart: a cell of level *l*
sits at its row's Y plus 100 less 900 × *l*, so level 0 is the lowest.

Turning and moving levels never move the camera. A turn rotates the root by an eighth of a turn (its yaw, by 512
units of 4096), and when it ends the yaw is set exactly from the heading, `gHeadingYawEighths[gGridHeading]` × 512
(`src/nodes.c`; entries 8 to 15 are never read). A level move slides the root by 900 along Y.

## What is drawn each frame

`UpdateGridScene` runs once a frame, called by every state of the grid menu. It first starts the command the menu
left in `gSceneCommand`, or steps the animation already running (`gSceneAnimation`), then steps the descent and the
light fade, sorts the scene (`SortGridScene`), projects the cells to the screen (`ProjectCellsToScreen`), draws the
orbiting quad and, in the ordinary front view, steps and sorts the ring. After it the menu draws Lain's clip
(`DrawClipFrame`) and the horizon and streaks (`StepHorizonAndStreaks`).

`SortGridScene` draws three levels: the current one and its neighbours, or the bottom or top three at the ends.
During a level move it also draws the level coming into view. For each level it sorts the models of the cells that
hold a node (an empty cell is never drawn), then five level models (`BuildLevelModels`, `gLevelModels`):

| Model | What it is |
| --- | --- |
| `gTextBandTmd` | the text band round the level, at `gTextBandPosition`, yaw fixed at -227 |
| `gLabelBandTmd` | the label band at `gLabelBandPosition`, turned 22 units further each frame |
| `gEvenLevelLabelTmd` or `gOddLevelLabelTmd`, twice | the level's number, on opposite sides of the label band (yaw 0 and 2048), riding on it |
| `gTextBandTetrahedronTmd` | a tetrahedron at the text band's position |

`BuildLevelModels` takes a yaw for the text band but sets -227 over it. The models of the three levels are rebuilt
when the level changes (`gRebuildLevelModels`), keeping each label band's angle.

The cell models fill the start of `gCellModels`, four levels of 24 at most; slot 98 is the cursor highlight and
slots 99 to 102 the flying cell and its pieces. `ProjectCellsToScreen` writes the screen position of each of the 72
cells in view into its `GridCell`, which is where the caption goes.

### Cells and the cursor highlight

A cell's model is its node's kind, opened or not (`gCellTmds`, [nodes.md](nodes.md)), each with its own icon
texture (`gCellTims`). When the caption has come to rest, the cursor's cell is drawn as `gCursorHighlightTmd`
instead (`SortCursorHighlight`): the cell's icon pixels are copied into the highlight's texture in VRAM, and the
highlight's palette fades toward the cell's own (`StepClutFade`); when the fade ends the highlight's TIM is
uploaded again and it starts over.

### Level numbers

Even and odd levels use different label models and textures, so two neighbouring levels never share one.
`SetLevelNumberTexture(index)` prints `index` + 1 as two digits (`gLevelNumberFormat`, "%02d") and copies each
digit's glyph out of `gLevelDigitsTim` into the tens and ones textures of the index's parity (`gEvenLevelTensTim`,
`gEvenLevelOnesTim`, `gOddLevelTensTim`, `gOddLevelOnesTim`) with `MoveImage`. The level moves call it as they go.
The Site's own caption texture, `gSiteALevelTim` or `gSiteBLevelTim`, is uploaded by `InitGridScene` according to
`gCurrentDisc`.

## The camera

The view (`gView`) has four modes in `gViewMode` (`enum ViewMode` in `include/graphics.h`): `VIEW_FRONT` (1) the
settled front view, `VIEW_TOP` (2) looking down from above, `VIEW_FRONT_RETURN` (3) the front preset and return
path from above, and `VIEW_DESCENT` (4) the descent. The fixed views are `GsRVIEW2`s in `src/graphics.c`:

| View | Eye (X, Y, Z) |
| --- | --- |
| `gFrontView` | 0, 0, -1750 |
| `gTopView` | 0, -2700, 0 |
| `gHighTopView` | 0, -17000, 0 |

A path (`StartViewPath`, `StepViewPath`) moves the eye along a three-point Bezier curve in 50 steps, through
`gFrontTopPathPoint` between the front and top views and through `gHighFrontPathPoint` on the descent. `VIEW_TOP`
rises to the top view for the system menu, `VIEW_FRONT_RETURN` returns, and at its end the ring is rebuilt
(`InitRing`) and `gViewMode` becomes `VIEW_FRONT` again.

**The descent.** `ResetGridSceneToCell`, used whenever the grid is set up for a cursor (`InitGridState`,
`RestoreGridCursor`), rebuilds the scene (`InitGridScene`), puts the grid at the cursor's level and heading at
once, and starts `VIEW_DESCENT`, from `gHighTopView` down to the front view. Vertical streaks rise past the eye on the way
down. At the bottom the ring is built and `gViewMode` becomes `VIEW_FRONT`.

**Tilting.** `SCENE_CMD_TILT_UP` and `SCENE_CMD_TILT_DOWN` turn the view's root about X by 34 units in each of two frames, and move
Lain's frame and the horizon by 21 lines a frame with it (`gTiltOffsetY`). `gViewTiltedUp` and `gViewTiltedDown`
say which way the view is tilted.

## Scene commands

The menu asks for an animation with `gSceneCommand`, and the scene tracks the active one in `gSceneAnimation`; both
use `enum SceneCommand` (`include/pad.h`, `SCENE_CMD_NONE` = 0 when idle). The scene sets `gGridBusy` to 255 while
an animation runs and clears both selectors and `gGridBusy` when it ends.

| Command | Animation |
| --- | --- |
| `SCENE_CMD_TILT_UP` (10), `SCENE_CMD_TILT_DOWN` (11) | tilt up, tilt down, or back to level from the other tilt |
| `SCENE_CMD_TURN_LEFT` (12), `SCENE_CMD_TURN_RIGHT` (13) | turn one column: after 10 frames, 25 (or 26) steps of 19.34 units, each cut to a whole unit, sound 11 every seventh; then `gGridHeading` changes by one |
| `SCENE_CMD_LEVEL_UP` (15) | up a level: after 12 frames, 9 steps of 100, sounds 8 and 9; then `gGridLevel` + 1 |
| `SCENE_CMD_LEVEL_DOWN` (16) | down a level: after 13 frames, 12 steps of 75, sounds 8, 9 and 10; then `gGridLevel` - 1 |
| `SCENE_CMD_TRAVEL_TO_CURSOR` (22) | travel to the cursor's cell: 100 a frame through the levels, then turn until its column is `gGridHeading` (`StepTravelToCursor`); during the descent, all at once |
| `SCENE_CMD_LEVEL_TRAVEL` (23) | travel for Jump To: the levels at 100 × the count a frame going up, 75 × the count going down (`StepLevelTravel`) |
| `SCENE_CMD_VIEW_TO_TOP` (24), `SCENE_CMD_VIEW_TO_FRONT` (25) | the view path up to the top view and back |
| `SCENE_CMD_CELL_FLIGHT` (26) | the cell flight |
| `SCENE_CMD_RING_BURST` (27) | the ring's burst |
| `SCENE_CMD_QUAD_EXPAND` (28) | the quad expands toward the viewer |
| `SCENE_CMD_FLASH_CLIP_FRAME` (29) | Lain's frame flashes |

`SCENE_CMD_TRAVEL_TO_CURSOR` turns column − `gGridHeading` steps in the direction that sign gives, never wrapping round the shorter
way. It takes the column as the cell less 8 while the cell is 9 or more, so cells 8 and 16, column 0 of a level's
second and third rows, count as column 8: from heading 0 the grid makes a whole turn to reach them.

### The cell flight

When a node is chosen (`SCENE_CMD_CELL_FLIGHT`, `StartCellFlight`) its cell's model is hidden and a copy takes off from where it
stood: `gCellModels[99]`, the empty cell's model reshaped to `gFlatCellPoints`, wearing the cursor highlight's pixels
and palette (copied into `gFlyingCellTim`'s place in VRAM). It flies along a Bezier curve from the cell through
`gFlightPathControls` to `gFlightPathEnds`, both indexed by Lain's clip set (`gFlightClipSet`, copied from
`gLoadedOpenClipSet`, `enum NodeOpenClipSet` in `include/lapks_player.h`), turned with the heading, in
`gFlightStepCounts` steps (14, 14, 8, 4, 4), while its eight vertices morph into the cube `gCubeCellPoints` (`POINT_DELTA_CELL_FLIGHT`). The
ending depends on the clip set:

| Set | Ending |
| --- | --- |
| `OPEN_CLIP_SPIN_AWAY` (0) | follows `gClipSet0FlightEndPath`, then spins away; sounds 12, 13, 14 |
| `OPEN_CLIP_COLLAPSE_SPLIT` (1) | follows `gClipSet1FlightEndPath` and collapses toward a point; then splits into the four `gCellPieceTmds`, which fly apart under 20 sparks; sounds 12, 15, 16 |
| `OPEN_CLIP_REFUSE_SPARK` (2) | its TIM is uploaded again and it spins under five sparks a frame, then vanishes for the last 6 frames; sounds 17, 31 |
| `OPEN_CLIP_REFUSE_RETURN` (3) | flies back into its cell; sound 18 twice |
| `OPEN_CLIP_REFUSE_VANISH` (4) | vanishes after 26 frames; sounds 18 four times, 19, 31 |

After `OPEN_CLIP_REFUSE_SPARK`, `OPEN_CLIP_REFUSE_RETURN` and `OPEN_CLIP_REFUSE_VANISH`, which open nothing, the cell is drawn in its place again. After `OPEN_CLIP_SPIN_AWAY` and `OPEN_CLIP_COLLAPSE_SPLIT` it stays hidden
(`gCellFlying`) until the menu opens the node. The `OPEN_CLIP_REFUSE_VANISH` ending fills a point list and `gFlightEndCoords` from a
matrix it never sets; nothing reads either.

### Lain's frame

Lain's clip frames are drawn from a 320 by 480 area of VRAM at X 320 (`InitClipFrameSprites` clears it) as four
16-bit sprites (`gClipFrameSprites`), placed from the frame's own position and size (`gClipFrameInfo`) and offset
by the tilt. `SCENE_CMD_FLASH_CLIP_FRAME` flashes them from 255 down by 10 a frame for 12 frames, then sets them back to 128 once
`gTiltOffsetY` is not above 0, that is when the view is not tilted up.

## The ring

`gRingTmd` (`src/ring_tmd.c`) is model 7, hung from the view's root, so it does not turn with the grid; it rests at
`gRingRestPosition`, inside the cylinder in front of its centre. `InitRing` places it 900 below its rest and sets
it rising. Each frame in the front view `StepRing`:

- rotates its texture (`gRingTim`) one column to the right in VRAM (`RotateRingTimColumnsRight`: three `MoveImage`s
  through the area just right of the texture);
- moves it (`gRingMotion`, `enum RingMotion` in `include/grid_effects.h`): `RING_MOTION_IDLE` (0) at rest;
  `RING_MOTION_LEVEL_UP` (1) or `RING_MOTION_LEVEL_DOWN` (2) with the grid during a level move, then
  `RING_MOTION_RISE_OVERSHOOT` (3) or `RING_MOTION_DROP_OVERSHOOT` (4) past its rest to `gRingRiseOvershoot` or
  `gRingDropOvershoot`, and `RING_MOTION_SETTLE_FROM_ABOVE` (5) or `RING_MOTION_SETTLE_FROM_BELOW` (6) back in
  steps of 10; `RING_MOTION_TURN_LEFT` (7) or `RING_MOTION_TURN_RIGHT` (8) sideways during a turn and
  `RING_MOTION_RETURN_FROM_LEFT` (9) or `RING_MOTION_RETURN_FROM_RIGHT` (10) back;
- rocks it (`gRingTilt`, `enum RingTilt`, `gRingTiltDegrees`): `RING_TILT_NONE` (0) level,
  `RING_TILT_PITCH_POS` (1) / `RING_TILT_PITCH_NEG` (2) about X after a level move, `RING_TILT_ROLL_NEG` (3) /
  `RING_TILT_ROLL_POS` (4) about Z after a turn, and `RING_TILT_SETTLE_PITCH` (5) / `RING_TILT_SETTLE_ROLL` (6)
  back to level;
- morphs its 64 points toward a shape (`StartPointMorph`, `enum PointDeltaMode` in `include/graphics.h`:
  `POINT_DELTA_RING_FLAT` (1), `POINT_DELTA_RING_BENT_DOWN` (2), `POINT_DELTA_RING_BENT_UP` (3) or
  `POINT_DELTA_RING_DIP` (4); `gRingFlatPoints`, `gRingBentDownPoints`, `gRingBentUpPoints`, `gRingDipPoints`). A
  level move bends it and lets it flatten; about once a minute of vertical blanks, three times in four, a resting
  ring dips and recovers;
- writes its vertices into the TMD in place (`SetTmdVertex`), in pairs: vertex *i* and vertex *i* plus half the
  count get the same offset, chosen by `gRingRippleMode` (`enum RingRippleMode` in `include/grid_effects.h`). In
  `RING_RIPPLE_WAVE` (0) the offset is added to Y and its amplitude grows back after a move (`gRingRippleShift`
  falls by one every 30 vertical blanks, to 4); `RING_RIPPLE_RESET` (1), set on level moves, falls into the default
  arm to write unrippled vertices and reset phase and shift; in `RING_RIPPLE_SCATTER` (2), after a turn, a random
  offset is added to all three axes. Both active modes read single bytes out of `gSineTable`, the low byte of an
  entry, so the offsets do not follow a sine.

### The burst

`SCENE_CMD_RING_BURST`, for the system menu, replaces the ring with the 32 `gBurstPieceTmds` (`InitPieceBurst`, models 8 to 39) at
its rest position, driven by `gBurstState` (`enum BurstState`) and `gBurstPulse` (`enum BurstPulse`). Over 51
frames the pieces rise (`BURST_RISE` (1)), pause (`BURST_WAIT_FIRST_PULSE` (2), `BURST_WAIT_SECOND_PULSE` (4)) and
jump out 50 units along their own directions (11.25 degrees apart) and back twice (`BURST_FIRST_PULSE` (3),
`BURST_SECOND_PULSE` (5), stepping `gBurstPulse` through `BURST_PULSE_HOLD` (0), `BURST_PULSE_OUT` (1),
`BURST_PULSE_IN` (255)), pause (`BURST_WAIT_SCATTER` (6)), then fly outward at 100 a frame (`BURST_SCATTER` (7));
at frame 51 sound 23 plays, the pieces are hidden, `gBurstState` returns to `BURST_IDLE` (0), and the ring is
rebuilt. From frame 40 to 42 the code raises a tilt of the pieces toward 15 degrees, but its step,
`(short)(gRingTiltDegrees + 0.5)`, never leaves 0, so the pieces stay level.

## The quad

`gTexturedQuad` is a single semi-transparent `POLY_FT4` of `gQuadTim` (`src/tim_data_3.s`). In `VIEW_FRONT`
`DrawOrbitingTexturedQuad` turns `gQuadOrbitPoint` about X and, twice as fast, about Z, two degrees a frame,
centres the quad there on the screen with a random brightness between 128 and 255, and puts flat light 2 opposite
it. `SCENE_CMD_QUAD_EXPAND`, before an idle media, sends it toward the middle of the screen over 20 frames (15 if it is in front
of the centre), growing 4 a side each frame. Two of its corners take the half-width and half-height the other way
round (the second corner's Y uses the half-width, the third corner's X the half-height), so the quad is drawn
skewed unless they are equal.

## The horizon and the streaks

`InitHorizonAndStreaks` lays ten copies of `gHorizonTim` side by side at Y 88 (`gHorizonSprites`, sorted behind the
scene) and prepares 100 `Streak`s, lines whose head moves from `start` toward `end` at `speed` with the tail
`length` behind, coloured from `gStreakColors` at the head and fading to black. While the eye is below the top
view's height, `StepHorizonAndStreaks` draws the horizon, moving it with a tilt and with the eye's height on a view
path, runs 32 streaks diagonally toward the viewer on four paths, each restarting at a random height, speed and
length when it passes its end, and spins the vertical bars (`gVerticalBarsTmd`, `src/vertical_bars.c`, model 6, 45
units a frame). During the descent vertical streaks rise from Y 500 toward the eye's height: all 100 while the eye
is above Y -2700, then 50 of them.

## The data

| File | Holds |
| --- | --- |
| `src/tmd_data_5.c` | the cell models, opened and not, the empty cell, the cursor highlight, the four cell pieces and the 32 burst pieces (and the end roll's and gate scene's models) |
| `src/ring_tmd.c` | `gRingTmd`, and `gUnusedQuadTmd`, which nothing reads |
| `src/band_tmd.c` | `gTextBandTmd` and `gLabelBandTmd`, and `gUnusedSlabTmd`, which nothing reads |
| `src/level_label_tmd.s` | the even and odd level labels, and `gPointTmd` |
| `src/tim_data.s` | the horizon, `gLifLogoTim`, `gLifeInstinctFunctionTim` and `gTextBandBarTim`, the Sites' level captions, the ring texture and the level digits |
| `src/tim_data_5.s` | the cell icons, opened and not, the flying cell and the cursor highlight |
| `src/tim_data_3.s` | `gQuadTim` |
| `src/vertical_bars.c` | `gVerticalBarsTmd` |
| `src/media_cube_tmd.c` | `gTextBandTetrahedronTmd` (with the media player's cube) |

The grid's setup uploads these textures (`InitGridScene`, `InitRing`, `InitTexturedQuad`, `InitHorizonAndStreaks`).
Two of the grid's TMDs are not constant: `gRingTmd` and `gEmptyCellTmd`, which the flying cell wears, have their
vertices rewritten in place as they move.
