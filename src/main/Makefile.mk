ARTIFACT := boot

.PHONY: main
main: ${ARTIFACT}.com

#
# Disassembleable object file with layout of final product
#

.PHONY: ${ARTIFACT}.o
${ARTIFACT}.o: ${objdir}/${ARTIFACT}.o

${objdir}/${ARTIFACT}.o: LDFLAGS := ${LDFLAGS} -Wl,--script=${subdir}/mbr.ld -Wl,--require-defined=main
${objdir}/${ARTIFACT}.o: ${OBJS}
	$(recipe_link)

#
# Final product: flat binary (boot sector)
#

.PHONY: ${ARTIFACT}.com
${ARTIFACT}.com: ${objdir}/${ARTIFACT}.com

${objdir}/${ARTIFACT}.com: ${objdir}/${ARTIFACT}.o
	$(OBJCOPY) $< $@ -O binary

#
# raw disk image containing boot sector
# (used to satisfy emulator minimum image size requirement)
#

.PHONY: ${ARTIFACT}.img
${ARTIFACT}.img: ${objdir}/${ARTIFACT}.img

${objdir}/${ARTIFACT}.img: ${objdir}/${ARTIFACT}.com
	cp $< $@
	truncate --size=4M $@

#
# VMWare disk image containing boot sector
#

.PHONY: ${ARTIFACT}.vmdk
${ARTIFACT}.vmdk: ${objdir}/${ARTIFACT}.vmdk

${objdir}/${ARTIFACT}.vmdk: ${objdir}/${ARTIFACT}.img
	$(QEMU_IMG) convert -f raw -O vmdk $< $@

-include ${MDEPS}
