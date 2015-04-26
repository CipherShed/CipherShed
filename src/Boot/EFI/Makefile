
include 	Makefile.inc

CFLAGS += -I.

OBJS_CTR        = cs_controller.o cs_crypto.o cs_debug.o cs_common.o cs_options.o cs_ui.o cs_service.o
OBJS_DRV        = cs_driver.o     cs_crypto.o cs_debug.o cs_common.o
OBJS_COMMON	= $(patsubst %.o,../../Common/%.o,Crc.o Xts.o GfMul.o Pkcs5.o Endian.o)
OBJS_CRYPTO	= $(patsubst %.o,../../Crypto/%.o,Aescrypt.o Aeskey.o Aestab.o Twofish.o Serpent.o Rmd160.o)

TARGET          = CSController.efi CSDriver.efi

%.o:	%.c %.h

all: crypt $(TARGET)

PHONY: all clean stick crypt

CSController.so: $(OBJS_CTR) $(OBJS_COMMON) $(OBJS_CRYPTO)
	ld $(LDFLAGS) $(OBJS_CTR) $(OBJS_COMMON) $(OBJS_CRYPTO) -o $@ -lefi -lgnuefi

CSDriver.so: $(OBJS_DRV) $(OBJS_COMMON) $(OBJS_CRYPTO)
	ld $(LDFLAGS) $(OBJS_DRV) $(OBJS_COMMON) $(OBJS_CRYPTO) -o $@ -lefi -lgnuefi

CSController.efi: CSController.so
	@echo [OBJ] $@
	objcopy -j .text -j .sdata -j .data -j .dynamic \
	        -j .dynsym  -j .rel -j .rela -j .reloc \
	        --target=efi-app-$(ARCH) $< $@

CSDriver.efi: CSDriver.so
	@echo [OBJ] $@
	objcopy -j .text -j .sdata -j .data -j .dynamic \
	        -j .dynsym  -j .rel -j .rela -j .reloc \
	        --target=efi-bsdrv-$(ARCH) $< $@

crypt:
	$(MAKE) -C ../../Crypto -f Makefile.efi 
	$(MAKE) -C ../../Common -f Makefile.efi

clean:
	rm -f *.efi *.so *.o *~
	$(MAKE) -C ../../Crypto -f Makefile.efi clean
	$(MAKE) -C ../../Common -f Makefile.efi clean

stick:	all
	if test -d /media/fn/6529-E05D; then cp *.efi  /media/fn/6529-E05D/copy/; fi; sync
