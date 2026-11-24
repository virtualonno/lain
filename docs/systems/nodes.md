# Nodes

Everything the player opens on the site is a node: a movie, a sound file, Lain's voice, a gate, a P2 file, an
upgrade. The game holds all 716 of them, for both Sites, in one table, `gNodes` (`src/nodes.c`, record `Node` in
`include/nodes.h`). The table is also the whole of the player's progress through them: which nodes are shown and
which opened live in each node's `flags`, and the save keeps those bits. Placing the nodes on the grid, deciding
which are visible, and opening and unlocking them is `src/grid_nodes.c`; the menu that walks the grid and opens
them is in [grid-menu.md](grid-menu.md), the scene that draws them in [grid.md](grid.md).

## The record

A `Node` is 40 bytes:

| Field | What it holds |
| --- | --- |
| `name` | The node's name, such as `Dc1001`; the caption shows its first six characters (`CopyCellNodeText`) |
| `titles[4]` | Indexes into `gTitles` (51 strings, `src/string_tables.c`), drawn on the caption's four title plates |
| `words[3]` | Indexes into `gWords` (417 strings), -1 for none: shown before a media node opens, and its links |
| `imageParts[3]` | Parts of the current Site's file (`gSiteAParts` or `gSiteBParts`) the media player shows as the track's images, -1 for none |
| `track` | The track `RunMediaPlayer` plays; equal to the node's own index in every entry |
| `gridPos` | Where the node sits: bit 15 the Site (set for Site B), bits 3 to 14 its row counted from 1 (0 is off the grid), bits 0 to 2 its column |
| `flags` | Bits 16 to 19 an upgrade count; 20 to 23 the kind; 24 shown; 25 shown at the start; 26 opened; 27 an end-roll node; 28 to 31 an end-roll threshold. Bits 0 to 15 are 0 |
| `unlockedBy` | The node whose opening shows this one, 0xFFFF for none |
| `unused` | 0 in every node; nothing reads it |

The names are a family prefix and a number. Two names differ only past the sixth character, `SSkn04` (Site A) and
`SSkn04#` (Site B), so both captions read `SSkn04`.

## Kinds

The kind (`enum NodeKind`, `include/nodes.h`) decides what opening a node does and which cell model stands for it
on the grid (`gNodeKindModelTypes`, an index into `gCellTmds` and `gCellTims`, `src/graphics.c`). Kinds below
`NODE_KIND_UPGRADE` are media nodes (`IsCursorNodeMedia`).

| Kind | Names | Nodes | Opening | Cell model |
| --- | --- | --- | --- | --- |
| `NODE_KIND_LDA` (0) | Lda, Env | 247 | the media player | `gLainCellTmd` |
| `NODE_KIND_TDA` (2) | Tda | 92 | the media player | `gToukoCellTmd` |
| `NODE_KIND_COU` (3) | Cou | 53 | the media player | `gLabCellTmd` |
| `NODE_KIND_DC` (4) | Dc | 57 | the media player | `gMovieCellTmd` |
| `NODE_KIND_DIA` (5) | Dia | 48 | the media player | `gPsychiatryCellTmd` |
| `NODE_KIND_TRANSFER` (6) | Eda, Ekm, Ere, TaK | 202 | the media player; the 181 TaK nodes' tracks (518 to 698) go on to the voice screen | `gTransferCellTmd` |
| `NODE_KIND_UPGRADE` (7) | SSkn | 7 | the upgrade screen (`RunUpgradeScreen`, `src/upgrade.c`) | `gSaiseikunCellTmd` |
| `NODE_KIND_GATE` (8) | GaTE | 4 | the gate screen (`RunGateScreen`, `src/memcard.c`) | `gTransferCellTmd` |
| `NODE_KIND_P2` (9) | P2- | 6 | the P2 screen (`RunP2Screen`, `src/loading.c`) | `gTransferCellTmd` |

No node has kind 1; its model, `gLainDarkCellTmd`, is never chosen (and `gNodeKindModelTypes` entries 10 and 11 are
likewise unused). An opened node uses the model eight entries further on, the `gOpened...` variant of the same
picture. The media player and the voice screen are in [media-player.md](media-player.md).

## Placing nodes on the grid

A Site is a stack of levels, each three rows of eight cells: 22 levels on Site A, 13 on Site B
(`GetGridLevelCount`, by `gCurrentDisc`, 0 for Site A). `gGridCells[23][24]` holds one `GridCell` per cell of the
current Site: the node in it (`nodeIndex`, -1 for none), the model type, the cell's model slot in the scene and its
projected screen position. A node with row *r* (counted from 1) and column *c* goes to level (*r* - 1) / 3, cell
*c* + 8 × ((*r* - 1) mod 3). Row numbers grow upward: the cursor's up moves to the next row, and past the third row
of a level to the level above.

`BuildNodeGrid` empties every level, then walks the table and places each node of the current Site whose row is not 0
and that is visible, with the model type of its kind, plus 8 when it is opened. Site A has 453 nodes (rows up to 66),
Site B 263 (rows up to 39); no two share a cell. The ten Env nodes have a `gridPos` of 0 and are never placed. The
grid is rebuilt at boot and before each return to the start screen (`src/main.c`), and by `RestoreGridCursor` after a
load from the start screen or the system menu, a Site change or the continue prompt.

