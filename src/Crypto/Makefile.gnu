ifeq (${ConfigurationName},Debug)
	NASMFLAG = -g
else ifeq (${Configuration},Debug)
	NASMFLAG = -g
else
	NASMFLAG = 
endif

.PHONY: compile_asm

SRC_asm = \
Aes_hw_cpu.asm \
Aes_x86.asm \
#END of SRC_asm

compile_asm: $(SRC_asm:%.asm=$(OutDir)/%.obj)

OBJ=$(SRC_asm:.asm=$(OutDir)/.obj)

$(OutDir)/%.obj: %.asm
	nasm -Xvc -f win32 -Ox $(NASMFLAG) --prefix _ -o $@ -l $(@:.obj=.lst) $<

