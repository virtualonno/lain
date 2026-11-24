# What the program holds that play does not reach

A finding here is a fact about the shipped executable (`SLPS_016.03` / `SLPS_016.04`) or the two retail discs that play does not show or that contradicts what play suggests, proven directly from the decompiled source, its tables, and the retail disc images. Some of what follows was known to players or reported before this decompilation; every finding is stated here on the proof given, and where an earlier count differs, the proof shows why. Throughout, **no caller** means no call instruction, no address taken, and no table or callback reference in the linked executable, counted over the 321 compiled game routines (`0x80013138`–`0x8003E864`, listed in `config/lain/reports/SLPS_016.03.json` and ordered by `config/lain/SLPS.LNK`, distinct from the 19 `static inline` helpers whose bodies are expanded into their callers).

## Code nothing reaches

### Forty-four of the game's 321 routines have no caller

Of the game's 321 compiled routines (177,964 bytes), 44 routines (5,472 bytes, 3.07%) across 15 units have no caller in the linked executable. They include whole-file and named-file CD readers (`RequestFileRead`, `RequestNamedFileRead`, `FindFileSector`, `SeekCurrentRequest`), duplicate boot and reset wrappers (`InitProgram`, `InitGraphicsAndFont`, `ResetForStartScreen`), memory-card helpers (`UnformatMemCard`, `AcceptMemCardAndWait`), heap checks (`CheckHeapBlock`, `GetHeapPeakUsed`), `FreeMenuQuads`, six media helpers, seven graphics helpers, three sound helpers, three speech helpers, two voice-screen helpers, and six empty or constant stubs. In `RequestNamedFileRead`, `CdSearchFile` stores the found file position in `file.pos`, which the read queue never reads, so if called it would seek to whatever sector that slot previously held.

Proof: `config/lain/reports/SLPS_016.03.json` (`0x80013138`–`0x8003E864`); `src/cd.c` (`RequestFileRead`, `RequestNamedFileRead`, `SeekCurrentRequest`, `FindFileSector`), `src/disc_id.c`, `src/graphics.c`, `src/grid_menu.c`, `src/heap.c`, `src/host_log.c`, `src/lapks_player.c`, `src/main.c`, `src/media_player.c`, `src/memcard.c`, `src/sound.c`, `src/speech.c`, `src/system_menu.c`, `src/voice_screen.c`, `src/vram_dump.c`.

### A host VRAM screenshot dumper is linked into the retail executable

`DumpVramTim` (636 bytes) allocates a heap block, reads a rectangle of video memory and an optional 256-entry palette with `StoreImage2`, formats a TIM header, and writes the file to a development host in 4,096-byte chunks through Sony's `LIBSN` (`PCcreat`, `PCwrite`, `PCclose`). Nothing in the program calls `DumpVramTim`, and a retail console has no host filesystem. Its presence is the only reason the retail executable links four modules of `LIBSN` (264 bytes: `close`, `creat`, `write`, and `snwrite`).

Proof: `src/vram_dump.c` (`DumpVramTim`), `src/lib/libsn/`.

### Host logging and the debug font run at boot and produce no output

At boot, `main` calls `OpenHostLog("d:\\usr\\tmp\\psx.log")`, and before and after each run of the start screen it calls `HostLogPrintf("1left memory %d\n", GetLeftMemory())` and `"2left memory %d\n"`. All four routines in `src/host_log.c` are empty in the shipped build, so those calls discard their arguments. `main` also loads Sony's debug font at VRAM `(960, 256)` (`FntLoad`, `FntOpen`, `SetDumpFnt`), and clip pollers print `"MEMORY TOO FEW"` and `"PurgeIdle"` via `FntPrint`, but nothing in the program calls `FntFlush`, so the debug font is never drawn.

Proof: `src/main.c` (`main`, boot `OpenHostLog` / `FntLoad` / `FntOpen` / `SetDumpFnt` calls and outer-loop `HostLogPrintf` calls), `src/host_log.c` (`OpenHostLog`, `HostLogPrintf`), `src/lapks_player.c` (`PurgeIdle`, `PollColumnTurnClips`, `PollLevelMoveClip`, `PollNodeOpenClips`, `PollSystemMenuClip`, `PollGridEntryClip`, `PollVoiceScreenClips`).

### Fog and light fading are polled at run time but never enabled

`UpdateGridScene` calls `StepLightFade` on every grid frame, which steps a flat light toward a target colour while `gLightFading` is nonzero, and `RunGateScreen` calls `DisableFog` when leaving the gate scene. Nothing calls `StartLightFade` or `EnableFog` (both are among the 44 uncalled routines), so `gLightFading` stays 0 and the GPU light mode stays 0 throughout play.

Proof: `src/graphics.c` (`StepLightFade`, `StartLightFade`, `DisableFog`, `EnableFog`), `src/grid_scene.c` (`UpdateGridScene`), `src/memcard.c` (`RunGateScreen`).

### Four turn states in the grid menu have no transition into them

`RunGridMenu` dispatches on `gGridMenuState` (`enum GridMenuState`), and its turn-left and turn-right arms include `case 11`, `case 12`, `case 14`, and `case 15` alongside `GRID_MENU_TURN_LEFT` (10) and `GRID_MENU_TURN_RIGHT` (13). No assignment in the program sets `gGridMenuState` to 11, 12, 14, or 15, leaving those four labels unreachable.

Proof: `src/grid_menu.c` (`enum GridMenuState`; `RunGridMenu`, `switch (gGridMenuState)` arms `GRID_MENU_TURN_LEFT` through `15`).

### `SnapNextCursorToNode`'s horizontal exclusion guards are never triggered

