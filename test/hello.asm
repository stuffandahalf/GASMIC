com1:   EQU 0xE000
.port:  EQU com1 + 1

    ORG 0xE000

_start:
    LDS 0x7FFF
    LDX #msg
    CALL print

print:
    LDA ,X+
    BEQ .end
    STA com1.port+1
    BRA print
.end:
    RET
    
msg:
    db "Hello World", 0x0A, 0

    REPEAT 0xFFF0-*, db 0

vectors:
.reserved: dw 0
.swi3: dw 0
.swi2: dw 0
.firq: dw 0
.irq: dw 0
.swi: dw 0
.nmi: dw 0
.reset: dw _start


