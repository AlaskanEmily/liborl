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
* Description:  Entry points for ORL routines.
*
****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "orl.h"
#include "orl_endian.h"
#include "orllevel.h"
#include "orlentry.h"
#include "orlflhnd.h"
#include "omf/pcobj.h"

orl_handle ORLInit( struct orl_funcs *funcs )
/*********************************************/
{
    orl_handle orl_hnd;

    orl_hnd = (orl_handle)ORL_CLI_ALLOC( funcs, sizeof( ORL_LCL_STRUCT( orl_handle ) ) );
    if( orl_hnd != NULL ) {
        LCL_ORL_HND( orl_hnd )->error = ORL_OKAY;
        LCL_ORL_HND( orl_hnd )->funcs = *funcs;
#ifdef ORL_ENABLE_ELF
        LCL_ORL_HND( orl_hnd )->elf_hnd = ElfInit( &LCL_ORL_HND( orl_hnd )->funcs );
        if( LCL_ORL_HND( orl_hnd )->elf_hnd == NULL ) {
            ORL_CLI_FREE( funcs, (void *)orl_hnd );
            return NULL;
        }
#endif
#ifdef ORL_ENABLE_COFF
        LCL_ORL_HND( orl_hnd )->coff_hnd = CoffInit( &LCL_ORL_HND( orl_hnd )->funcs );
        if( LCL_ORL_HND( orl_hnd )->coff_hnd == NULL ) {
            ORL_CLI_FREE( funcs, (void *)orl_hnd );
            return NULL;
        }
#endif
#ifdef ORL_ENABLE_OMF
        LCL_ORL_HND( orl_hnd )->omf_hnd = OmfInit( &LCL_ORL_HND( orl_hnd )->funcs );
        if( LCL_ORL_HND( orl_hnd )->omf_hnd == NULL ) {
            ORL_CLI_FREE( funcs, (void *)orl_hnd );
            return NULL;
        }
#endif

        LCL_ORL_HND( orl_hnd )->first_file_hnd = NULL;
    }
    return orl_hnd;
}

orl_return ORLGetError( orl_handle orl_hnd )
/***************************************************/
{
    return LCL_ORL_HND( orl_hnd )->error;
}

orl_return ORLFini( orl_handle orl_hnd )
/***********************************************/
{
    orl_return return_val = ORL_OKAY;
    (void)return_val;

#ifdef ORL_ENABLE_ELF
    if( ( return_val = ElfFini( LCL_ORL_HND( orl_hnd )->elf_hnd ) ) != ORL_OKAY )
        return( return_val );
#endif
#ifdef ORL_ENABLE_COFF
    if( ( return_val = CoffFini( LCL_ORL_HND( orl_hnd )->coff_hnd ) ) != ORL_OKAY )
        return( return_val );
#endif
#ifdef ORL_ENABLE_OMF
    if( ( return_val = OmfFini( LCL_ORL_HND( orl_hnd )->omf_hnd ) ) != ORL_OKAY )
        return( return_val );
#endif
    while( LCL_ORL_HND( orl_hnd )->first_file_hnd != NULL ) {
        return_val = ORLRemoveFileLinks( LCL_ORL_HND( orl_hnd )->first_file_hnd );
        if( return_val != ORL_OKAY )
            return( return_val );
    }
    ORL_FUNCS_FREE( LCL_ORL_HND( orl_hnd ), (void *)orl_hnd );
    return ORL_OKAY;
}

