# Graphics

The program draws with Sony's LIBGS on top of LIBGPU: 3D models as LIBGS objects sorted from TMD data, and sprites,
text and the clip frames as LIBGS sprites, beside some lines and polygons. `src/graphics.c` holds the display
set-up, the two ordering tables and their packet areas, the frame's beginning and end, the model record and its
helpers, image upload, and the view, light and palette helpers; `src/text.c` draws text by writing glyphs into video
memory. When a frame is begun and shown is in [frame.md](frame.md); what the grid draws is in [grid.md](grid.md).

## The display

`InitGraphics` (called once from `main`) resets the GPU and the GTE and sets up a 320 by 240 screen,
non-interlaced, dithered and 16-bit (`GsInitGraph(320, 240, 4, 1, 0)`), with the two display buffers one above
the other at video-memory rows 0 and 240 (`GsDefDispBuff2(0, 0, 0, 240)`). The geometry offset is the screen's
centre, (160, 120). It finishes by clearing the 320 by 512 strip at the left of video memory to black, both
buffers and the 32 rows below them.

The buffer being built is `gActiveBuffer`, read from LIBGS at the start of each frame. Each buffer has:

| Buffer's | Is | Size |
| --- | --- | --- |
| 3D ordering table | `gOt[n]`, on `gOtTags0` or `gOtTags1` | 2^14 = 16,384 entries |
| 2D ordering table | `gSubOt[n]`, on `gSubOtTags[n]` | 2^8 = 256 entries |
| Packet area | `gPacketArea[n]`, LIBGS's work base | 36,000 bytes |

3D models are sorted into `gOt` (`SortModel`: `GsSortObject4` with a shift of 0, the scratchpad as its work area).
Sprites and text are sorted into `gSubOt`, the clip frames and a few other sprites into `gOt`. `EndFrame` links the
whole 2D table into the 3D one with `GsSortOt`; `GsClearOt` gave the 2D table point 0, so it goes in at the 3D
table's entry 0, which is drawn last: the 2D layer lies over the scene. Screens without a 3D scene draw `gSubOt`
alone. `GsSortClear` starts every table drawn with a clear to black, white on the disc-change screen.

## Models

A `Model` (`include/graphics.h`) is one LIBGS object (`GsDOBJ2`) with its coordinate system (`GsCOORDINATE2`),
a rotation (`rot`), and the TMD's object count and first object. The program keeps three pools of them,
`gModels[51]`, `gCellModels[103]` and `gLevelModels[21]`, used by the grid and its scenes. A model is set up with:

- `SetModelTmd(tmd, model)`, which maps the TMD where it lies (`GsMapModelingData`) and records its objects.
- `LinkModelObjects(model, parent)` links each object (`GsLinkObject4`) to the model's coordinate system,
  counts them in `gLinkedObjectCount`, and makes the system a child of `parent` when one is given.
- `InitModelCoordinate`, which sets the coordinate system to world and the rotation to zero.

Each frame a caller sets the model's position in `coord.coord.t` and its angles in `rot`, builds the matrix
with `UpdateModelMatrix` (`RotMatrix` from the identity), and sorts it with `SortModel`, which loads each
object's local light and screen matrices before sorting it. `UpdateCoordinatePairMatrices` and
`InitChildCoordinatePair` do the same for two coordinate systems sharing one rotation.

`SetTmdVertex`, `GetTmdVertex` and `SetTmdNormal` read and write a TMD object's vertex and normal arrays in
place, which is how the grid bends and morphs its models. A morph moves a set of points toward a target over a
number of steps: `InitPointDeltas` divides each point's distance to its target by the step count into
`gPointDeltas` (up to 256 points), and each call of `ApplyPointDeltas` adds them once and returns the steps
left. The division is an integer one, so a morph can stop short of its target by its remainder. `gPointDeltaMode`
(`enum PointDeltaMode` in `include/graphics.h`) records which morph is active: `POINT_DELTA_NONE` (0), the ring
shapes `POINT_DELTA_RING_FLAT` (1), `POINT_DELTA_RING_BENT_DOWN` (2), `POINT_DELTA_RING_BENT_UP` (3) and
`POINT_DELTA_RING_DIP` (4), `POINT_DELTA_CELL_FLIGHT` (5) for a flying cell, and `POINT_DELTA_SPHERE_COLLAPSE`
(6) for the end roll's spheres.

