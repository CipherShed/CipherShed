CPPFLAGS += -I${subdir}/include

lib := ${libdir}/lib.a

${libdir}/lib.a: ${OBJS}
	$(recipe_archive)

-include ${MDEPS}