`SnapNextCursorToNode(int columnMode, int rowMode)` (`enum CursorSnapMode`) skips searching right when `columnMode == CURSOR_SNAP_NO_RIGHT` (`4`) and skips searching left when `columnMode == CURSOR_SNAP_NO_LEFT` (`5`), while using `rowMode` values `CURSOR_SNAP_NO_UP` (`2`) and `CURSOR_SNAP_NO_DOWN` (`3`) to exclude upward or downward rows. All seven call sites in `src/grid_menu.c` pass `CURSOR_SNAP_ANY` (`0`) or `CURSOR_SNAP_SAME_COLUMN` (`1`) as `columnMode`; the two horizontal searches in `MoveGridCursor` pass `CURSOR_SNAP_NO_RIGHT` and `CURSOR_SNAP_NO_LEFT` in `rowMode` instead (`SnapNextCursorToNode(CURSOR_SNAP_ANY, CURSOR_SNAP_NO_RIGHT)` and `SnapNextCursorToNode(CURSOR_SNAP_ANY, CURSOR_SNAP_NO_LEFT)`). As a result, `columnMode != CURSOR_SNAP_NO_RIGHT` and `columnMode != CURSOR_SNAP_NO_LEFT` always evaluate to true.

Proof: `src/grid_menu.c` (`enum CursorSnapMode`; `SnapNextCursorToNode`, `columnMode != CURSOR_SNAP_NO_RIGHT` and `columnMode != CURSOR_SNAP_NO_LEFT` guards; `MoveGridCursor`, `UpdateCursorDuringMove`, and `RunGridMenu` (`GRID_MENU_JUMP_TO`) call sites).

### `UploadTextToVram`'s clear mode has no caller

`UploadTextToVram` (`src/text.c`) accepts three values of `enum TextUploadMode`: `TEXT_UPLOAD_LARGE` (0), `TEXT_UPLOAD_SMALL` (1), and `TEXT_UPLOAD_CLEAR` (2), which uploads a zeroed 64×8 strip to erase a block of VRAM. Every call site in the program passes `TEXT_UPLOAD_LARGE` (0) or `TEXT_UPLOAD_SMALL` (1); no call passes `TEXT_UPLOAD_CLEAR`.

Proof: `include/text.h` (`enum TextUploadMode`), `src/text.c` (`UploadTextToVram`, height `switch (mode)` case `TEXT_UPLOAD_CLEAR`).

### Memory-card save and load carry unreachable error branches

In `UpdateSystemMenu`'s `MENU_SAVE` state (`enum MemCardSaveStep`, `gMemCardStep` cases `MEMCARD_SAVE_RETRY_OVERWRITE_ACCEPT` (`9`) and `MEMCARD_SAVE_RETRY_FORMAT_ACCEPT` (`39`)), `switch (-result)` handles `case McErrNone` (`0`) inside `if (result != 0)`, so `case McErrNone` can never run. In both `UpdateSystemMenu`'s `MEMCARD_LOAD_POLL_READ` step (`MENU_LOAD`, `case McErrCardNotExist` (`1`)) and `RunStartScreen`'s `START_LOAD` state (`5`, `enum StartScreenState`, `sub == MEMCARD_LOAD_POLL_READ` (`2`)), a missing card during read decrements the retry counter (`gMenuCounter` or `waitCount`) and returns to the accept step (`MEMCARD_LOAD_ACCEPT` (`0`)), but reopening the file (`MEMCARD_LOAD_OPEN` (`100`)) resets that counter to `10` on every pass, so the `--counter == 0` branch never fires from the read step.

Proof: `src/system_menu.c` (`enum MemCardSaveStep`; `UpdateSystemMenu`, `MENU_SAVE` cases `MEMCARD_SAVE_RETRY_OVERWRITE_ACCEPT` and `MEMCARD_SAVE_RETRY_FORMAT_ACCEPT` `switch (-result)` at `McErrNone`, and `MENU_LOAD` cases `MEMCARD_LOAD_OPEN` and `MEMCARD_LOAD_POLL_READ`), `include/memcard.h` (`enum MemCardLoadStep`), `src/kana_entry.c` (`enum StartScreenState`; `RunStartScreen`, `START_LOAD`, `sub` cases `MEMCARD_LOAD_OPEN` and `MEMCARD_LOAD_POLL_READ`).

### Never-set guards never disable media playback or idle clip loading

In `src/media_player.c`, `gMovieStartBlocked` and `gTrackImageScaleDisabled` are cleared to `0` and tested for `== 0` before starting a movie or scaling track images, and no code ever writes a nonzero value to either. In `src/lapks_player.c`, `sIdleLoadHolds[6]` is initialized to `-1` and never written again, so `sIdleLoadHolds[0] < 0` in `UpdateIdleClips` is always true.

Proof: `src/media_player.c` (`gMovieStartBlocked` in `PlayMediaTrack`; `gTrackImageScaleDisabled` in `PlayIdleAudio` and `UpdateTrackImage`), `src/lapks_player.c` (`sIdleLoadHolds` in `InitClipPlaybackSlots` and `UpdateIdleClips`).

### The movie end-roll branch and `PlayMediaTrack`'s `StopMovie` pause check never run

In `RunMediaPlayer`, the branch `if (gTrackXaFile == 0) gNodes[track].flags |= 0x4000000;` under `if (gEndRollState == END_ROLL_PENDING)` (`2`, `enum EndRollState`) never runs because all four nodes carrying the end-roll bit (`Cou053`, `Dia048`, `Lda237`, `Tda092`) are XA audio tracks (`xaFile != 0`). In `PlayMediaTrack`, `gMovieState` is reset to `MOVIE_DECODE_STILL` (`-1`) immediately before calling `StopMovie`, so `StopMovie`'s `if (gMovieState == MOVIE_PLAYING)` (`1`) drive-pause check is never entered from the media player.

Proof: `include/media_player.h` (`enum EndRollState`), `src/media_player.c` (`enum MovieState`; `RunMediaPlayer`, `if (gEndRollState == END_ROLL_PENDING)` block; `PlayMediaTrack` exit cleanup; `StopMovie`, test of `gMovieState`), `src/nodes.c` (`gNodes[109, 157, 425, 517]`).

### Sequence clips never start their sound program