`ProjectModelToScreen` and `ProjectCoordinatePairToScreen` find where a coordinate system's origin falls on the
screen: the local-to-screen translation scaled by the projection distance over its depth, plus the screen
centre. A depth of 0 is taken as 40.

`EvalBezier` and `WeightBezierPoints` evaluate Bezier curves in `double`, which the console computes in
software: `WeightBezierPoints` multiplies each control point by its binomial coefficient and `EvalBezier` sums the
weighted points against powers of t and 1 - t. Up to 12 control points. The grid's flights and the special
scenes use them.

## Images

Images are TIMs. `UploadTim` (`src/graphics.c`) loads a TIM's pixels, and its palette when it has one, to the places
the TIM itself records; it is how the grid, its effects and the special scenes load the TIMs linked into the
executable. The loading screens, the text and the menus, which read their images from the disc, use LIBGPU's
`OpenTIM` and `ReadTIM` instead and place the pixels and palette at positions of their own, after `UnpackNapk` when
the part is packed ([files.md](files.md)).

`StepClutFade(image, target)` moves every entry of `image`'s palette one level of 32 toward `target`'s in each
colour channel, keeping each entry's top bit from the target, and reloads the palette; `gClutFading` stays
nonzero until the two are equal. The grid uses it to fade the cursor's highlight into a cell's icon palette.
`StepClutToBlack` does the same toward black.

## The view, lights and fog

`SetViewMode` chooses one of three fixed views into `gView` using `enum ViewMode` (`include/graphics.h`):
`VIEW_FRONT_RETURN` (3) the front view (`gFrontView`), `VIEW_TOP` (2) the top view (`gTopView`), `VIEW_DESCENT`
(4) the high top view (`gHighTopView`); any other mode falls back to `VIEW_FRONT_RETURN`, and `VIEW_FRONT` (1) is
set by the grid scene once a path settles on the front view. It also sets the
projection distance to 350; `InitView` sets `VIEW_FRONT_RETURN` and the clip planes (near 100, far 1,000).
`SetViewLights` loads three flat lights into `gFlatLights`: the top view's set (`gTopViewLights`) in `VIEW_TOP`
and `VIEW_DESCENT`, the front view's (`gFrontViewLights`) otherwise. The ambient colour is `GsSetAmbient(128, 128, 128)`.

`StepLightFade`, which the grid scene calls every frame, moves one light two steps a frame toward a target colour,
but only after `StartLightFade` has started a fade, and nothing calls `StartLightFade`. Nothing calls `EnableFog`
either, so the light mode stays 0 and `DisableFog` (called by the gate screen, `src/memcard.c`) has nothing to turn
off.

## Text

The font is a 4-bit TIM, `BIN.BIN` part 1, read by `InitText` into `sFontTim`. Only its palette goes to video
memory (row 481); the pixels stay in main memory, `gFontPixels`, 128 bytes a row, and are copied glyph by glyph
by the processor. A text is built in `sTextImage`, a 256 by 24 4-bit image, and uploaded to video memory as a
64 by 24, 16 or 8 block; sprites then show that block.

There are two faces, each glyph described by two bytes in its table (`gLargeFontGlyphs`, `gSmallFontGlyphs`,
`src/text.c`): its x in the atlas, and a band number in the low three bits with a size minus one above them.

| Face | Atlas rows | Size field | Advance |
| --- | --- | --- | --- |
| Large (`CopyLargeGlyph`) | bands 0, 1 16 high from row 0; 2 up 24 high from row 32 | width | its width; a 16-pixel cell if fixed |
| Small | bands 8 rows apart from row 80 | height | 8 |

`UploadTextToVram(text, x, y, mode, fixedWidth)` (`enum TextUploadMode` in `include/text.h`) clears `sTextImage`, copies the string in the large face (`TEXT_UPLOAD_LARGE` (0)) or the small one (`TEXT_UPLOAD_SMALL` (1)), rounds each position up to an even pixel (a byte holds two), uploads the block at (x, y) and returns the width in pixels. `TEXT_UPLOAD_CLEAR` (2) uploads an empty 8-row block, which clears one. Characters from 128 up have no glyph.

