	.org 0x100

_start:
    ;abx
    ;adca #$5, "banana"
	lda ,X++
	ldb ,--Y

.str:
    db "Hello World"
    db 0x0a, 0x0d, 0
