.arch 6809
.org 0

_start:
    adc a, 0x5
    .include "test/include1.asm"
    ;.include "test3.asm"
.data: .resb 1
