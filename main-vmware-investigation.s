# 16-bit
.code16gcc
.code16

# Place something identifiable at the beginning, and jump past it
jmp start
.ascii "===BEGIN==="

# Some bytes to finish the "instructions" of "Hello, world!" when ran
# through a disassembler
.byte 0
.byte 0
.byte 0
nop
nop
nop
start:

# Clear segments
mov $0x0, %ax
#mov %ax, %cs
#mov %ax, %ss
mov %ax, %ds
#mov %ax, %es

# Get instruction pointer (verify we're actually running from around
# 0x7c00)
call get_ip
get_ip: pop %bx
mov %bh, %al
mov $0x0e, %ah
int $0x10
mov %bl, %al
mov $0x0e, %ah
int $0x10

# Print code segment
mov %cs, %bx
mov %bh, %al
mov $0x0e, %ah
int $0x10
mov %bl, %al
mov $0x0e, %ah
int $0x10

# Print stack segment
mov %ss, %bx
mov %bh, %al
mov $0x0e, %ah
int $0x10
mov %bl, %al
mov $0x0e, %ah
int $0x10

# Print data segment
mov %ds, %bx
mov %bh, %al
mov $0x0e, %ah
int $0x10
mov %bl, %al
mov $0x0e, %ah
int $0x10

# Print extra segment
mov %es, %bx
mov %bh, %al
mov $0x0e, %ah
int $0x10
mov %bl, %al
mov $0x0e, %ah
int $0x10

# Print the 512 bytes at 0x7c00
mov $0x0, %cx
printloop:
mov $0x7c00, %si
add %cx, %si
mov (%si), %al
mov $0x0e, %ah
int $0x10
inc %cx
cmp $0x1ff, %cx
jle printloop

# Print "Hello, world!"
mov $hello, %si
hello_loop:
mov (%si), %al
cmp $0x0, %al
je hello_out
mov 7, %bx
mov $0x0e, %ah
int $0x10
inc %si
jmp hello_loop
hello_out:

# Halt indefinitely
hltloop:
hlt
jmp hltloop

hello:
	.ascii "Hello, world!"
	.byte 0
