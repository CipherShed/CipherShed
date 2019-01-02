.PHONY: test
test: test_run

.PHONY: test_run
test_run: bindir:=${bindir}
test_run: ${bindir}/kernel ${bindir}/floppy00.img ${bindir}/drive80.img
	$(QEMU_SYSTEM_I386) \
		-display none \
		-nodefaults \
		-no-user-config \
		-m 2M \
		-device isa-debug-exit \
		-chardev file,id=ttyS0,path=/dev/fd/2 \
		-device isa-serial,chardev=ttyS0 \
		-kernel ${bindir}/kernel \
		-drive file=${bindir}/floppy00.img,format=raw,if=floppy \
		-drive file=${bindir}/drive80.img,format=raw \
		; test $$? -eq 3

.PHONY:
test_build: ${bindir}/kernel

${bindir}/kernel: LDFLAGS := ${LDFLAGS} -Wl,--script=${subdir}/kernel-unisection.ld
${bindir}/kernel: ${OBJS} ${lib}
	$(recipe_link)

${bindir}/floppy00.img:
	truncate --size=$$((2880*512)) $@

${bindir}/drive80.img:
	truncate --size=$$((2880*512)) $@

-include ${MDEPS}