orl_file_format ORLFileIdentify( orl_handle orl_hnd, orl_file_id file )
/*********************************************************************/
{
    unsigned char *magic;
    uint16_t machine_type;
    uint16_t offset;
    uint16_t len;
    unsigned char chksum;

    magic = ORL_FUNCS_READ( LCL_ORL_HND( orl_hnd ), file, 4 );
    if( magic == NULL ) {
        return ORL_UNRECOGNIZED_FORMAT;
    }
    if( ORL_FUNCS_SEEK( LCL_ORL_HND( orl_hnd ), file, -4, SEEK_CUR ) == -1 ) {
        return ORL_UNRECOGNIZED_FORMAT;
    }
#ifdef ORL_ENABLE_ELF
    if( magic[0] == 0x7f && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F' ) {
        return ORL_ELF;
    }
#endif

    /* See if this is the start of an OMF object file
     * the first record must be a THEADR record and we check that it is
     * valid, if it is then we assume that this is an OMF object file.
     */
    if( magic[0] == CMD_THEADR ) {
        len = magic[1] | ( magic[2] << 8 );
        len -= magic[3];
        len -= 2;
        if( len == 0 ) {
            /* This looks good so far, we must now check the record */
            len = magic[3] + 1;
            if( ORL_FUNCS_SEEK( LCL_ORL_HND( orl_hnd ), file, 4, SEEK_CUR ) == -1 ) {
                return ORL_UNRECOGNIZED_FORMAT;
            }
            chksum = magic[0] + magic[1] + magic[2] + magic[3];
            magic = ORL_FUNCS_READ( LCL_ORL_HND( orl_hnd ), file, len );
            if( ORL_FUNCS_SEEK( LCL_ORL_HND( orl_hnd ), file, -(long)( 4 + len ), SEEK_CUR ) == -1 ) {
                return ORL_UNRECOGNIZED_FORMAT;
            }
            if( magic != NULL ) {
                /* Go on to check record checksum */
                while( len ) {
                    chksum += *magic;
                    len--;
                    magic++;
                }
                magic--;
                if( *magic == 0 || chksum == 0 ) {
                    /* seems to be a correct OMF record to start the OBJ */
#ifdef ORL_ENABLE_OMF
                    return ORL_OMF;
#else
                    return ORL_UNRECOGNIZED_FORMAT;
#endif
                }
            }
            else {
                magic = ORL_FUNCS_READ( LCL_ORL_HND( orl_hnd ), file, 4 );
                if( magic == NULL ) {
                    return ORL_UNRECOGNIZED_FORMAT;
                }
                else if( ORL_FUNCS_SEEK( LCL_ORL_HND( orl_hnd ), file, -4, SEEK_CUR ) == -1 ) {
                    return ORL_UNRECOGNIZED_FORMAT;
                }
            }
        }
    }

    machine_type = *(uint16_t *)magic;
    switch( machine_type ) {
    case IMAGE_FILE_MACHINE_I860:
    case IMAGE_FILE_MACHINE_I386A:
    case IMAGE_FILE_MACHINE_I386:
    case IMAGE_FILE_MACHINE_R3000:
    case IMAGE_FILE_MACHINE_R4000:
    case IMAGE_FILE_MACHINE_ALPHA:
    case IMAGE_FILE_MACHINE_POWERPC:
    case IMAGE_FILE_MACHINE_AMD64:
    case IMAGE_FILE_MACHINE_UNKNOWN:
        return( ORL_COFF );
    }
    /* Is it PE? */
    if( magic[0] == 'M' && magic[1] == 'Z' ) {
        if( ORL_FUNCS_SEEK( LCL_ORL_HND( orl_hnd ), file, 0x3c, SEEK_CUR ) == -1 ) {
            return ORL_UNRECOGNIZED_FORMAT;
        }
        magic = ORL_FUNCS_READ( LCL_ORL_HND( orl_hnd ), file, 0x4 );
        if( magic == NULL ) {
            return ORL_UNRECOGNIZED_FORMAT;
        }
        offset = *(uint16_t *)magic;
        if( ORL_FUNCS_SEEK( LCL_ORL_HND( orl_hnd ), file, offset - 0x40, SEEK_CUR ) == -1 ) {
            return ORL_UNRECOGNIZED_FORMAT;
        }
        magic = ORL_FUNCS_READ( LCL_ORL_HND( orl_hnd ), file, 4 );
        if( magic == NULL ) {
            return ORL_UNRECOGNIZED_FORMAT;
        }
        if( magic[0]=='P' && magic[1] == 'E' && magic[2] == '\0' && magic[3] == '\0' ) {
            magic = ORL_FUNCS_READ( LCL_ORL_HND( orl_hnd ), file, 4 );
            if( magic == NULL ) {
                return ORL_UNRECOGNIZED_FORMAT;
            }
            if( ORL_FUNCS_SEEK( LCL_ORL_HND( orl_hnd ), file, -(long)(offset+8), SEEK_CUR ) == -1 ) {
                return ORL_UNRECOGNIZED_FORMAT;
            }
            machine_type = *(uint16_t *)magic;
#ifdef ORL_ENABLE_COFF
            switch( machine_type ) {
            case IMAGE_FILE_MACHINE_I860:
            case IMAGE_FILE_MACHINE_I386:
            case IMAGE_FILE_MACHINE_R3000:
            case IMAGE_FILE_MACHINE_R4000:
            case IMAGE_FILE_MACHINE_ALPHA:
            case IMAGE_FILE_MACHINE_POWERPC:
            case IMAGE_FILE_MACHINE_AMD64:
                return ORL_COFF;
            }
#endif
        }
    }
    return ORL_UNRECOGNIZED_FORMAT;
}

