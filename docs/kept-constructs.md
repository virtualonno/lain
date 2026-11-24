# Constructs the original's bytes need

The original's code has constructs that plain C would not write. Each one below is in the source
because the original's bytes need it: written in its plain form, it compiles to different code. The
first list says what each is and why; the second keeps the few where the cause, or whether the
authors wrote it so, is not established.

58 constructs in 38 routines: 53 whose cause is established and 5 whose cause is not; 17 more in 1 routine written so in a published source.

## What the original has, and why

The original's frame keeps room for a local that no instruction touches: the source declares the local and does not use it. (24)

- `src/about.c`, `LoadTimPartClut`: `RECT rect;`
  The unused RECT rect declared beside TIM_IMAGE tim matches its sibling helpers LoadTimPartAt and LoadTimPart and accounts for 8 untouched stack bytes in both LoadTimPartClut and RunAboutScreen, which inlines it three times; removing it shrinks both frames and shifts stack offsets.
- `src/grid_effects.c`, `DrawOrbitingTexturedQuad`: `MATRIX unused[2];`
  The MATRIX unused[2], declared after matrix and never read, holds 64 bytes of the original's frame; without it the frame is 104 bytes instead of 168 and 25 stack offsets move.
- `src/grid_effects.c`, `InitRing`: `char unused[40];`
  The 40-byte array unused, never read, is frame room the original keeps; without it the frame is 24 bytes instead of 64 and 6 stack offsets move.
- `src/grid_effects.c`, `StepHorizonAndStreaks`: `SVECTOR unusedRotation;`
  The SVECTOR unusedRotation, declared and never read, holds 8 bytes of the original's frame; without it the frame is 48 bytes instead of 56 and 14 stack offsets move.
- `src/grid_effects.c`, `StepPieceBurst`: `VECTOR unused;`
  The VECTOR unused, declared after delta and never read, holds 16 bytes of the original's frame; without it the frame is 112 bytes instead of 128 and 12 stack offsets move.
- `src/grid_effects.c`, `StepRing`: `unsigned char unused[8];`
  The 8-byte array unused, declared after object and never read, holds 8 bytes of frame room the original keeps; without it the frame is 96 bytes instead of 104 and 34 stack offsets move.
- `src/grid_menu.c`, `InitGridState`: `short unused[4];`
  The 8-byte array unused is frame room the original keeps and nothing reads: without it the frame shrinks from 32 to 24 bytes (4 stack-offset differences), and every used local form tried (returning a two-int struct from an inline helper, passing int or short out-pointers to an inline helper, or a local two-int array) emits extra stack traffic (37 to 48 instructions against 33).
- `src/grid_menu.c`, `RestoreGridCursor`: `short unused[4];`
  The 8-byte array unused is frame room nothing reads, both in RestoreGridCursor itself (40 bytes, 32 without it) and in RunGridMenu where the inline helper is expanded (104 bytes, 96 without it), so one declaration reproduces both frames.
- `src/grid_scene.c`, `ResetGridSceneToCell`: `unsigned char unusedFrame[8];`
  The 8-byte array unusedFrame is frame room the original keeps above the outgoing arguments and nothing reads; without it the frame is 32 bytes instead of 40 and 8 stack offsets move.
- `src/grid_scene.c`, `SetLevelNumberTexture`: `GsIMAGE unusedTim;`; `RECT unusedRect;`
  The RECT unusedRect and GsIMAGE unusedTim, declared and never read, hold 8 and 32 bytes of the original's 152-byte frame; without the RECT the frame is 144 bytes and 28 stack offsets move, without the GsIMAGE 120 bytes and 8 move.
- `src/grid_scene.c`, `SortCursorHighlight`: `RECT clut;`
  The RECT clut, declared between pixels and oldSelection and never read, holds 8 bytes of the original's frame; without it the frame is 112 bytes instead of 120 and 24 stack offsets move.
- `src/grid_scene.c`, `StartCellFlight`: `SVECTOR unusedAngle;`; `VECTOR unusedPoint;`
  The SVECTOR unusedAngle and VECTOR unusedPoint, declared after angle and point and never read, hold 8 and 16 bytes of the original's 288-byte frame; without the SVECTOR it is 280 bytes and 42 stack offsets move, without the VECTOR 272 bytes and 32 move.
- `src/grid_scene.c`, `StepCellFlight`: `SVECTOR unused;`
  The SVECTOR unused, declared after the double pos[3] and never read, holds 8 bytes of the original's frame; without it the frame is 96 bytes instead of 104 and 28 stack offsets move.
- `src/grid_scene.c`, `StepCellFlightEnd`: `VECTOR unused;`
  The VECTOR unused, declared between out and pos and never read, holds 16 bytes of the original's frame; without it the frame is 472 bytes instead of 488 and 67 stack offsets move.
