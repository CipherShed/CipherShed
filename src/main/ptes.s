// TODO: Better solution?
//
// See: ld info PHDRS:
//    The linker will normally set the segment flags based on the sections
// which comprise the segment.  You may use the 'FLAGS' keyword to
// explicitly specify the segment flags.  The value of FLAGS must be an
// integer.  It is used to set the 'p_flags' field of the program header.
//
// Are the flag values generic or specific to the executable format
// being linked?
.section .ptes