`LoadColumnTurnClips` assigns sound program 11 (`gColumnTurnSoundPrograms[1]`) to the middle clip of a column turn, and `PlaySequenceClips` copies each slot's sound program into `gClipSoundProgram` and clears it when the clip ends. Only `PlayIdleClip` ever calls `StartSound(gClipSoundProgram)`, so program 11 is never triggered by the column-turn clip.

Proof: `src/lapks_player.c` (`gColumnTurnSoundPrograms`, `LoadColumnTurnClips`, `PlayIdleClip`, `PlaySequenceClips`).

### The ring-burst pieces never tilt because `(short)(0 + 0.5)` stays zero

During frames 40 to 42 of the system-menu ring burst (`StepPieceBurst`, `enum BurstState` cases `BURST_WAIT_SCATTER` (`6`) and `BURST_SCATTER` (`7`)), the code starts `short gRingTiltDegrees` at `0` and updates it with `if (gRingTiltDegrees < 15) gRingTiltDegrees = (short)(gRingTiltDegrees + 0.5);`. Casting `0.5` to `short` truncates it back to `0` on every frame, so the 32 burst pieces remain flat.

Proof: `src/grid_effects.c` (`enum BurstState`; `StepPieceBurst`, `switch (gBurstState)` cases `BURST_WAIT_SCATTER` and `BURST_SCATTER`).

### Re-initializing sound on return to the start screen does nothing

When leaving the grid for the start screen, `main` calls `InitSound(1)`. Every statement in `InitSound` (`src/sound.c`) sits inside `if (reset == 0)`, so passing `1` returns immediately without reloading `SND.BIN` or resetting `LIBSND`.

Proof: `src/main.c` (`main`, outer-loop reset after the grid loop), `src/sound.c` (`InitSound`, `if (reset == 0)` guard).

### No debug menu, button combination, or special player name exists in the code

`ReadPadEvent` (`src/pad.c`) reads only controller port 1 and maps its state to a single `gPadEvent` code (`enum PadEvent`) per frame in fixed priority order (circle, cross, start, triangle/select, square, shoulder buttons, analog sticks, directions), suppressing repeated frames until the state changes; multi-button combinations cannot be detected. `gPlayerName` is written by `RomanizeKanaText` and `UnpackCardSaveBlock` and read only by `PackCardSaveBlock` and `RunVoiceScreen` (which passes it to `BuildSpeechPcm`), with no string comparison against any special name.

Proof: `include/pad.h` (`enum PadEvent`), `src/pad.c` (`ReadPadEvent`), `src/kana_entry.c` (`RomanizeKanaText`), `src/memcard.c` (`PackCardSaveBlock`, `UnpackCardSaveBlock`), `src/voice_screen.c` (`RunVoiceScreen`), `src/speech.c` (`BuildSpeechPcm`).

---

## Table rows and disc content nothing references

### Ten `Env` nodes sit off the grid, hiding twelve of the 51 title strings

`gNodes` holds 716 rows, of which 706 have a nonzero grid row and are openable during play. The remaining 10 rows—`Env001`, `Env002`, `Env004`–`Env008`, and `Env010`–`Env012` (rows 169–178; `Env003` and `Env009` do not exist)—all have `gridPos == 0`, so `BuildNodeGrid` skips them. Their XA audio tracks and image parts are played as Disc 1 idle audio (`gIdleTrackBase[0] == 169`), but their node names and the 12 strings of `gTitles[51]` referenced exclusively by those 10 rows (`gTitles[20]` `"network_voices?"`, `gTitles[22]` `"***unknown"`, and `gTitles[23..32]` `"ftp/..."`) never appear on screen.

Proof: `src/nodes.c` (`gNodes[169..178]`), `src/grid_nodes.c` (`BuildNodeGrid`, `row > 0` filter), `src/string_tables.c` (`gTitles[20, 22..32]`).

### Node kind 1 has zero entries, leaving two cell models and two icons unused

`gNodeKindModelTypes[12]` (`src/grid_nodes.c`) maps node kind 1 (omitted from `enum NodeKind` in `include/nodes.h`) to `gLainDarkCellTmd` and `gOpenedLainDarkCellTmd` (`gCellTmds[1]` and `[9]`) and to `gLainDarkIconTim` and `gOpenedLainDarkIconTim` (`gCellTims[1]` and `[9]`). Across all 716 rows of `gNodes`, zero nodes have kind 1 (or kinds 10 and 11), so neither model and neither icon texture is ever displayed on the grid.

Proof: `include/nodes.h` (`enum NodeKind`), `src/grid_nodes.c` (`gNodeKindModelTypes`), `src/graphics.c` (`gCellTmds`, `gCellTims`), `src/tmd_data_5.c`, `src/tim_data_5.s`, `src/nodes.c` (`gNodes[0..715]`).

### Three TMD models compiled into the executable have no reference

Of the 78 TMD models and 35 TIM images linked into `SLPS_016.03`, three TMD arrays (4,336 bytes) have zero references anywhere in code: `gUnusedBandTmd` (3,880 bytes, 64 vertices / 64 polygons, at `0x800831B0`), `gUnusedSlabTmd` (344 bytes, 8 vertices / 6 polygons, at `0x8007E0D8`), and `gUnusedQuadTmd` (112 bytes, 4 vertices / 1 polygon, at `0x800803C0`).

Proof: `src/media_cube_tmd.c` (`gUnusedBandTmd`), `src/band_tmd.c` (`gUnusedSlabTmd`), `src/ring_tmd.c` (`gUnusedQuadTmd`), `config/lain/assets.tsv`.

### Fifteen of `LAPKS.BIN`'s 59 clips never play

