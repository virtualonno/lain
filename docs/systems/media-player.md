# The media player

Opening a media node on the grid runs the media player: `RunMediaPlayer(track, &endRollOrWord, imageParts)`
(`src/media_player.c`), called by `RunGridMenu` (`src/grid_menu.c`) with the node's track, its end-roll flag (bit 27
of the node's flags) and its three image parts. A track is a movie, streamed from the disc and decoded by the MDEC,
or an XA audio channel, played by the drive straight into the sound processor while the screen shows the node's
images and words. The same unit plays idle media when the grid is left alone, and the end roll. A node's track from
518 on is not played here but by the voice screen ([speech.md](speech.md)). The cube of the player's Play and Exit
controls is `src/media_cube.c` and its models `src/media_cube_tmd.c`.

## Tracks

`gTracks` has 736 entries (`Track`, `include/media_player.h`), and a node's track is its own index in `gNodes`.
`xaFile` is 0 for a movie and otherwise a row of `gXaFiles` counted from 1; `source` is the movie's row of the
disc's file table, or the XA channel; `length` is the movie's frame count, or the channel's size in bytes.

| Tracks | Are | Played by |
| --- | --- | --- |
| 0 to 56 | movies, the nodes of kind `NODE_KIND_DC` (4) | the player |
| 57 to 517 | XA tracks | the player |
| 518 to 698 | XA tracks | the voice screen; below 639 it also speaks the player's name |
| 699 to 708 | XA tracks of the gate and P2 nodes, which do not play them | idle media on disc 2 |
| 709 to 732 | movies no node plays | idle media |
| 733, 734 | XA tracks | the voice screen after the end roll |
| 735 | the end roll movie | `PlayEndRoll` |

Twenty-one XA files hold the channels (`gXaFiles` gives each one's row of `gDiscFiles`). The disc's file table,
`gDiscFiles`, is the current disc's ([files.md](files.md)), so the same track can name a different file on each
disc.

## What the player returns

`RunMediaPlayer` mutes the CD audio, records the track and its images, and for a track from 518 on runs
`RunVoiceScreen` and returns `MEDIA_RESULT_COMPLETE` (2). Otherwise it shows the caption "Play" and calls
`PlayMediaTrack` until the player asks to leave (`gPlayerExit`): a track that ends leaves the player on its screen,
ready to play again. It returns `gPlayerResult` (`enum MediaPlayerResult` in `include/media_player.h`), which starts
at `MEDIA_RESULT_NONE` (0) and becomes `MEDIA_RESULT_COMPLETE` (2) once a movie has run to its end or once an XA
track has (from the drive's `CdlComplete`), and stores `gChosenWord`, the focused word's slot plus one when the player
left by a word (0 otherwise), through `endRollOrWord`.

`gEndRollState` (`enum EndRollState` in `include/media_player.h`) tracks the end roll:

| State | Value | Meaning |
| --- | ---: | --- |
| `END_ROLL_NONE` | 0 | normal playback; words are drawn and focusable, leaving is allowed |
| `END_ROLL_READY` | 1 | node has the end-roll flag (bit 27); words are hidden and unfocusable until Play is pressed |
| `END_ROLL_PENDING` | 2 | Play was pressed on an end-roll node: leaving is refused and XA end sets `gTrackDone` instead of rewinding (also set by `PlayIdleAudio` and `StepVoiceScreen` for one-shot XA playback) |
| `END_ROLL_PLAYING` | 3 | `PlayEndRoll` is streaming the end-roll movie and `GetMovieFrame` tracks `gEndRollSegments` |

Once an `END_ROLL_PENDING` track has ended, `RunMediaPlayer` plays the end roll (`PlayEndRoll`), runs the voice screen
as track 733 and returns `MEDIA_RESULT_END_ROLL` (1). The four nodes with the flag (`Cou053`, `Dia048`, `Lda237`,
`Tda092` in `src/nodes.c`) are all XA tracks on disc 2, so the branch that marks a movie's node opened before the end
roll never runs.

## The player's screen

On its first round `PlayMediaTrack` loads the player's interface image (part 21 of `BIN.BIN`, napk-packed, to
448,0; `LoadMediaPlayerHudTimInline`) and sets up its sprites: the progress bar and its frame, the name plate, and
three word plates with the node's words. Each frame then:

- clears the draw buffer to black for an XA track; for a movie the buffer receives the decoded frame, or the still;
- steps the caption (`UpdateCaption`, `src/text.c`) and the track image (`UpdateTrackImage`);
- draws the progress bar's frame and, while a track plays, the bar itself from `gTrackProgress` (0 to 18);
- for an XA track, draws the level meter, the three words and their focus lines, the animated image-corner mark,
  the track image (`gTrackImage`, a `SpriteQuad`) and the system menu's tile grid (`DrawTileGrid`,
  `src/system_menu.c`);
