#ifndef UPGRADE_PACK_BUILDER_LIBRARY_ELF_HPP
#define UPGRADE_PACK_BUILDER_LIBRARY_ELF_HPP

#include <cstdint>

struct elf_header_t
{
    struct
    {
        uint32_t magic;  /* should be 0x7f 'E' 'L' 'F' */
        uint8_t  elfclass;
        uint8_t  data;
        uint8_t  version;
        uint8_t  reserved[9];
    } ident;
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t program_header_offset; /* from start of file */
    uint32_t section_header_offset; /* from start of file */
    uint32_t flags;
    uint16_t elf_header_size;
    uint16_t program_header_entry_size;
    uint16_t program_header_entry_count;
    uint16_t section_header_entry_size;
    uint16_t section_header_entry_count;
    uint16_t string_table_index; /* index in the section header table */
};

struct elf_program_header_t
{
    uint32_t type;
    uint32_t data_offset;
    uint32_t virtual_address;
    uint32_t physical_address;
    uint32_t data_size_in_file;
    uint32_t data_size_in_memory;
    uint32_t flags;
    uint32_t alignment;
};

struct elf_section_header_t
{
	uint32_t name;
	uint32_t type;
	uint32_t flags;
	uint32_t virtual_address;
	uint32_t data_offset;
	uint32_t data_size_in_file;
	uint32_t link;
	uint32_t info;
	uint32_t allingment;
	uint32_t entry_size;
};


#endif