`LAPKS.BIN` holds 59 masked animation clips (`gLapksParts[59]`), identical on both discs, of which 44 are played. Eleven clips (parts 6, 10, 11, 12, 13, 17, 19, 23, 26, 28, and 51; 2,168,548 bytes, 15.6% of the archive, each 10 to 39 frames, holding 148 compressed frames that appear in no played clip) are referenced by no code and no table. Four more clips (parts 7, 55, 56, and 58; 48,956 bytes, each a single unique 320×240 frame) are read into the heap by `LoadVoiceScreenClips` into slots 1–4 on every voice screen and freed unseen when the closing clip starts, because `StepVoiceScreen` plays only slot 0 (part 53) and slot 5 (part 57).

Proof: `src/file_parts.c` (`gLapksParts[59]`), `src/lapks_player.c` (`gIdleClipParts`, `gColumnTurnClipParts`, `gLevelMoveClipParts`, `gNodeOpenClipParts`, `gSystemMenuClipParts`, `gGridEntryClipParts`, `gVoiceScreenClipParts`, `LoadVoiceScreenClips`, `PlayLastVoiceScreenClip`), `src/voice_screen.c` (`StepVoiceScreen`), disc file `LAPKS.BIN`.

### Five parts of `SITEA.BIN` and one of `SITEB.BIN` are never loaded

Of 790 parts in `SITEA.BIN` (`gSiteAParts`), 785 are referenced by reachable nodes, Disc 1 idle audio, or the voice screen, leaving 5 unreferenced parts (44,539 bytes): part 357 (8,261 bytes, byte-identical to part 783), three unique napk-packed 160×120 8-bit TIM images (parts 631, 651, and 744; 12,398, 12,132, and 11,434 bytes), and part 787 (314 bytes, an unpacked 21×7 16-bit TIM). Of 558 parts in `SITEB.BIN` (`gSiteBParts`), 557 are referenced, leaving 1 unreferenced part: part 552 (314 bytes), byte-identical to `SITEA.BIN` part 787, sitting beside the five portrait and mouth-tile TIMs in `RunVoiceScreen`'s `timParts`.

Proof: `src/file_parts.c` (`gSiteAParts[790]`, `gSiteBParts[558]`), `src/nodes.c` (`gNodes`), `src/media_player.c` (`gIdleTrackImageParts`, `PlayEndRoll`), `src/voice_screen.c` (`RunVoiceScreen`, `timParts`), disc files `SITEA.BIN` and `SITEB.BIN`.

### Three VAB programs and three VAG samples in `SND.BIN` never play

`SND.BIN` parts 0 and 1 hold a VAB bank of 32 programs (0–31) and 34 VAG audio samples (`pBAV` header in part 0). Programs 2 and 3 (both using VAG 3, 5,248 bytes) and program 22 (using VAG 23, 20,480 bytes) are never triggered by `PlaySound`, `StartSound`, idle clip cues, or either SEQ track (part 2 uses programs 24–25; part 3 uses programs 6, 7, 9, 18, and 23). A third sample, VAG 4 (5,776 bytes, 87.6% nonzero bytes), is not referenced by any of the 32 programs in the VAB header, leaving 3 of 34 VAG samples (31,504 bytes) unreachable.

Proof: `src/sound.c` (`InitSound`, `PlaySound`, `StartSound`), `src/file_parts.c` (`gSndParts[4]`), disc file `SND.BIN` parts 0–3.

### Forty-eight clips in `VOICE.BIN` cannot be formed by any kana sequence

`VOICE.BIN` holds 639 PCM clips named by the 639 entries of `BIN.BIN` part 20 (`gVoiceParts[639]`). Forty-eight clips cannot be requested by `BuildSpeechPcm` from any input `RunNameEntry` can produce: 12 `*_CAVE.WAV` (154–168 bytes) and `*_END.WAV` (2,782–4,540 bytes) entries exceed the 1–3 letter syllable format; 18 `*_JYA`/`*_JYO`/`*_JYU.WAV` joins are never requested because `RomanizeKanaText` spells `じゃ`/`じゅ`/`じょ` as `ZY*`; and 18 `*_DYA`/`*_DYO`/`*_DYU.WAV` joins are never requested because `ApplySelectedKana` skips voicing `ち` (code 16), so code 60 (`ぢ`) can never be entered even though `ApplySelectedKana`'s row-11 switch still lists `case 60`. `BuildSpeechPcm` also checks for `'H'` as a second syllable letter, which `RomanizeKanaText` never emits, and `BIN.BIN` part 20 contains only two `*_K*` joins (`O_KO.WAV` and `U_KO.WAV`).

Proof: `src/kana_entry.c` (`ApplySelectedKana`, row-11 `switch` and row-12 `col == 2` voicing check; `RomanizeKanaText`), `src/speech.c` (`BuildSpeechPcm`, main romaji loop), `src/file_parts.c` (`gVoiceParts[639]`), disc files `BIN.BIN` part 20 and `VOICE.BIN`.

### Seven more `VOICE.BIN` clips are unreachable because `RomanizeKanaText` stops at code 69

In `ApplySelectedKana`, applying the handakuten mark (`row == 12`, `col == 3`) to the `は` row (codes 25–29) produces codes 69–73 (`ぱ`, `ぴ`, `ぷ`, `ぺ`, `ぽ`), and row 11 accepts small `ゃ`/`ゅ`/`ょ` after code 70 (`ぴ`). In `RomanizeKanaText`, the voiced branch tests `c >= 49 && c < 70`, which covers `ぱ` (69) and excludes codes 70–73. As read from the code, `ぴ`, `ぷ`, `ぺ`, `ぽ` and `ぴゃ`/`ぴゅ`/`ぴょ` append no letters to `gPlayerName`, leaving seven single-syllable clips (`PI.WAV`, `PU.WAV`, `PE.WAV`, `PO.WAV`, `PYA.WAV`, `PYO.WAV`, `PYU.WAV`) unreachable—bringing the strictly unreachable count in `VOICE.BIN` to 55 of 639 clips (1,005,288 bytes).

Proof: `src/kana_entry.c` (`ApplySelectedKana`, row-11 `case 70` and row-12 `case 3`; `RomanizeKanaText`, `c >= 49 && c < 70` branch), `src/file_parts.c` (`gVoiceParts[639]`), disc file `BIN.BIN` part 20.

