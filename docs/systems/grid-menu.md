# The grid menu

While the player is on the site the game runs `RunGridMenu` (`src/grid_menu.c`): one loop, one state
(`gGridMenuState`, `enum GridMenuState`) and, inside the longer states, a step (`gGridMenuStep`). The menu moves
nothing itself. It reads the pad, decides, and hands the scene a command in `gSceneCommand` (`enum SceneCommand`);
the scene animates it over the next frames and holds `gGridBusy` nonzero until it is done ([grid.md](grid.md)).
Most actions also play a short clip of Lain from `LAPKS.BIN` (`src/lapks_player.c`), and the menu waits for that
too. The nodes themselves, what makes one visible or openable and what opening one changes, are in
[nodes.md](nodes.md).

Every state draws a whole frame the same way: `BeginFrame`, `UpdateGridScene`, Lain's clip (`DrawClipFrame`, unless
`gShowClipFrame` is 0), the horizon and streaks (`StepHorizonAndStreaks`), the state's own sprites and the caption,
then `EndFrame`.

## Entering and leaving

`src/main.c` sets the grid up at boot (`InitGridScene`, `InitGridState`, `BuildNodeGrid`; the last two again before
each return to the start screen) and calls `RunGridMenu` after the start screen. `InitGridState` puts the cursor on
column 6, row 10 of Site A and the other Site's saved cursor on column 5, row 0, and clears the progress counters.
`RunGridMenu` starts the music (`PlayMusic`), puts the caption on the cursor's cell and enters `GRID_MENU_ENTER`,
which plays the grid entry clip (`LoadGridEntryClip`, loaded beforehand by `main`) and waits for the scene to settle
before going to `GRID_MENU_REST`.

It returns an `enum GridMenuResult` (`include/grid_menu.h`):

| Result | When | What `main` does next |
| --- | --- | --- |
| `GRID_MENU_EXIT_LOAD` (1) | the system menu finished a load (`UpdateSystemMenu` gave `MENU_RESULT_LOAD`) | rebuilds the grid and enters again |
| `GRID_MENU_EXIT_SITE_CHANGE` (2) | the system menu's Site change (`UpdateSystemMenu` gave `MENU_RESULT_CHANGE`) | the same; `EnsureSiteDisc` asks for the other disc if the drive holds the wrong one |
| `GRID_MENU_EXIT_CONTINUE` (3) | an end roll, then continue on the continue prompt | shows the loading screen and enters again |
| `GRID_MENU_EXIT_START` (0) | an end roll, then end on the continue prompt | back to the start screen, with the node table reset (`ResetNodeStates`) |

A Site change swaps `gCursorColumn` and `gCursorRow` with `gOtherDiscCursorColumn` and `gOtherDiscCursorRow` and
flips `gCurrentDisc`, so each Site keeps its own cursor. Continuing after an end roll also changes Site, but puts
the cursor on column 6, row 10 and the other Site's cursor on column 5, row 0.

## States

`gGridMenuState` takes values of `enum GridMenuState` (`src/grid_menu.c`):