- draws the cube (`DrawMediaPlayerCube`), and ends with `VSync(5)`, or `VSync(0)` while a movie is under way.

While the caption moves no input is read. Otherwise the pad event (`enum PadEvent`, [frame.md](frame.md)) acts on
`gPlayerSelection` (`enum PlayerSelection` in `include/media_player.h`):

| Input | Selection (`gPlayerSelection`) | Does |
| --- | --- | --- |
| up | `PLAYER_SELECT_EXIT` (0) | `PLAYER_SELECT_PLAY` (1), caption "Play" |
| down | `PLAYER_SELECT_PLAY` (1) | `PLAYER_SELECT_EXIT` (0), caption "Exit" |
| circle | `PLAYER_SELECT_PLAY` | an XA track starts (`StartXaTrack`); a movie is started once the caption "Exit" has arrived (`gMovieStartPending`) |
| circle | `PLAYER_SELECT_EXIT`, or `PLAYER_SELECT_WORD` (2) | leave, unless `gEndRollState` is `END_ROLL_PENDING`; leaving by a word sets `gChosenWord` |
| right | `PLAYER_SELECT_PLAY` or `PLAYER_SELECT_EXIT`, XA only | focus the words (`PLAYER_SELECT_WORD`) |
| left | `PLAYER_SELECT_WORD` | back to `PLAYER_SELECT_PLAY` or `PLAYER_SELECT_EXIT` |
| up, down | `PLAYER_SELECT_WORD` | turn the ring of words (`PLAYER_SELECT_WORD_UP` (3) or `PLAYER_SELECT_WORD_DOWN` (4)) until the next word reaches the focus |

Up and down are ignored while a movie plays, so its selection stays on Exit. Accepting plays sound program 26 and
leaving 27 ([sound.md](sound.md)).

The three words sit on a ring of 30 positions (`gWordRing`), ten apart (`gWordRingPos`); while the ring turns, each
frame moves every word one position and the focus point (`gFocusRingPos`) one the other way, until the focus meets
a word, which becomes `gFocusedWord` and is drawn highlighted.

## The cube

`InitMediaPlayerCube`, called each time the player starts, builds the cube from nine models: a point model
(`gModels[40]`, `gPointTmd`, at `gCubePosition`) and eight pieces linked to it (`gModels[41]` to `[48]`), four Play
(`CUBE_PIECE_PLAY` (0)) and four Exit (`CUBE_PIECE_EXIT` (1)) pieces (`enum CubePieceKind`) as `gCubePieceKinds` says
(`gPlayPieceTmd`, `gExitPieceTmd`), and uploads their two textures (`gCubePieceTim`, `gCubePieceSelectedTim`,
`src/tim_data_4.s`). In `gCubePhase` (`enum CubePhase`), `CUBE_ASSEMBLING` (0) moves each piece a tenth of the way
from its start (`gCubePieceStarts`) to its corner (`gCubeCorners`) for ten frames; after that `CUBE_ASSEMBLED` (1)
draws the cube assembled. The four pieces of the selected control are drawn with their selected models
(`gPlayPieceSelectedTmd`, `gExitPieceSelectedTmd`), and the first of them, piece 0 for Play or 1 for Exit, turns 45
units a frame about its vertical axis. While a word is focused or the caption moves, the turn is reset to 0;
`selectedKind` is then not set in the source.

## XA tracks

`StartXaTrack(channel)` sets the drive's filter to file number 1 and the track's channel (`CdlSetfilter`) and works
out the sectors to play: from the first sector of the XA file to that plus 32 sectors for every 2,336 bytes of the
channel's length (for tracks below 518, less 5,139.2 bytes first). `StartXaRead` sets the drive to double speed,
real-time audio and the subheader filter (`CdlModeSpeed | CdlModeRT | CdlModeSF`) and starts reading
(`RestartXaRead`, `CdlReadS`); for tracks below 518 it also turns the sound processor's interrupt on, for the level
meter (below). The drive was already moved to the file's start by `SeekXaFile` (or `SeekCurrentXaFile` in `PlayIdleAudio`).

The audio needs no help from the game while it plays: the drive decodes the selected channel and sends it to the
sound processor's CD input. The game only watches. `PollXaPlayback`, run from the vertical-blank callback
(`HandleVSync`, [frame.md](frame.md)) on every fourth blank while `gXaReading` is set, asks the drive for its state
without waiting (`CdSync(1, ...)`):

