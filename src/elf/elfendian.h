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
* Description:  Top level ELF loading code, parses ELF headers.
*
****************************************************************************/

#ifndef ELF_ENDIAN_INCLUDED
#define ELF_ENDIAN_INCLUDED
#pragma once

/* Taken from Watcom.h: */

/*  Macros for little/big endian conversion; These exist to simplify writing
 *  code that handles both little and big endian data on either little or big
 *  endian host platforms. Some of these macros could be implemented as inline
 *  assembler where instructions to byte swap data in registers or read/write
 *  memory access with byte swapping is available.
 *
 *  NOTE:   The SWAP_XX macros will swap data in place. If you only want to take a 
 *          a copy of the data and leave the original intact, then use the SWAPNC_XX 
 *          macros.
 */
#define SWAPNC_16(w)    (\
                            (((w) & 0x000000FFUL) << 8) |\
                            (((w) & 0x0000FF00UL) >> 8)\
                        )
#define SWAPNC_32(w)    (\
                            (((w) & 0x000000FFUL) << 24) |\
                            (((w) & 0x0000FF00UL) << 8) |\
                            (((w) & 0x00FF0000UL) >> 8) |\
                            (((w) & 0xFF000000UL) >> 24)\
                        )
#define SWAPNC_64(w)    (\
                            (((w) & 0x00000000000000FFULL) << 56) |\
                            (((w) & 0x000000000000FF00ULL) << 40) |\
                            (((w) & 0x0000000000FF0000ULL) << 24) |\
                            (((w) & 0x00000000FF000000ULL) << 8) |\
                            (((w) & 0x000000FF00000000ULL) >> 8) |\
                            (((w) & 0x0000FF0000000000ULL) >> 24) |\
                            (((w) & 0x00FF000000000000ULL) >> 40) |\
                            (((w) & 0xFF00000000000000ULL) >> 56)\
) 

#if defined( __BIG_ENDIAN__ )
    /* Macros to get little endian data */
    #define GET_LE_16(w)    SWAPNC_16(w)
    #define GET_LE_32(w)    SWAPNC_32(w)
    #define GET_LE_64(w)    SWAPNC_64(w)
    /* Macros to get big endian data */
    #define GET_BE_16(w)    (w)
    #define GET_BE_32(w)    (w)
    #define GET_BE_64(w)    (w)
    /* Macros to convert little endian data in place */
    #define CONV_LE_16(w)   (w) = SWAPNC_16(w)
    #define CONV_LE_32(w)   (w) = SWAPNC_32(w)
    #define CONV_LE_64(w)   (w) = SWAPNC_64(w)
    /* Macros to convert big endian data in place */
    #define CONV_BE_16(w)
    #define CONV_BE_32(w)
    #define CONV_BE_64(w)
    /* Macros to swap byte order */
    #define SWAP_16     CONV_LE_16
    #define SWAP_32     CONV_LE_32
    #define SWAP_64     CONV_LE_64
#else
    /* Macros to get little endian data */
    #define GET_LE_16(w)    (w)
    #define GET_LE_32(w)    (w)
    #define GET_LE_64(w)    (w)
    /* Macros to get big endian data */
    #define GET_BE_16(w)    SWAPNC_16(w)
    #define GET_BE_32(w)    SWAPNC_32(w)
    #define GET_BE_64(w)    SWAPNC_64(w)
    /* Macros to convert little endian data in place */
    #define CONV_LE_16(w)
    #define CONV_LE_32(w)
    #define CONV_LE_64(w)
    /* Macros to convert big endian data in place */
    #define CONV_BE_16(w)   (w) = SWAPNC_16(w)
    #define CONV_BE_32(w)   (w) = SWAPNC_32(w)
    #define CONV_BE_64(w)   (w) = SWAPNC_64(w)
    /* Macros to swap byte order */
    #define SWAP_16     CONV_BE_16
    #define SWAP_32     CONV_BE_32
    #define SWAP_64     CONV_BE_64
#endif

#define SCONV_BE_16 CONV_BE_16
#define SCONV_LE_16 CONV_LE_16
#define SCONV_BE_32 CONV_BE_32
#define SCONV_LE_32 CONV_LE_32
#define SCONV_BE_64 CONV_BE_64
#define SCONV_LE_64 CONV_LE_64

/* End Watcom.h thievery */

#endif
