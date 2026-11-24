# The discs and their formats

Each disc is one MODE2/2352 data track holding an ISO 9660 file system. `SYSTEM.CNF` boots the executable, which
stays resident and is the whole program: no other file holds code. Everything else is data the executable reads
or streams. This page lists the files, says how the game finds them, and gives the layout of every format it
reads as the code reads it, with the routine to read and the record the source declares.

Standard PlayStation formats are named as such and not described again. For the game's own formats the source
is the specification; each section names the routine that reads one.

## The files

| File | Disc 1 | Disc 2 | Holds |
| --- | ---: | ---: | --- |
| `SYSTEM.CNF` | 69 B | 69 B | The boot line (`SLPS_016.03` or `SLPS_016.04`), `TCB = 4`, `EVENT = 10`, `STACK = 801fff00` |
| `SLPS_016.03`, `SLPS_016.04` | 618,496 B | 618,496 B | The executable, the same bytes on both discs ([building.md](building.md)) |
| `LAIN_1.INF`, `LAIN_2.INF` | 18 B | 18 B | The text "LAIN SITE A DISC" or "LAIN SITE B DISC": the disc's identity |
| `BIN.BIN` | 256,000 B | same | 44 parts: interface images and the voice clips' names |
| `LAPKS.BIN` | 13,936,640 B | same | 59 animation clips ("lapk") |
| `SND.BIN` | 438,272 B | same | The sound bank and the two music sequences |
| `VOICE.BIN` | 14,690,304 B | same | 639 voice clips: the syllables speech is built from |
| `SITEA.BIN`, `SITEB.BIN` | 7,057,408 B | 5,328,896 B | The site's images: 790 parts on disc 1, 558 on disc 2 |
| `MOVIE/*.STR` | 48 | 40 | Movies: INS01 to INS22 and 26 files F001 to F043 on disc 1; INS16 to INS22, PO1, PO2 and 31 files F039 to F103 on disc 2 |
| `MOVIE2/ENDROLL1.STR` | - | 1 | The end roll movie |
| `XA/LAIN*.XA` | 13 | 8 | XA audio: LAIN01 to LAIN13 on disc 1, LAIN14 to LAIN21 on disc 2 |

`BIN.BIN`, `LAPKS.BIN`, `SND.BIN` and `VOICE.BIN` are byte for byte the same on both discs; the site file, the
movies and the audio are each disc's own (INS16 to INS22 are on both).

### How the game finds a file

