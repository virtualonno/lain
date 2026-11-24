# Gouraud-shaded quads with a normal per vertex: transform, cull, light and link packets.

.set noreorder
.text
.globl GsTMDfastG4GL

# In: a0 polygons, a1 vertices, a2 normals, a3 packet; Out: v0 next packet.
GsTMDfastG4GL:
    addiu $29,$29,-8
    lw    $25,24($29)
    lw    $2,28($29)
    lw    $3,32($29)
    lw    $9,36($29)
    addu  $10,$4,$0
    sw    $16,0($29)
    lw    $3,4($3)
    addu  $11,$0,$0
    sw    $2,12($9)
    blez  $25,.Ldone
    sw    $3,16($9)
    addiu $15,$9,40
    lui   $24,0x8000
    addiu $14,$9,24
    lui   $13,0x00ff
    ori   $13,$13,0xffff
    lui   $16,0x0800
    addiu $8,$4,32
.Lquad:
    lhu   $4,-10($8)
    lhu   $3,-6($8)
    lhu   $2,-2($8)
    sll   $4,$4,3
    addu  $4,$5,$4
    sll   $3,$3,3
    addu  $3,$5,$3
    sll   $2,$2,3
    addu  $2,$5,$2
    lwc2  $0,0($4)
    lwc2  $1,4($4)
    lwc2  $2,0($3)
    lwc2  $3,4($3)
    lwc2  $4,0($2)
    lwc2  $5,4($2)
    nop
    nop
    .word 0x4a280030 # RTPT
    cfc2  $12,$31
    nop
    sw    $12,0($15)
    lw    $2,40($9)
    nop
    and   $2,$2,$24
    bne   $2,$0,.Lnext
    nop
    nop
    nop
    .word 0x4b400006 # NCLIP
    swc2  $24,0($14)
    lw    $2,24($9)
    nop
    blez  $2,.Lnext
    nop
    swc2  $12,8($7)
    swc2  $13,16($7)
    swc2  $14,24($7)

    # Project the fourth vertex, then use all four depths for the OT bucket.
    lhu   $2,2($8)
    nop
    sll   $2,$2,3
    addu  $2,$5,$2
    lwc2  $0,0($2)
    lwc2  $1,4($2)
    nop
    nop
    .word 0x4a180001 # RTPS
    cfc2  $12,$31
    nop
    sw    $12,0($15)
    lw    $2,40($9)
    nop
    and   $2,$2,$24
    bne   $2,$0,.Lnext
    addiu $2,$7,32
    swc2  $14,0($2)
    nop
    nop
    .word 0x4b68002e # AVSZ4
    swc2  $7,0($14)
    addiu $2,$10,4
    lwc2  $6,0($2)
    lhu   $2,-12($8)
    nop
    sll   $2,$2,3
    addu  $2,$6,$2
    lwc2  $0,0($2)
    lwc2  $1,4($2)
    nop
    nop
    .word 0x4b08041b # NCCS
    lw    $3,24($9)
    lw    $2,12($9)
    nop
    srav  $3,$3,$2
    lw    $2,16($9)
    sll   $3,$3,2
    addu  $2,$2,$3
    sw    $2,56($9)
    addiu $2,$7,4
    swc2  $22,0($2)
    addiu $2,$10,8
    lwc2  $6,0($2)
    lhu   $2,-8($8)
    nop
    sll   $2,$2,3
    addu  $2,$6,$2
    lwc2  $0,0($2)
    lwc2  $1,4($2)
    nop
    nop
    .word 0x4b08041b # NCCS
    lw    $2,56($9)
    nop
    lw    $2,0($2)
    nop
    and   $2,$2,$13
    or    $2,$2,$16
    sw    $2,0($7)
    addiu $2,$7,12
    swc2  $22,0($2)
    addiu $2,$10,12
    lwc2  $6,0($2)
    lhu   $2,-4($8)
    nop
    sll   $2,$2,3
    addu  $2,$6,$2
    lwc2  $0,0($2)
    lwc2  $1,4($2)
    nop
    nop
    .word 0x4b08041b # NCCS
    lw    $3,56($9)
    and   $2,$7,$13
    sw    $2,0($3)
    addiu $2,$7,20
    swc2  $22,0($2)
    addiu $2,$10,16
    lwc2  $6,0($2)
    lhu   $2,0($8)
    nop
    sll   $2,$2,3
    addu  $2,$6,$2
    lwc2  $0,0($2)
    lwc2  $1,4($2)
    nop
    nop
    .word 0x4b08041b # NCCS
    addiu $2,$7,28
    swc2  $22,0($2)
    addiu $7,$7,36
.Lnext:
    addiu $11,$11,1
    addiu $8,$8,36
    slt   $2,$11,$25
    bne   $2,$0,.Lquad
    addiu $10,$10,36
.Ldone:
    addu  $2,$7,$0
    lw    $16,0($29)
    jr    $31
    addiu $29,$29,8
    nop
    nop
