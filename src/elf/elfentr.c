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


#include "elfentr.h"
#include "elfload.h"
#include "elfflhn.h"
#include "elflwlv.h"
#include "elforl.h"
#include "orlhash.h"

elf_handle ElfInit( struct orl_funcs *funcs )
{
    const elf_handle elf_hnd = (elf_handle)ORL_CLI_ALLOC( funcs, sizeof( struct elf_handle_struct ) );
    if( elf_hnd != NULL ) {
        elf_hnd->funcs = funcs;
        elf_hnd->first_file_hnd = NULL;
    }
    return( elf_hnd );
}

orl_return ElfFini( elf_handle elf_hnd )
{
    while( elf_hnd->first_file_hnd != NULL ) {
        const orl_return return_val = ElfRemoveFileLinks( elf_hnd->first_file_hnd );
        if( return_val != ORL_OKAY ) {
            return return_val;
        }
    }
    ORL_PTR_FREE( elf_hnd, elf_hnd );
    return ORL_OKAY;
}

orl_return ElfFileInit( elf_handle elf_hnd, orl_file_id file, elf_file_handle *pefh )
{
    const elf_file_handle elf_file_hnd = (elf_file_handle)ORL_PTR_ALLOC( elf_hnd, sizeof( struct elf_file_handle_struct ) );
    elf_file_hnd->sec_handles = NULL;
    elf_file_hnd->file = file;
    elf_file_hnd->sec_name_hash_table = NULL;
    ElfAddFileLinks( elf_hnd, elf_file_hnd );
    {
        const orl_return return_val = ElfLoadFileStructure( elf_file_hnd );
        if( return_val != ORL_OKAY ) {
            ElfRemoveFileLinks( elf_file_hnd );
            pefh[0] = NULL;
            /* Isn't elf_file_hnd leaked here? */
            return return_val;
        }
    }
    *pefh = elf_file_hnd;
    return ORL_OKAY;
}

orl_return ElfFileFini( elf_file_handle elf_file_hnd )
{
    return( ElfRemoveFileLinks( elf_file_hnd ) );
}

orl_return ElfFileScan( elf_file_handle elf_file_hnd, const char *desired, orl_sec_return_func return_func, void *cookie )
{
    struct orl_hash_key h_key;

    if( desired == NULL ) {
        /* global request */
        elf_quantity i;
        for( i = 0; i < elf_file_hnd->num_sections; ++i ) {
            const orl_return return_val = return_func( (orl_sec_handle)elf_file_hnd->sec_handles[i], cookie );
            if( return_val != ORL_OKAY ) {
                return return_val;
            }
        }
    }
    else {
        orl_hash_data_entry data_entry;
        if( elf_file_hnd->sec_name_hash_table == NULL ) {
            const orl_return return_val = ElfBuildSecNameHashTable( elf_file_hnd );
            if( return_val != ORL_OKAY ) {
                return return_val;
            }
        }
        h_key.u.string = desired;
        for( data_entry = ORLHashTableQuery( elf_file_hnd->sec_name_hash_table, h_key ); data_entry != NULL; data_entry = data_entry->next ) {
            const orl_return return_val = return_func( data_entry->data.u.sec_handle, cookie );
            if( return_val != ORL_OKAY ) {
                return return_val;
            }
        }
    }
    return ORL_OKAY;
}

orl_machine_type ElfFileGetMachineType( elf_file_handle elf_file_hnd )
{
    return( elf_file_hnd->machine_type );
}

orl_file_flags ElfFileGetFlags( elf_file_handle elf_file_hnd )
{
    return( elf_file_hnd->flags );
}

orl_file_type ElfFileGetType( elf_file_handle elf_file_hnd )
{
    return( elf_file_hnd->type );
}

orl_file_size ElfFileGetSize( elf_file_handle elf_file_hnd )
{
    return( elf_file_hnd->size );
}

elf_sec_handle ElfFileGetSymbolTable( elf_file_handle elf_file_hnd )
{
    return( elf_file_hnd->symbol_table );
}

const char * ElfSecGetName( elf_sec_handle elf_sec_hnd )
{
    return( elf_sec_hnd->name );
}

orl_return ElfSecGetBase( elf_sec_handle elf_sec_hnd, orl_sec_base *sec_base )
{
    sec_base[0] = elf_sec_hnd->base;
    return ORL_OKAY;
}

orl_sec_size ElfSecGetSize( elf_sec_handle elf_sec_hnd )
{
    return elf_sec_hnd->size;
}

orl_sec_type ElfSecGetType( elf_sec_handle elf_sec_hnd )
{
    return( elf_sec_hnd->type );
}

orl_sec_flags ElfSecGetFlags( elf_sec_handle elf_sec_hnd )
{
    return( elf_sec_hnd->flags );
}

orl_sec_alignment ElfSecGetAlignment( elf_sec_handle elf_sec_hnd )
{
    return( elf_sec_hnd->alignment );
}