### The last voice clip, `U_ZU.WAV`, sits past the search loop bound

Both `HasVoicePart` and `LoadSpeechClip` (`src/speech.c`) scan `BIN.BIN` part 20 with upper bound `VOICE_NAME_COUNT` (`638`, indices `0` to `637`), stopping one entry short of index 638 (`U_ZU.WAV`, 26,796 bytes). As read from the code, entering `うず` fails `HasVoicePart("U_ZU.WAV")` and falls back to `ZU.WAV`. `LoadSpeechClip` reaches `gVoiceParts[638]` only when a lookup fails to match entries `0..637` and exits the loop with `part == 638`, which occurs when one of the dropped codes 70–73 (`ぴ`–`ぽ`) precedes `ー` (for instance `ぴー`, leaving `gPlayerName` as `"-"` so `BuildSpeechPcm` requests `"\\_\\.WAV"`).

Proof: `src/speech.c` (`VOICE_NAME_COUNT`, `HasVoicePart`, `LoadSpeechClip` lookup loops, and `BuildSpeechPcm` `'-'` branch), disc file `BIN.BIN` part 20 (final entry `U_ZU.WAV`).

### Three voice clips retain 44-byte RIFF/WAVE headers played as raw PCM

Of the 639 parts of `VOICE.BIN`, 636 are headerless 16-bit signed PCM, while parts 9, 10, and 11 (`BYA.WAV`, `BYO.WAV`, `BYU.WAV`) begin with 44-byte `RIFF`/`WAVE` headers (44.1 kHz, 16-bit mono PCM). `LoadSpeechClip` reads every part from byte 0 without parsing a header, so as read from the code those 44 header bytes are copied into `gSpeechPcm` as audio samples whenever the clip's head is not trimmed (such as at the start of a name).

Proof: `src/speech.c` (`LoadSpeechClip`, `TrimSpeechClips`, `JoinSpeechClips`), `src/file_parts.c` (`gVoiceParts[9..11]`), disc file `VOICE.BIN` parts 9–11.

### Every disc file is referenced, and all 18 unreferenced XA channels are copies of track 733

Every file on Disc 1 (69 files) and Disc 2 (57 files) is listed in `gDisc1Files` and `gDisc2Files`, every movie and XA file is played by `gTracks[736]` (rows 0–2 are the executable, `SYSTEM.CNF`, and `LAIN_1.INF`/`LAIN_2.INF`), and all 44 parts of `BIN.BIN` (0–43) are loaded by fixed index in code. Across the 21 XA files on both discs (672 interleaved audio channels, 32 per file), 654 channels are played by `gTracks` and 18 (`LAIN13.XA` channels 29–31 and `LAIN21.XA` channels 17–31) are unreferenced. All 18 unreferenced channels have identical 8-sector (18,432-byte) ADPCM payloads matching `LAIN21.XA` channel 15 (track 733, the post-end-roll voice clip).

Proof: `src/media_player.c` (`gDisc1Files[69]`, `gDisc2Files[57]`, `gXaFiles[21]`, `gTracks[736]`), Disc 1 and Disc 2 ISO 9660 file lists, disc files `XA/LAIN13.XA` and `XA/LAIN21.XA`.

### The system menu renders `"Site"` to VRAM and defines two unused message sprites

`RenderMenuText` (`src/system_menu.c`) draws `"Site"` (`gSiteLabel`) into VRAM at `(704, 440)`, but `gMenuItems[5]` (`enum MenuItemId`) defines only Load, About, Change, Save, and Exit, so no tile ever shows it. In the same unit, `gMessageImages[17]` defines 17 memory-card message sprites in `BIN.BIN` part 22, all non-empty in the texture, yet messages 2 (`u=0, v=8, 160x16`) and 12 (`u=0, v=192, 88x8`) are omitted from `enum MessageId` and never passed to `DrawMessageWindow`.

Proof: `include/system_menu.h` (`enum MessageId`), `src/system_menu.c` (`enum MenuItemId`, `gSiteLabel`, `RenderMenuText`, `gMenuItems`, `gMessageImages[17]`, `DrawMessageWindow`), disc file `BIN.BIN` part 22.

### Entries 8 to 15 of `gHeadingYawEighths` are never read

`gHeadingYawEighths[16]` (`src/nodes.c`) holds 16 yaw multipliers `{ 0, 7, 6, 5, 4, 3, 2, 1, 0, 1, 2, 3, 4, 5, 6, 7 }`. Every lookup in `src/grid_scene.c` indexes it by `gGridHeading`, which stays in `0..7`, so the ascending second half (entries 8–15) is never read.

Proof: `src/nodes.c` (`gHeadingYawEighths[16]`), `src/grid_scene.c` (`UpdateGridScene`, `SCENE_CMD_TURN_LEFT` and `SCENE_CMD_TURN_RIGHT`; `StepTravelToCursor`, `gColumnsToTurn` branches).

### One keyword never finds a partner, and one keyword is split by a trailing space

All 417 entries of `gWords` (`src/string_tables.c`) appear on at least one reachable node, but `gWords[125]` (`"open the nExt."`) appears on only one node in the table (`Dc1058`, Site B), so selecting it in the media player always fails `FindNodeSharingWord` and opens the no-shared-node panel. Because `FindNodeSharingWord` compares word indices rather than text, `gWords[34]` (`"decision"`) and `gWords[101]` (`"decision "`, with a trailing space) are treated as unrelated words; `gWords[176]` (`"seem "`) and `gWords[217]` (`"hatred                 "`) also carry trailing spaces.

Proof: `src/string_tables.c` (`gWords[34, 101, 125, 176, 217]`), `src/nodes.c` (`gNodes[56]`), `src/grid_nodes.c` (`FindNodeSharingWord`).

### The end-roll montage skips the last two of its 57 movie cues

