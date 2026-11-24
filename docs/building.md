# Building

Serial Experiments Lain was built with SN Systems' PsyQ development kit for Sony's Runtime Library 4.3 (the
Japanese disc, DTL-S2340). The game's own code, its startup and the NITR-BS movie decoder are compiled here with
the tools of that kit. Sony's libraries are decompiled in `src/lib` too, but Sony compiled them, LIBSN
apart, with tools it never shipped, so a build from period tools links the kit's own archives, as the game's
authors did. Two inputs are not source: the order of the uninitialized globals, and the bytes the tree does
not hold (the game's pictures and models, and six controller-library modules), which are read from the
reader's own executable. The
script `build.sh` at the root of the public tree does all of this and checks the result's SHA-256.

## The discs and the executable

Both discs boot one executable, `SLPS_016.03` on disc 1 (SLPS-01603) and `SLPS_016.04` on disc 2 (SLPS-01604):
the same 618,496 bytes, SHA-256 `33e8e8f8ae7eea1a94d392dc851f639090633bcfb037de164ac5c3a39ef02f95`. It loads at
`0x80010000`, starts at `0x8001307C` and stays resident; every other file on the discs is data it reads
([formats.md](formats.md)). So the tree builds one program, `SLPS`, once (`config/lain/builds.tsv`).

The image is laid out in the command file's section order: read-only data, then the code (the startup, the
game's own 177,964 bytes, then the libraries' 206,568 bytes), then initialized data; the uninitialized data
follows it in memory.

## What `build.sh` does

`sh build.sh` builds the executable from the tree, the reader's PsyQ 4.3 kit and the reader's retail executable,
and stops with an error at the first input whose SHA-256 is not the one the tables give:

1. **Check the tools and headers.** Each tool of `config/lain/tools.tsv` is found in the reader's kit directory
   by file name and SHA-256, and each of the 51 kit headers by `config/lain/headers.sha256`.
2. **Extract the arrays.** The 113 arrays of the game's own data that the source includes but the tree does not
   hold (78 TMD models and 35 TIM images, 106,624 bytes) are cut from the reader's executable at the
   address and size `config/lain/assets.tsv` gives, checked by SHA-256 and written as the include files the
   units name.
3. **Compile the game.** Each C unit of `src/` is preprocessed by CPPPSX, compiled by CC1PSX at `-O2 -G8` and
   assembled by ASPSX 2.77 with its divide checks. An object left holding the placeholder words of the GTE macros
   would be rewritten by DMPSX; the game's own code uses no GTE instruction.
4. **Assemble the hand-written units.** The seven data units `src/*.s`, the startup `src/lib/obj/2MBYTE.s` and the
   decoder's `src/lib/nitrbs/DECODE.s` are assembled by ASMPSX.
5. **Build NITRBS.** The NITR-BS decoder is a library of the program's own (`src/lib/nitrbs/NITRBS.LST`), not a
   kit library: its C member `INIT.c` is compiled by the same CC1PSX at `-O0 -G0`, unoptimised.
6. **Take Sony's libraries from the kit.** The sixteen libraries the game links are the kit's archives, each
   checked by the SHA-256 in `config/lain/libraries.tsv`. LIBPAD is rebuilt from the kit's archive with six of
   its modules replaced by objects made from the reader's executable (`config/lain/stated.tsv`, below).
7. **Declare the commons' order** (`config/lain/commons.tsv`, below).
8. **Link** with PSYLINK 2.73: the 47 objects in the order `config/lain/SLPS.LNK` lists them (the startup, then
   the 46 game units), then the seventeen libraries in its order; convert the result to a PS-X executable and
   compare its SHA-256 with `builds.tsv`'s.

```
PSYQ_4_3=/path/to/psyq-4.3 SLPS_EXE=/path/to/SLPS_016.03 sh build.sh
```

The first variable names the kit's root (holding `PSX/INCLUDE`, `PSX/LIB` and the tools), the second the
retail executable of either disc. All tools of this build are Win32 programs: on Windows they run directly,
elsewhere a third variable names the command that runs one (`sh build.sh --help` lists every option).
`--build-dir DIR` chooses the working directory (default `build-verify`), and `--report DIR` copies the match
report `config/lain/reports/SLPS_016.03.json` there once the SHA-256 has passed. A successful run ends with
`Verified 1 executable(s) cleanly.`

On pushes to `master`, `.github/workflows/verify.yml` runs the same `build.sh --report` step on a hosted Linux
runner with `wibo` (checked by SHA-256), removes the build inputs before uploading, and publishes
`SLPS_016.03_report` once the executable's SHA-256 passes. The kit and the retail executable are not in this
repository.

### What it proves, and what it does not

