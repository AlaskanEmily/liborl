/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#ifndef ELF_TYPE_INCLUDED
#define ELF_TYPE_INCLUDED

#include "elfspec.h"

TYPEDEF_LOCAL_TYPE( elf_handle );
TYPEDEF_LOCAL_TYPE( elf_file_handle );
TYPEDEF_LOCAL_TYPE( elf_sec_handle );
TYPEDEF_LOCAL_TYPE( elf_symbol_handle );

/* handle definitions */

typedef uint8_t elf_file_flags;
typedef uint32_t elf_file_index;
typedef uint64_t elf_file_offset;
typedef uint64_t elf_file_size;
typedef uint32_t elf_headers_size;

typedef uint32_t elf_sec_flags;
typedef uint64_t elf_sec_base;
typedef uint64_t elf_sec_size;
typedef uint32_t elf_sec_offset;
typedef int64_t elf_sec_addend;

typedef uint64_t elf_symbol_value;

typedef uint16_t elf_reloc_type;

typedef uint32_t elf_quantity;

typedef uint16_t elf_half;
typedef uint32_t elf_word;
typedef elf_word elf_index;

struct elf_handle_struct {
    struct orl_funcs *funcs;
    elf_file_handle first_file_hnd;
};

struct elf_file_handle_struct {
    elf_handle elf_hnd;
    elf_file_handle next;
    elf_sec_handle *sec_handles;
    elf_sec_handle *orig_sec_handles;
    orl_file_id file;
    uint8_t *contents_buffer1;
    elf_half shentsize;
    orl_machine_type machine_type;
    orl_file_type type;
    orl_file_size size;
    orl_file_flags flags;
    elf_quantity num_sections;
    elf_sec_handle symbol_table;
    orl_hash_table sec_name_hash_table;
};

#define SEC_NAME_HASH_TABLE_SIZE 53

struct elf_normal_assoc_struct {
    elf_sec_handle reloc_sec;
    char unused[8];
};

struct elf_reloc_assoc_struct {
    elf_sec_handle orig_sec;
    elf_sec_handle symbol_table;
    orl_reloc relocs;
};

struct elf_sym_assoc_struct {
    elf_sec_handle string_table;
    elf_symbol_handle symbols;
    char unused[4];
};

struct elf_import_assoc_struct {
    elf_sec_handle string_table;
    char unused[8];
};

struct elf_export_assoc_struct {
    elf_sec_handle symbol_table;
    char unused[8];
};

struct elf_sec_handle_struct {
    orl_file_format file_format;
    elf_file_handle elf_file_hnd;
    elf_sec_handle next;
    char *name;
    elf_sec_size size;
    elf_file_offset file_offset;
    orl_sec_type type;
    orl_sec_flags flags;
    orl_sec_alignment alignment;
    uint8_t *contents;
    elf_sec_base base;
    orl_table_index index;
    /* elf_quantity        index; */
    elf_sec_size entsize;
    /* assoc - things associated with the section */
    union {
        struct elf_normal_assoc_struct  normal;
        struct elf_reloc_assoc_struct   reloc;
        struct elf_sym_assoc_struct     sym;
        struct elf_import_assoc_struct  import;
        struct elf_export_assoc_struct  export;
    } assoc;
};

struct elf_symbol_handle_struct {
    orl_file_format file_format;
    elf_file_handle elf_file_hnd;
    orl_symbol_binding binding;
    orl_symbol_type type;
    char *name;
    elf_symbol_value value;
    elf_half shndx;
    uint8_t info;
};

#endif
