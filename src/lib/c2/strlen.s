    .set noreorder
    .text
    .globl strlen
strlen:
    addu    $3, $0, $0
    beq     $4, $0, .Lreturn
    addu    $2, $0, $0
    j       .Ltest
    nop
.Lcount:
    addiu   $3, $3, 1
.Ltest:
    lb      $2, 0($4)
    nop
    bne     $2, $0, .Lcount
    addiu   $4, $4, 1
    addu    $2, $3, $0
.Lreturn:
    jr      $31
    nop
    nop
    nop
    nop
