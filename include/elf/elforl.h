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


#ifndef ELF_ORL_INCLUDED
#define ELF_ORL_INCLUDED

#include "elftype.h"

elf_handle ElfInit( struct orl_funcs * );
orl_return ElfFini( elf_handle );

orl_return ElfFileInit( elf_handle, orl_file_id, elf_file_handle * );
orl_return ElfFileFini( elf_file_handle );
orl_return ElfFileScan( elf_file_handle, const char *, orl_sec_return_func, void *cookie );
orl_machine_type ElfFileGetMachineType( elf_file_handle );
orl_file_flags ElfFileGetFlags( elf_file_handle );
orl_file_type ElfFileGetType( elf_file_handle );
orl_file_size ElfFileGetSize( elf_file_handle );
elf_sec_handle ElfFileGetSymbolTable( elf_file_handle );

const char * ElfSecGetName( elf_sec_handle );
orl_return ElfSecGetBase( elf_sec_handle, orl_sec_base * );
orl_sec_size ElfSecGetSize( elf_sec_handle );
orl_sec_type ElfSecGetType( elf_sec_handle );
orl_sec_flags ElfSecGetFlags( elf_sec_handle );
orl_sec_alignment ElfSecGetAlignment( elf_sec_handle );
elf_sec_handle ElfSecGetStringTable( elf_sec_handle );
elf_sec_handle ElfSecGetSymbolTable( elf_sec_handle );
elf_sec_handle ElfSecGetRelocTable( elf_sec_handle );
orl_return ElfSecGetContents( elf_sec_handle, unsigned char ** );
orl_return ElfSecQueryReloc( elf_sec_handle, elf_sec_offset, orl_reloc_return_func, void *cookie );
orl_return ElfSecScanReloc( elf_sec_handle, orl_reloc_return_func, void *cookie );

orl_table_index ElfCvtSecHdlToIdx( elf_sec_handle );
elf_sec_handle ElfCvtIdxToSecHdl( elf_file_handle, orl_table_index );

orl_return ElfRelocSecScan( elf_sec_handle, orl_reloc_return_func, void *cookie );
orl_return ElfSymbolSecScan( elf_sec_handle, orl_symbol_return_func, void *cookie );
orl_return ElfNoteSecScan( elf_sec_handle, struct orl_note_callbacks *, void * );

const char *ElfSymbolGetName( elf_symbol_handle );
orl_return ElfSymbolGetValue( elf_symbol_handle, orl_symbol_value * );
orl_symbol_binding ElfSymbolGetBinding( elf_symbol_handle );
orl_symbol_type ElfSymbolGetType( elf_symbol_handle );
unsigned char ElfSymbolGetRawInfo( elf_symbol_handle );
elf_sec_handle ElfSymbolGetSecHandle( elf_symbol_handle );
elf_symbol_handle ElfSymbolGetAssociated( elf_symbol_handle elf_symbol_hnd );

#endif
