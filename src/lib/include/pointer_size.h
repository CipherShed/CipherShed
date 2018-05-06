// Allows the usage of call_c in assembly to call C code with the
// correct instruction depending on target.
//
// Note: Requires -DPOINTER16 or -DPOINTER32 on gcc command-line.
//
// TODO: Knowing that GCC is what's making the decisions about operand
// size in the first place (we're just matching them in our assembly),
// does GCC provide any builtin macros to determine operand size?

#if defined POINTER32
	#define jmp_c jmpl
	#define call_c calll
	#define ret_c retl
	#define sp_c %esp
	#define push_c pushl
	#define pop_c popl
	#define ptr_bytes 4
#elif defined POINTER16
	#define jmp_c jmpw
	#define call_c callw
	#define ret_c retw
	#define sp_c %sp
	#define push_c pushw
	#define pop_c popw
	#define ptr_bytes 2
#else
	#error "Can't determine pointer size."
#endif