## Visible and openable

`IsNodeVisible` lets a node onto the grid when all of these hold:

- its shown bit (24) is set;
- it is not a Site B node while the player is on Site A;
- it is not an upgrade node (`NODE_KIND_UPGRADE`) whose upgrade count is below `gUpgradesApplied`;
- its end-roll threshold is 0, or, once the end roll has been seen (`gEndRollCount` above 0), at most
  `gEndRollCount` + 1.

Being visible is not being openable. Pressing circle on a node asks `PickCursorClipSet`, which lets it open only
when the threshold is at most `gEndRollCount` and the upgrade count at most `gUpgradesApplied`. For an open node
one of Lain's opening clip sets (`OPEN_CLIP_SPIN_AWAY` (0) or `OPEN_CLIP_COLLAPSE_SPLIT` (1) in
`enum NodeOpenClipSet`, `include/lapks_player.h`) plays and the node opens; for a locked one a refusal set
(`OPEN_CLIP_REFUSE_SPARK` (2) to `OPEN_CLIP_REFUSE_VANISH` (4)) plays and nothing
opens. So a threshold one past the present count shows a node the player cannot open yet, and of the upgrade nodes
only the one whose count equals `gUpgradesApplied` can be opened, which makes the upgrades go in order. The
thresholds run from 0 to 5: 446 nodes have 0, 105 have 1, 60 have 2, 51 have 3, 37 have 4 and 17 have 5.

## Unlocking

At the start 466 nodes are shown and 250 hidden. Each hidden node names in `unlockedBy` the node that shows it, and
no shown node names one. `MarkNodeOpened(id, soundPlayed)`, called whenever a node has been opened:

1. does nothing if the node is already opened, else sets its opened bit;
2. sets the shown bit of every node it unlocks that has a row, on either Site;
3. places each such node of the current Site at once if its cell is empty and it is visible, rebuilding the cell's
   model, and plays sound 31 for the first one unless `soundPlayed` says a sound has already played;
4. clears its own shown bit if it is an upgrade, gate or P2 node (`NODE_KIND_UPGRADE` to `NODE_KIND_P2`), so it does not come back when the
   grid is rebuilt. The menu also empties its cell at once.

Two unlocks cross Sites: `Cou016` on Site A shows `Dc1055` on Site B, and `Dc1001` shows `Dc1058`. Those nodes
appear the next time Site B's grid is built.

`ResetNodeStates`, run when the game returns to the start screen, clears every opened bit and puts bit 25 back into
bit 24, which returns the table to its first state. The save keeps, for each node, bytes 34 to 37 of its record:
bits 16 to 31 of `flags` and `unlockedBy` ([memory-card.md](memory-card.md)).

## Words and links

A media node's three words are uploaded beside its name as it opens. In the media player the player can choose a
word; `RunMediaPlayer` hands back its slot plus one, and the menu looks for another node sharing it
(`FindNodeSharingWord`): the first visible node after this one in the table, wrapping round to the start, that has
the same word in any slot and stands on the current grid. If there is one the view moves to it and the cursor lands
on it; if not, sound 28 plays and a panel (`DrawNoSharedNodePanel`, `src/text.c`) stays until circle
([grid-menu.md](grid-menu.md)).

## End-roll nodes

Four nodes have bit 27: `Cou053`, `Dia048`, `Lda237` and `Tda092`, the last of their families. The menu passes the bit
into `RunMediaPlayer`. Once the player starts such a node's track the player's Exit no longer leaves, and when the
track is over the end roll plays (`PlayEndRoll`), then the voice screen with track 733, and the media player
returns `MEDIA_RESULT_END_ROLL`. The menu then adds one to `gEndRollCount` (up to 9999), marks the node opened and runs the continue prompt
(`RunContinuePrompt`). The end roll has a segment for each of the 57 Dc nodes, a still in place of each one not
opened, but it goes no further than segment 54, so the last two are never reached.

## Progress counters

`src/grid_menu.c` keeps the counters the nodes move. The first four are in the save:

| Counter | Moved by |
| --- | --- |
| `gEndRollCount` | each end roll seen |
| `gUpgradesApplied` | each upgrade node taken; the system menu's version number shows it |
| `gGatesOpened` | the gate screen, once per gate; four open the system menu's Site change |
| `gP2NodesOpened` | a bit per P2 node (`P2-06` 1, `P2-01` 2, `P2-02` 4, `P2-04` 8, `P2-03` 0x10, `P2-05` 0x20); all six (63) change the P2 screen and the idle media |
| `gTokimekiPercent` | each node that opens, up to 100 |
| `gGakkuriPercent` | each frame spent opening a node except the one it opens on, refusals included, up to 100 |
| `gHarumagePercent` | each circle on an empty cell, up to 100 |
| `gDokudenpaCount` | each idle media played, up to 9 |

The last four are not saved: they start again at 0 whenever `RestoreGridCursor` rebuilds the grid, and only the
save file's title shows them, one chosen at random ([memory-card.md](memory-card.md)).
