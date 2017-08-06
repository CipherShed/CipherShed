### Bootstrapping

# Enable secondary expansion so variables like OBJS can differ on a
# per-target basis.
.SECONDEXPANSION:

# Make make silent by default
ifndef VERBOSE
.SILENT:
endif

# Don't strip artifacts if DEBUG is enabled
ifdef DEBUG
NOSTRIP=1
endif

## Cross compile overrides
ifdef CROSS_WINDOWS
	CROSS_PREFIX=i686-w64-mingw32-
endif

## Build tools
CC=${CROSS_PREFIX}gcc
CXX=${CROSS_PREFIX}g++
AS=nasm
AR=${CROSS_PREFIX}ar

## Determine target platform
TARGET_MACHINE:=$(shell ${CC} -dumpmachine)

# Default to Unix-like if platform-specific checks below don't pass
TARGET_PLATFORM=unix

# Configure environment to make platform-specific checks easy

# Arch
ifneq ("$(shell echo ${TARGET_MACHINE} | grep -o "i386\|i486\|i586\|i686")", "")
	TARGET_ARCH=x86
endif
ifneq ("$(shell echo ${TARGET_MACHINE} | grep -o "x86_64\|amd64")", "")
	TARGET_ARCH=x86_64
endif

# OS
ifneq ("$(shell echo ${TARGET_MACHINE} | grep -o "darwin")", "")
	TARGET_PLATFORM=mac
	ifeq "${TARGET_ARCH}" "x86"
		ASM_OBJ_FORMAT=macho32
	endif
	ifeq "${TARGET_ARCH}" "x86_64"
		ASM_OBJ_FORMAT=macho64
	endif
endif

ifneq ("$(shell echo ${TARGET_MACHINE} | grep -o "mingw")", "")
	TARGET_PLATFORM=windows
endif

ifneq ("$(shell echo ${TARGET_MACHINE} | grep -o "cygwin")", "")
	TARGET_PLATFORM=cygwin
endif

ifneq ("$(shell echo ${TARGET_MACHINE} | grep -o "linux")", "")
	TARGET_PLATFORM=linux
	ifeq "${TARGET_ARCH}" "x86"
		ASM_OBJ_FORMAT=elf32
	endif
	ifeq "${TARGET_ARCH}" "x86_64"
		ASM_OBJ_FORMAT=elf64
	endif
endif

## Sets prefixes and suffixes according to target determined above
# Default to Unix-like if platform-specific checks below don't pass
DYNAMIC_LIBOUTPREFIX=lib
DYNAMIC_LIBOUTSUFFIX=.so
STATIC_LIBOUTPREFIX=lib
STATIC_LIBOUTSUFFIX=.a
EXEOUTSUFFIX=

# Mac naming
ifeq "${TARGET_PLATFORM}" "mac"
	DYNAMIC_LIBOUTPREFIX=lib
	DYNAMIC_LIBOUTSUFFIX=.dylib
	STATIC_LIBOUTPREFIX=lib
	STATIC_LIBOUTSUFFIX=.a
	EXEOUTSUFFIX=
endif

# Cygwin naming
ifeq "${TARGET_PLATFORM}" "cygwin"
	DYNAMIC_LIBOUTPREFIX=cyg
	DYNAMIC_LIBOUTSUFFIX=.dll
	STATIC_LIBOUTPREFIX=
	STATIC_LIBOUTSUFFIX=.lib
	EXEOUTSUFFIX=.exe
endif

# Windows naming
ifeq "${TARGET_PLATFORM}" "windows"
	DYNAMIC_LIBOUTPREFIX=
	DYNAMIC_LIBOUTSUFFIX=.dll
	STATIC_LIBOUTPREFIX=
	STATIC_LIBOUTSUFFIX=.lib
	EXEOUTSUFFIX=.exe
endif



### General compiler options

## Base flags
# BASE_*FLAGS are the flags to compilers/assemblers that are used
# consistently throughout the entire project. Individual modules can
# specify their own additions in their Makefile.

# Flags for *both* C and C++ compilers
BASE_C_CXXFLAGS=-fPIC

# Flags for C compilers, specifically
BASE_CFLAGS=

# Flags for C++ compilers, specifically
BASE_CXXFLAGS=

# Flags for assembler (currently nasm)
BASE_ASFLAGS=-Ox -D__GNUC__ -f ${ASM_OBJ_FORMAT}

# Flags for linking
BASE_LIBS=

## Modifiers to base flags

ifdef NOGUI
BASE_C_CXXFLAGS+=-DTC_NO_GUI
endif

ifdef WARNINGS
BASE_C_CXXFLAGS+=-Wall
endif

ifdef DEBUG
BASE_C_CXXFLAGS+=-DDEBUG -ggdb
else
BASE_C_CXXFLAGS+=-O2 -fno-strict-aliasing
endif