The game reads by table, not by name. `gDiscFiles` (`src/media_player.c`) holds, for each file of the disc in the
drive, its first sector and its size (`DiscFile`, `include/media_player.h`): one row per file in the order the
files lie on the disc, row 0 the executable. It has room for 100 rows; at start it holds disc 1's 69, and when
the player changes discs `src/disc_change.c` copies `gDisc1Files` (69 rows) or `gDisc2Files` (57) into it. Rows 0
to 7 hold the same files on both discs (row 6 the disc's site file), so the code names them by number:

| Row | File | Read by |
| ---: | --- | --- |
| 3 | `BIN.BIN` | `src/loading.c`, `src/about.c`, `src/upgrade.c`, `src/system_menu.c`, `src/text.c`, `src/speech.c`, `src/media_player.c` |
| 4 | `LAPKS.BIN` | `src/lapks_player.c` |
| 5 | `SND.BIN` | `src/sound.c` |
| 6 | `SITEA.BIN` or `SITEB.BIN` | `src/media_player.c`, `src/voice_screen.c` |
| 7 | `VOICE.BIN` | `src/speech.c` |

Rows 0 to 2 (the executable, `SYSTEM.CNF`, the `.INF` file) are read by nothing. A movie is named by its row:
each of the 736 tracks of `gTracks` (`Track`, `include/media_player.h`) is either a movie (82 tracks), giving its
row on its own disc and its frame count, or an XA channel (654), giving the XA file, the channel (0 to 31) and
the channel's size in bytes. The XA file is a row of `gXaFiles`, which lists the audio files' rows so that its
*k*th row is the file numbered *k* (LAIN01.XA to LAIN21.XA) on whichever disc holds it.

A row's size is the file's size in bytes, with two exceptions. The two small text files are rounded up to a
word (72 and 20 for 69 and 18). The movies and the audio files, which are read as raw Mode 2 sectors, are
measured in sectors of 2,336 bytes (each sector less its sync and header), so their rows are larger than the
sizes the file system gives, which count 2,048 bytes a sector.

`IdentifyDisc` (`src/disc_id.c`) tells the discs apart by searching the file system for `\LAIN_1.INF;1` and
`\LAIN_2.INF;1`; it reads neither file. `src/cd.c` also has two routines that search the file system by name
(`RequestNamedFileRead`, `FindFileSector`); nothing calls them.

### Reading

`RequestFileRead(file, dest)` and `RequestFilePartRead(file, part, parts, dest)` (`src/cd.c`) queue a read of a
whole file or of one part of it; the queue holds 24 requests and `PollCdRequest` reports one done. LIBCD's sync
and ready callbacks do the work: seek (`CdlSetloc`), set double speed with whole sectors (`CdlModeSpeed |
CdlModeSize1`), and read (`CdlReadN`). For each sector `HandleCdReady` reads the 12-byte header, checks that the
sectors arrive in order and copies the 2,048 bytes of data to the destination; a sector out of order fails the
read. The last sector is copied only as far as the size asks, rounded up to a word.

## Archives without a directory: the part tables

`BIN.BIN`, `LAPKS.BIN`, `SND.BIN`, `VOICE.BIN` and the site files are archives with no header or directory: the
executable's tables are their only index. `src/file_parts.c` declares one table per file (`gBinParts[44]`,
`gLapksParts[59]`, `gSndParts[4]`, `gVoiceParts[639]`, `gSiteAParts[790]`, `gSiteBParts[558]`), each row a
`FilePart` (`include/cd.h`): the part's first sector counted from the file's first, and its size in bytes. Every
part starts on a sector boundary. Moving a part on the disc means changing its table.

## Standard PlayStation formats

| Format | Where | Read by |
| --- | --- | --- |
| PS-X executable and `SYSTEM.CNF` | the root | The console's BIOS |
| TIM image | inside `BIN.BIN` and the site files (most of them packed, below), and 35 arrays in the executable | `src/loading.c`, `src/graphics.c` and the screens that show them |
| TMD model | 78 arrays in the executable (`src/band_tmd.c`, `src/ring_tmd.c`, `src/media_cube_tmd.c`, ...) | `GsMapModelingData` and `GsLinkObject4`, called from `src/graphics.c` |
| VAB sound bank, header and body | `SND.BIN` parts 0 and 1 | `SsVabOpenHead`, `SsVabTransBody`, called from `src/sound.c` |
| SEQ music | `SND.BIN` parts 2 and 3 | `SsSeqOpen`, `SsSeqPlay`, called from `src/sound.c` |
| STR movie: video sectors interleaved with XA audio | `MOVIE/*.STR`, `MOVIE2/ENDROLL1.STR` | `src/media_player.c`, through LIBCD's stream ring (`StSetRing`, `StGetNext`); the video is NITR-BS, below |
| XA-ADPCM audio | `XA/*.XA` | `src/media_player.c`, choosing a channel with the CD filter (`CdlSetfilter`, file 1) |

The arrays the executable holds are listed by `config/lain/assets.tsv`: every TMD model and every TIM image built
into the program, with its address, size and SHA-256. The movies examined are 320 by 240. The XA files
examined hold 32 channels of mono 4-bit ADPCM at 18.9 kHz, and the movies' audio sectors have the same coding.

## The game's own formats

### "napk": packed data

Most parts of `BIN.BIN` and the site files are packed. `UnpackNapk` (`src/napk.c`) reads:

| Offset | Holds |
| ---: | --- |
| 0 | the tag "napk" |
| 4 | the unpacked size in bytes (a word) |
| 8 | the packed stream |

The stream is groups of a flag byte and up to eight items, the flag's bits read from the most significant: a 0
bit is one literal byte; a 1 bit is two bytes, a distance less one and a length less three, and copies that many
bytes from that far back in the output (so 1 to 256 bytes back, 3 to 258 long). The output goes to a block of
the heap (`AllocHeapBlock`, `src/heap.c`) of the unpacked size.