elf_sec_handle ElfSecGetStringTable( elf_sec_handle elf_sec_hnd )
{
    switch( elf_sec_hnd->type ) {
    case ORL_SEC_TYPE_SYM_TABLE:
    case ORL_SEC_TYPE_DYN_SYM_TABLE:
        return( elf_sec_hnd->assoc.sym.string_table );
    case ORL_SEC_TYPE_IMPORT:
        return( elf_sec_hnd->assoc.import.string_table );
    default:
        return( NULL );
    }
}

elf_sec_handle ElfSecGetSymbolTable( elf_sec_handle elf_sec_hnd )
{
    switch( elf_sec_hnd->type ) {
    case ORL_SEC_TYPE_RELOCS:
    case ORL_SEC_TYPE_RELOCS_EXPADD:
        return( elf_sec_hnd->assoc.reloc.symbol_table );
    case ORL_SEC_TYPE_EXPORT:
        return( elf_sec_hnd->assoc.export.symbol_table );
    default:
        return( NULL );
    }
}

elf_sec_handle ElfSecGetRelocTable( elf_sec_handle elf_sec_hnd )
{
    switch( elf_sec_hnd->type ) {
    case ORL_SEC_TYPE_RELOCS:
    case ORL_SEC_TYPE_RELOCS_EXPADD:
    case ORL_SEC_TYPE_SYM_TABLE:
    case ORL_SEC_TYPE_DYN_SYM_TABLE:
    case ORL_SEC_TYPE_IMPORT:
    case ORL_SEC_TYPE_EXPORT:
        return( NULL );
    default:
        return( elf_sec_hnd->assoc.normal.reloc_sec );
    }
}

orl_return ElfSecGetContents( elf_sec_handle elf_sec_hnd, unsigned char **buffer )
{
    if( elf_sec_hnd->contents != NULL ) {
        *buffer = elf_sec_hnd->contents;
        return ORL_OKAY;
    }
    return ORL_ERROR;
}

orl_return ElfSecQueryReloc( elf_sec_handle elf_sec_hnd, elf_sec_offset sec_offset, orl_reloc_return_func return_func, void *cookie )
{
    uint64_t index;
    elf_sec_handle reloc_sec_hnd;
    orl_reloc reloc;

    if( elf_sec_hnd->type != ORL_SEC_TYPE_PROG_BITS )
        return ORL_ERROR;
    reloc_sec_hnd = elf_sec_hnd->assoc.normal.reloc_sec;
    if( reloc_sec_hnd == NULL )
        return ORL_FALSE;
    if( reloc_sec_hnd->assoc.reloc.relocs == NULL ) {
        const orl_return return_val = ElfCreateRelocs( elf_sec_hnd, reloc_sec_hnd );
        if( return_val != ORL_OKAY ) {
            return return_val;
        }
    }
    reloc = reloc_sec_hnd->assoc.reloc.relocs;
    for( index = 0; index < reloc_sec_hnd->size; index += reloc_sec_hnd->entsize ) {
        if( reloc->offset == sec_offset ) {
            const orl_return return_val = return_func( reloc, cookie );
            if( return_val != ORL_OKAY ) {
                return return_val;
            }
        }
        reloc++;
    }
    return ORL_OKAY;
}

orl_return ElfSecScanReloc( elf_sec_handle elf_sec_hnd, orl_reloc_return_func return_func, void *cookie )
{
    uint64_t index;
    elf_sec_handle reloc_sec_hnd;
    orl_reloc reloc;

    if( elf_sec_hnd->type != ORL_SEC_TYPE_PROG_BITS ) {
        return ORL_ERROR;
    } else if ( elf_sec_hnd->assoc.normal.reloc_sec == NULL ) {
        return ORL_FALSE;
    }
    reloc_sec_hnd = elf_sec_hnd->assoc.normal.reloc_sec;
    if( reloc_sec_hnd->assoc.reloc.relocs == NULL ) {
        const orl_return return_val = ElfCreateRelocs( elf_sec_hnd, reloc_sec_hnd );
        if( return_val != ORL_OKAY ) {
            return return_val;
        }
    }
    reloc = reloc_sec_hnd->assoc.reloc.relocs;
    for( index = 0; index < reloc_sec_hnd->size; index += reloc_sec_hnd->entsize ) {
        const orl_return return_val = return_func( reloc, cookie );
        if( return_val != ORL_OKAY )
            return return_val;
        reloc++;
    }
    return ORL_TRUE;
}

/* One of these should be implimented O(1), but the section handles
 * get reordered in ElfLoad.
 */
orl_table_index ElfCvtSecHdlToIdx( elf_sec_handle elf_sec_hnd )
{
    elf_quantity index;
    const elf_file_handle fhdl = elf_sec_hnd->elf_file_hnd;
    const elf_quantity limit = fhdl->num_sections;
    for( index = 0; index < limit; index++ ) {
        const elf_sec_handle hnd = fhdl->sec_handles[index];
        if( hnd == elf_sec_hnd ) {
            return hnd->index;
        }
    }
    return 0;
}