# Include relevant directories for generated embedded file headers
embed_tmp_inc=-I$(shell dirname "$@") -I${BUILD_TMP}
BASE_C_CXXFLAGS+=${embed_tmp_inc}
BASE_ASFLAGS+=${embed_tmp_inc}

# Flags for generating autodepedencies
BASE_C_CXXFLAGS+=-MMD -MP -MF ${@:%.o=%.d}
BASE_ASFLAGS+=-MP -MD ${@:%.o=%.d}



### Set C/C++ defines depending on target platform

ifeq "${TARGET_PLATFORM}" "unix"
	BASE_C_CXXFLAGS+=-DTC_UNIX
endif
ifeq "${TARGET_PLATFORM}" "linux"
	BASE_C_CXXFLAGS+=-DTC_UNIX -DTC_LINUX
endif

ifeq "${TARGET_ARCH}" "x86"
	BASE_C_CXXFLAGS+=-DTC_ARCH_X86
endif
ifeq "${TARGET_ARCH}" "x86_64"
	BASE_C_CXXFLAGS+=-DTC_ARCH_X64
endif

# Statically link libgcc and libstdc++ when building with mingw; this
# just makes executables more easily portable because we don't have to
# package these .dlls, but this makes our executables and libraries a
# lot bigger and have a lot of duplicate code.
ifeq "${TARGET_PLATFORM}" "windows"
	BASE_LIBS+=-static-libgcc -static-libstdc++
endif



### Output dirs
# Default prefix name for build output files
BUILD_PREFIX=./obj-${TARGET_MACHINE}

# Intermediary files used to generate other files (such as .o files)
BUILD_TMP=${BUILD_PREFIX}/tmp

# End libraries we generate, either for binaries, or standalone
LIB=${BUILD_PREFIX}/lib

# End binaries
BIN=${BUILD_PREFIX}/bin



### Targets

# Make sure clean and all is never skipped
.PHONY: clean all

# Default target
# This is defined here, even with nothing in it, simply because clean would be
# the default target for Makefiles that include this file.
all:

# Template for clean
clean::
	-[ -e "${BUILD_TMP}" ] && find "${BUILD_TMP}" -mindepth 1 -maxdepth 1 | while read file ; do rm -rf "$${file}" ; done
	-[ -e "${LIB}" ] && find "${LIB}" -mindepth 1 -maxdepth 1 | while read file ; do rm -rf "$${file}" ; done
	-[ -e "${BIN}" ] && find "${BIN}" -mindepth 1 -maxdepth 1 | while read file ; do rm -rf "$${file}" ; done

## Common intermediary targets

# Compiled/assembled code-modules (and autodependencies)
${BUILD_TMP}/%.o: %.c
	@mkdir -p "$$(dirname $@)"
	@echo "CC	$@"
	$(CC) ${C_CXXFLAGS} ${CFLAGS} -o $@ -c $<

${BUILD_TMP}/%.o: %.cpp
	@mkdir -p "$$(dirname $@)"
	@echo "C++	$@"
	$(CXX) ${C_CXXFLAGS} ${CXXFLAGS} -o $@ -c $<

${BUILD_TMP}/%.o: %.asm
	@mkdir -p "$$(dirname $@)"
	@echo "NASM	$@"
	$(AS) ${ASFLAGS} -o $@ $<

# Embedded files
${BUILD_TMP}/%.h: %
	@mkdir -p "$$(dirname $@)"
	@echo "OD	$@"
	od -v -t u1 -A n $< | tr '\n' ' ' | tr -s ' ' ',' | sed -e 's/^,//g' -e 's/,$$/n/' | tr 'n' '\n' > $@

# Strip definitions for targets below
ifndef NOSTRIP
define STRIP
	@echo "STRIP	$@"
	strip $@
endef
define STRIP_AR
	@echo "STRIP	$@"
	strip --strip-unneeded $@
endef
else
STRIP=
STRIP_AR=
endif

# Static libraries
%.a %.lib: ${OBJS}
	@mkdir -p "$$(dirname $@)"
	@echo "AR	$@"
	ar rcs $@ $^
	$(STRIP_AR)

# Dynamic libraries (canned recipe)
override define DYNLIB_RECIPE
	@mkdir -p "$$(dirname $@)"
	@echo "LINK	$@"
	${CXX} ${C_CXXFLAGS} ${CXXFLAGS} -o $@ -shared ${OBJS} ${LIBS}
	$(STRIP)
endef

# Executables (canned recipe)
override define EXE_RECIPE
	@mkdir -p "$$(dirname $@)"
	@echo "LINK	$@"
	${CXX} ${C_CXXFLAGS} ${CXXFLAGS} -o $@ ${OBJS} ${LIBS}
	$(STRIP)
endef

### Include generated auto-dependencies
-include $(shell [ -e "${BUILD_TMP}" ] && find "${BUILD_TMP}" -type f -name '*.d')
