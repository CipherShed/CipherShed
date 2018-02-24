__attribute__((noreturn))
__attribute__((stdcall))
__attribute__((always_inline))
inline void halt()
{
	while (1) asm volatile ("hlt");
}
