SRC_asm = \
BootCrt.asm \
#END of SRC_asm

SRC_c = \
../../Common/Crc.c \
../../Common/Crypto.c \
../../Common/Endian.c \
../../Common/Pkcs5.c \
../../Common/Volumes.c \
../../Common/Xts.c \
../../Crypto/Rmd160.c \
# end of SRC_c

SRC_cpp = \
BootConfig.cpp \
BootConsoleIo.cpp \
BootDebug.cpp \
BootDiskIo.cpp \
BootEncryptedIo.cpp \
BootMain.cpp \
BootMemory.cpp \
IntFilter.cpp \
Platform.cpp \
# end of SRC_cpp

ifneq (,$(findstring _,$(OBJDIR)))
#SINGLE CIPHER
ifneq (,$(findstring _AES,$(OBJDIR)))
#AES cipher
SRC_asm += \
../../Crypto/AesSmall_x86.asm \
../../Crypto/Aes_hw_cpu.asm \
#END of SRC_asm
SRC_c += \
../../Crypto/Aestab.c \
# end of SRC_c
endif
ifneq (,$(findstring _SERPENT,$(OBJDIR)))
SRC_c += \
../../Crypto/Serpent.c \
# end of SRC_c
endif
ifneq (,$(findstring _TWOFISH,$(OBJDIR)))
SRC_c += \
../../Crypto/Twofish.c \
# end of SRC_c
endif
else
#multi cipher
SRC_c += \
../../Crypto/AesSmall.c \
../../Crypto/Serpent.c \
../../Crypto/Twofish.c \
# end of SRC_c
endif

SRCS=$(SRC_c) $(SRC_cpp) $(SRC_asm)

OBJS=$(addprefix $(OBJDIR)/,$(notdir $(SRC_c:.c=.obj) $(SRC_cpp:.cpp=.obj) $(SRC_asm:.asm=.obj)))

#98 - .asm{$(OUTDIR)}.obj:
$(OBJDIR)/%.obj: %.asm
	@mkdir -p $(@D)
	ml /nologo /omf /c /I $(@D) /Fo $@ $<

#103 - {..\..\Crypto}.asm{$(OUTDIR)}.obj:
$(OBJDIR)/%.obj: ../../Crypto/%.asm
	@mkdir -p $(@D)
	nasm -Xvc -f obj -Ox -o $(@) -l $(basename $(@)).lst $<

#109 - {..\..\Crypto}.c{$(OUTDIR)}.obj:
$(OBJDIR)/%.obj: ../../Crypto/%.c
	@mkdir -p $(@D)
	cmd /c "set PATH=. & set LIBPATH=. & set LIB=. & set INCLUDE=. & echo > NUL: & $(CC) $(CFLAGS) $(CFLAGS_$(@D)) /c /Fo$@ /Fc$(basename $@).cod $< || exit 1"

#114 - {..\..\Common}.c{$(OUTDIR)}.obj:
$(OBJDIR)/%.obj: ../../Common/%.c
	@mkdir -p $(@D)
	cmd /c "set PATH=. & set LIBPATH=. & set LIB=. & set INCLUDE=. & echo > NUL: & $(CC) $(CFLAGS) $(CFLAGS_$(@D)) /c /Fo$@ /Fc$(basename $@).cod $< || exit 1"

#119 - .c{$(OUTDIR)}.obj:
$(OBJDIR)/%.obj: %.c
	@mkdir -p $(@D)
	cmd /c "set PATH=. & set LIBPATH=. & set LIB=. & set INCLUDE=. & echo > NUL: & $(CC) $(CFLAGS) $(CFLAGS_$(@D)) /c /Fo$@ /Fc$(basename $@).cod $< || exit 1"

#124 - .cpp{$(OUTDIR)}.obj:
$(OBJDIR)/%.obj: %.cpp
	@mkdir -p $(@D)
	cmd /c "set PATH=. & set LIBPATH=. & set LIB=. & set INCLUDE=. & echo > NUL: & $(CC) $(CFLAGS) $(CFLAGS_$(@D)) /c /Fo$@ /Fc$(basename $@).cod $< || exit 1"

