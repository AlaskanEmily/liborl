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


#ifndef COFF_ORL_INCLUDED
#define COFF_ORL_INCLUDED
#pragma once

#include "cofftype.h"

coff_handle CoffInit( struct orl_funcs * );
orl_return CoffFini( coff_handle );

orl_return CoffFileInit( coff_handle, orl_file_id, coff_file_handle * );
orl_return CoffFileFini( coff_file_handle );
orl_return CoffFileScan( coff_file_handle, const char *, orl_sec_return_func );
orl_machine_type CoffFileGetMachineType( coff_file_handle );
orl_file_flags CoffFileGetFlags( coff_file_handle );
orl_file_type CoffFileGetType( coff_file_handle );
orl_file_size CoffFileGetSize( coff_file_handle );
coff_sec_handle CoffFileGetSymbolTable( coff_file_handle );

const char *CoffSecGetName( coff_sec_handle );
coff_sec_base CoffSecGetBase( coff_sec_handle );
coff_sec_size CoffSecGetSize( coff_sec_handle );
orl_sec_type CoffSecGetType( coff_sec_handle );
orl_sec_flags CoffSecGetFlags( coff_sec_handle );
orl_sec_alignment CoffSecGetAlignment( coff_sec_handle );
coff_sec_handle CoffSecGetStringTable( coff_sec_handle );
coff_sec_handle CoffSecGetSymbolTable( coff_sec_handle );
coff_sec_handle CoffSecGetRelocTable( coff_sec_handle );
orl_linnum CoffSecGetLines( coff_sec_handle );
orl_table_index CoffSecGetNumLines( coff_sec_handle );
coff_sec_offset CoffSecGetOffset( coff_sec_handle );
orl_return CoffSecGetContents( coff_sec_handle, unsigned char ** );
orl_return CoffSecQueryReloc( coff_sec_handle, coff_sec_offset, orl_reloc_return_func );
orl_return CoffSecScanReloc( coff_sec_handle, orl_reloc_return_func );

orl_table_index CoffCvtSecHdlToIdx( coff_sec_handle );
coff_sec_handle CoffCvtIdxToSecHdl( coff_file_handle, orl_table_index );

orl_return CoffRelocSecScan( coff_sec_handle, orl_reloc_return_func );
orl_return CoffSymbolSecScan( coff_sec_handle, orl_symbol_return_func );
orl_return CoffNoteSecScan( coff_sec_handle, struct orl_note_callbacks *, void * );

const char *CoffSymbolGetName( coff_symbol_handle );
coff_symbol_value CoffSymbolGetValue( coff_symbol_handle );
orl_symbol_binding CoffSymbolGetBinding( coff_symbol_handle );
orl_symbol_type CoffSymbolGetType( coff_symbol_handle );
coff_sec_handle CoffSymbolGetSecHandle( coff_symbol_handle );
coff_symbol_handle CoffSymbolGetAssociated( coff_symbol_handle );

coff_rva CoffExportTableRVA( coff_file_handle );

#endif
