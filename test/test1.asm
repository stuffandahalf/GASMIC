.arch 6809
;.syntax motorola
.org 0

_start:
    adca 0x5
    .include "test/include1.asm"
    ;.include "test3.asm"
.data: .resb 1
