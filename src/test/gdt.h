/*
 * gdt.h - helper functions for GDT entries
 *
 * These functions just do the math. For field definitions,
 * see: Intel SDM vol. 3 3.4.5 Segment Descriptors
 */

// Simply entry data
#define GDT_ENTRY(base, segment_type, descriptor_type, privilege_level, present, limit, avl, long, opsize, granularity) \
	.short limit; \
	.short base; \
	.byte base >> 16; \
	.byte (segment_type & 0x0f) | ((descriptor_type & 0x01) << 4) | ((privilege_level & 0x03) << 5) | ((present & 0x01) << 7); \
	.byte ((limit >> 16) & 0x0f) | ((avl & 0x01) << 4) | ((long & 0x01) << 5) | ((opsize & 0x01) << 6) | ((granularity & 0x01) << 7); \
	.byte base >> 24

// Entry data, and increment of the given variable (automated
// counting of entries)
#define GDT_ADD_ENTRY(base, segment_type, descriptor_type, privilege_level, present, limit, avl, long, opsize, granularity, entry_count_var) \
	GDT_ENTRY(base, segment_type, descriptor_type, privilege_level, present, limit, avl, long, opsize, granularity); \
	entry_count_var = entry_count_var + 1

// Calculate limit for GDTR
#define GDT_LIMIT(entry_count) (8*entry_count)-1

// Calculate segment selector
#define GDT_SELECT_ENTRY(entry) \
	(entry << 3)