| State | Role |
| --- | --- |
| `GRID_MENU_REST` (0), `GRID_MENU_TILTED_UP` (1), `GRID_MENU_TILTED_DOWN` (2) | Resting views: level, tilted up and tilted down |
| `GRID_MENU_OPEN_NODE` (3) | Plays the node-opening clips and cell flight, then opens the node's screen |
| `GRID_MENU_SYSTEM_MENU` (4) | Opens, runs and closes the system menu (`enum SystemMenuStep`) |
| `GRID_MENU_JUMP_TO` (5) | Runs the Jump To panel and level travel (`enum JumpToStep`) |
| `GRID_MENU_TILT_UP` (6), `GRID_MENU_UNTILT_FROM_UP` (7) | Tilts from level up to `GRID_MENU_TILTED_UP` and back |
| `GRID_MENU_TILT_DOWN` (8), `GRID_MENU_UNTILT_FROM_DOWN` (9) | Tilts from level down to `GRID_MENU_TILTED_DOWN` and back |
| `GRID_MENU_TURN_LEFT` (10), `GRID_MENU_TURN_RIGHT` (13) | Turns the grid one column left or right; states 11, 12, 14 and 15 share these arms but nothing enters them |
| `GRID_MENU_LEVEL_UP` (16), `GRID_MENU_LEVEL_DOWN` (17) | Moves one level up or down |
| `GRID_MENU_UNTILT_UP_LEVEL_UP` (18), `GRID_MENU_UNTILT_UP_LEVEL_DOWN` (19) | Returns from `GRID_MENU_TILTED_UP` to level, then enters `GRID_MENU_LEVEL_UP` or `GRID_MENU_LEVEL_DOWN` |
| `GRID_MENU_UNTILT_DOWN_LEVEL_UP` (20), `GRID_MENU_UNTILT_DOWN_LEVEL_DOWN` (21) | Returns from `GRID_MENU_TILTED_DOWN` to level, then enters `GRID_MENU_LEVEL_UP` or `GRID_MENU_LEVEL_DOWN` |
| `GRID_MENU_UNTILT_UP_OPEN_NODE` (22), `GRID_MENU_UNTILT_DOWN_OPEN_NODE` (23) | Returns from a tilt to level, then enters `GRID_MENU_OPEN_NODE` |
| `GRID_MENU_UNTILT_UP_SYSTEM_MENU` (24), `GRID_MENU_UNTILT_DOWN_SYSTEM_MENU` (25) | Returns from a tilt to level, then enters `GRID_MENU_SYSTEM_MENU` |
| `GRID_MENU_ENTER` (26) | Initial state playing the grid entry clip before `GRID_MENU_REST` |
| `GRID_MENU_IDLE_MEDIA` (27) | Expands the quad and plays idle media after 500 idle frames |
| `GRID_MENU_LINK_TRAVEL` (28) | Travels to a node sharing a chosen word and flashes Lain's frame (`enum LinkTravelStep`) |
| `GRID_MENU_NO_SHARED_NODE` (29) | Shows the no-shared-node panel until circle |

## The resting states

`GRID_MENU_REST`, `GRID_MENU_TILTED_UP` and `GRID_MENU_TILTED_DOWN` are the grid at rest: the level view, the view
tilted up and the view tilted down. Each frame they read the pad (`ReadPadEvent`), play Lain's idle clips and, once
the caption has stopped moving and the frames to skip after a screen (`gSkipInputFrames`) are spent, hand the pad
event (`enum PadEvent`, `include/pad.h`) to `HandleGridInput`:

| Pad event | Button | On the grid |
| --- | --- | --- |
| `PAD_EVENT_UP`..`PAD_EVENT_DOWN_RIGHT` (1 to 8) | d-pad or left stick | move the cursor (`MoveGridCursor`) |
| `PAD_EVENT_L1` (12), `PAD_EVENT_R1` (13) | L1, R1, or the right stick left and right | turn the grid one column left or right (`GRID_MENU_TURN_LEFT` and `GRID_MENU_TURN_RIGHT`) |
| `PAD_EVENT_RIGHT_STICK_UP` (10), `PAD_EVENT_RIGHT_STICK_DOWN` (11) | the right stick up and down | tilt the view up (`GRID_MENU_TILT_UP`) or down (`GRID_MENU_TILT_DOWN`), or back to level (`GRID_MENU_UNTILT_FROM_DOWN` and `GRID_MENU_UNTILT_FROM_UP`) |
| `PAD_EVENT_R2` (9) | R2 | from the level view, tilt up and down on alternate presses; from a tilt, back to level |
| `PAD_EVENT_CIRCLE` (17) | circle | open the node under the cursor |
| `PAD_EVENT_TRIANGLE` (19) | triangle or select | the system menu (`GRID_MENU_SYSTEM_MENU`) |
| `PAD_EVENT_L2` (14) | L2 | Jump To (`GRID_MENU_JUMP_TO`) |
| `PAD_EVENT_SQUARE` (21) | square | show or hide the caption's four title plates (`gShowAllTitles`) |