The caption that follows the grid's cursor is `src/text.c`'s other half. `SetCellCaption` writes the node's name (up to six characters) into its own block at (256, 480), one 16-pixel cell a glyph, and gives each glyph a sprite (`PrepareCaptionSprites`): the first glyph takes the palette at (16, 481) and the others (32, 481), unless a caption style is set (`SetCaptionStyle`, `enum CaptionStyle` in `include/text.h`: `CAPTION_STYLE_CELL` (0), `CAPTION_STYLE_PLAIN` (1), `CAPTION_STYLE_RESTORE_CELL` (2)), when all take the first. The node's four titles go in the small face to (384, 480), (384, 496), (448, 480) and (448, 496). `UpdateCaption` draws the caption each frame in one of three states, `gCaptionState` (`enum CaptionState`): `CAPTION_IDLE` (0, standing — `DrawCaption`: the name, its two bars, the title plates), `CAPTION_LEAVING` (1) and `CAPTION_ARRIVING` (2, each glyph on its own delay), while `gCaptionTextOnly` (`enum CaptionDisplay`: `CAPTION_DISPLAY_FULL` (0), `CAPTION_DISPLAY_TEXT_ONLY` (1), `CAPTION_DISPLAY_MOVE` (2)) controls whether the bars and title plates are drawn, and `SetTemporaryCaption` swaps another text in and back.
`DrawNoSharedNodePanel` draws the panel the grid shows when no other node shares the word chosen in the
media player.

The text of a node comes from `src/string_tables.c`: a node's `titles[4]` index `gTitles` (51 strings), its
`words[3]` index `gWords` (417). The words are shown when a media node is opened, in the small face from
(320, 72) down, the node's name at (320, 120).

## Video memory

What these units place at fixed positions (x in 16-bit units, so a 4-bit image's x covers four pixels each):

| Where | What | Placed by |
| --- | --- | --- |
| (0, 0) and (0, 240), 320 by 240 | The two display buffers | `InitGraphics` |
| (256, 480), 64 by 24 | The caption's name | `PrepareCaptionSprites` |
| (320, 480), 64 by 32 | `BIN.BIN` part 0, the caption's bars and title plates | `InitText` |
| (384, 480) to (511, 511) | The caption's four titles | `SetCellCaption` |
| (0, 481) to (0, 484) | Palettes: the font, part 0, the menu tiles, part 9 | `InitText`, `LoadMenuTileImage` |
| (512, 256) | "press ANY button" | `src/loading.c`, `src/memcard.c` |
| (640, 256), palette (640, 376) | The loading animation | `LoadLoadingImage`, `LoadAltLoadingImage` |
| (640, 456), 62 by 48 | The system menu's tiles, `BIN.BIN` part 2 | `LoadMenuTileImage` |
| (704, 296) to (791, 511) | The system menu's labels and prompts | `src/system_menu.c` |
| (960, 256) | The debug font | `FntLoad` in `main` |
| (960, 416), 62 by 96 | `BIN.BIN` part 9, with the no-shared-node panel | `InitText` |

The TIMs that `UploadTim` loads carry their own positions; the media player and the clips place theirs in their
own code ([media-player.md](media-player.md)).

## The video-memory dump

`DumpVramTim(name, pixelMode, x, y, width, height, clutX, clutY)` (`src/vram_dump.c`) writes a rectangle of
video memory to a file on the development host as a TIM, through LIBSN's `PCcreat`, `PCwrite` and `PCclose`, in
blocks of 4,096 bytes. `pixelMode` (`enum TimPixelMode` in `include/vram_dump.h`: `TIM_PIXEL_4BIT` (0),
`TIM_PIXEL_8BIT` (1) and `TIM_PIXEL_16BIT` (2)) selects 4-, 8- and 16-bit pixels; the first two also store a 256-entry
palette from (`clutX`, `clutY`). `width` counts pixels and is rounded up to fill whole 16-bit units. The file it
writes records the palette as 16 by 16 at (0, 0) and the image at (0, 0), not where they were read from.
Nothing calls it; on a retail console there is no host to write to.

## Where to look

| What | Where |
| --- | --- |
| The display, the tables, a frame's start and end | `InitGraphics`, `BeginFrame`, `EndFrame` (`src/graphics.c`) |
| A model | `Model` (`include/graphics.h`); `SetModelTmd`, `LinkModelObjects`, `UpdateModelMatrix`, `SortModel` |
| Morphs and curves | `InitPointDeltas`, `ApplyPointDeltas`, `EvalBezier`, `WeightBezierPoints` |
| Palette fades | `StepClutFade`, `StepClutToBlack` |
| Text and the caption | `UploadTextToVram`, `SetCellCaption`, `UpdateCaption` (`src/text.c`) |
| Node titles and words | `gTitles`, `gWords` (`src/string_tables.c`) |
| The dump | `DumpVramTim` (`src/vram_dump.c`) |
