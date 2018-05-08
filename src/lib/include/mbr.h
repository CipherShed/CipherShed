#include <stdint.h>

typedef struct
{
	uint8_t status;
	uint8_t start_chs_h;
	uint16_t start_chs_cs;
	uint8_t type;
	uint16_t end_chs_h;
	uint8_t end_chs_cs;
	uint32_t start_lba;
	uint32_t length_lba;
} __attribute__((packed)) partition_table_entry;

extern partition_table_entry partition_table_entries[];

#define partition_active 0x80