- after a disc error it starts the read again (`RestartXaRead`, which reads from where the head is: the start
  position it computes is not passed on);
- once a position report has arrived (`CdlGetlocP`), it takes the absolute sector into `gXaSector` and moves the
  progress (`gTrackProgress`, eighteenths of the track) forward, then asks for the next report;
- when `gXaSector` is past the end, or more than 300 sectors before the start, the track is over: the interrupt is
  turned off, the CD audio muted, the drive paused, the progress cleared and the result set. Unless an end roll is
  pending, the first image is shown again and the drive moved back to the file's start for the next Play; with one
  pending (`END_ROLL_PENDING` (2)) the track is ended instead (`gTrackDone`).

`gEndRollState` is compared with the drive's status there, which at that point is always `CdlComplete`.

### Track images

A node has up to three images, parts of the current disc's site file (`SITEA.BIN` or `SITEB.BIN`, file 6) given by
`imageParts` (-1 for none). `LoadTrackImages` reads each, unpacks it (napk, [formats.md](../formats.md)), loads the
8-bit image and its palette into video memory (at 320,0, 320,256 and 448,256, palettes on rows 201 to 203), and
moves it three units up and left to drop its border; `sTrackImageSizes` keeps each one's shown size. The first is
shown at once. `UpdateTrackImage` steps `gImageChangeStep` (`enum ImageChangeStep`): `IMAGE_CHANGE_SQUASH` (0) squashes the sprite's
height to nothing in steps of a quarter, `IMAGE_CHANGE_SWITCH` (1) switches image, and `IMAGE_CHANGE_GROW` (2) grows
it back — at progress 6 to the second image and at 12 to the third, or with only two images at 9 to the second. A
missing image stops the changes.

### The level meter

Fifteen bars of up to seven 16-pixel cells (`sMeterRows`), one above the other, show the loudness while an XA track
plays. The middle bar's length is the level (`GetCdPeakLevel`, below, at most 7); the bars on either side step down
one at a time from it, and a bar that would be empty gets a small random length instead (0 to 4, or 0 to 2 when the
level is 0). The outer half of each bar's cells use the next palette row.

### Reading the sound back

The sound processor keeps the CD audio it has just played in a small ring in sound memory, and can raise an
interrupt when that ring reaches a set address. The player uses this to measure the loudness. `PlayMediaTrack` (and
the voice screen, the same way) allocates `gDecodedAudio` (4 KB), sets two callbacks and the interrupt address
0x100:

- `ReadDecodedAudio`, the interrupt callback, turns the interrupt off and starts a transfer of the decoded CD audio
  into `gDecodedAudio` (`SpuReadDecodedData(gDecodedAudio, gDecodedDataFlag)`, the flag 5);
- `SwapSpuIrqAddr`, the transfer callback, moves the interrupt address between 0 and 0x200 (`gSpuIrqAddr`) and
  turns the interrupt back on.

`GetCdPeakLevel` scans one half of the left channel's samples in `gDecodedAudio` (the first 256 when the interrupt
address is 0, the second 256 otherwise, each less its last 50) for the largest magnitude, and divides it by 4,681,
giving 0 to 7. The interrupt is turned off and the callbacks cleared when the track ends or the player leaves.

## Movies

A movie is an `.STR` file read by Sony's streaming library into a ring of 32 sectors (`gStreamRing`, `StSetRing`,
`StSetStream`) and decoded with the MDEC. Each frame in the ring is a `MovieFrame`: the two quantizers, the frame's
number and its bitstream, the same NITR-BS bitstream as a clip's ([clips.md](clips.md)). The decoder's state is
`sMovieDecoder` (`MovieDecoder`): two buffers of MDEC commands (`runLevel`, 0x28000 bytes each), two slice buffers
(`slice`), and the frame and slice rectangles.

`StartMovie(sector)` allocates the buffers, resets the MDEC with `LoadDecodedSlice` as its output callback, starts
the stream at double speed and real time (`StartStreamRead`, retrying seek and read until the read starts), builds
the code table (`BuildSignedRunLevelCodes(gNitrBsRunLevelCodes)`) and unpacks the first frame
(`UnpackNitrBsFrame`). If no frame comes it seeks again and retries. Each frame after that, `DecodeNextMovieFrame`:

1. starts the MDEC on the unpacked frame (`DecDCTin`, mode `gDecodeMode`, 2) and asks for the first slice
   (`DecDCTout`);
2. meanwhile takes the next frame from the ring (`ReadMovieFrame`, `GetMovieFrame`, `StGetNext`, each retried up to
   2,000 times) and unpacks it into the other command buffer, then frees its ring sectors;
