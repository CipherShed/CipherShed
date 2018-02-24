.code16
.section .text.startup
.type startup, @function
startup:
	// Clear segments
	mov $0, %ax
	mov %ax, %ds
