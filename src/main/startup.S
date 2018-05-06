.code16
.section .text.startup
startup:
	// Clear segments
	mov $0, %ax
	mov %ax, %ds
