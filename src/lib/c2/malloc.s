    .set noreorder
    .text

    .globl malloc
malloc:
    addiu   $10, $0, 0xa0
    jr      $10
    addiu   $9, $0, 0x33

    .globl free
free:
    addiu   $10, $0, 0xa0
    jr      $10
    addiu   $9, $0, 0x34

    .globl calloc
calloc:
    addiu   $10, $0, 0xa0
    jr      $10
    addiu   $9, $0, 0x37

    .globl realloc
realloc:
    addiu   $10, $0, 0xa0
    jr      $10
    addiu   $9, $0, 0x38