3. waits for the MDEC to finish the frame (`WaitMovieFrameDone`, which after 0x800000 turns gives up and marks it
   done).

`LoadDecodedSlice` runs each time the MDEC has filled a slice: it asks for the next slice into the other buffer,
or, at the frame's right edge, marks the frame done, and loads the slice it was given into video memory: into the
draw buffer, or, before the movie has started, into the area at 320,240. A slice is a column 16 pixels wide and the
frame's height, in 16-bit colour (`gSliceWidth`, `gSliceBufferSize` 0x1E00).

`GetMovieFrame` also keeps the progress (the frame count from the sector header over an eighteenth of the length)
and ends the movie when that count, plus one, reaches the track's length less 4 (`gPlayerResult` becomes `MEDIA_RESULT_COMPLETE` (2)).

In the player a movie runs in three states (`gMovieState`, `enum MovieState`): `MOVIE_DECODE_STILL` (-1) decodes the
first frame into the still area at 320,240 and then pauses the drive, `MOVIE_STILL` (0) shows that still each frame
and waits, and `MOVIE_PLAYING` (1), set once the caption "Exit" has arrived after Play, restarts the stream
(`CdRead2`) and decodes into the draw buffer. When the movie ends or the player leaves, `StopMovie` clears the MDEC
callback, releases the ring (`StUnSetRing`) and frees the buffers; it pauses the drive only in `MOVIE_PLAYING`, which
the player has already reset to `MOVIE_DECODE_STILL`. On a later round of the same player the still is not loaded
again.

## Idle media

After 500 frames left alone (`gIdleFrames`), `RunGridMenu` calls `PlayIdleMedia`, which mutes the CD audio and, at
random, plays an idle movie (`PlayIdleMovie`) or an idle XA track (`PlayIdleAudio`, over the tile grid). Either
ends at the track's end or when circle is pressed.

- **The movie** is one of the tracks 709 to 730 on disc 1, or 724 to 731 on disc 2 (`rand()` scaled); once
  `gP2NodesOpened` is 63, disc 2 has a one-in-three chance of 731 or 732 instead. It plays full screen, without the
  player's interface.
- **The track** is one of ten from `gIdleTrackBase` (169 on disc 1, 699 on disc 2), shown with its images from
  `gIdleTrackImageParts`.

## The end roll

`PlayEndRoll` plays track 735 in 24-bit colour. It sets the display to 24 bits (`GsInitGraph2(320, 240, 4, 0, 1)`),
widens the frame to 480 video-memory columns (`gMovieWidth`, 320 pixels of three bytes), slices of 24 columns
(`gSliceWidth`, `gSliceBufferSize` 0x2D00), the MDEC's mode to 3 and the streaming library's mode to 1, under which
`LoadDecodedSlice` also services the library's CD interrupt (`StCdInterrupt`). It reads a still image, part 554 of
`SITEB.BIN` (napk; the end roll follows only disc 2 nodes), into `gEndRollStill`, plays the movie to its end, then
frees everything and puts the 16-bit settings and display back.

The end roll is cut into segments (`gEndRollSegments`, `EndRollSegment`: a still flag, a first and a last frame),
one for each node of kind `NODE_KIND_DC` (4) in node order. A segment whose node has never been opened (bit 26 of its flags) shows
the still instead of the movie: its frames are not unpacked or decoded, and the still is drawn in their place
(`gEndRollStillShown`). The segment counter stops at 54, so the last two of the 57 segments never begin.

## The CD mix

The CD input's volume is set through `SpuSetCommonAttr` everywhere: `UnmuteCdAudio` sets the master volume and the
CD volume to 0x3FFF, `MuteCdAudio` the CD volume to 0 and the master volume up to 0x4FFF. Idle media and the end
roll set the same pairs themselves.

## Unused and never set

`GetCdPeakLevel5` (the peak over 0 to 5) is called by nothing. `gMovieStartBlocked` and `gTrackImageScaleDisabled`
are cleared and tested but never set, and several bytes and words of the unit are written and never read
(`gTrackCancelled`, `gCompletedFrameIndex`, `gMovieBitstreamHead`, `gUnpackVSyncs`, and those named `gUnused...`).
`media_cube_tmd.c` also holds `gUnusedBandTmd`, which nothing reads, and the tetrahedron drawn on each level's text
band (`gTextBandTetrahedronTmd`); `media_cube.c` holds the band and ring positions the grid uses
(`gTextBandPosition`, `gLabelBandPosition`, `gRingRestPosition` and the two overshoots).
