.code16
.global halt
.type halt, @function
halt:
	hlt
	jmp halt
