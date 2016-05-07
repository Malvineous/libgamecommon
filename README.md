Camoto: Classic-game Modding Tools
==================================
Copyright 2010-2016 Adam Nielsen <<malvineous@shikadi.net>>  
<http://www.shikadi.net/camoto/>  
Linux/OSX: [![Build Status (Linux/OSX)](https://travis-ci.org/Malvineous/libgamecommon.svg?branch=master)](https://travis-ci.org/Malvineous/libgamecommon)
Windows: [![Build Status (Windows)](https://ci.appveyor.com/api/projects/status/aw7kjrx99iac8dgo?svg=true)](https://ci.appveyor.com/project/Malvineous/libgamecommon)

Camoto is a collection of utilities for editing (modding) "classic" PC
games - those running under MS-DOS from the 1980s and 1990s.

This is **libgamecommon**, the lowest-level component of the Camoto suite.  It
contains shared code and routines used by most of the other Camoto libraries.

It is most likely of little use to anyone other than Camoto developers,
although of course it is a prerequisite to installing the rest of the Camoto
suite!

The library currently contains the following elements:

  - **stream**: Replacement for C++ iostream, all other stream elements
    implement this interface.  Also includes **stream\_file** and
    **stream\_string**.

  - **stream\_sub**: Access a subsection of another stream, transparently to
    the user of the class instance.

  - **stream\_seg**: A stream allowing data to be inserted and removed at
    arbitrary positions within the stream, and the resulting on-disk data
    shuffling only happening once, at `flush()`.

  - **stream\_filtered**: Transparently filter data read from and written to
    the stream.  Filters can compress/decompress, encrypt/decrypt, etc.

  - **lzss**: Generic implementation of the LZSS compression algorithm,
    implemented in the form of a filter (suitable for use with
    **stream_filtered**.)

  - **lzw**: Generic implementation of the LZW compression algorithm, again
    implemented as a stream filter.

  - **bitstream**: Stream-like interface allowing reading and writing of
    individual bits.  Used by the LZW algorithm which initially reads data in
    units of 9-bits.

  - **iostream\_helpers**: Helper classes to simplify reading and writing data
    in a platform neutral manner.  `stream << u32le(123)` will write the number
    123 as a 32-bit unsigned little-endian integer regardless of the
    endian-ness of the host platform, and likewise
    `uint16_t i; stream >> u16be(i);` will read a 16-bit big-endian integer
    from the stream.

Each element contains a number of tests to confirm it is working as expected,
and these are run in the usual manner: `make check`

The library is compiled and installed in the usual way:

    ./configure && make
    sudo make install
    sudo ldconfig

If you downloaded the git release, run `./autogen.sh` before the commands
above.

This library is released under the GPLv3 license.