#129 - $(OUTDIR)\BootDefs.i: BootDefs.h
$(OBJDIR)/BootDefs.i: BootDefs.h
	@mkdir -p $(@D)
	cmd /c "set PATH=. & set LIBPATH=. & set LIB=. & set INCLUDE=. & echo > NUL: & $(CC) $(CFLAGS) $(CFLAGS_$(@D)) /D TC_ASM_PREPROCESS /EP BootDefs.h || exit 1" > $@

OBJS_ASM = $(patsubst %.asm,${OBJDIR}/%.obj,${SOURCES_ASM})
#$(shell echo D: ${OBJS_ASM} >&2)

${OBJS_ASM}: $(OBJDIR)/BootDefs.i

#134 - $(OUTDIR)\BootSector.bin: $(OUTDIR)\BootSector.obj
$(OBJDIR)/BootSector.bin: $(OBJDIR)/BootSector.obj
	$(LD) $(LFLAGS) $(subst /,\,$<),$(subst /,\,$@),$(patsubst %.bin,%.map,$(subst /,\,$@)),,,

#140 - $(OUTDIR)\Decompressor.com: $(OUTDIR)\BootCrt.obj $(OUTDIR)\Decompressor.obj
$(OBJDIR)/Decompressor.com: $(OBJDIR)/BootCrt.obj $(OBJDIR)/Decompressor.obj
	$(LD) $(LFLAGS) $(@D)\BootCrt.obj $(@D)\Decompressor.obj,$(@D)\Decompressor.com,$(@D)\Decompressor.map,$(MSVC16_ROOT)\lib\+slibce,,

#~134 - $(OUTDIR)\BootSector.bin: $(OUTDIR)\BootSector.obj
#~140 - $(OUTDIR)\Decompressor.com: $(OUTDIR)\BootCrt.obj $(OUTDIR)\Decompressor.obj
#~146 - $(OUTDIR)\$(PROJ).$(TARGETEXT): $(OBJS)
$(OBJDIR)/$(PROJ).flp: $(OBJDIR)/BootSector.bin $(OBJDIR)/Decompressor.com $(OBJDIR)/$(PROJ).$(TARGETEXT).gz
	dd conv=notrunc bs=512 if=$(@D)/BootSector.bin of=$@
	dd conv=notrunc,sync bs=512 seek=1 if=$(@D)/Decompressor.com of=$@
	dd conv=notrunc,sync bs=512 seek=5 if=$(@D)/$(PROJ).$(TARGETEXT).gz of=$@

$(OBJDIR)/$(PROJ).crf: $(OBJS)
	echo '$(subst /,\,$^)' | sed 's/ \+/ +\n/g;' > $@
	#echo >> $@
	echo '$(@D)\$(PROJ).$(TARGETEXT)' >> $@
	echo '$(@D)\$(PROJ).map' >> $@
	echo '$(MSVC16_ROOT)\lib\+$(LIBS)' >> $@
	echo ';' >> $@

#146 - $(OUTDIR)\$(PROJ).$(TARGETEXT): $(OBJS)
$(OBJDIR)/$(PROJ).$(TARGETEXT): $(OBJDIR)/$(PROJ).crf
	$(LD) $(LFLAGS) @$(subst /,\,$<)

$(OBJDIR)/$(PROJ).$(TARGETEXT).gz: $(OBJDIR)/$(PROJ).$(TARGETEXT)
	gzip -c -n --best $(@D)/$(PROJ).$(TARGETEXT) > $(@D)/$(PROJ).$(TARGETEXT).tmp
ifneq (,$(findstring _,$(OBJDIR))) # single cipher
ifneq (,$(findstring Rescue_,$(OBJDIR)))
else # not rescue disk
	test $$(stat -c %s $(@D)/$(PROJ).$(TARGETEXT).tmp) -le 13312 #   check size, if compressed file is bigger than 13312, it is too big
endif
endif
	mv $(@D)/$(PROJ).$(TARGETEXT).tmp $(@D)/$(PROJ).$(TARGETEXT).gz