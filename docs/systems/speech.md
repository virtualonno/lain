# Speech and the voice screen

The game speaks the player's name. Kana entry spells the name the player chose in romaji into `gPlayerName`
(`RomanizeKanaText`, `src/kana_entry.c`; the name is also saved on the memory card). When a voice-screen track is
opened, `src/speech.c` turns that spelling into one stretch of 16-bit PCM by joining short recordings from
`VOICE.BIN`, and `src/voice_screen.c` encodes it for the sound processor and plays it on voice 23 under a portrait
whose mouth moves. It is the one sound the game builds at run time; the tracks themselves are XA audio streamed
from the disc ([media-player.md](media-player.md)).

## The recordings

`VOICE.BIN` is file 7 of the disc's file table on both discs. Its 639 parts (`gVoiceParts`, `src/file_parts.c`)
are bare sound, 16-bit signed little-endian samples, though the game knows them by names ending in `.WAV`. Three
(parts 9 to 11) begin with a RIFF header; the code reads no header, so for those its bytes are joined in with the
samples unless the head is cut (below). The names are a separate list, part 20 of `BIN.BIN` (file 3), packed with
napk ([formats.md](../formats.md)) and unpacked into `gVoicePartNames`: 639 names, each line ending in carriage
return and line feed, the name on line `n` being part `n`. A name is looked up by comparing it with each of the
first 638 lines over the line's length (`strncmp`); a name that is on none of them gives 638, so the last part of
the table is read in its place.

The code asks for three kinds of name (the format strings are `gSingleKanaWavFormat`, `gSyllableWavFormat`,
`"%c_%c.WAV"` and `"%c_%s.WAV"` in `BuildSpeechPcm`):

| Name | Is |
| --- | --- |
| "A.WAV", "#.WAV" | a vowel or `#` on its own |
| "KA.WAV", "KYA.WAV" | a syllable: two letters, or three when the second is Y or H |
| "A_SA.WAV", "A_A.WAV" | a join: the vowel the previous sound ended on, running into the next sound |

Kana entry's spelling is plain (`gRomajiConsonants` holds the letters KSTNHM, so SI, TI, TU and HU), and it never
writes H as a second letter; the speech code accepts it all the same.

## Building the name

`BuildSpeechPcm(romaji, &bytes, &clips)` reads and unpacks the name list (drawing a loading frame while it waits,
`DrawSpeechLoadingFrame`, except after the end roll, track 733), then walks the spelling. `gPreviousPhoneticSymbol`
holds the vowel the last sound ended on, a backslash before the first.

- One of `AIUEO#` (`gSingleKanaSymbols`) is a single sound. After another sound the join of the previous vowel to
  it (previous, underscore, the sound) is tried first (`HasVoicePart`), else the sound's own recording.
- `-` loads the join of the previous vowel to itself ("A_A.WAV" after an A) and leaves the previous vowel as `-`.
- Anything else starts a syllable of two letters, or three when the second is Y or H. Again the join from the
  previous vowel is tried first after another sound, else the syllable's own recording, and the syllable's last
  letter becomes the previous vowel.

Each recording found is read whole into its own heap block by `LoadSpeechClip`, which records it in
`sSpeechClips` (16 entries; `SpeechClip`) with two marks: cut the head, cut the tail. The first recording keeps its
head. A join has both marked. A recording that is not a join keeps its head and also clears the tail mark of the
one before it, as does the end of the name for the last recording. `TrimSpeechClips` turns the marks into lengths:
with `t` two ninths of a recording's length, rounded up to an even number of bytes, a marked head loses `2t` bytes
and a marked tail `t`. So the first four ninths of a join are never heard, and a recording followed by a join
loses its last two ninths.

`JoinSpeechClips` copies what remains of each recording, in order, into one block (`gSpeechPcm`), freeing each
recording's block, and `LimitSpeechAmplitude` clamps every sample to a limit that rises linearly from 5,000 at
the first sample to 15,000 at the middle and falls back to 5,000 at the last (`gSpeechEnvelope`, three points read
by `InterpolateCurve`). The name list is freed and the routine returns the block, its size in bytes and the number
of recordings.

## The voice screen

The media player hands every track from 518 on to `RunVoiceScreen` instead of playing it itself
(`RunMediaPlayer`, `src/media_player.c`), and runs it once more as track 733 after the end roll. Its steps:

1. For track 733, start the end-roll spheres (`InitEndRollSpheres`, `gHasEndRollSpheres`), stepped and drawn
   with the screen throughout.
