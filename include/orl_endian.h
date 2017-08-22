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
* Description: Endian macros for ORL, taken from libwatcom
*
****************************************************************************/

#ifndef ORL_ENDIAN_INCLUDED
#define ORL_ENDIAN_INCLUDED
#pragma once

#if defined( __BIG_ENDIAN__ )
    #define I64LO32     1
    #define I64HI32     0
    #define I64LO16     3
    #define I64HI16     0
    #define I64LO8      7
    #define I64HI8      0
#else
    #define I64LO32     0
    #define I64HI32     1
    #define I64LO16     0
    #define I64HI16     3
    #define I64LO8      0
    #define I64HI8      7
#endif

#endif
