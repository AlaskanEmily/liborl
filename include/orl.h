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
* Description:  Object Reader Library public interface.
*
****************************************************************************/


#ifndef ORL_H
#define ORL_H

#include <stddef.h>
#include "orlglobl.h"
#include "orlcomon.h"

struct orl_funcs;

orl_handle ORLInit( struct orl_funcs *funcs );
orl_return ORLGetError( orl_handle );
orl_return ORLFini( orl_handle );

orl_file_format ORLFileIdentify( orl_handle, orl_file_id );
orl_file_handle ORLFileInit( orl_handle, orl_file_id, orl_file_format );
orl_return ORLFileFini( orl_file_handle );

/**
 * @brief Scans for the specified section, calling the return func on the section handle
 *
 * If NULL is provided as the section name, the return function will be called on all sections.
 * If the return function returns other than ORL_OKAY, iteration will stop.
 */
orl_return ORLFileScan( orl_file_handle, const char *, orl_sec_return_func );
orl_machine_type ORLFileGetMachineType( orl_file_handle );
orl_file_flags ORLFileGetFlags( orl_file_handle );
orl_file_type ORLFileGetType( orl_file_handle );
orl_file_format ORLFileGetFormat( orl_file_handle );
orl_file_size ORLFileGetSize( orl_file_handle );
orl_sec_handle ORLFileGetSymbolTable( orl_file_handle );

const char *ORLSecGetName( orl_sec_handle );
orl_return ORLSecGetBase( orl_sec_handle, orl_sec_base * );
orl_sec_size ORLSecGetSize( orl_sec_handle );
orl_sec_type ORLSecGetType( orl_sec_handle );
orl_sec_flags ORLSecGetFlags( orl_sec_handle );
orl_sec_alignment ORLSecGetAlignment( orl_sec_handle );
orl_sec_handle ORLSecGetStringTable( orl_sec_handle );
orl_sec_handle ORLSecGetSymbolTable( orl_sec_handle );
orl_sec_handle ORLSecGetRelocTable( orl_sec_handle );
orl_linnum ORLSecGetLines( orl_sec_handle );
orl_table_index ORLSecGetNumLines( orl_sec_handle );
orl_sec_offset ORLSecGetOffset( orl_sec_handle );
orl_return ORLSecGetContents( orl_sec_handle, unsigned char ** );
orl_return ORLSecQueryReloc( orl_sec_handle, orl_sec_offset, orl_reloc_return_func );
orl_return ORLSecScanReloc( orl_sec_handle, orl_reloc_return_func );
orl_table_index ORLCvtSecHdlToIdx( orl_sec_handle );
orl_sec_handle ORLCvtIdxToSecHdl( orl_file_handle, orl_table_index );

const char *ORLSecGetClassName( orl_sec_handle );
orl_sec_combine ORLSecGetCombine( orl_sec_handle );
orl_sec_frame ORLSecGetAbsFrame( orl_sec_handle );
orl_sec_handle ORLSecGetAssociated( orl_sec_handle );
orl_group_handle ORLSecGetGroup( orl_sec_handle );

orl_return ORLRelocSecScan( orl_sec_handle, orl_reloc_return_func );
orl_return ORLSymbolSecScan( orl_sec_handle, orl_symbol_return_func );
orl_return ORLNoteSecScan( orl_sec_handle, struct orl_note_callbacks *, void * );

const char *ORLSymbolGetName( orl_symbol_handle );
orl_return ORLSymbolGetValue( orl_symbol_handle, orl_symbol_value * );
orl_symbol_binding ORLSymbolGetBinding( orl_symbol_handle );
orl_symbol_type ORLSymbolGetType( orl_symbol_handle );
unsigned char ORLSymbolGetRawInfo( orl_symbol_handle );
orl_sec_handle ORLSymbolGetSecHandle( orl_symbol_handle );
orl_symbol_handle ORLSymbolGetAssociated( orl_symbol_handle );

orl_return ORLGroupsScan( orl_file_handle, orl_group_return_func );
const char *ORLGroupName( orl_group_handle );
orl_table_index ORLGroupSize( orl_group_handle );
const char *ORLGroupMember( orl_group_handle, orl_table_index );
orl_rva ORLExportTableRVA( orl_file_handle );

#endif
