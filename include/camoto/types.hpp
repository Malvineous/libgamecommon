/**
 * @file   types.hpp
 * @brief  Various types used across the libraries.
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _CAMOTO_TYPES_HPP_
#define _CAMOTO_TYPES_HPP_

#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <iostream>

/// Main namespace
namespace camoto {

/// Shared pointer to an iostream
typedef boost::shared_ptr<std::iostream> iostream_sptr;

/// Truncate function callback (to truncate an iostream)
/**
 * @see gamearchive::Archive::fnTruncate
 */
typedef boost::function<void(unsigned long)> FN_TRUNCATE;

} // namespace camoto

/**

\mainpage libgamecommon

libgamecommon provides a number of utility functions for working with binary
files.

\section structure Classes

<ul>
  <li>
    segmented_stream - transparently add and remove chunks of data in the
    middleof a C++ iostream
  </li><li>
    substream - make a C++ iostream appear as a subsection of a larger stream
  </li>
</ul>

\section example Examples

\subsection endian Reading endian-specific data

@code
std::fstream myfile("test.dat");
uint16_t value;

// Read a 16-bit (two byte) little-endian integer
myfile >> u16le(value);

// Same again but value in file is in big-endian instead
myfile >> u16be(value);

// Reading multiple values
uint32_t value;
uint8_t byte;
myfile >> u32le(value) >> u8(byte);
@endcode

\subsection substream Creating a substream

@code
std::fstream myfile("test.dat");

// Create a stream starting 10 bytes into test.dat and 15 bytes long.
camoto::substream sub(myfile, 10, 15);

sub.seekp(0);
sub.write(...); // data is written at offset 10 in test.dat
@endcode

\subsection substream Creating a segmented_stream

See the segmented_stream test code.

\section info More information

Additional information including a mailing list is available from the Camoto
homepage <http://www.shikadi.net/camoto>.

**/

#endif // _CAMOTO_TYPES_HPP_