2. For tracks below 639, and for 733, build the name. The PCM is encoded to the sound processor's ADPCM in one pass
   (`EncSPU`, LIBPRESS), sent to sound memory (`SpuMalloc`, `SpuWrite`) and set up on voice 23 at volume 13,000 on
   both sides; the voice is keyed on later. `gSpeechFrameCount` is 18 frames per recording; `gSpeechClipCount`, the
   number of mouth cycles, at most 6. If the ADPCM block cannot be allocated or the encoder fails, the screen
   returns at once.
3. Clear the display and set aside two strips of video memory (128 by 32 at 320,480 and 72 by 32 at 448,480, moved
   to 640,256 and 640,288), which are put back at the end.
4. Read five images from the current disc's site file (`SITEA.BIN` or `SITEB.BIN`, file 6, the part chosen by
   `gCurrentDisc` from `timParts`) and load them into video memory: the portrait, 256 by 240 in 16-bit colour at
   320,256, shown as the sprite `gTrackImage`, and the mouth tiles beside it at 576,256.
5. Start reading back the CD audio as the media player does for XA, into `gDecodedAudio`
   ([media-player.md](media-player.md), "Reading the sound back").
6. Read six clips from `LAPKS.BIN` into the sequence slots (`LoadVoiceScreenClips`, `gVoiceScreenClipParts`,
   [clips.md](clips.md)) and wait for them.

Then each frame draws either the current clip or the portrait, and `StepVoiceScreen` advances its state
(`gVoiceScreenState`, `enum VoiceScreenState`):

| State | Value | Does | Then |
| --- | ---: | --- | --- |
| `VOICE_FIRST_CLIP` | 0 | play the first clip (`PlayFirstVoiceScreenClip`, slot 0) | `VOICE_SPEAK_BEFORE` for tracks below 639, else `VOICE_PLAY_TRACK` |
| `VOICE_SPEAK_BEFORE` | 1 | speak the name | `VOICE_PLAY_TRACK` |
| `VOICE_PLAY_TRACK` | 2 | play the track's XA audio until it ends (`IsTrackDone`) | `VOICE_SPEAK_AFTER` for track 733, else `VOICE_LAST_CLIP` |
| `VOICE_SPEAK_AFTER` | 3 | speak the name (track 733 only) | `VOICE_LAST_CLIP` |
| `VOICE_LAST_CLIP` | 4 | play the last clip (`PlayLastVoiceScreenClip`, slot 5); for 733, track 734's XA plays with it | `VOICE_DONE` (5) |

Only slots 0 and 5 are played; the four clips read into slots 1 to 4 are freed unseen when the last clip starts.
Track 733 reaches `VOICE_SPEAK_AFTER` without passing through `VOICE_SPEAK_BEFORE`, so after the end roll the name is
spoken once, after the XA track. Tracks from 639 on (other than 733) speak no name at all.

Speaking keys voice 23 on with its pitch raised to 0x85F (0x1000 plays at 44.1 kHz) and runs for
`gSpeechFrameCount` frames; in `VOICE_SPEAK_BEFORE` each frame also ends with `VSync(2)`. Then the voice is keyed off
and its sound memory freed.

## The mouth

The mouth is drawn by copying tiles inside video memory (`MoveImage`) onto the portrait, around 442,411. There are
three tiles (`tileX`, `tileSize`, `tileDest` in `StepVoiceScreen`), and a 22 by 16 patch is copied over the mouth
first each frame. While the name is spoken the mouth steps through `gSpeechTileSequence` (seven steps) once per
recording, for at most six recordings, then stays on tile 0. While the XA track plays it steps through the first
nine entries of `gTrackTileSequence`, but only while the track is loud: `GetCdPeakLevel` (the loudest of the first
206 samples in one half of the read-back CD audio, divided by 4,681, so 0 to 7) is read at the start of each cycle,
and a level of 0 holds tile 0 and reads the level again the next frame.

## Leaving

After the last state, track 733 lets the spheres collapse (`StepEndRollSpheresCollapse`) until they are done. The
sound processor's interrupt and transfer callbacks are cleared, the CD audio buffer freed and the speech's sound
memory freed (`SpuFree(gSpeechSpuAddr)`), the two strips of video memory put back, and the alternate loading image
loaded. That block was already freed when the name finished, so a spoken name's sound memory is freed twice; for
tracks that speak no name the call frees whatever address the last spoken name left in `gSpeechSpuAddr`.

## Unused

`src/speech.c` also has `FindVoicePart`, `LoadVoicePartNames` and `ReadVoicePart`, a name lookup, the list's load
and a lookup-and-read, which nothing calls; `BuildSpeechPcm` does the same work inline. `src/voice_screen.c` has
`KeyOnVoice` and `KeyOffVoice`, declared in its header, which key a voice on at a pitch rising with its number, and
off; nothing calls them either. `src/media_player.c`'s `GetCdPeakLevel5`, the peak scaled to 0 to 5, is not called.
