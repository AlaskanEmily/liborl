liborl
======

Watcom Object Reader Library
----------------------------

ORL is the Watcom Object Reader Library, which supports reading OMF objects,
COFF (including PE) and ELF objects, executables, and libraries. ORL is open
source under the Sybase license 1.0, with portions under a BSD-like license.

ORL was used in the Watcom linker and debugger. It's similar to libbfd in the
GNU toolchain, but supports just a OMF, PE/COFF, and ELF.

This version of ORL has had the libwatcom dependencies removed, and several
watcom libc components replaced with modified versions of similar functions
from OpenBSD's libc. It compiles and functions using GCC, Clang, and Visual
Studio in addition to Watcom. Some anachronisms have also been updated, and
support for some features of COFF that MSVC lacks have been reenabled.

A number of shortcomings of the Watcom release have also been fixed, including
the ability to load GCC-generated ELF files.

Support for OMF, COFF, and ELF can individually be toggled on and off. The API
remains the same, but attempting to read or write such a file will now produce
an error.
