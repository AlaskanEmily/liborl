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


#ifndef OMF_ORL_INCLUDED
#define OMF_ORL_INCLUDED
#pragma once

#include "omftype.h"
#include "pcobj.h"

omf_handle OmfInit( struct orl_funcs * );
orl_return OmfFini( omf_handle );

orl_return OmfFileInit( omf_handle, orl_file_id, omf_file_handle * );
orl_return OmfFileFini( omf_file_handle );
orl_return OmfFileScan( omf_file_handle, const char *, orl_sec_return_func );
orl_machine_type OmfFileGetMachineType( omf_file_handle );
orl_file_flags OmfFileGetFlags( omf_file_handle );
orl_file_type OmfFileGetType( omf_file_handle );
orl_file_size OmfFileGetSize( omf_file_handle );
omf_sec_handle OmfFileGetSymbolTable( omf_file_handle );

const char *OmfSecGetName( omf_sec_handle );
omf_sec_base OmfSecGetBase( omf_sec_handle );
omf_sec_size OmfSecGetSize( omf_sec_handle );
orl_sec_type OmfSecGetType( omf_sec_handle );
orl_sec_flags OmfSecGetFlags( omf_sec_handle );
orl_sec_alignment OmfSecGetAlignment( omf_sec_handle );
omf_sec_handle OmfSecGetStringTable( omf_sec_handle );
omf_sec_handle OmfSecGetSymbolTable( omf_sec_handle );
omf_sec_handle OmfSecGetRelocTable( omf_sec_handle );
orl_return OmfSecGetContents( omf_sec_handle, unsigned char ** );
orl_return OmfSecQueryReloc( omf_sec_handle, omf_sec_offset, orl_reloc_return_func );
orl_return OmfSecScanReloc( omf_sec_handle, orl_reloc_return_func );
const char *OmfSecGetClassName( omf_sec_handle );
orl_sec_combine OmfSecGetCombine( omf_sec_handle );
orl_sec_frame OmfSecGetAbsFrame( omf_sec_handle );
orl_sec_handle OmfSecGetAssociated( omf_sec_handle );
orl_group_handle OmfSecGetGroup( omf_sec_handle );
orl_table_index OmfSecGetNumLines( omf_sec_handle );
orl_linnum OmfSecGetLines( omf_sec_handle );

orl_table_index OmfCvtSecHdlToIdx( omf_sec_handle );
omf_sec_handle OmfCvtIdxToSecHdl( omf_file_handle, orl_table_index );

orl_return OmfRelocSecScan( omf_sec_handle, orl_reloc_return_func );
orl_return OmfSymbolSecScan( omf_sec_handle, orl_symbol_return_func );

const char *OmfSymbolGetName( omf_symbol_handle );
omf_symbol_value OmfSymbolGetValue( omf_symbol_handle );
orl_symbol_binding OmfSymbolGetBinding( omf_symbol_handle );
orl_symbol_type OmfSymbolGetType( omf_symbol_handle );
unsigned char OmfSymbolGetRawInfo( omf_symbol_handle );
omf_sec_handle OmfSymbolGetSecHandle( omf_symbol_handle );

orl_return OmfNoteSecScan( omf_sec_handle, struct orl_note_callbacks *, void * );

orl_return OmfGroupsScan( omf_file_handle, orl_group_return_func );
orl_table_index OmfGroupSize( omf_grp_handle );
const char *OmfGroupName( omf_grp_handle );
const char *OmfGroupMember( omf_grp_handle, orl_table_index );

#endif