Unpacked, every packed part of these files is a TIM image except one: `BIN.BIN` part 20, a text file.

| File | Parts | What they hold |
| --- | ---: | --- |
| `BIN.BIN` | 44 | 43 TIM images (parts 0, 1 and 9 unpacked, the rest packed: 4-bit, and 8-bit for parts 32 to 39), and part 20, the voice clips' names (below) |
| `SITEA.BIN` | 790 | 784 packed 8-bit TIM images (most of them 160 by 120 pixels) and 6 unpacked 16-bit ones |
| `SITEB.BIN` | 558 | 551 packed 8-bit TIM images, one packed 24-bit image of 320 by 240, and 6 unpacked 16-bit ones |

Which part each screen loads is in its unit: `src/loading.c` for the loading screens, `src/about.c` for the About
screen, `src/upgrade.c` for the upgrade screen; the site images are named by the nodes' `imageParts`
(`Node`, `include/nodes.h`) and read by `src/media_player.c`, and the voice screen's by `src/voice_screen.c`.

### "lapk": the animation clips (`LAPKS.BIN`)

Each of the 59 parts of `LAPKS.BIN` is one clip. `OpenClip` (`src/clip.c`) reads its header:

| Offset | Holds |
| ---: | --- |
| 0 | the tag "lapk" and a word the code skips |
| 8 | the frame count (a word; 1 to 53 in the clips on the disc) |
| 12 | one 12-byte `ClipFrame` per frame: the frame's offset from the end of this table, its position (two halfwords), and its sound cue (a word) |
| | the frames |

A frame is a `ClipChunk`: its width and height (halfwords), its luminance and chrominance quantisers, the offset
of its mask counted from that offset's own field, then the frame's NITR-BS bitstream (below). The mask is a
`ClipMask`: a word nothing reads, the unpacked length, then the mask packed as "napk"'s stream is (the same
flag bytes and two-byte copies, with no header of its own). `DecodeClipFrame` decodes the image with the MDEC
one 16-pixel column at a time, applies the mask to each column (`ApplyClipMask`) and loads it into VRAM. The
frame's sound cue is handed back to the clip player (`src/lapks_player.c`) in `ClipFrameInfo`.

Unpacked, the mask has two bits for each pixel, a byte for four, in rows of a quarter of the frame's width. A
pixel's code 0 clears it to 0, which the GPU draws as transparent in a texture; 1 and 2 keep it with bit 15 (the
GPU's semi-transparency bit) set; 3 keeps it with bit 15 clear, but where both pixels of a half byte are 3 they are left as
decoded. A byte of 0 clears its four pixels and a byte of
255 leaves them as decoded, without looking at the codes.

### NITR-BS: the video bitstream

The movies and the clips code their pictures as NITR-BS bitstreams rather than the MDEC bitstreams of Sony's
movie library. A movie frame, as the stream ring hands it over, is a `MovieFrame` (`include/media_player.h`):
its luminance and chrominance quantisers (a byte each), its number (a halfword, which the end roll reads),
then the bitstream. Every bitstream opens
with its count of MDEC run/level codes (a word). `UnpackNitrBsFrame` (`src/lib/nitrbs/DECODE.s`, declared in
`include/DECODE.h`) turns the bitstream into those codes, which `DecDCTin` and `DecDCTout` (LIBPRESS) decode
into pixels. It looks codes up in `gSignedRunLevelCodes`, which `BuildSignedRunLevelCodes`
(`src/lib/nitrbs/INIT.c`) fills from a table of 111 codes followed by the same codes with their level negated.
The two kinds of picture use different tables: a movie's is `gNitrBsRunLevelCodes` (in `DECODE.s`, handed to it by
`src/media_player.c` before a movie starts), a clip's `gRunLevelCodes` (`src/clip.c`, by `InitClipDecoder`).
`DecodeClipFrame` refuses a clip frame of more than 0x3FFFF codes or of 360 rows or more.

### `VOICE.BIN` and the voice clips' names

