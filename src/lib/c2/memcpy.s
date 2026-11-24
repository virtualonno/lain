    .set noreorder
    .text
    .globl memcpy
memcpy:
    beq     $4, $0, .Lreturn
    addu    $2, $0, $0
    blez    $6, .Ldone
    addu    $3, $4, $0
.Lcopy:
    lbu     $2, 0($5)
    addiu   $5, $5, 1
    addiu   $6, $6, -1
    sb      $2, 0($4)
    bgtz    $6, .Lcopy
    addiu   $4, $4, 1
.Ldone:
    addu    $2, $3, $0
.Lreturn:
    jr      $31
    nop
    nop
    nop
    nop
