# Note: all sed calls could be a lot simpler, but try to be compatible with
# OS X's sed (BSD sed).

# Include common makefile
# This defines common things such as flags, %.o, clean, artifact naming sceme,
# and so on.
include Build/Include/common.mk

# Include Makefiles for library components
include Driver/Fuse/Makefile.mk
include Platform/Makefile.mk
include Volume/Makefile.mk
include Core/Makefile.mk

# Include Makefile for main executable
include Main/Makefile.mk

# Assume static unless specified otherwise.
ifdef DYNAMIC_COMPONENTS
FUSEDRV=${DYNAMIC_FUSEDRV}
PLATFORM=${DYNAMIC_PLATFORM}
VOLUME=${DYNAMIC_VOLUME}
CORE=${DYNAMIC_CORE}
else
FUSEDRV=${STATIC_FUSEDRV}
PLATFORM=${STATIC_PLATFORM}
VOLUME=${STATIC_VOLUME}
CORE=${STATIC_CORE}
endif

# "all" (default target) makes the main executable
all: ${MAIN}
${MAIN}: ${FUSEDRV} ${PLATFORM} ${VOLUME} ${CORE}
