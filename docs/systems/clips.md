# Clips

A clip is one of the short animations of Lain on the grid screen and the voice screen: a run of compressed frames,
each with a mask, stored as a part of `LAPKS.BIN` (file 4 of the disc's file table, the same on both discs; the
part table is `gLapksParts`, `src/file_parts.c`). `src/clip.c` opens a clip and decodes its frames into video
memory, using the MDEC and the game's own bitstream decoder (`src/lib/nitrbs/`); `src/lapks_player.c` reads clips
from the disc into slots on the heap and decides which one plays. Drawing the decoded frame on screen is the grid's
(`DrawClipFrame`, `src/grid_scene.c`). Movies are a different thing, streamed `.STR` files
([media-player.md](media-player.md)), though their frames use the same bitstream.

## A clip

The clip's own layout, as `OpenClip` and `DecodeClipFrame` read it (the file-level description is in
[formats.md](../formats.md)):

- 8 bytes: the magic "lapk" and a size, in 57 of the 59 clips the size of what follows. `OpenClip` skips them.
- the frame count, a 32-bit word (`gClipFrameCount`);
- one 12-byte record per frame (`struct ClipFrame`): the offset of its chunk from the end of the records, its
  position (`x`, `y`) and its sound cue (`soundCue`);
- the chunks (`struct ClipChunk`): width and height in pixels, the luminance and chrominance quantizers, the offset
  of the frame's mask counted from that field, and the frame's bitstream, which opens with its count of run/level
  codes;
- each frame's mask (`struct ClipMask`): an unused word, the unpacked length, and the packed bytes.

`gClipFrame` points at the current frame's record, `gClipFrameData` at the first chunk; `AdvanceClipFrame` moves to
the next record and returns 0 after the last.

## Decoding a frame

`InitClipDecoder(320, 0)`, run once at boot by `InitClipPlaybackSlots`, resets the MDEC (`DecDCTReset`), sets the
place frames are decoded to (`gClipVramX`, `gClipVramY`: 320,0, beside the display area) and builds the decoder's
sign table from `gRunLevelCodes` (`BuildSignedRunLevelCodes`). Then `DecodeClipFrame(&gClipFrameInfo)` takes the
current frame:

1. A frame whose run/level count is above 0x3FFFF or whose height is 360 or more is refused: the area from 320,0 to
   640,480 is cleared to white instead.
2. The area from 320,0, 320 wide and the frame's height, is cleared to black.
3. The quantizers go into `gLumaQuant` and `gChromaQuant`, and `UnpackNitrBsFrame` turns the bitstream into MDEC
   commands in `sRunLevels`.
4. The MDEC is started on them in 16-bit mode (`DecDCTin(sRunLevels, 0)`) and the mask is unpacked meanwhile
   (`DecompressClipMask`).
5. The frame is taken out a column 16 pixels wide at a time: each column is received into `sColumnPixels`
   (`DecDCTout`, waiting with `DecDCToutSync`), masked (`ApplyClipMask`) and loaded into video memory at its place.

`gClipFrameInfo` then holds the frame's position, size and sound cue for the drawer and the player. The routine
also counts frames and the vertical blanks each stage took (`gClipFramesDecoded`, `gClipUnpackVSyncs`,
`gClipMaskVSyncs`, `gClipUploadVSyncs` and others); nothing reads those counts.

## The bitstream decoder