Cross (`PAD_EVENT_CROSS`, 18) and start (`PAD_EVENT_START`, 20) do nothing here. `PAD_EVENT_NEXT_LEVEL` (15) and
`PAD_EVENT_PREVIOUS_LEVEL` (16), a level up and down, come from `MoveGridCursor` alone. In a tilt, moving a level,
opening a node and the system menu first bring the view back to level (`GRID_MENU_UNTILT_UP_LEVEL_UP` to
`GRID_MENU_UNTILT_DOWN_SYSTEM_MENU`) and then go on as from `GRID_MENU_REST`; turning and Jump To keep the tilt.

## The cursor

The cursor is a column and a row: `gCursorColumn`, 0 to 7, and `gCursorRow`, counted up from the bottom row of
level 0, so its level is `gCursorRow` / 3 and its cell `gCursorColumn` + 8 × (`gCursorRow` mod 3). `gGridHeading`
counts the grid's turns: column `gGridHeading` is the one at the front on the left.

The eight columns stand round a cylinder that the view looks through: the two columns nearest the viewer are on the
outside, and the two at the far side of the cylinder appear between them. Counted from `gGridHeading`, these four
are columns 0, 3, 4 and 7 from left to right, and `gColumnLeftOf`, `gColumnRightOf` and `gColumnOrder` give their
order. Left at the leftmost or right at the rightmost turns the grid instead (`PAD_EVENT_L1` and `PAD_EVENT_R1`).

Up and down move a row; past the top or bottom row of a level they move a level (`PAD_EVENT_NEXT_LEVEL` and
`PAD_EVENT_PREVIOUS_LEVEL`), if there is one. When the cell the cursor would reach is empty, `SnapNextCursorToNode`
looks for a node close by on the same level, first in the other rows of the column, then in the neighbouring
columns facing the viewer, filtered by `enum CursorSnapMode` (`CURSOR_SNAP_ANY` 0, `CURSOR_SNAP_SAME_COLUMN` 1,
`CURSOR_SNAP_NO_UP` 2, `CURSOR_SNAP_NO_DOWN` 3, `CURSOR_SNAP_NO_RIGHT` 4, `CURSOR_SNAP_NO_LEFT` 5; `MoveGridCursor`
passes `CURSOR_SNAP_NO_RIGHT` and `CURSOR_SNAP_NO_LEFT` in `rowMode` rather than `columnMode`). A vertical move
that finds none moves a whole level; a sideways move that finds none turns the grid. During a turn or a level move
the caption rides on the cursor's cell (`UpdateCursorDuringMove`); at the end the cursor settles on the cell it was
heading for, or on a node next to it, and if there is none the caption reads "Unknown".

The caption itself, the node's name and titles on a plate that slides from cell to cell, is drawn by `src/text.c`
(`UpdateCaption`, `PlaceCellCaption`); while it slides the menu takes no input.

## Turning, tilting and moving a level

A turn (`GRID_MENU_TURN_LEFT` and `GRID_MENU_TURN_RIGHT`) loads Lain's three turning clips, waits for the reads and
for her current idle clip to end, then starts `SCENE_CMD_TURN_LEFT` or `SCENE_CMD_TURN_RIGHT` with the first clip
and plays sound 6. The state ends with the last clip.

A tilt (`GRID_MENU_TILT_UP` to `GRID_MENU_UNTILT_FROM_DOWN`) plays sound 29 and sends `SCENE_CMD_TILT_UP` or
`SCENE_CMD_TILT_DOWN`; the state ends when the scene is no longer busy, in `GRID_MENU_TILTED_UP` or
`GRID_MENU_TILTED_DOWN` for a tilt and `GRID_MENU_REST` for level.

