#ifndef NODES_H
#define NODES_H

/* Values of Node.flags bits 20..23; kind 1 (gLainDarkCellTmd) is unused by gNodes. */
enum NodeKind {
    NODE_KIND_LDA,
    NODE_KIND_TDA = 2,
    NODE_KIND_COU,
    NODE_KIND_DC,
    NODE_KIND_DIA,
    NODE_KIND_TRANSFER,
    NODE_KIND_UPGRADE,
    NODE_KIND_GATE,
    NODE_KIND_P2
};

/* One node of the grid.
   gridPos: bit 15 set for a disc 2 node; bits 3..14 its row counted from 1, 0 off the grid
   (three rows to a level); bits 0..2 its column.
   flags: bits 16..19 an upgrade count compared with gUpgradesApplied; 20..23 its kind
   (enum NodeKind), below NODE_KIND_UPGRADE a media node; 24 shown; 25 what ResetNodeStates
   restores 24 to; 26 opened; 27 set, the player starts with gEndRollState 1; 28..31 a
   threshold compared with gEndRollCount.
   unlockedBy: the node whose opening shows it, 0xffff none.
   unused: 0 in every node; nothing reads or writes it. */
typedef struct {
    char name[8];
    short titles[4];
    short words[3];
    short imageParts[3];
    short track;
    unsigned short gridPos;
    unsigned int flags;
    unsigned short unlockedBy;
    short unused;
} Node;

extern Node gNodes[716];

extern int gHeadingYawEighths[16];

#endif