`PlayEndRoll` (`src/media_player.c`) fills all 57 entries of `gEndRollSegments[57]` (one for each `Dc` movie node) so that unwatched movies show the 24-bit still `SITEB.BIN` part 554 while `ENDROLL1.STR` audio plays. However, `GetMovieFrame` increments `gEndRollSegment` only while `gEndRollSegment < 54`, so entries 55 (`Dc1057`, whose `start` frame 4741 exceeds its `end` frame 4557) and 56 (`Dc1058`, frames 4558–4740) are never entered. In addition, entries 0 (`180..244`) and 1 (`180..408`) share the same start frame.

Proof: `include/media_player.h` (`enum EndRollState`), `src/media_player.c` (`GetMovieFrame`, `if (gEndRollState == END_ROLL_PLAYING)` (`3`) block; `PlayEndRoll`, `gNodes` loop; `gEndRollSegments[57]`), `src/nodes.c` (`gNodes[0..56]`), disc file `SITEB.BIN` part 554.

---

## Rare and conditional behaviour

### One in 70 saves writes "Genome" in the memory-card title

When building a save block, `PackCardSaveBlock` (`src/memcard.c`) tests `if (rand() % 70 != 0)`. On 69 of 70 outcomes it writes `gCardTitlePrefix` (`連続ゲイム小説［ｌａｉｎ］　第`), and on 1 of 70 (`rand() % 70 == 0`) it writes `gCardTitlePrefixRare` (`連続ゲノム小説［ｌａｉｎ］　第`), changing one katakana character (`イ` to `ノ`).

Proof: `src/memcard.c` (`gCardTitlePrefix`, `gCardTitlePrefixRare`, `PackCardSaveBlock`, `rand() % 70` test).

### Four session counters appear only in the save title, and interior zeros are dropped

`PackCardSaveBlock` (`src/memcard.c`) picks one of four counters with `rand() % 4` to append to the memory-card title: `gTokimekiPercent` (+1 per opened node, cap 100), `gDokudenpaCount` (+1 per idle media played, cap 9; falls through to `gHarumagePercent` when 0), `gHarumagePercent` (+1 per Circle press on an empty cell, cap 100), or `gGakkuriPercent` (+1 on every frame of `GRID_MENU_OPEN_NODE` except the frame a node opens, cap 100, counting all node-open clips rather than refusals alone). None of the four is written into `CardSaveData`, nothing else reads them, and all four reset to 0 on every grid entry (`InitGridState`, `RestoreGridCursor`). As read from the code, `AppendWideNumber` (`src/memcard.c`) tests each decimal digit for `!= 0` independently without tracking whether a higher digit was printed, so an interior zero is omitted when unpadded (`101` prints as `１１`) or replaced by a full-width space when padded (`100` prints as `１　０`).

Proof: `src/memcard.c` (`AppendWideNumber`; `PackCardSaveBlock`, `switch (rand() % 4)`), `src/grid_menu.c` (`HandleGridInput`, `RunGridMenu` (`GRID_MENU_REST` idle-timer transition and `GRID_MENU_OPEN_NODE`), `RestoreGridCursor`, `InitGridState`).

### The save file records load count, both discs' cursors, and every node's unchanged unlock parent

`CardSaveData` (`include/card.h`) stores `gLoadCount`, `gCurrentDisc`, `gGatesOpened`, `gP2NodesOpened`, `gUpgradesApplied`, `gEndRollCount`, both discs' cursor coordinates, and four bytes per node covering the upper half of `flags` and `unlockedBy`. `gLoadCount` increments only in `UnpackCardSaveBlock` when a save is loaded (capped at 9,999), never when saving, and appears nowhere in-game except as `第(gLoadCount + 1)回` in the memory-card title. No routine modifies any node's `unlockedBy` during play, and no routine checks play time or load count to unlock content.

Proof: `include/card.h` (`CardSaveData`), `src/memcard.c` (`PackCardSaveBlock`, `UnpackCardSaveBlock`).

### All six P2 nodes unlock two silent Disc 2 idle movies, with a 1-in-32,768 leak for `PO1.STR`

After 500 frames without input on the grid (`gIdleFrames`), `PlayIdleMedia` chooses an idle movie or idle audio track by `(rand() & 1) == 0`. On Disc 2 once all six P2 nodes are opened (`gP2NodesOpened == 63`), `PlayIdleMovie` tests `if (rand() / 10922 == 1)`—which holds for `rand()` in `10922..21843`, 10,922 of 32,768 outcomes (33.33%)—and picks `rand() % 2 + 731`, playing either `MOVIE/PO1.STR` (track 731, 25 frames) or `MOVIE/PO2.STR` (track 732, 75 frames), both Form-1 silent movies without interleaved XA audio. When `gP2NodesOpened != 63`, Disc 2 computes `gTrackIndex = rand() / 4681 + 724` over `rand()` in `0..32767`; because `32767 / 4681 == 7`, `PO1.STR` (731) still has a 1-in-32,768 chance per idle movie (1 in 65,536 per idle event), while `PO2.STR` cannot play until `gP2NodesOpened == 63`.

Proof: `src/media_player.c` (`PlayIdleMedia`, `(rand() & 1) == 0` branch; `PlayIdleMovie`, `gP2NodesOpened` track selection; `gTracks[709..732]`), Disc 2 files `MOVIE/PO1.STR` and `MOVIE/PO2.STR`.

### The tenth idle audio clip plays 1 time in 4,096

`PlayIdleAudio` (`src/media_player.c`) chooses among 10 idle audio tracks with `gIdleTrackBase[gCurrentDisc] + rand() / 3640`: tracks 169–178 on Disc 1 (`LAIN13.XA` channels 19–28, corresponding to `Env001`–`Env012`) and their byte-identical copies at tracks 699–708 on Disc 2 (`LAIN21.XA` channels 5–14, filed under the `GaTE` and `P2-` node indices). Across `rand()` in `0..32767`, offsets 0–8 each cover 3,640 values (11.11% each), whereas offset 9 (`Env012` / track 178 on Disc 1, track 708 on Disc 2, 46.51 s) matches only `rand()` in `32760..32767`—8 of 32,768 outcomes, or 1 in 4,096 per idle audio (1 in 8,192 per idle event).