`UnpackNitrBsFrame` (`src/lib/nitrbs/DECODE.s`, a library of the program's own, in assembly) turns a frame's
bitstream into the command buffer the MDEC takes. Its input is the bitstream with its halfword count first; the
output is rounded up to a 64-halfword block and padded with the MDEC's end code. Each macroblock is two chrominance
and four luminance blocks; each block opens with a ten-bit DC level, written with the luminance or chrominance
quantizer (`gLumaQuant`, `gChromaQuant`), and continues with variable-length AC codes, looked up eight bits at a
time, with an escape that gives six run bits and a level byte and, when needed, more. `BuildSignedRunLevelCodes`
(`src/lib/nitrbs/INIT.c`) prepares the code table it reads, `gSignedRunLevelCodes`: the first 111 run/level codes
of a table, then the same codes with the level negated. Clips build it from `gRunLevelCodes` (`src/clip.c`), movies
from `gNitrBsRunLevelCodes` (`DECODE.s`); the two tables begin with the same 111 codes.

## Masks

A frame's mask is packed with LZSS. `DecompressClipMask` reads a flag byte, then for each of its eight bits,
highest first, either one literal byte or a pair (distance less one, length less three) copying earlier output,
until the mask's length is reached. The unpacked mask (`sMaskBuffer`, 28,800 bytes) has two bits a pixel, a byte
for four pixels, in rows as wide as the frame.

`ApplyClipMask` applies it to each 16-pixel column before the column is loaded. A byte of 0 clears its four pixels
to 0 and a byte of 255 leaves them alone. Otherwise each half byte covers two pixels: 0 clears both, 15 leaves both
alone, and in any other half byte each two-bit code acts on one pixel: 0 clears it, 1 or 2 keeps it with bit 15
(the GPU's semi-transparency bit) set, 3 keeps it with bit 15 clear (`sMaskAndBits`, `sMaskOrBits`). A texel of 0
is transparent to the GPU, so the cleared parts of the frame do not show when it is drawn as a sprite; the codes 1
and 2 mark the pixels drawn semi-transparent where the sprite asks for it.

## Slots

`InitClipPlaybackSlots` (from `main`, `src/main.c`) also sets up the heap ([frame.md](frame.md)): `sHeapArea`,
868,352 bytes in `lapks_player.c`, is handed to `InitHeapArea`, and every clip buffer is a heap block. It then
reads two clips and waits for them: part 9 into the default idle slot (`sDefaultIdleSlot`), which stays loaded, and
part 8 into the one idle slot (`sIdleSlots`). A slot (`ClipSlot`) holds the buffer, the read's request handle, a state (`enum ClipSlotState`: `CLIP_SLOT_FREE` (0),
`CLIP_SLOT_READING` (1), `CLIP_SLOT_READY` (2), `CLIP_SLOT_FAILED` (-1)), a sound program and a play count. There are
two kinds:

- **Idle clips** play while nothing else does. `UpdateIdleClips`, called each frame by the grid, polls the idle
  slot's read, and when the slot is free (and its argument does not forbid it) loads a random clip into it
  (`LoadRandomIdleClip`): one of the 38 entries of `gIdleClipParts`, every other one of which is part 8, so half of
  the draws are part 8 again. Parts 36 and 42 bring a sound program (20 and 21). Then `PlayIdleClip` plays one
  frame: the ready idle slot with the lowest play count if there is one, otherwise the default clip. When an idle
  clip ends its buffer is freed, but the slot is not refilled while it is still the one last played, so the default
  clip plays while the next random clip is read.
- **Sequences** are one to six clips that the grid or the voice screen asks for by name, read into the sequence slots
  (`sSequenceSlots`). A loader allocates each clip's buffer, freeing the ready idle clip unless it is playing
  (`PurgeIdle`) when an allocation fails, and starts all the reads once every buffer is had. A loader of one clip
  then tries its allocation once more; the loaders of several clips do not. If a buffer cannot be had it returns 0
  (the loaders of several clips free their buffers, the loaders of one clip mark the slot failed) and the grid asks
  again on a later frame. A poller returns 1 once every slot is ready, and 0 while a read is under way or when a slot
  is marked failed, printing `MEMORY TOO FEW` on the debug font for a failed one. A player plays the slots in order,
  one frame per call (`PlaySequenceClips`), freeing each clip as it ends: it returns 1 while a clip plays, -1 on the
  frame a clip before the last ends, and 0 when the last has ended; called after that, it plays an idle frame and
  returns 0. It opens a slot only when no clip is playing, so an idle clip still running when the sequence becomes
  ready is played to its end first, the player returning -1 on its frames.

| Sequence | Parts of `LAPKS.BIN` | Loader, poller, player | Started by |
| --- | --- | --- | --- |
| grid entry | 54 | `LoadGridEntryClip`, `PollGridEntryClip`, `PlayGridEntryClip` | loaded by `main`, played by `RunGridMenu` |
| column turn | 0, 18, 1 or 2, 20, 3 | `LoadColumnTurnClips(group)`, `PollColumnTurnClips`, `PlayColumnTurnClips` | the grid |
| level move | 14 or 15 | `LoadLevelMoveClip(group)`, `PollLevelMoveClip`, `PlayLevelMoveClip` | the grid |
| node open | a row of `gNodeOpenClipParts` (five rows of four) | `LoadNodeOpenClips(set)`, `PollNodeOpenClips`, `PlayNodeOpenClips` | the grid |
| system menu | 52 | `LoadSystemMenuClip`, `PollSystemMenuClip`, `PlaySystemMenuClip` | the grid |
| voice screen | 53, 55, 56, 58, 7, 57 | `LoadVoiceScreenClips`, `PollVoiceScreenClips` | `RunVoiceScreen` |

The voice screen plays only the first and the last of its six ([speech.md](speech.md)). Which grid action plays
which sequence, and the set chosen for a node, are the grid's (`src/grid_menu.c`).

An idle clip's sound program is started on every frame whose sound cue is set and stopped when the clip ends
(`StartSound`, `StopSound`, [sound.md](sound.md)). Only `PlayIdleClip` does this. The sequence players take a
slot's sound program into `gClipSoundProgram` and clear it at the end, but never start it, so the one sequence clip
with a program, the column turn's middle clip (program 11, `gColumnTurnSoundPrograms`), is played without it.

## Unused

`DecodeSequenceClipFirstFrame` (with `gFirstFrameSlots`), `UnusedReturnOne` and `UnusedClipStub1` to
`UnusedClipStub3` in `lapks_player.c` are called by nothing, and `sIdleLoadHolds` is only ever -1.