A pass proves that the game's 46 units, its startup and the NITR-BS decoder, built from this tree with the period
tools, linked with the kit's libraries, give the retail executable byte for byte. Of the 618,496 bytes, 12,432
(LIBPAD's six modules) and the 113 arrays (106,624) come from the reader's executable rather than from source.

It does not compile the Sony modules in `src/lib`: no compiler of the 4.3 kit reproduces them (next section).

## The tools

All hashes are SHA-256; `config/lain/tools.tsv` lists each tool with its size and what it is used for.

| Role | Tool | Version | SHA-256 | Kit |
| --- | --- | --- | --- | --- |
| Preprocessor | CPPPSX | 2.8.0 SN32 Build 4.0.0007 | `7becd3512121a037d6c413e35ec4d895c968ec2a52e84a24020e9e21e81a1919` | 4.3 |
| Compiler (game, NITRBS) | CC1PSX | 2.8.0 SN32 Build 4.0.0007 | `17081b8e39a92078b16aa788f419306c0d77b28c30a02b9bb9c208cac172e5be` | 4.3 |
| Assembler (game, NITRBS) | ASPSX | 2.77 | `60a72cc29aa556cd45db1224669583e0e00ecb7b19ecc2efa5da63a6e1598bbb` | 4.3 |
| Assembler (hand-written units) | ASMPSX | 2.45 | `c4d686bd6ec9035b3b2295429e765f90b3d997a7f2666790008676ce4d0e85fa` | 4.3 |
| GTE post-processor | DMPSX | 3.04 | `d4b19e89d485fb5536303200f7dd53e328178d9b5e80fe079a7d4ce36955eb92` | 4.3 |
| Linker | PSYLINK | 2.73 | `0ee365fa3d31da71ba7086ccecd3d0df2fa39d4ad429e6a983fd9e4037272c6b` | 4.3 |

`tools.tsv` also lists the tools Sony's libraries were built with (next section): two that no kit ships, and
for LIBSN, CC1PSX 2.6.0 (the DOS compiler of the 3.5 kit) and ASPSX 2.21 (the DOS assembler of the 3.3 kit).

### What the game's code shows of its tools

These facts of the retail executable's own code (between the startup and the first library module) decide the
tools and options; a reader comparing the source with the original's code meets each of them.

- **`-G8`.** The game's code reaches small data through `$gp` 2,793 times; the library code never does.
  At `-G8` every global of eight bytes or less goes to small data.
- **`li` is `addiu`.** The assembler writes `li` of a constant below 0x8000 as `addiu $r,$zero,N` (2,884 times
  in the game's code), as ASPSX 2.77 does; the only three `ori $r,$zero,N` load constants of 0x8000 and above,
  which `addiu` cannot.
- **Divides are checked.** Each of the game's 24 divides (23 signed, one unsigned) is followed by the
  assembler's check for a zero divisor (`break 7`, 24 times) and, when signed, for overflow (`break 6`, 23
  times). The game was not assembled with the option that drops them.
- **No GTE instruction.** Every coprocessor 2 instruction of the executable (1,770) is in Sony's library code;
  the game calls the library for all its geometry.

The image links Runtime Library 4.3: it carries LIBGPU's `sys.c` stamp (revision 1.140, 1998-01-12) and
LIBETC's `intr.c` stamp (revision 1.75), as the 4.3 kit's archives do. Every routine of the game is reproduced
by that kit's CC1PSX; that no other build of the compiler would reproduce them is not established.

## Sony's libraries: built with tools Sony never shipped

Every module the game links from the sixteen kit libraries is decompiled in `src/lib/<library>/`, one file per
module, with each library's member list (`<LIBRARY>.LST`) in archive order. But the kit's own compiler is not
the one that built them. Sony compiled Runtime Library 4.3 with a GNU C 2.7.2 and a MIPS assembler of its own,
neither of which any PsyQ kit ships, and with options of its own (`config/lain/flags.tsv`):

| Libraries | Compiler and flags | Assembler | Divide checks |
| --- | --- | --- | --- |
| LIBAPI, LIBC2, LIBCARD, LIBDS, LIBETC, LIBGTE, LIBMATH, LIBMCRD, LIBPRESS, LIBSPU | GNU C 2.7.2 (Sony's), `-O2 -G0 -fsigned-char -mmips-as` | Sony's MIPS assembler | yes |
| LIBCD, LIBPAD, LIBSND, LIBGS | the same, with `-mhard-float` | the same | yes |
| LIBGPU | the same, with `-fno-builtin -mhard-float` | the same | yes |
| LIBSN | CC1PSX 2.6.0, `-O2 -G0` | ASPSX 2.21 | yes |
| NITRBS (the game's own) | CC1PSX 2.8.0, `-O0 -G0` | ASPSX 2.77 (ASMPSX for `DECODE.s`) | yes |

`-G0` is why no library global lives in small data. The options are those each library's modules reproduce
with; LIBGS, for one, needs `-mhard-float`: without it its `GsSortObject4` allocates its registers otherwise.

No kit holds the compiler that built these libraries, so their source could not be checked the way the game's
is. It was checked module by module instead. Each module was compiled from the source here by a GNU C 2.7.2
built from its released source and adjusted where Sony's build behaves differently, and assembled in the forms
Sony's assembler writes; the object was then compared with the member of the kit's archive, or, for the six
modules of LIBPAD, with the bytes in the executable. LIBSN's modules were built with the DOS tools the table
names. All 445 modules the game links from the sixteen libraries reproduce. (CC1PSX 2.7.2 of the 4.1 kit, the
nearest compiler that ships, does not reproduce most of them.)

That compiler is not a period tool and is not in this tree, so the reader cannot repeat the check from a kit
alone. What `build.sh` shows is the other half: the game's source, linked with the kit's own archives as its
authors linked it, gives the retail executable.

### The stated modules

The LIBPAD the game links is not the 4.3 kit's. Six of its seven modules differ from that archive's members;
only `WAITRC2` is the kit's. `config/lain/stated.tsv` names each section of the six by address, size and SHA-256,
with the names it exports and imports:

| Module | Holds | Bytes |
| --- | --- | ---: |
| `PADENTRY` | the public entry points (`PadChkVsync`, `PadGetState`, `PadInfoMode`, `PadSetAct`, ...) | 1,248 |
| `PADMAIN` | the serial port driver and its vertical-blank hook, and the table of handlers | 3,664 |
| `PADCMD` | the configuration commands (modes, actuators, alignment) | 3,072 |
| `PADIF` | the exchange of bytes with a controller | 1,584 |
| `PADPORTD` | `PadInitDirect`, the direct port mode | 1,648 |
| `PADSEQD` | the direct mode's automatic sequence | 1,216 |

The bytes are initialized sections only (their uninitialized storage is listed with its size). The build
assembles each module's sections from the reader's executable into an object with the same names and puts it
in the kit's LIBPAD archive in place of the kit's member. The source of all seven modules is in
`src/lib/libpad/`.

## The one link input that is not source: the order of uninitialized globals

A global defined without an initializer (`volatile int gVSyncCount;` at file scope) is a *common*: the compiler emits
only its name and size and leaves its placement to the linker. Two commons carry different declarations in two
units to match the original's accesses (`gVSyncCount`, `volatile int` in `include/clip.h` and `src/clip.c` but
plain `int` in `src/media_player.c`; `gWaitFrames`, `unsigned short` in `src/grid_scene.c` and `int` in
`src/special_scenes.c`); both are listed in [kept-constructs.md](kept-constructs.md). Where PSYLINK 2.73 puts each common follows
their names, and the names are lost: the executable keeps no symbols. With the names this tree uses, the linker
would place the commons in another order and every address after the first would move. The original's order is
therefore declared.

`config/lain/commons.tsv` lists every common in the order the original holds them, with its address and the
bytes it spans to the next. Each row is a global whose address the original's code binds, read from that code.
It has 339 rows: 263 of the game's (227 small, in `.sbss`, and 36 large, in `.bss`), one of NITRBS's and 75 of
Sony's libraries' (all large: the libraries are built at `-G0`), which are listed at their addresses with the
rest. At link time the table becomes definitions of the game's commons: one object, taken from a library
linked after all the others, defines the small commons in the table's order, with the gaps between them; and for each unit with
more than one large common, an object linked right after the unit defines them in the table's order. The
libraries' commons are left to the linker. Renaming a global never moves it.

## The build as facts

The tree holds everything the build consumes except the tools, the kit's headers and libraries, and the bytes
read from the reader's executable, and states those by hash:

| File | Holds |
| --- | --- |
| `config/lain/tools.tsv` | Each tool by file name, version, size and SHA-256, its kit, and what it is used for. |
| `config/lain/flags.tsv` | For the game's code and each library: the compiler and its options, the assembler, divide checks. |
| `config/lain/builds.tsv` | The executable: its command file, source and header directories, commons order and SHA-256. |
| `config/lain/SLPS.LNK` | The link order: the startup, the 46 game units, then the libraries. |
| `config/lain/libraries.tsv` | Each library's member list, the kit file and its SHA-256, and how a link from this tree's library source and `build.sh` each take it. |
| `config/lain/commons.tsv` | The declared order of the uninitialized globals. |
| `config/lain/assets.tsv` | The 113 arrays read from the executable: unit, address, size, section, SHA-256 and layout. |
| `config/lain/stated.tsv` | LIBPAD's six modules read from the executable, section by section. |
| `config/lain/headers.sha256` | The 51 Runtime Library 4.3 headers by SHA-256. |
| `config/lain/reports/SLPS_016.03.json` | Per-executable match report copied out by `build.sh --report` once the SHA-256 passes. |

`build.sh` is generated from these tables and reads them as it runs.
