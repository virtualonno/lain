# Unlit Gouraud-shaded triangles: transform, cull and link packets.

.set noreorder
.text
.globl GsTMDfastG3GNL

# In: a0 polygons, a1 vertices, a3 packet; Out: v0 next packet.
GsTMDfastG3GNL:
    lw    $14,16($sp)
    lw    $2,20($sp)
    lw    $3,24($sp)
    lw    $8,28($sp)
    lw    $3,4($3)
    addu  $10,$0,$0
    sw    $2,12($8)
    blez  $14,.Ldone
    sw    $3,16($8)
    addiu $15,$8,36
    lui   $25,0x8000
    addiu $13,$8,24
    lui   $11,0x00ff
    ori   $11,$11,0xffff
    lui   $24,0x0600
    addiu $6,$4,12
    addiu $9,$7,20
.Ltriangle:
    lhu   $4,6($6)
    lhu   $3,10($6)
    lhu   $2,14($6)
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
    lw    $2,-8($6)
    nop
    sw    $2,-16($9)
    cfc2  $12,$31
    nop
    sw    $12,0($15)
    lw    $2,36($8)
    nop
    and   $2,$2,$25
    bne   $2,$0,.Lnext
    nop
    nop
    nop
    .word 0x4b400006 # NCLIP
    lw    $2,-4($6)
    nop
    sw    $2,-8($9)
    swc2  $24,0($13)
    lw    $2,24($8)
    nop
    blez  $2,.Lnext
    nop
    swc2  $12,8($7)
    swc2  $13,16($7)
    swc2  $14,24($7)
    nop
    nop
    .word 0x4b58002d # AVSZ3
    lw    $2,0($6)
    nop
    sw    $2,0($9)
    swc2  $7,0($13)
    lw    $3,24($8)
    lw    $2,12($8)
    addiu $9,$9,28
    srav  $3,$3,$2
    lw    $2,16($8)
    sll   $3,$3,2
    addu  $2,$2,$3
    sw    $2,52($8)
    lw    $2,0($2)
    and   $3,$7,$11
    and   $2,$2,$11
    or    $2,$2,$24
    sw    $2,0($7)
    lw    $2,52($8)
    addiu $7,$7,28
    sw    $3,0($2)
.Lnext:
    addiu $10,$10,1
    slt   $2,$10,$14
    bne   $2,$0,.Ltriangle
    addiu $6,$6,28
.Ldone:
    jr    $31
    addu  $2,$7,$0
    nop
    nop
    nop
