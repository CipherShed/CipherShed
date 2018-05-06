#
# root Makefile - setup `make` for all modules
#


# tools

CROSS_PREFIX :=

CC := ${CROSS_PREFIX}gcc
CXX := ${CROSS_PREFIX}g++
AS := ${CROSS_PREFIX}as
OBJCOPY := ${CROSS_PREFIX}objcopy
AR := ${CROSS_PREFIX}ar

QEMU_IMG := qemu-img
QEMU_SYSTEM_I386 := qemu-system-i386

machine := $(shell ${CC} -dumpmachine)
machine := $(if ${machine},${machine},unknown)


# flags

ifeq "$(filter-out ia16-%,${machine})" "" # if $machine == ia16-*
cc_arch_flags :=
else
cc_arch_flags := -m16
endif

ASFLAGS := ${cc_arch_flags}
CPPFLAGS :=
C_CXX_FLAGS := ${cc_arch_flags} -O0 -ffreestanding -ffunction-sections
CFLAGS := -std=gnu11
CXXFLAGS := -std=gnu++11
LDFLAGS := ${cc_arch_flags} -nostdlib
LOADLIBES :=
LDLIBS :=


# target declaration helper variables

outdir_root := obj-${machine}
objdir_root := ${outdir_root}/obj
subdir = $(dir $(lastword ${MAKEFILE_LIST}))
objdir = ${objdir_root}/${subdir}
bindir = ${outdir_root}/bin
libdir = ${outdir_root}/lib
mkoutdir = test -d $(dir $@) || mkdir -p $(dir $@)

SRCS_ASM = $(wildcard ${subdir}/*.s)
SRCS_ASM_PREPROC = $(wildcard ${subdir}/*.S)
SRCS_C = $(wildcard ${subdir}/*.c)
SRCS_CXX = $(wildcard ${subdir}/*.cpp)

OBJS_ASM = $(patsubst %.s,${objdir_root}/%.o,${SRCS_ASM})
OBJS_ASM_PREPROC = $(patsubst %.S,${objdir_root}/%.o,${SRCS_ASM_PREPROC})
OBJS_C = $(patsubst %.c,${objdir_root}/%.o,${SRCS_C})
OBJS_CXX = $(patsubst %.cpp,${objdir_root}/%.o,${SRCS_CXX})

OBJS = ${OBJS_ASM} ${OBJS_ASM_PREPROC} ${OBJS_C} ${OBJS_CXX}

MDEPS = $(wildcard ${objdir}/*.d)

# default and primary targets

.PHONY: all
all:

.PHONY: clean
clean:
	rm -rf ${outdir_root}/

.PHONY: distclean
distclean:
	rm -rf obj-*/


# pattern targets

${objdir_root}/%.o: %.s
	@$(mkoutdir)
	$(CC) ${ASFLAGS} -o $@ -c $<

${objdir_root}/%.o: %.S
	@$(mkoutdir)
	$(CC) ${CPPFLAGS} ${ASFLAGS} -MMD -MP -o $@ -c $<

${objdir_root}/%.o: %.c
	@$(mkoutdir)
	$(CC) ${CPPFLAGS} ${C_CXX_FLAGS} ${CFLAGS} -MMD -MP -o $@ -c $<

${objdir_root}/%.o: %.cpp
	@$(mkoutdir)
	$(CXX) ${CPPFLAGS} ${C_CXX_FLAGS} ${CXXFLAGS} -MMD -MP -o $@ -c $<


# canned recipes
override define recipe_link
	@$(mkoutdir)
	$(CC) ${LDFLAGS} -o $@ ${LOADLIBES} ${LDLIBS} $^
endef

override define recipe_archive
	@$(mkoutdir)
	$(AR) rcs $@ $^
endef


# finally!: include module Makefiles for actual source and targets

# TODO: good way to include this from modules that use this?
include src/lib/Makefile.mk

include src/main/Makefile.mk
all: main

include src/test/Makefile.mk
all: test