A level move (`GRID_MENU_LEVEL_UP` and `GRID_MENU_LEVEL_DOWN`) loads one clip, and when it starts sends
`SCENE_CMD_LEVEL_UP` (sound 4) or `SCENE_CMD_LEVEL_DOWN` (sound 5). The scene moves the grid a level and changes
`gGridLevel`.

States 11, 12, 14 and 15 share the turn's code (11 and 12 with `GRID_MENU_TURN_LEFT`, 14 and 15 with
`GRID_MENU_TURN_RIGHT`), but nothing enters them.

## Opening a node

Circle asks `PickCursorClipSet` which of Lain's five clip sets to play. On an empty cell there is none, and only
`gHarumagePercent` counts the press. Otherwise sound 0 plays, the set's four clips load, and `GRID_MENU_OPEN_NODE`
waits for them. When the first starts, the scene begins the cell's flight (`SCENE_CMD_CELL_FLIGHT`, `gOpeningNode`
set). When the last ends the menu returns to its resting state and, for sets below `OPEN_CLIP_REFUSE_SPARK`
(`OPEN_CLIP_SPIN_AWAY` (0) and `OPEN_CLIP_COLLAPSE_SPLIT` (1) in `enum NodeOpenClipSet`, `include/lapks_player.h`), opens the node:

- **A media node.** The music fades, the node's name and words are written into VRAM at X 320 (`TEXT_UPLOAD_SMALL`)
  for the media player to show, Lain's idle clips are freed, and `RunMediaPlayer` runs with the node's track, its
  end-roll bit and its image parts ([media-player.md](media-player.md)). It returns `MEDIA_RESULT_COMPLETE` when
  the track played to its end (and always for the voice screen): the node is marked opened (`MarkNodeOpened`) and
  its cell takes the opened model. It returns `MEDIA_RESULT_END_ROLL` after an end roll: the menu counts it, marks
  the node, runs the continue prompt (`RunContinuePrompt`) and leaves.
  If the player chose one of the node's words, `FindNodeSharingWord` looks for another node with it: found, the
  cursor goes to it and the scene travels there with Lain's frame hidden (`GRID_MENU_LINK_TRAVEL`,
  `SCENE_CMD_TRAVEL_TO_CURSOR` in `LINK_TRAVEL_MOVE` (0)), and the frame flashes back in
  (`SCENE_CMD_FLASH_CLIP_FRAME` in `LINK_TRAVEL_FLASH` (1)); not found, sound 28 and a panel
  (`DrawNoSharedNodePanel`) until circle (`GRID_MENU_NO_SHARED_NODE`).
- **An upgrade node (`NODE_KIND_UPGRADE`).** `RunUpgradeScreen`; if the upgrade is taken the node is marked, its
  cell emptied and `gUpgradesApplied` counted.
- **A gate (`NODE_KIND_GATE`).** `RunGateScreen`; the node is marked and its cell emptied.
- **A P2 node (`NODE_KIND_P2`).** Its bit is set in `gP2NodesOpened`, `RunP2Screen` runs, and the node is marked
  and its cell emptied.

The music fades back in afterwards, and after the media player the menu skips two frames of input. The refusal sets
(`OPEN_CLIP_REFUSE_SPARK` (2) through `OPEN_CLIP_REFUSE_VANISH` (4)) open
nothing. Every frame of `GRID_MENU_OPEN_NODE` except the one on which a node opens raises `gGakkuriPercent`, up to
100, so it counts the frames of every opening clip, not refusals alone.

## Jump To

L2 opens a panel for choosing a level to travel to (`GRID_MENU_JUMP_TO`, sprites `gJumpToSprites` indexed by
`enum JumpToSprite`: `JUMP_TO_SPRITE_TENS` 0, `JUMP_TO_SPRITE_ONES` 1, `JUMP_TO_SPRITE_UP_ARROW` 2,
`JUMP_TO_SPRITE_DOWN_ARROW` 3, `JUMP_TO_SPRITE_BAR` 5), driven by `enum JumpToStep`:

| Step | |
| --- | --- |
| `JUMP_TO_INIT` (0) | The caption moves to (138, 106), switches to `CAPTION_STYLE_PLAIN` and reads "Jump To"; the panel's two digits show the current level, counted from 1 |
| `JUMP_TO_SLIDE_IN` (1) | The panel slides in, nine frames (`SlideJumpToSpritesIn`), while the caption settles |
| `JUMP_TO_CHOOSE` (2) | Up raises the level and down lowers it, by one a press, at most five from the current level and within the Site's levels; an arrow changes colour as the level moves and hides at the limit. Circle accepts, cross cancels, and leaving restores `CAPTION_STYLE_RESTORE_CELL` |
| `JUMP_TO_SLIDE_OUT` (3) | The panel slides out |
| `JUMP_TO_CHECK_TRAVEL` (4) | If the chosen level changed (`gJumpToLevelChanged`), advances to `JUMP_TO_LOAD_CLIP`; otherwise returns to the resting state |
| `JUMP_TO_LOAD_CLIP` (5) | The level-move clip loads |
| `JUMP_TO_START_TRAVEL` (6) | When it starts, the cursor moves by the levels chosen, settling on a node if its cell is empty, and the scene travels there (`SCENE_CMD_LEVEL_TRAVEL`) |
| `JUMP_TO_WAIT_TRAVEL` (7) | The clip and the scene finish |
| `JUMP_TO_FINISH` (8) | The caption is put on the new cell and the menu rests again |

The digits come from one texture: 0 to 7 in a row at V 24, 8 and 9 at V 56, 24 texels each (`SetJumpToDigits`).

## The system menu

Triangle (`GRID_MENU_SYSTEM_MENU`) plays sound 7, loads Lain's system-menu clip and steps through
`enum SystemMenuStep`:

| Step | |
| --- | --- |
| `SYSTEM_MENU_LOAD_CLIP` (0) | Loads and polls the system-menu clip; once read, sends `SCENE_CMD_RING_BURST` |
| `SYSTEM_MENU_WAIT_BURST` (1) | Waits for the clip and the burst to finish |
| `SYSTEM_MENU_START_RISE` (2), `SYSTEM_MENU_WAIT_RISE` (3) | Sends `SCENE_CMD_VIEW_TO_TOP` and waits for the camera path |
| `SYSTEM_MENU_OPEN` (4) | Runs `StepSystemMenuOpen` until the menu is open |
| `SYSTEM_MENU_RUN` (5) | Runs `UpdateSystemMenu` ([system-menu.md](system-menu.md)); `MENU_RESULT_EXIT` advances to closing, `MENU_RESULT_LOAD` or `MENU_RESULT_CHANGE` leaves `RunGridMenu` with `GRID_MENU_EXIT_LOAD` or `GRID_MENU_EXIT_SITE_CHANGE` |
| `SYSTEM_MENU_START_CLOSE` (6), `SYSTEM_MENU_CLOSE` (7) | Sends `SCENE_CMD_VIEW_TO_FRONT` and runs `StepSystemMenuClose` |
| `SYSTEM_MENU_WAIT_RETURN` (8) | Waits for `gGridBusy == 0` and returns to `gGridMenuReturnState` |

## Idle

`gIdleFrames` counts frames with no pad event in the resting states. Lain's idle clips (`UpdateIdleClips`) keep
playing; from 100 idle frames on, new ones are loaded, one at a time, chosen at random from `gIdleClipParts` (38
entries, every other one `LAPKS.BIN` part 8). After 500 idle frames, with no clip read outstanding,
`GRID_MENU_IDLE_MEDIA` sends `SCENE_CMD_QUAD_EXPAND` (the expanding quad), waits for the scene, clears the screen,
frees the idle clips, fades the music, plays sound 30 and an idle media (`PlayIdleMedia`,
[media-player.md](media-player.md)), then returns to the resting state; `gDokudenpaCount` counts these, up to 9.
