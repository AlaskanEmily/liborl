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


#include "coffentr.h"
#include "coffflhn.h"
#include "cofflwlv.h"
#include "coffload.h"
#include "cofforl.h"
#include "orlhash.h"

coff_handle CoffInit( struct orl_funcs *funcs )
{
    const coff_handle coff_hnd = malloc(sizeof( struct coff_handle_struct ) );
    coff_hnd->funcs = funcs;
    coff_hnd->first_file_hnd = NULL;
    return coff_hnd;
}

orl_return CoffFini( coff_handle coff_hnd )
{
    while( coff_hnd->first_file_hnd != NULL ) {
        const orl_return return_val = CoffRemoveFileLinks( coff_hnd->first_file_hnd );
        if( return_val != ORL_OKAY )
            return return_val;
    }
    free(coff_hnd);
    return ORL_OKAY;
}

orl_return CoffFileInit( coff_handle coff_hnd, orl_file_id file, coff_file_handle *pcfh )
{
    const size_t file_handle_size = sizeof(struct coff_file_handle_struct);
    const coff_file_handle coff_file_hnd = (coff_file_handle)ORL_PTR_ALLOC( coff_hnd, file_handle_size );
    memset( coff_file_hnd, 0, file_handle_size );
    coff_file_hnd->file = file;
    CoffAddFileLinks( coff_hnd, coff_file_hnd );
    
    {
        const orl_return return_val = CoffLoadFileStructure( coff_file_hnd );
        if( return_val != ORL_OKAY ) {
            CoffRemoveFileLinks( coff_file_hnd );
            pcfh[0] = NULL;
            return return_val;
        }
    }
    
    pcfh[0] = coff_file_hnd;
    return ORL_OKAY;
}

orl_return CoffFileFini( coff_file_handle coff_file_hnd )
{
    return( CoffRemoveFileLinks( coff_file_hnd ) );
}

orl_return CoffFileScan( coff_file_handle coff_file_hnd, const char *desired, orl_sec_return_func return_func )
{
    if( desired == NULL ) {
        /* global request */
        coff_quantity i;
        for( i = 0; i < coff_file_hnd->num_sections; i++ ) {
            const orl_return return_val = return_func( (orl_sec_handle)coff_file_hnd->coff_sec_hnd[i] );
            if( return_val != ORL_OKAY )
                return return_val;
        }
    }
    else {
        orl_hash_data_entry data_entry;
        struct orl_hash_key h_key;
        if( coff_file_hnd->sec_name_hash_table == NULL ) {
            const orl_return return_val = CoffBuildSecNameHashTable( coff_file_hnd );
            if( return_val != ORL_OKAY )
                return return_val;
        }
        h_key.u.string = desired;
        for( data_entry = ORLHashTableQuery( coff_file_hnd->sec_name_hash_table, h_key ); data_entry != NULL; data_entry = data_entry->next ) {
            const orl_return return_val = return_func( data_entry->data.u.sec_handle );
            if( return_val != ORL_OKAY )
                return return_val;
        }
    }
    return ORL_OKAY;
}

coff_rva CoffExportTableRVA( coff_file_handle coff_file_hnd )
{
    return coff_file_hnd->export_table_rva;
}

orl_machine_type CoffFileGetMachineType( coff_file_handle coff_file_hnd )
{
    return coff_file_hnd->machine_type;
}

orl_file_flags CoffFileGetFlags( coff_file_handle coff_file_hnd )
{
    return coff_file_hnd->flags;
}

orl_file_type CoffFileGetType( coff_file_handle coff_file_hnd )
{
    return coff_file_hnd->type;
}

orl_file_size CoffFileGetSize( coff_file_handle coff_file_hnd )
{
    return coff_file_hnd->size;
}

coff_sec_handle CoffFileGetSymbolTable( coff_file_handle coff_file_hnd )
{
    return coff_file_hnd->symbol_table;
}

coff_sec_handle CoffSecGetStringTable( coff_sec_handle coff_sec_hnd )
{
    return coff_sec_hnd->coff_file_hnd->string_table;
}

coff_sec_handle CoffSecGetSymbolTable( coff_sec_handle coff_sec_hnd )
{
    return coff_sec_hnd->coff_file_hnd->symbol_table;
}

const char * CoffSecGetName( coff_sec_handle coff_sec_hnd )
{
    return coff_sec_hnd->name;
}

coff_sec_base CoffSecGetBase( coff_sec_handle coff_sec_hnd )
{
    return coff_sec_hnd->base;
}

