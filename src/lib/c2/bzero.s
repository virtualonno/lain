    .set noreorder
    .text
    .globl bzero
bzero:
    beq     $4, $0, .Lreturn
    addu    $2, $0, $0
    bgtz    $5, .Lclear
    addu    $2, $4, $0
    j       .Lreturn
    addu    $2, $0, $0
.Lclear:
    sb      $0, 0($4)
    addiu   $5, $5, -1
    bgtz    $5, .Lclear
    addiu   $4, $4, 1
.Lreturn:
    jr      $31
    nop