elf_sec_handle ElfCvtIdxToSecHdl( elf_file_handle fhdl, orl_table_index idx )
{
    elf_quantity index;
    const elf_quantity limit = fhdl->num_sections;
    for( index = 0; index < limit; index++ ) {
        const elf_sec_handle hnd = fhdl->sec_handles[index];
        if( hnd->index == idx ) {
            return hnd;
        }
    }
    return 0;
}

orl_return ElfRelocSecScan( elf_sec_handle elf_sec_hnd, orl_reloc_return_func return_func, void *cookie )
{
    uint64_t index;
    orl_reloc reloc;

    if( elf_sec_hnd->type != ORL_SEC_TYPE_RELOCS && elf_sec_hnd->type != ORL_SEC_TYPE_RELOCS_EXPADD )
        return ORL_ERROR;
    if( elf_sec_hnd->assoc.reloc.relocs == NULL ) {
        const orl_return return_val = ElfCreateRelocs( elf_sec_hnd->assoc.reloc.orig_sec, elf_sec_hnd );
        if( return_val != ORL_OKAY ) {
            return return_val;
        }
    }
    reloc = elf_sec_hnd->assoc.reloc.relocs;
    for( index = 0; index < elf_sec_hnd->size; index += elf_sec_hnd->entsize ) {
        const orl_return return_val = return_func( reloc, cookie );
        if( return_val != ORL_OKAY )
            return return_val;
        reloc++;
    }
    return ORL_TRUE;
}

orl_return ElfSymbolSecScan( elf_sec_handle elf_sec_hnd, orl_symbol_return_func return_func, void *cookie )
{
    uint64_t index;
    elf_symbol_handle elf_symbol_hnd;

    switch( elf_sec_hnd->type ) {
    case ORL_SEC_TYPE_SYM_TABLE:
    case ORL_SEC_TYPE_DYN_SYM_TABLE:
        if( elf_sec_hnd->assoc.sym.symbols == NULL ) {
            const orl_return return_val = ElfCreateSymbolHandles( elf_sec_hnd );
            if( return_val != ORL_OKAY )
                return return_val;
        }
        elf_symbol_hnd = elf_sec_hnd->assoc.sym.symbols;
        break;
    default:
        return ORL_ERROR;
    }
    for( index = 0; index < elf_sec_hnd->size; index += elf_sec_hnd->entsize ) {
        const orl_return return_val = return_func( (orl_symbol_handle)elf_symbol_hnd, cookie );
        if( return_val != ORL_OKAY )
            return return_val;
        elf_symbol_hnd++;
    }
    return ORL_OKAY;
}

orl_return ElfNoteSecScan( elf_sec_handle elf_sec_hnd, struct orl_note_callbacks *cbs, void *cookie )
/*****************************************************************************************************/
{
    if( elf_sec_hnd->type != ORL_SEC_TYPE_NOTE )
        return ORL_ERROR;
    if( strcmp( elf_sec_hnd->name, ".drectve" ) != 0 )
        return ORL_OKAY;
    if( elf_sec_hnd->size == 0 )
        return ORL_OKAY;
    return ElfParseDrectve( (char *)elf_sec_hnd->contents, elf_sec_hnd->size, cbs, cookie );
}

const char * ElfSymbolGetName( elf_symbol_handle elf_symbol_hnd )
{
    return elf_symbol_hnd->name;
}

orl_return ElfSymbolGetValue( elf_symbol_handle elf_symbol_hnd, orl_symbol_value *sym_value )
{
    sym_value[0] = elf_symbol_hnd->value;
    return ORL_OKAY;
}

orl_symbol_binding ElfSymbolGetBinding( elf_symbol_handle elf_symbol_hnd )
{
    return elf_symbol_hnd->binding;
}

orl_symbol_type ElfSymbolGetType( elf_symbol_handle elf_symbol_hnd )
{
    return elf_symbol_hnd->type;
}

unsigned char ElfSymbolGetRawInfo( elf_symbol_handle elf_symbol_hnd )
{
    return elf_symbol_hnd->info;
}

elf_sec_handle ElfSymbolGetSecHandle( elf_symbol_handle elf_symbol_hnd )
{
    switch( elf_symbol_hnd->shndx ) {
    case SHN_ABS:
    case SHN_COMMON:
    case SHN_UNDEF:
        return NULL;
    default:
        return elf_symbol_hnd->elf_file_hnd->orig_sec_handles[elf_symbol_hnd->shndx - 1];
    }
}

elf_symbol_handle ElfSymbolGetAssociated( elf_symbol_handle elf_symbol_hnd )
{
    return elf_symbol_hnd;
}
