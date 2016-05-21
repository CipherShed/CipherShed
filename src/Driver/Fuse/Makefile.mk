# Sets some output file names for less repetition
DYNAMIC_FUSEDRV=${LIB}/${DYNAMIC_LIBOUTPREFIX}fusedrv${DYNAMIC_LIBOUTSUFFIX}
STATIC_FUSEDRV=${LIB}/${STATIC_LIBOUTPREFIX}fusedrv${STATIC_LIBOUTSUFFIX}

# Sets compiler flags
${DYNAMIC_FUSEDRV} ${STATIC_FUSEDRV}: C_CXXFLAGS=${BASE_C_CXXFLAGS} -IDriver/Fuse $(shell pkg-config fuse --cflags)
${DYNAMIC_FUSEDRV} ${STATIC_FUSEDRV}: CFLAGS=${BASE_CFLAGS}
${DYNAMIC_FUSEDRV} ${STATIC_FUSEDRV}: CXXFLAGS=${BASE_CXXFLAGS}
${DYNAMIC_FUSEDRV} ${STATIC_FUSEDRV}: ASFLAGS=${BASE_ASFLAGS}
${DYNAMIC_FUSEDRV} ${STATIC_FUSEDRV}: LIBS=${BASE_LIBS} $(shell pkg-config fuse --libs)

# Generate list of needed object files from list of source files
FUSEDRV_OBJS=$(shell find Driver/Fuse -mindepth 1 -maxdepth 1 -type f | sed -ne '/\.c$$/{s/\.c$$/.o/;p;};/\.cpp$$/{s/\.cpp$$/.o/;p;}' | sed -e 's:.*:${BUILD_TMP}/&:')
${DYNAMIC_FUSEDRV} ${STATIC_FUSEDRV}: OBJS=${FUSEDRV_OBJS}

# Dynamic library target
${DYNAMIC_FUSEDRV}: $${OBJS}
	$(DYNLIB_RECIPE)

# Static library target
${STATIC_FUSEDRV}: $${OBJS}
# (Static libs handled by common.mk)