- `src/lib/libgs/GS_131.c`, `GsSetRefView2`: `MATRIX unused;`
  The MATRIX unused, declared between matrices and translation and never read, holds 32 untouched bytes of the original's 192-byte stack frame; without it the frame shrinks to 160 bytes and 23 stack offsets shift.
- `src/line_effect.c`, `StepLineTrails`: `char unusedFrame[88];`
  The 88-byte array unusedFrame, declared before the routine's other locals and never read, holds 88 bytes of frame room the original keeps; without it the frame is 48 bytes instead of 136 and 16 stack offsets move.
- `src/media_player.c`, `LoadDecodedSlice`: `char unused[8];`
  The 8-byte array unused, declared after RECT snapRect and never read, is frame room the original keeps; without it the frame is 32 bytes instead of 40 and 6 stack offsets move.
- `src/special_scenes.c`, `InitGateScene`: `SVECTOR unused;`
  The SVECTOR unused, declared between pos and image and never read, holds 8 bytes of the original's frame; without it image moves down 8 bytes, the frame is 96 bytes instead of 104 and 31 stack offsets move.
- `src/special_scenes.c`, `UpdateGateScene`: `SVECTOR unused;`; `VECTOR unused2;`
  The SVECTOR unused and VECTOR unused2, declared before and after out and never read, hold 8 and 16 bytes of the original's 232-byte frame; without them out moves and the frame is 224 or 216 bytes.
- `src/system_menu.c`, `StepConfirmClose`: `RECT unused;`
  The RECT unused, declared and never read, is the 8 bytes the original's 32-byte frame holds beyond the outgoing arguments and the return address; without it the frame is 24 bytes and 6 stack offsets move.
- `src/system_menu.c`, `StepConfirmOpen`: `RECT unused;`
  The RECT unused, declared and never read, is the 8 bytes the original's 32-byte frame holds beyond the outgoing arguments and the return address; without it the frame is 24 bytes and 6 stack offsets move.

Both arms of a test are the same statements: the compiler keeps one copy of them and the test before it. (2)

- `src/lib/snd/cc_6.c`, `_SsContDataEntry`: `if (below_center) { coarse = value * 6400; } else { coarse = value * 6400; }`; `if (below_center) { fine = ((value * 100) / 8192) << 13; } else { fine = ((value * 100) / 8192) << 13; }`
  The fine and coarse cases each branch on below_center with identical arms because the original computes below_center before the tone loop and never tests it: the loop pass hoists the invariant while both branches still exist, and cross-jumping after reload merges the identical arms and deletes the branches.

A jump the original takes where every loop and conditional tried compiles differently. (15)

- `src/continue_prompt.c`, `RunContinuePrompt`: `goto exit;`
  The confirm case leaves the frame loop from inside the pad switch with goto exit, straight to the clear-and-return after the loop, as the original's jump table does; structured alternatives either compile the clear inside the loop, shrink the jump table, or test an exit condition on every frame.
- `src/grid_menu.c`, `HandleGridInput`: `goto retry;` (2 places)
  The two goto retry jumps re-dispatch a rewritten pad event without a loop, because every structured loop tried (for (;;), while (1), while (1) breaking to a shared tail, do-while on a repeat flag, and while (gPadEvent >= 0)) lets the compiler's loop optimiser hoist the inner switches' compare constants 1 and 2 into saved registers and grow the frame from 32 to 40 bytes.
- `src/grid_menu.c`, `RunGridMenu`: `goto exit;` (3 places)
  The menu loop's three exits jump with goto exit to the single clear-screen and StopGridMusic tail after the endless loop, as the original does; a done flag makes the loop test it, and a return at each exit leaves a copy of the tail inside the loop.
- `src/kana_entry.c`, `RunStartScreen`: `goto exit;` (2 places)
  Both goto exit sites, the save-load wait in state 5's sub-state 3 and a confirmed name entry in state 11, leave the frame loop from nested switches straight to the shared VRAM and heap cleanup after the loop, skipping the frame's VSync tail; a loop-exit flag adds a test on every frame, and duplicating the cleanup with return at each site places the merged cleanup inside the loop.
- `src/lib/c2/sprintf.c`, `sprintf`: `goto do_dec;`; `goto do_hex;`; `goto done;`; `goto reswitch;` (3 places)
  The six gotos (reswitch 3 times, do_dec, do_hex, done) are this Macintosh-lineage formatter's jumps within and out of the conversion switch, and every structured form tried compiles differently.
- `src/upgrade.c`, `RunUpgradeScreen`: `goto done;`
  The phase switch's case 4 leaves the frame loop with goto done because the original's five-entry jump table sends case 4 straight to the closing clear after the loop while the default branch and cases 0 to 3 reach the single frame tail after the switch; every structured form tried changes the jump table, the block placement, or register allocation.