`VOICE.BIN` holds 639 clips of 16-bit PCM with no header; the code copies each clip as samples. Three of them
(parts 9 to 11) begin with a RIFF header, which is copied as samples like the rest. What each clip is
called is not in the file: `BIN.BIN` part 20, unpacked, is a text of 639 lines ended by carriage return and line
feed, line *n* the name of part *n*, from A.WAV to U_ZU.WAV: syllables such as KA.WAV and KYA.WAV, and names
joined by an underscore such as A_CAVE.WAV and A_END.WAV.

`BuildSpeechPcm` (`src/speech.c`) reads that text into `gVoicePartNames`, turns a romaji string into WAV names
(`%c_%c.WAV` or `%c_%s.WAV` for a join where the list has one), finds each name's line, reads that part (`LoadSpeechClip`),
trims the joins and joins the clips into one buffer of PCM (`gSpeechPcm`). The search covers 638 lines
(`VOICE_NAME_COUNT`), one fewer than the list: a name it does not find leaves it at 638, the last part
(U_ZU.WAV), so that clip is played for its own name as for any name the list lacks.

### `SND.BIN`

Four parts (`gSndParts`): the sound bank's VAB header (18,976 bytes, sector 0) and body (413,312 bytes, sector
10), the grid's music (a SEQ of 1,402 bytes, sector 212) and the About screen's music (727 bytes, sector 213).
`src/sound.c` reads the header into `sVabHeader`, which has exactly that size, opens the bank (`SsVabOpenHead`),
sends the body to the sound processor (`SsVabTransBody`) and opens both sequences on the one bank (`SsSeqOpen`).

### The save file

The game keeps one save on the card in the first port, in the file `BISLPS-01603LAINdata` (the same name on
both discs) of one 8 KB block. Its layout is `CardSaveBlock` (`include/card.h`); `PackCardSaveBlock` and
`UnpackCardSaveBlock` (`src/memcard.c`) write and read it:

| Offset | Size | Holds |
| ---: | ---: | --- |
| 0 | 512 | `CardHeader`, a copy of `gCardHeaderTemplate`: "SC", 0x13 (an icon of three frames), one block |
| 4 | < 64 | the title, Shift-JIS: a prefix (`gCardTitlePrefix`, or one time in 70 `gCardTitlePrefixRare`), the load number, and one of the player's counters chosen at random |
| 96 | 32 | the icon's 16-colour palette |
| 128 | 384 | the icon's three frames, 16 by 16 pixels of 4 bits |
| 512 | 4 | `CardSaveData.signature`, 0x4C41494E (`CARD_SAVE_SIGNATURE`: the bytes "NIAL") |
| 516 | 20 | the player's name (`gPlayerName`) |
| 536 | 4 | the load count |
| 540 | 18 | nine halfwords, in order `currentDisc`, `gatesOpened`, `p2NodesOpened`, `upgradesApplied`, `endRollCount`, `cursorColumn`, `cursorRow`, `otherDiscCursorColumn`, `otherDiscCursorRow` |
| 558 | 2,864 | `nodes[716]`: for each node of `gNodes`, bytes 34 to 37 of its `Node` (the upper half of `flags` and `unlockedBy`) |
| 3,456 | 27 | the checks: byte *i* is the exclusive or of the 128 bytes of page *i*, for the 27 pages before it |

`CardSaveData` ends at 3,424; the bytes up to the checks are zero. `PackCardSaveBlock` returns 28, the number of
128-byte pages that hold all this, and the writer sends 28 pages (3,584 bytes). `UnpackCardSaveBlock` refuses a
save if any check fails, then if the signature is wrong.

## Oddities

- **A save block is built to measure a read.** Both places that read a save (`src/kana_entry.c`,
  `src/system_menu.c`) call `PackCardSaveBlock` on the buffer the card is then read into, only for the length it
  returns: the whole block is filled and checksummed, then overwritten by the read.
- **The last voice clip is reached by failing.** See `VOICE.BIN` above.
- **Unread rows and routines.** `gDiscFiles`' rows for the executable, `SYSTEM.CNF` and the `.INF` file are
  never read, nor are `src/cd.c`'s two routines that read by name.

## What is not known

- **How the data was written:** the encoder of the NITR-BS movies and clips, how the clip masks were made, and
  which tool packed the "napk" parts.
- **The word after "lapk":** in 57 of the 59 clips it is the clip's size less 8; the code skips it.