coff_sec_size CoffSecGetSize( coff_sec_handle coff_sec_hnd )
{
    return coff_sec_hnd->size;
}

orl_sec_type CoffSecGetType( coff_sec_handle coff_sec_hnd )
{
    return coff_sec_hnd->type;
}

orl_sec_flags CoffSecGetFlags( coff_sec_handle coff_sec_hnd )
{
    return coff_sec_hnd->flags;
}

orl_sec_alignment CoffSecGetAlignment( coff_sec_handle coff_sec_hnd )
{
    return coff_sec_hnd->align;
}

coff_sec_handle CoffSecGetRelocTable( coff_sec_handle coff_sec_hnd )
{
    if( coff_sec_hnd->type != ORL_SEC_TYPE_RELOCS )
        return coff_sec_hnd->assoc.normal.reloc_sec;
    else
        return NULL;
}

orl_table_index CoffSecGetNumLines( coff_sec_handle coff_sec_hnd )
{
    if( coff_sec_hnd->hdr != NULL )
        return coff_sec_hnd->hdr->num_lineno;
    else
        return 0;
}

orl_linnum CoffSecGetLines( coff_sec_handle coff_sec_hnd )
{
    const orl_table_index numlines = CoffSecGetNumLines( coff_sec_hnd );
    if( numlines > 0 )
        return CoffConvertLines( coff_sec_hnd, numlines );
    else
        return NULL;
}

coff_sec_offset CoffSecGetOffset( coff_sec_handle coff_sec_hnd )
{
    if( coff_sec_hnd->hdr != NULL )
        return coff_sec_hnd->hdr->offset;
    else
        return 0;
}

orl_return CoffSecGetContents( coff_sec_handle coff_sec_hnd, unsigned char **buffer )
{
    if( coff_sec_hnd->contents != NULL ) {
        buffer[0] = coff_sec_hnd->contents;
        return ORL_OKAY;
    }
    else{
        return ORL_ERROR;
    }
}

orl_return CoffSecQueryReloc( coff_sec_handle coff_sec_hnd, coff_sec_offset sec_offset, orl_reloc_return_func return_func )
{
    coff_sec_handle reloc_sec_hnd;

    if( coff_sec_hnd->type != ORL_SEC_TYPE_PROG_BITS )
        return ORL_ERROR;
    if( (reloc_sec_hnd = coff_sec_hnd->assoc.normal.reloc_sec) == NULL )
        return ORL_FALSE;
    if( reloc_sec_hnd->assoc.reloc.relocs == NULL ) {
        const orl_return return_val = CoffCreateRelocs( coff_sec_hnd, reloc_sec_hnd );
        if( return_val != ORL_OKAY )
            return return_val;
    }
    
    {
        unsigned index;
        const unsigned limit = reloc_sec_hnd->assoc.reloc.num_relocs;
        const orl_reloc reloc = reloc_sec_hnd->assoc.reloc.relocs;
        
        if(limit == 0)
            return ORL_ERROR;
        
        for( index = 0; index < limit; index++ ) {
            if( reloc->offset == sec_offset ) {
                const orl_return return_val = return_func( reloc + index );
                if( return_val != ORL_OKAY )
                    return return_val;
            }
        }
    }
    
    return ORL_OKAY;
}

orl_table_index CoffCvtSecHdlToIdx( coff_sec_handle shdl )
/******************************************************************/
/* NYI: would be nice to have a O(1) way of doing this. */
{
    coff_quantity index = 0;
    const coff_file_handle fhdl = shdl->coff_file_hnd;
    const coff_quantity limit = fhdl->f_hdr_buffer->num_sections;
    while( index < limit ) {
        if( fhdl->orig_sec_hnd[index++] == shdl )
            return index;
    }
    return 0;
}

coff_sec_handle CoffCvtIdxToSecHdl( coff_file_handle fhdl, orl_table_index idx )
/**********************************************************************************************/
{
    return fhdl->orig_sec_hnd[idx - 1];
}

orl_return CoffSecScanReloc( coff_sec_handle coff_sec_hnd, orl_reloc_return_func return_func )
{
    coff_sec_handle reloc_sec_hnd;

    if( coff_sec_hnd->type != ORL_SEC_TYPE_PROG_BITS )
        return ORL_ERROR;
    if( (reloc_sec_hnd = coff_sec_hnd->assoc.normal.reloc_sec) == NULL )
        return ORL_FALSE;
    if( reloc_sec_hnd->assoc.reloc.relocs == NULL ) {
        const orl_return return_val = CoffCreateRelocs( coff_sec_hnd, reloc_sec_hnd );
        if( return_val != ORL_OKAY )
            return return_val;
    }
    
    {
        unsigned index;
        const unsigned limit = reloc_sec_hnd->assoc.reloc.num_relocs;
        const orl_reloc reloc = reloc_sec_hnd->assoc.reloc.relocs;
        
        if(limit == 0)
            return ORL_ERROR;
        
        for( index = 0; index < limit; index++ ) {
            const orl_return return_val = return_func( reloc + index );
            if( return_val != ORL_OKAY )
                return return_val;
        }
    }
    
    return ORL_OKAY;
}