A declaration nothing uses whose presence still changes how the compiler lays out the routine. (4)

- `src/grid_effects.c`, `StartExpandingTexturedQuad`: `unsigned char unused[32];`
  The 32-byte array unused is this leaf routine's whole frame: the original reserves 32 bytes of stack and stores nothing there, and without the declaration the routine has no frame at all.
- `src/kana_entry.c`, `RunNameEntry`: `int unused[2];`
  The unused 8-byte array int unused[2] declared before KanaEntry state reserves the 8 untouched stack bytes at sp+16..23 that place state at sp+24 in the original's 4704-byte frame; removing it shrinks the frame by 8 bytes and shifts all five frame-addressing instructions.
- `src/lib/nitrbs/INIT.c`, `BuildSignedRunLevelCodes`: `unsigned short unused;`
  The 2-byte local unused, declared before level and never read, holds the frame slot that puts level where the original keeps it; the library builds this routine without optimisation, so every declared local gets its own slot in declaration order, and without it 6 instructions differ.
- `src/media_player.c`, `PlayIdleAudio`: `RECT rect;`
  The RECT rect, never read, holds the 8 bytes before the SpuCommonAttr inside inlined SetCommonVolume that put it where the original keeps it; its sibling PlayIdleMovie declares the same RECT and uses it to clear the screen, a step PlayIdleAudio does not have.

A declaration or form the original's code needs: each plainer one tried compiles differently. (8)

- `src/lib/libgs/GS_001.c`, `GsSortClear`: `clearPacket[buf].w = *(u_short *)&HWD0;`; `h = *(u_short *)&VWD0;`
  The clear packet's height and its 16-bit-colour width read the low halfword of VWD0 and HWD0 through u_short * casts because InitGeometry's block-scope extern volatile declaration earlier in the same file makes HWD0 and VWD0 volatile for the rest of the translation unit, blocking the compiler from narrowing plain reads to halfword loads.
- `src/lib/libgs/GS_001.c`, `InitGeometry`: `clip->h = *(u_short *)&VWD0;`; `clip->w = *(u_short *)&HWD0;`; `extern volatile long HWD0, VWD0;`
  HWD0 and VWD0 are declared volatile inside InitGeometry because the original stores w and h to them and immediately reloads both from memory for the aspect divide, whereas a plain declaration lets the compiler forward the stored registers; that block-scope volatile declaration also propagates to the file-scope symbols in GCC 2.7.2, so the clip rectangle's width and height must cast through u_short * to keep their halfword loads.
- `src/lib/math/muldf3.c`, `__muldf3`: `DoubleBits savedExponent;`
  The raw exponent sum is held in a DoubleBits local so the compiler keeps the initial exponent calculation before the mantissa calls and spills the sum to its own 8-byte stack slot.
- `src/media_player.c`, at file scope: `extern int gVSyncCount;`
  gVSyncCount is one four-byte common declared as plain int here and as volatile int in the clip unit and its header, matching the non-volatile halfword reads this unit emits against the volatile reads in the clip decoder.
- `src/special_scenes.c`, at file scope: `int gWaitFrames;`
  gWaitFrames is one four-byte common declared as int here and as unsigned short in the grid scene unit, matching the word loads and stores this unit emits and the halfword ones the other emits.

## Kept with the cause not established

Each construct below is in the source because the original's bytes need it: every plainer form tried compiles differently, and why the compiler needs this one is not established. It stays as written until its cause is found. (5)

- `src/lib/snd/vm_vol.c`, `_SsVmSetVol`: `do {` (2 places); `do { do { balance = _svm_pg[_svm_voice[voice].prog].mpan; } while (0); } while (0);`; `do { if (pan == 0) { pan = 1; } } while (0);`; `initial_gain = _svm_voice[voice].vol * volume / 127;`
  The do/while(0) around pan's default and the two nested around the mpan read are stand-ins whose original source construct is not established: the compiler counts register uses inside loop notes at higher weight, which raises pan ahead of count and voice ahead of the /63 constant, while the dead gain copy inside the channel-volume check pairs the /127 and /16129 constants during loop hoisting.

## Written so in a published source

Each construct below is written this way in a published source the routine derives from: it is not kept only for the compiled bytes, and is counted apart from the constructs above. (17)

A jump the routine's published source writes: the routine derives from that source and keeps its form. (17)

- `src/lib/c2/prnt.c`, `prnt`: `goto nosign;` (3 places); `goto number;`; `goto pforw;` (2 places); `goto rflag;` (11 places)
  All 17 gotos (rflag 11 times, nosign 3 times, number once, pforw twice) are written at the same labels and cases in 4.3BSD-Tahoe doprnt.c, revision 5.35 of that release.
