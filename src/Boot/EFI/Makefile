
include 	Makefile.inc

CFLAGS += -I.

OBJS_CTR        = cs_controller.o cs_crypto.o cs_debug.o cs_common.o cs_options.o cs_ui.o cs_service.o
OBJS_DRV        = cs_driver.o     cs_crypto.o cs_debug.o cs_common.o
OBJS_COMMON	= $(patsubst %.o,Common/%.o,Crc.o Xts.o GfMul.o Pkcs5.o Endian.o)
OBJS_CRYPTO	= $(patsubst %.o,crypto/%.o,Aescrypt.o Aeskey.o Aestab.o Twofish.o Serpent.o Rmd160.o)
#OBJS_CRYPTO	= $(patsubst %.o,crypto/%.o,AesSmall.o Aeskey.o Aestab.o Twofish.o Serpent.o Rmd160.o)

TARGET          = CSController.efi CSDriver.efi
#TARGET          = CSController.efi

%.o:	%.c %.h

all: crypt $(TARGET)

PHONY: all clean backup stick crypt

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
	$(MAKE) -C crypto
	$(MAKE) -C Common

backup:	clean
	tar zcf CSLoader-$(shell date +%Y-%m-%d-%H%M%S).tgz Makefile* *.c *.h include doc crypto Common

stick:	all
	if test -d /media/fn/6529-E05D; then cp *.efi  /media/fn/6529-E05D/copy/; fi; sync

clean:
	rm -f *.efi *.so *.o *~
	$(MAKE) -C crypto clean
	$(MAKE) -C Common clean