Proof: `src/media_player.c` (`PlayIdleAudio`, `gTrackIndex` assignment; `gIdleTrackBase`; `gTracks[169..178, 699..708]`), disc files `XA/LAIN13.XA` and `XA/LAIN21.XA`.

### Lain's eyes appear on the continue prompt at 1 in 500 frames

`RunContinuePrompt` (`src/continue_prompt.c`) calls `VSync(0)` twice per loop iteration (~30 frames per second) and, whenever `eyesTimer == 0`, tests `if (rand() % 500 == 0)`. When triggered, it sets `eyesTimer = 180` (~6 seconds), ramping sprite 10 (184×16, from `BIN.BIN` part 31) from shade 0 to 128 over 17 frames (`eyesTimer` 180 to 164, 16 steps of 8), holding it for 147 frames (163 to 17), and fading it out over 16 frames (16 to 1)—an average wait of 500 frames (~16.7 seconds) between appearances.

Proof: `src/continue_prompt.c` (`gContinuePromptSprites[10]`, `RunContinuePrompt`, `eyesTimer` / `rand() % 500` block and paired `VSync(0)` calls).

### The grid ring dips three times in four every 3,600 vertical blanks

`StepRing` (`src/grid_effects.c`) tests `now - gRingDipRollAt >= 3600` (~60 seconds at 60 Hz) and, when the ring is at rest (`gPointDeltaMode == POINT_DELTA_NONE` (0), `gSceneAnimation == SCENE_CMD_NONE` (0), `gRingMotion == RING_MOTION_IDLE` (0)), morphs the ring into `gRingDipPoints` (`POINT_DELTA_RING_DIP`) if `(rand() & 0x3000) != 0`. Because `0x3000` masks two bits, three of the four outcomes (75%) trigger a dip.

Proof: `src/grid_effects.c` (`StepRing`, `gRingDipRollAt` timer check and `StartPointMorph` call), `include/graphics.h` (`enum PointDeltaMode`), `include/pad.h` (`enum SceneCommand`), `include/grid_effects.h` (`enum RingMotion`).

### Seventeen nodes require five end rolls, even though only four end-roll nodes exist

`IsNodeVisible` (`src/grid_nodes.c`) places a node on the grid once `gEndRollCount > 0` and its threshold (`flags >> 28`) is at most `gEndRollCount + 1`, whereas `PickCursorClipSet` allows opening it only when `gEndRollCount >= threshold`. Nodes appear one end roll before they can be opened: the 17 threshold-5 nodes (`TaK028`–`TaK121`, 12 on Site A and 5 on Site B) become visible after four end rolls and require a fifth end roll to open. Because only four nodes carry the end-roll bit (`Cou053`, `Dia048`, `Lda237`, `Tda092`), reaching `gEndRollCount == 5` requires replaying at least one of them (each completion increments `gEndRollCount` up to 9,999).

Proof: `src/grid_nodes.c` (`IsNodeVisible`, `PickCursorClipSet`), `src/grid_menu.c` (`RunGridMenu`, `GRID_MENU_OPEN_NODE`, `switch (playResult)` on `MEDIA_RESULT_END_ROLL` (`1`)), `src/nodes.c` (`gNodes[109, 157, 425, 517]` and the 17 threshold-5 rows).

### Two Site A nodes unlock Site B movies

Two unlock links in `gNodes` cross discs: `Cou016` (row 72, Site A) unlocks `Dc1055` (row 53, Site B), and `Dc1001` (row 0, Site A) unlocks `Dc1058` (row 56, Site B). `MarkNodeOpened` sets the target node's shown bit regardless of which Site is active, so both Site B movies appear the next time Site B's grid is built.

Proof: `src/nodes.c` (`gNodes[0, 53, 56, 72]`), `src/grid_nodes.c` (`MarkNodeOpened`, `unlockedBy` scan).

### Taking `SSkn04` on Site A hides `SSkn04#` on Site B, capping upgrades at six

Among the seven upgrade nodes (`NODE_KIND_UPGRADE` (7), rows 709–715 of `gNodes`), both `SSkn04` (row 712, Site A) and `SSkn04#` (row 713, Site B) have upgrade count 3. Once either is taken, `gUpgradesApplied` increases from 3 to 4, and `IsNodeVisible` (`src/grid_nodes.c`) hides every `NODE_KIND_UPGRADE` node with upgrade count below `gUpgradesApplied` (`3 < 4`). As a result, at most six of the seven upgrade nodes can be taken in one playthrough, and `Application Version` in the system menu reaches at most `7.0` (`gUpgradesApplied + 1`).

Proof: `include/nodes.h` (`enum NodeKind`), `src/nodes.c` (`gNodes[709..715]`), `src/grid_nodes.c` (`IsNodeVisible`, `NODE_KIND_UPGRADE` check), `src/system_menu.c` (`RenderMenuText`).

### The grid resets the random seed to 10 on every grid build

`InitGridScene` (`src/grid_scene.c`) calls `srand(0xa)`, resetting `LIBC2`'s random generator to the constant seed `10` at boot and whenever `ResetGridSceneToCell` rebuilds the grid (`InitGridState`, `RestoreGridCursor`). The only other `srand` calls in the executable are `srand(rand())` in `PlayIdleMovie` and `PlayIdleAudio`.

Proof: `src/grid_scene.c` (`InitGridScene`, `srand(0xa)` call; `ResetGridSceneToCell`), `src/media_player.c` (`PlayIdleMovie` and `PlayIdleAudio`, `srand(rand())` calls).

---

## How the game was built

### Runtime Library 4.3 with an interim 4.3–4.4 `LIBPAD` and unoptimised `NITR-BS REV1.3`

