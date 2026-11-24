# Sound

The game's own sound is one VAB bank and two SEQ sequences, all from `SND.BIN`, played through Sony's LIBSND
(`src/sound.c`, declared in `include/sound.h`). Everything else that is heard comes from elsewhere: the XA tracks
of media nodes and the voice screen are streamed from the disc by the media player
([media-player.md](media-player.md)), movies carry their own XA sound, and the spoken player name is built from
`VOICE.BIN` and played on a voice of its own ([speech.md](speech.md)). The layout of `SND.BIN` is in
[formats.md](../formats.md).

## Starting LIBSND

`InitSound(0)` runs once, from `main` (`src/main.c`), before the first screen. It starts LIBSND with the master
volume at zero (`SsInit`, `SsSetMVol(0, 0)`), gives it 23 voices (`SsSetReservedVoice(23)`), so that LIBSND keys
on only voices 0 to 22 and voice 23 stays free for the voice screen's speech, makes room for two sequences of one
track each (`SsSetTableSize(sSeqTable, 2, 1)`) and sets the tick mode to `SS_TICK240`.

It then reads the four parts of `SND.BIN` (file 5 of the disc's file table, the same on both discs), each with
`RequestFilePartRead(5, part, gSndParts, ...)` ([files.md](files.md)) and a loop that draws a loading frame
(`DrawLoadingFrame(0)`) until `PollCdRequest` reports the read done:

| Part | Holds | Read into |
| ---: | --- | --- |
| 0 | the VAB header, 18,976 bytes | `sVabHeader`, exactly that size, resident |
| 1 | the VAB body (the samples), 413,312 bytes | a heap block (`AllocHeapBlock`), freed once in sound memory |
| 2 | the grid's music, a SEQ of 1,402 bytes | `sGridMusicData`, 3,072 bytes, resident |
| 3 | the About screen's music, a SEQ of 727 bytes | `sAboutMusicData`, 3,072 bytes, resident |

The header is opened (`SsVabOpenHead`, the bank id kept in `gVabId`) and the body sent to sound memory
(`SsVabTransBody`), drawing loading frames until the transfer completes. Room reverb is turned on at depth 48 on
both sides, both sequences are opened on the bank (`gGridMusicSeq`, `gAboutMusicSeq`), the grid's music is
selected, LIBSND is started (`SsStart`), and after ten more loading frames the master volume goes up to 127.

If the header does not open or the body is refused, none of that happens: no sequence is opened, LIBSND is never
started, the master volume stays at zero and the body's heap block is not freed. The game runs silent of its own
sounds.

`main` calls `InitSound(1)` again each time it goes back to the start screen. That call does nothing: the whole
body of `InitSound` is inside `if (reset == 0)`. The bank and the sequences loaded at boot simply stay.

## Effects

An effect is a program of the one bank, keyed on at note 60 and full volume on both sides
(`SsUtKeyOn(gVabId, program, 0, 60, 0, 127, 127)`); LIBSND chooses the voice. There are two ways of playing one:

- `PlaySound(program)` plays and forgets. It keeps the voice in `gSoundVoice` and writes 120 into
  `gUnusedSoundBuf[0]`; nothing reads either. Almost every sound is played this way, by the grid
  (`src/grid_scene.c`, `src/grid_menu.c`, `src/grid_nodes.c`, `src/grid_effects.c`), the system menu, kana entry,
  the continue prompt, the P2 screen (`RunP2Screen`, `src/loading.c`), the gate screen (`RunGateScreen`,
  `src/memcard.c`) and the media player. The program number is the caller's choice; across the menus and the
  media player 1 accompanies a cursor move, 0 or 26 a choice taken and 27 a choice declined or an exit.
- `StartSound(program)` keeps the voice in `gStartedSoundVoice`, and `StopSound` keys that voice off
  (`SsUtKeyOffV`). Two things use it: a caption starting its transition (`UpdateCaption`, `src/text.c`, program 1,
  stopped when the leaving caption gives way to the new text), and an idle clip's own sound (`gClipSoundProgram`,
  `src/lapks_player.c`), started on each frame that carries a sound cue and stopped when the clip ends
  ([clips.md](clips.md)). There is only one remembered voice, so
  `StopSound` stops whichever of the two was started last.

## Music

The two sequences are the game's music: the grid's and the About screen's. `gMusicIndex` says which one the
music calls act on, 0 for the grid and 1 for About (`SelectMusic`):

- `PlayMusic` sets the sequence's volume to 127 on both sides and plays it, repeating without end
  (`SsSeqPlay(seq, SSPLAY_PLAY, SSPLAY_INFINITY)`).
- `FadeOutMusic` and `FadeInMusic` fade it down or up (`SsSeqSetDecrescendo` and `SsSeqSetCrescendo`, both
  with 127 and 240). Neither stops or starts it.
- `StopGridMusic` stops the grid's sequence, whatever `gMusicIndex` says.

`RunGridMenu` (`src/grid_menu.c`) plays the grid's music when it starts, fades it out when it opens a node and
before idle media, fades it back in when they return, and stops it on its way out. The About screen
(`RunAboutScreen`, `src/about.c`) fades the grid's music out, selects and plays its own, and on leaving fades that
out and selects and plays the grid's again.

`main` also mutes everything for one load: after the continue prompt's continue (`RunGridMenu` returning `GRID_MENU_EXIT_CONTINUE` (3)) the
master volume is zero while `StepTimLoadSequence(1)` loads and 127 again afterwards.

## The vertical blank

`HandleVSync` (`src/main.c`, [frame.md](frame.md)), the vertical-blank callback, counts the blank (`CountVSync`),
steps XA playback (`PollXaPlayback`, [media-player.md](media-player.md)) and calls `SoundVSync`, which is empty:
the game drives no sound of its own from the blank.

## Unused

`sound.c` also defines three routines that nothing calls and its header does not declare: `KeyOffVoice0`
(`SsUtKeyOff` on voice 0), `KeyOnProgram24` (program 24 at note 60) and `StopMusic` (stops the selected
sequence).
