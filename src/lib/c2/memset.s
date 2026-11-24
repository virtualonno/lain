    .set noreorder
    .text
    .globl memset
memset:
    beq     $4, $0, .Lreturn
    addu    $2, $0, $0
    bgtz    $6, .Lfill
    addu    $2, $4, $0
    j       .Lreturn
    addu    $2, $0, $0
.Lfill:
    sb      $5, 0($4)
    addiu   $6, $6, -1
    bgtz    $6, .Lfill
    addiu   $4, $4, 1
.Lreturn:
    jr      $31
    nop