As detailed in [building.md](building.md), the game's 321 own routines were compiled with CC1PSX 2.8.0 at `-O2 -G8`, assembled with ASPSX 2.77 divide checks enabled, and contain zero GTE instructions, linking 15 of Sony's 16 libraries from Runtime Library 4.3 (embedding `sys.c` v1.140 dated `1998-01-12`, `bios.c` v1.86, and `intr.c` v1.75). `LIBPAD` is a later interim build between 4.3 and 4.4 (`PADENTRY`, `PADCMD`, `PADPORTD`, and `PADSEQD` match 4.4 identically; `PADMAIN` agrees with 4.4 on 883 of 884 words; `PADIF` sits between 4.3 and 4.4), and the custom MDEC bitstream library stamped `NITR-BS REV1.3` compiles `INIT.c` unoptimised at `-O0 -G0`.

Proof: [building.md](building.md), `config/lain/stated.tsv`, `config/lain/libraries.tsv`, `src/lib/nitrbs/INIT.c`, `src/lib/nitrbs/DECODE.s` (`NITR-BS REV1.3`).

### The executable and `BIN.BIN` were rebuilt three weeks after the discs' volume date

Both discs' ISO 9660 Primary Volume Descriptors record creation dates on `1998-09-30` (`20:42:15` JST on Disc 1 and `23:05:17` JST on Disc 2), matching the latest movie file (`MOVIE/F079.STR`, `1998-09-30T20:34:58+09:00`). However, `BIN.BIN` (`1998-10-21T16:40:14+09:00`) and `SLPS_016.03` / `SLPS_016.04` (`1998-10-21T17:00:06+09:00`) are dated 21 days later, 19 minutes apart; `VOICE.BIN` (`1998-08-13T15:39:06+09:00`) is the oldest file on both discs.

Proof: ISO 9660 Primary Volume Descriptors (sector 16) and file-system directory timestamps of both retail disc images.

### Skipped header words and zero-filled fields in the game's own data formats

Every clip in `LAPKS.BIN` starts with `"lapk"` followed by a 32-bit word that `OpenClip` (`src/clip.c`) skips: it equals `size - 8` in 57 of 59 clips, but `size - 12` in part 18 (277,324 vs 277,328) and `size - 2,904` in part 53 (312,852 vs 315,748). Every `ClipMask` starts with an unread 32-bit word; every `Node` ends with an unread 16-bit `unused` field (0 in all 716 rows); and every `Track` has an unread `unused` byte (0 in all 736 rows).

Proof: `src/clip.c` (`OpenClip`, `struct ClipMask`), `include/nodes.h` (`Node`), `include/media_player.h` (`Track`), `src/nodes.c`, `src/media_player.c`, disc file `LAPKS.BIN` parts 18 and 53.

### Unread profiling counters remain active in the clip decoder and CD driver

`DecodeClipFrame` (`src/clip.c`) records `gClipFramesDecoded`, `gClipDecodeVSyncs`, `gClipDecodeVSyncsLow`, `gClipUnpackVSyncs`, `gClipMaskVSyncs`, `gClipUploadVSyncs`, `gClipRunLevelCount`, and `gClipPeakRunLevelCount[0]` on every decoded clip frame, and `HandleCdSync` (`src/cd.c`) increments `gCdDiskErrorCount` on disc errors. No routine reads any of these counters.

Proof: `src/clip.c` (`DecodeClipFrame`, end-of-frame timing and run-level updates), `src/cd.c` (`gCdDiskErrorCount`, `HandleCdSync`, `CdlDiskError` branch).

### `InitText` reads 32 bytes past `sFontTim` into `sTextImage`

`InitText` (`src/text.c`) reads `BIN.BIN` part 1 (`gBinParts[1].size == 17,568` bytes) into `static u_long sFontTim[4384]` (17,536 bytes), writing 32 bytes past the end of `sFontTim` into the next static array in `.bss`, `sTextImage[3072]`. Both `PrepareCaptionSprites` and `UploadTextToVram` zero `sTextImage` with `memset` before drawing text.

Proof: `src/text.c` (`sFontTim`, `sTextImage`, `InitText` `RequestFilePartRead` for part 1, `PrepareCaptionSprites`, `UploadTextToVram`), `src/file_parts.c` (`gBinParts[1]`).

### `EnsureSiteDisc` copies 100 entries from 69- and 57-entry disc tables

`EnsureSiteDisc` (`src/disc_change.c`) copies 100 `DiscFile` entries into `gDiscFiles[100]` from `gDisc1Files[69]` or `gDisc2Files[57]`. That loop reads 31 entries past the end of `gDisc1Files` (into `gDisc2Files`) or 43 entries past the end of `gDisc2Files` (into `gTracks`), populating unused tail slots of `gDiscFiles`.

Proof: `src/disc_change.c` (`EnsureSiteDisc`, `gDiscFiles` copy loops), `src/media_player.c` (`gDisc1Files[69]`, `gDisc2Files[57]`, `gTracks`).

### `LoadMenuTileImage` calls C library `free` on a game-heap block, and `FreeMenuQuads` is never called

`LoadMenuTileImage` (`src/system_menu.c`) allocates a read buffer from the game's custom heap (`AllocHeapBlock(gBinParts[2].size)`) and passes it to `LIBC2`'s `free()` instead of `FreeHeapBlock()`, so the game heap never reclaims that block. Leaving the system menu by Load (`MENU_RESULT_LOAD`, `-1`) or Change (`MENU_RESULT_CHANGE`, `-2`) also returns from `RunGridMenu` without calling `StepSystemMenuClose` or the uncalled `FreeMenuQuads`, leaving the 128-quad `gMenuQuads` block allocated.

Proof: `include/system_menu.h` (`enum MenuResult`), `src/system_menu.c` (`LoadMenuTileImage`, `FreeMenuQuads`, `UpdateSystemMenu`), `src/grid_menu.c` (`RunGridMenu`, `GRID_MENU_SYSTEM_MENU` `UpdateSystemMenu` dispatch).
