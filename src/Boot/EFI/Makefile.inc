ARCH            = $(shell uname -m | sed s,i[3456789]86,ia32,)
# ARCH is either "ia32" or "x86_64"
#ARCH		= x86_64

ifeq ($(ARCH),x86_64)
  LIB             = /usr/lib64
  EFILIB          = /usr/lib64
else
  LIB             = /usr/lib
  EFILIB          = /usr/lib
endif

EFIINC          = /usr/include/efi
LOCALINC	= ./include
OTHERINCS	= -I$(EFIINC)/$(ARCH) -I$(EFIINC)/protocol
EFIINCS         = -I$(LOCALINC) -I$(EFIINC) $(OTHERINCS)
EFI_CRT_OBJS    = $(EFILIB)/crt0-efi-$(ARCH).o
EFI_LDS         = $(EFILIB)/elf_$(ARCH)_efi.lds

CFLAGS          = $(EFIINCS) -fno-stack-protector -fpic \
		  -fshort-wchar -mno-red-zone -Wall -DEFI -DEFI_DEBUG

LDFLAGS         = -nostdlib -znocombreloc -T $(EFI_LDS) -shared \
		  -Bsymbolic -L $(EFILIB) -L $(LIB) $(EFI_CRT_OBJS) 

ifeq ($(ARCH),x86_64)
  CFLAGS += -m64 -DEFI_FUNCTION_WRAPPER -DHAVE_USE_MS_ABI
endif
