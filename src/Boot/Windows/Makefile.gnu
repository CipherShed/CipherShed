.PHONY: all clean

CC = $(MSVC16_ROOT)\bin\cl.exe
LD = $(MSVC16_ROOT)\bin\link.exe

#CFLAGS += /Fc -- it is not https://msdn.microsoft.com/en-us/library/027c4t2s.aspx
CFLAGS = /nologo /W3 /I $(MSVC16_ROOT)\Include /I ../.. /I ../../Common /I ../../Crypto
CFLAGS += /D __int8=char /D __int16=int /D __int32=long /D BOOL=char /D FALSE=0 /D TRUE=1
CFLAGS += /D LITTLE_ENDIAN=1234 /D BYTE_ORDER=1234 /D TC_WINDOWS_BOOT /D TC_MINIMIZE_CODE_SIZE /D TC_NO_COMPILER_INT64
CFLAGS += /D malloc=malloc_NA
CFLAGS += /AT /Zl /f- /G3 /Oe /Os /Ob1 /OV0 /Gs /Gf /Gy /D NDEBUG

LFLAGS = /NOLOGO /ONERROR:NOEXE /NOI /BATCH
LFLAGS += /NOD /NOE /TINY

TARGETEXT = com

PROJ = BootLoader
LIBS = slibce

MODES := Rescue Release
CIPHERS := AES SERPENT TWOFISH

CAT      := $(shell which cat)
ECHO     := $(shell which echo)


#init all CFLAGS_mode_cipher vars
$(foreach mode, $(MODES), $(eval CFLAGS_$(mode) := ) $(foreach cipher, $(CIPHERS), $(eval CFLAGS_$(mode)_$(cipher) := ) ) )

# for rescue disks, define TC_WINDOWS_BOOT_RESCUE_DISK_MODE
_CFLAGS_Rescue := /D TC_WINDOWS_BOOT_RESCUE_DISK_MODE
CFLAGS_Rescue  := $(CFLAGS_Rescue) $(_CFLAGS_Rescue)
$(foreach cipher, $(CIPHERS), $(eval CFLAGS_Rescue_$(cipher) := $(CFLAGS_Rescue_$(cipher)) $(_CFLAGS_Rescue) )  )

#for single ciphers define TC_WINDOWS_BOOT_$cipher
_CFLAGS_SINGLECIPHER = /D TC_WINDOWS_BOOT_SINGLE_CIPHER_MODE /D TC_WINDOWS_BOOT_$(cipher)
$(foreach mode, $(MODES), $(foreach cipher, $(CIPHERS), $(eval CFLAGS_$(mode)_$(cipher) := $(CFLAGS_$(mode)_$(cipher)) $(_CFLAGS_SINGLECIPHER) ) ) )

##debug show the CFLAGS_*....
#$(foreach mode, $(MODES), $(shell echo CFLAGS_$(mode) = $(CFLAGS_$(mode)) >&2 ) $(foreach cipher, $(CIPHERS), $(shell echo CFLAGS_$(mode)_$(cipher) = $(CFLAGS_$(mode)_$(cipher)) >&2 ) ) )

SOURCES_ASM = $(wildcard *.asm)
#$(shell echo S: ${SOURCES_ASM} >&2)

#include the Makefile.inc for each build combo (e.g. Rescue/Release all/AES/etc...)
#$(foreach mode, $(MODES),              $(eval OBJDIR=$(mode))                   $(eval include Makefile.inc)   \
#  $(foreach SINGLE_CIPHER, $(CIPHERS), $(eval OBJDIR=$(mode)_$(SINGLE_CIPHER) ) $(eval include Makefile.inc) ) \
#)
DIRS := $(MODES)
$(foreach mode, $(MODES), $(foreach cipher, $(CIPHERS), $(eval DIRS += $(mode)_$(cipher) ) ) )

all: $(addsuffix /$(PROJ).flp,$(DIRS))

clean:
	rm -rf $(DIRS)
	
$(foreach trg, $(DIRS), $(eval OBJDIR=$(trg)) $(eval include Makefile.inc ))