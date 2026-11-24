    .set noreorder
    .set noat
    .text
    .globl _SsVmDamperOn
_SsVmDamperOn:
    addiu   $2, $0, 2
    lui     $1, %hi(_svm_damper)
    jr      $31
    sh      $2, %lo(_svm_damper)($1)
