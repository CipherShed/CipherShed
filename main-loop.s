.file "main.c"
#APP

.code16gcc
jmp loop
nop
OEMID: .ascii "CSBOOTLD"
BPB: .ascii "1234567890123456789012345"
EBPB: .ascii "12345678901234567890123456"

loop: inc %al
mov 7, %bx
mov $0x0e, %ah
int $0x10
jmp loop