orl_file_handle ORLFileInit( orl_handle orl_hnd, orl_file_id file, orl_file_format type )
{
#if defined( ORL_ENABLE_ELF ) || defined( ORL_ENABLE_COFF ) || defined( ORL_ENABLE_OMF )
    orl_file_handle orl_file_hnd;
    switch( type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
#endif
        orl_file_hnd = (orl_file_handle)ORL_FUNCS_ALLOC( LCL_ORL_HND( orl_hnd ), sizeof( ORL_LCL_STRUCT( orl_file_handle ) ) );
        LCL_FIL_HND( orl_file_hnd )->type = type;
        switch( type ) {
#ifdef ORL_ENABLE_ELF
        case ORL_ELF:
            LCL_ORL_HND( orl_hnd )->error = ElfFileInit( LCL_ORL_HND( orl_hnd )->elf_hnd, file, &LCL_FIL_HND( orl_file_hnd )->file_hnd.elf );
            break;
#endif
#ifdef ORL_ENABLE_COFF
        case ORL_COFF:
            LCL_ORL_HND( orl_hnd )->error = CoffFileInit( LCL_ORL_HND( orl_hnd )->coff_hnd, file, &LCL_FIL_HND( orl_file_hnd )->file_hnd.coff );
            break;
#endif
#ifdef ORL_ENABLE_OMF
        case ORL_OMF:
            LCL_ORL_HND( orl_hnd )->error = OmfFileInit( LCL_ORL_HND( orl_hnd )->omf_hnd, file, &LCL_FIL_HND( orl_file_hnd )->file_hnd.omf );
            break;
#endif
        default:
            break;
        }
        if( LCL_ORL_HND( orl_hnd )->error != ORL_OKAY ) {
            ORL_FUNCS_FREE( LCL_ORL_HND( orl_hnd ), (void *)orl_file_hnd );
            break;
        }
        ORLAddFileLinks( LCL_ORL_HND( orl_hnd ), LCL_FIL_HND( orl_file_hnd ) );
        return orl_file_hnd;
    default: /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
#endif
    return NULL;
}

orl_return ORLFileFini( orl_file_handle orl_file_hnd )
{
    orl_return return_val = ORL_ERROR;

    /* jump table replace: */
    switch( LCL_FIL_HND( orl_file_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return_val = ElfFileFini( LCL_FIL_HND( orl_file_hnd )->file_hnd.elf );
        break;
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return_val = CoffFileFini( LCL_FIL_HND( orl_file_hnd )->file_hnd.coff );
        break;
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return_val = OmfFileFini( LCL_FIL_HND( orl_file_hnd )->file_hnd.omf );
        break;
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        return ORL_ERROR;
    }

    if( return_val != ORL_OKAY )
        return return_val;
    return ORLRemoveFileLinks( LCL_FIL_HND( orl_file_hnd ) );
}


orl_rva ORLExportTableRVA( orl_file_handle orl_file_hnd )
{
    orl_rva rva = 0;

#ifdef ORL_ENABLE_COFF
    if( LCL_FIL_HND( orl_file_hnd )->type == ORL_COFF ) {
        rva = CoffExportTableRVA( LCL_FIL_HND( orl_file_hnd )->file_hnd.coff );
    }
#endif

    return rva;
}

orl_return ORLFileScan( orl_file_handle orl_file_hnd, const char *desired, orl_sec_return_func return_func )
{
    switch( LCL_FIL_HND( orl_file_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfFileScan( LCL_FIL_HND( orl_file_hnd )->file_hnd.elf, desired, return_func ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffFileScan( LCL_FIL_HND( orl_file_hnd )->file_hnd.coff, desired, return_func ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfFileScan( LCL_FIL_HND( orl_file_hnd )->file_hnd.omf, desired, return_func ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return ORL_ERROR;
}

orl_machine_type ORLFileGetMachineType( orl_file_handle orl_file_hnd )
{
    switch( LCL_FIL_HND( orl_file_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfFileGetMachineType( LCL_FIL_HND( orl_file_hnd )->file_hnd.elf ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffFileGetMachineType( LCL_FIL_HND( orl_file_hnd )->file_hnd.coff ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfFileGetMachineType( LCL_FIL_HND( orl_file_hnd )->file_hnd.omf ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return 0;
}

orl_file_flags ORLFileGetFlags( orl_file_handle orl_file_hnd )
{
    switch( LCL_FIL_HND( orl_file_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfFileGetFlags( LCL_FIL_HND( orl_file_hnd )->file_hnd.elf ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffFileGetFlags( LCL_FIL_HND( orl_file_hnd )->file_hnd.coff ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfFileGetFlags( LCL_FIL_HND( orl_file_hnd )->file_hnd.omf ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return 0;
}

orl_file_size ORLFileGetSize( orl_file_handle orl_file_hnd )
{
    switch( LCL_FIL_HND( orl_file_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfFileGetSize( LCL_FIL_HND( orl_file_hnd )->file_hnd.elf ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffFileGetSize( LCL_FIL_HND( orl_file_hnd )->file_hnd.coff ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfFileGetSize( LCL_FIL_HND( orl_file_hnd )->file_hnd.omf ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return 0;
}

orl_file_type ORLFileGetType( orl_file_handle orl_file_hnd )
{
    switch( LCL_FIL_HND( orl_file_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfFileGetType( LCL_FIL_HND( orl_file_hnd )->file_hnd.elf ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffFileGetType( LCL_FIL_HND( orl_file_hnd )->file_hnd.coff ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfFileGetType( LCL_FIL_HND( orl_file_hnd )->file_hnd.omf ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return 0;
}

orl_file_format ORLFileGetFormat( orl_file_handle orl_file_hnd )
{
    return( LCL_FIL_HND( orl_file_hnd )->type );
}

orl_sec_handle ORLFileGetSymbolTable( orl_file_handle orl_file_hnd )
{
    switch( LCL_FIL_HND( orl_file_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( (orl_sec_handle)ElfFileGetSymbolTable( LCL_FIL_HND( orl_file_hnd )->file_hnd.elf ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( (orl_sec_handle)CoffFileGetSymbolTable( LCL_FIL_HND( orl_file_hnd )->file_hnd.coff ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( (orl_sec_handle)OmfFileGetSymbolTable( LCL_FIL_HND( orl_file_hnd )->file_hnd.omf ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return NULL;
}

const char * ORLSecGetName( orl_sec_handle orl_sec_hnd )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfSecGetName( (elf_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffSecGetName( (coff_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSecGetName( (omf_sec_handle)orl_sec_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return NULL;
}

orl_return ORLSecGetBase( orl_sec_handle orl_sec_hnd, orl_sec_base *sec_base )
{
    if( sec_base != NULL ) {
        switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
        case ORL_ELF:
            return( ElfSecGetBase( (elf_sec_handle)orl_sec_hnd, sec_base ) );
#endif
#ifdef ORL_ENABLE_COFF
        case ORL_COFF:
            sec_base[0] = CoffSecGetBase( (coff_sec_handle)orl_sec_hnd );
            return ORL_OKAY;
#endif
#ifdef ORL_ENABLE_OMF
        case ORL_OMF:
            sec_base[0] = OmfSecGetBase( (omf_sec_handle)orl_sec_hnd );
            return ORL_OKAY;
#endif
        default:    /* ORL_UNRECOGNIZED_FORMAT */
            break;
        }
        sec_base[0] = 0;
    }
    return ORL_ERROR;
}

orl_sec_size ORLSecGetSize( orl_sec_handle orl_sec_hnd )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfSecGetSize( (elf_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffSecGetSize( (coff_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSecGetSize( (omf_sec_handle)orl_sec_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return 0;
}

orl_sec_type ORLSecGetType( orl_sec_handle orl_sec_hnd )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfSecGetType( (elf_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffSecGetType( (coff_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSecGetType( (omf_sec_handle)orl_sec_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return 0;
}

orl_sec_alignment ORLSecGetAlignment( orl_sec_handle orl_sec_hnd )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfSecGetAlignment( (elf_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffSecGetAlignment( (coff_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSecGetAlignment( (omf_sec_handle)orl_sec_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return 0;
}

orl_sec_flags ORLSecGetFlags( orl_sec_handle orl_sec_hnd )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfSecGetFlags( (elf_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffSecGetFlags( (coff_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSecGetFlags( (omf_sec_handle)orl_sec_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return 0;
}

orl_sec_handle ORLSecGetStringTable( orl_sec_handle orl_sec_hnd )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( (orl_sec_handle)ElfSecGetStringTable( (elf_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( (orl_sec_handle)CoffSecGetStringTable( (coff_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( (orl_sec_handle)OmfSecGetStringTable( (omf_sec_handle)orl_sec_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return NULL;
}

orl_sec_handle ORLSecGetSymbolTable( orl_sec_handle orl_sec_hnd )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( (orl_sec_handle)ElfSecGetSymbolTable( (elf_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( (orl_sec_handle)CoffSecGetSymbolTable( (coff_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( (orl_sec_handle)OmfSecGetSymbolTable( (omf_sec_handle)orl_sec_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return NULL;
}

orl_sec_handle ORLSecGetRelocTable( orl_sec_handle orl_sec_hnd )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( (orl_sec_handle)ElfSecGetRelocTable( (elf_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( (orl_sec_handle)CoffSecGetRelocTable( (coff_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( (orl_sec_handle)OmfSecGetRelocTable( (omf_sec_handle)orl_sec_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return NULL;
}

orl_linnum ORLSecGetLines( orl_sec_handle orl_sec_hnd )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffSecGetLines( (coff_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSecGetLines( (omf_sec_handle)orl_sec_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return NULL;
}

orl_table_index ORLSecGetNumLines( orl_sec_handle orl_sec_hnd )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffSecGetNumLines( (coff_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSecGetNumLines( (omf_sec_handle)orl_sec_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return 0;
}

orl_sec_offset ORLSecGetOffset( orl_sec_handle orl_sec_hnd )
{
#ifdef ORL_ENABLE_COFF
    if( LCL_SEC_HND( orl_sec_hnd )->type == ORL_COFF ) {
        return( CoffSecGetOffset( (coff_sec_handle)orl_sec_hnd ) );
    }
#endif
    return 0;
}

const char * ORLSecGetClassName( orl_sec_handle orl_sec_hnd )
{
#ifdef ORL_ENABLE_OMF
    if( LCL_SEC_HND( orl_sec_hnd )->type == ORL_OMF ) {
        return( OmfSecGetClassName( (omf_sec_handle)orl_sec_hnd ) );
    }
#endif
    return 0;
}

orl_sec_combine ORLSecGetCombine( orl_sec_handle orl_sec_hnd )
{
#ifdef ORL_ENABLE_OMF
    if( LCL_SEC_HND( orl_sec_hnd )->type == ORL_OMF ) {
        return( OmfSecGetCombine( (omf_sec_handle)orl_sec_hnd ) );
    }
#endif
    return( ORL_SEC_COMBINE_NONE );
}

orl_sec_frame ORLSecGetAbsFrame( orl_sec_handle orl_sec_hnd )
{
#ifdef ORL_ENABLE_OMF
    if( LCL_SEC_HND( orl_sec_hnd )->type == ORL_OMF ) {
        return( OmfSecGetAbsFrame( (omf_sec_handle)orl_sec_hnd ) );
    }
#endif
    return( ORL_SEC_NO_ABS_FRAME );
}

orl_sec_handle ORLSecGetAssociated( orl_sec_handle orl_sec_hnd )
{
#ifdef ORL_ENABLE_OMF
    if( LCL_SEC_HND( orl_sec_hnd )->type == ORL_OMF ) {
        return( OmfSecGetAssociated( (omf_sec_handle)orl_sec_hnd ) );
    }
#endif
    return NULL;
}

orl_group_handle ORLSecGetGroup( orl_sec_handle orl_sec_hnd )
{
#ifdef ORL_ENABLE_OMF
    if( LCL_SEC_HND( orl_sec_hnd )->type == ORL_OMF ) {
        return( OmfSecGetGroup( (omf_sec_handle)orl_sec_hnd ) );
    }
#endif
    return NULL;
}

orl_return ORLSecGetContents( orl_sec_handle orl_sec_hnd, unsigned char **buffer )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfSecGetContents( (elf_sec_handle)orl_sec_hnd, buffer ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffSecGetContents( (coff_sec_handle)orl_sec_hnd, buffer ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSecGetContents( (omf_sec_handle)orl_sec_hnd, buffer ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return ORL_ERROR;
}

orl_return ORLSecQueryReloc( orl_sec_handle orl_sec_hnd, orl_sec_offset sec_offset, orl_reloc_return_func return_func )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfSecQueryReloc( (elf_sec_handle)orl_sec_hnd, sec_offset, return_func ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffSecQueryReloc( (coff_sec_handle)orl_sec_hnd, sec_offset, return_func ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSecQueryReloc( (omf_sec_handle)orl_sec_hnd, sec_offset, return_func ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return ORL_ERROR;
}

orl_return ORLSecScanReloc( orl_sec_handle orl_sec_hnd, orl_reloc_return_func return_func )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfSecScanReloc( (elf_sec_handle)orl_sec_hnd, return_func ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffSecScanReloc( (coff_sec_handle)orl_sec_hnd, return_func ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSecScanReloc( (omf_sec_handle)orl_sec_hnd, return_func ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return ORL_ERROR;
}


orl_table_index ORLCvtSecHdlToIdx( orl_sec_handle orl_sec_hnd )
/**********************************************************************/
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfCvtSecHdlToIdx( (elf_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffCvtSecHdlToIdx( (coff_sec_handle)orl_sec_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfCvtSecHdlToIdx( (omf_sec_handle)orl_sec_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return 0;
}

orl_sec_handle ORLCvtIdxToSecHdl( orl_file_handle orl_file_hnd, orl_table_index idx )
/********************************************************************************************/
{
    switch( LCL_FIL_HND( orl_file_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( (orl_sec_handle)ElfCvtIdxToSecHdl( LCL_FIL_HND( orl_file_hnd )->file_hnd.elf, idx ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( (orl_sec_handle)CoffCvtIdxToSecHdl( LCL_FIL_HND( orl_file_hnd )->file_hnd.coff, idx ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( (orl_sec_handle)OmfCvtIdxToSecHdl( LCL_FIL_HND( orl_file_hnd )->file_hnd.omf, idx ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return NULL;
}

orl_return ORLRelocSecScan( orl_sec_handle orl_sec_hnd, orl_reloc_return_func return_func )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfRelocSecScan( (elf_sec_handle)orl_sec_hnd, return_func ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffRelocSecScan( (coff_sec_handle)orl_sec_hnd, return_func ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfRelocSecScan( (omf_sec_handle)orl_sec_hnd, return_func ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return ORL_ERROR;
}

orl_return ORLSymbolSecScan( orl_sec_handle orl_sec_hnd, orl_symbol_return_func return_func )
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfSymbolSecScan( (elf_sec_handle)orl_sec_hnd, return_func ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffSymbolSecScan( (coff_sec_handle)orl_sec_hnd, return_func ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSymbolSecScan( (omf_sec_handle)orl_sec_hnd, return_func ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return ORL_ERROR;
}

orl_return ORLNoteSecScan( orl_sec_handle orl_sec_hnd, struct orl_note_callbacks *cbs, void *cookie )
/*****************************************************************************************************/
{
    switch( LCL_SEC_HND( orl_sec_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfNoteSecScan( (elf_sec_handle)orl_sec_hnd, cbs, cookie ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffNoteSecScan( (coff_sec_handle)orl_sec_hnd, cbs, cookie ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfNoteSecScan( (omf_sec_handle)orl_sec_hnd, cbs, cookie ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return ORL_ERROR;
}


const char * ORLSymbolGetName( orl_symbol_handle orl_symbol_hnd )
{
    switch( LCL_SYM_HND( orl_symbol_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfSymbolGetName( (elf_symbol_handle)orl_symbol_hnd ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffSymbolGetName( (coff_symbol_handle)orl_symbol_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSymbolGetName( (omf_symbol_handle)orl_symbol_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return NULL;
}

orl_return ORLSymbolGetValue( orl_symbol_handle orl_symbol_hnd, orl_symbol_value *sym_value )
{
    if( sym_value != NULL ) {
        switch( LCL_SYM_HND( orl_symbol_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
        case ORL_ELF:
            return( ElfSymbolGetValue( (elf_symbol_handle)orl_symbol_hnd, sym_value ) );
#endif
#ifdef ORL_ENABLE_COFF
        case ORL_COFF:
            sym_value[0] = CoffSymbolGetValue( (coff_symbol_handle)orl_symbol_hnd );
            return ORL_OKAY;
#endif
#ifdef ORL_ENABLE_OMF
        case ORL_OMF:
            sym_value[0] = OmfSymbolGetValue( (omf_symbol_handle)orl_symbol_hnd );
            return ORL_OKAY;
#endif
        default:     /* ORL_UNRECOGNIZED_FORMAT */
            break;
        }
        sym_value[0] = 0;
    }
    return ORL_ERROR;
}

orl_symbol_binding ORLSymbolGetBinding( orl_symbol_handle orl_symbol_hnd )
{
    switch( LCL_SYM_HND( orl_symbol_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfSymbolGetBinding( (elf_symbol_handle)orl_symbol_hnd ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffSymbolGetBinding( (coff_symbol_handle)orl_symbol_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSymbolGetBinding( (omf_symbol_handle)orl_symbol_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return 0;
}

orl_symbol_type ORLSymbolGetType( orl_symbol_handle orl_symbol_hnd )
{
    switch( LCL_SYM_HND( orl_symbol_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfSymbolGetType( (elf_symbol_handle)orl_symbol_hnd ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( CoffSymbolGetType( (coff_symbol_handle)orl_symbol_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSymbolGetType( (omf_symbol_handle)orl_symbol_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return 0;
}

unsigned char ORLSymbolGetRawInfo( orl_symbol_handle orl_symbol_hnd )
{
    switch( LCL_SYM_HND( orl_symbol_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( ElfSymbolGetRawInfo( (elf_symbol_handle)orl_symbol_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfSymbolGetRawInfo( (omf_symbol_handle)orl_symbol_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return 0;
}

orl_sec_handle ORLSymbolGetSecHandle( orl_symbol_handle orl_symbol_hnd )
{
    switch( LCL_SYM_HND( orl_symbol_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( (orl_sec_handle)ElfSymbolGetSecHandle( (elf_symbol_handle)orl_symbol_hnd ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( (orl_sec_handle)CoffSymbolGetSecHandle( (coff_symbol_handle)orl_symbol_hnd ) );
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( (orl_sec_handle)OmfSymbolGetSecHandle( (omf_symbol_handle)orl_symbol_hnd ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return NULL;
}

orl_symbol_handle ORLSymbolGetAssociated( orl_symbol_handle orl_symbol_hnd )
{
    switch( LCL_SYM_HND( orl_symbol_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return( (orl_symbol_handle)ElfSymbolGetAssociated( (elf_symbol_handle)orl_symbol_hnd ) );
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return( (orl_symbol_handle)CoffSymbolGetAssociated( (coff_symbol_handle)orl_symbol_hnd ) );
#endif
    /* TODO: Legend has it there should be an OMF variant here, too */
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return NULL;
}

orl_return ORLGroupsScan( orl_file_handle orl_file_hnd, orl_group_return_func return_func )
{
    switch( LCL_FIL_HND( orl_file_hnd )->type ) {
#ifdef ORL_ENABLE_ELF
    case ORL_ELF:
        return ORL_OKAY;
#endif
#ifdef ORL_ENABLE_COFF
    case ORL_COFF:
        return ORL_OKAY;
#endif
#ifdef ORL_ENABLE_OMF
    case ORL_OMF:
        return( OmfGroupsScan( LCL_FIL_HND( orl_file_hnd )->file_hnd.omf, return_func ) );
#endif
    default:    /* ORL_UNRECOGNIZED_FORMAT */
        break;
    }
    return ORL_ERROR;
}

const char *ORLGroupName( orl_group_handle orl_group_hnd )
{
#ifdef ORL_ENABLE_OMF
    if( LCL_GRP_HND( orl_group_hnd )->type == ORL_OMF )
        return( OmfGroupName( (omf_grp_handle)orl_group_hnd ) );
#endif
    return NULL;
}

orl_table_index ORLGroupSize( orl_group_handle orl_group_hnd )
{
#ifdef ORL_ENABLE_OMF
    if( LCL_GRP_HND( orl_group_hnd )->type == ORL_OMF )
        return OmfGroupSize( (omf_grp_handle)orl_group_hnd );
#endif
    return 0;
}

const char * ORLGroupMember( orl_group_handle orl_group_hnd, orl_table_index idx )
{
#ifdef ORL_ENABLE_OMF
    if( LCL_GRP_HND( orl_group_hnd )->type == ORL_OMF )
        return OmfGroupMember( (omf_grp_handle)orl_group_hnd, idx );
#endif
    return NULL;
}
