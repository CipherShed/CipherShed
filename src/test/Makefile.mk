.PHONY: test
test: test_run

.PHONY: test_run
test_run: ${bindir}/kernel
	$(QEMU_SYSTEM_I386) \
		-display none \
		-nodefaults \
		-nodefconfig \
		-no-user-config \
		-m 1M \
		-device isa-debug-exit \
		-chardev file,id=ttyS0,path=/dev/fd/2 \
		-device isa-serial,chardev=ttyS0 \
		-kernel $<; \
		test $$? -eq 3

.PHONY:
test_build: ${bindir}/kernel

${bindir}/kernel: LDFLAGS := ${LDFLAGS} -Wl,--script=${subdir}/kernel-unisection.ld
${bindir}/kernel: ${OBJS} ${lib}
	$(recipe_link)

-include ${MDEPS}