orl_return CoffRelocSecScan( coff_sec_handle coff_sec_hnd, orl_reloc_return_func return_func )
{
    if( coff_sec_hnd->type != ORL_SEC_TYPE_RELOCS )
        return ORL_ERROR;
    if( coff_sec_hnd->assoc.reloc.relocs == NULL ) {
        const orl_return return_val  = CoffCreateRelocs( coff_sec_hnd->assoc.reloc.orig_sec, coff_sec_hnd );
        if( return_val != ORL_OKAY )
            return return_val;
    }
    
    {
        unsigned index;
        const unsigned limit = coff_sec_hnd->assoc.reloc.num_relocs;
        const orl_reloc reloc = coff_sec_hnd->assoc.reloc.relocs;
        
        if(limit == 0)
            return ORL_ERROR;
        
        for( index = 0; index < limit; index++ ) {
            const orl_return return_val = return_func( reloc + index );
            if( return_val != ORL_OKAY )
                return return_val;
        }
    }
    return ORL_OKAY;
}

orl_return CoffSymbolSecScan( coff_sec_handle coff_sec_hnd, orl_symbol_return_func return_func )
{
    coff_quantity index;
    coff_symbol_handle coff_symbol_hnd;

    if( coff_sec_hnd->type != ORL_SEC_TYPE_SYM_TABLE )
        return ORL_ERROR;
    if( coff_sec_hnd->coff_file_hnd->symbol_handles == NULL ) {
        const orl_return return_val = CoffCreateSymbolHandles( coff_sec_hnd->coff_file_hnd );
        if( return_val != ORL_OKAY )
            return return_val;
    }
    for( index = 0; index < coff_sec_hnd->coff_file_hnd->num_symbols; index++ ) {
        coff_symbol_hnd = coff_sec_hnd->coff_file_hnd->symbol_handles + index;
        {
            const orl_return return_val = return_func( (orl_symbol_handle)coff_symbol_hnd );
            if( return_val != ORL_OKAY )
                return return_val;
        }
        index += coff_symbol_hnd->symbol->num_aux;
    }
    return ORL_OKAY;
}

orl_return CoffNoteSecScan( coff_sec_handle hnd, struct orl_note_callbacks *cbs, void *cookie )
/************************************************************************************************/
{
    if( hnd->type != ORL_SEC_TYPE_NOTE )
        return ORL_ERROR;
    if( memcmp( hnd->name, ".drectve", 9 ) != 0 || hnd->size == 0)
        return ORL_OKAY;
    else
        return CoffParseDrectve( (char *)hnd->contents, hnd->size, cbs, cookie );
}

const char * CoffSymbolGetName( coff_symbol_handle coff_symbol_hnd )
{
    if( coff_symbol_hnd->type & ORL_SYM_TYPE_FILE )
        return coff_symbol_hnd->symbol->name.name_string + 1;
    else
        return coff_symbol_hnd->name;
}

coff_symbol_value CoffSymbolGetValue( coff_symbol_handle coff_symbol_hnd )
{
    return coff_symbol_hnd->symbol->value;
}

orl_symbol_binding CoffSymbolGetBinding( coff_symbol_handle coff_symbol_hnd )
{
    return coff_symbol_hnd->binding;
}

orl_symbol_type CoffSymbolGetType( coff_symbol_handle coff_symbol_hnd )
{
    return coff_symbol_hnd->type;
}

coff_sec_handle CoffSymbolGetSecHandle( coff_symbol_handle coff_symbol_hnd )
{
    if( coff_symbol_hnd->symbol->sec_num < 1 )
        return NULL;
    else
        return coff_symbol_hnd->coff_file_hnd->orig_sec_hnd[coff_symbol_hnd->symbol->sec_num - 1];
}

coff_symbol_handle CoffSymbolGetAssociated( coff_symbol_handle hnd )
{
    const struct coff_sym_weak * const weak = (struct coff_sym_weak *)( hnd->symbol + 1 );
    return hnd->coff_file_hnd->symbol_handles + weak->tag_index;
}

