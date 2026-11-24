    .set noreorder
    .set noat
    .text
    .globl _SsVmDamperOff
_SsVmDamperOff:
    lui     $1, %hi(_svm_damper)
    jr      $31
    sh      $0, %lo(_svm_damper)($1)
    nop
