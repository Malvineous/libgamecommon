/**
 * @file  camoto/util.hpp
 * @brief Utility functions used across the libraries.
 *
 * Copyright (C) 2010-2015 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _CAMOTO_UTIL_HPP_
#define _CAMOTO_UTIL_HPP_

#include <iostream>
#include <sstream>

#define TOSTRING_X(x)  #x
#define TOSTRING(x)    TOSTRING_X(x)

#if (__cplusplus < 201300L)
#include <memory>
namespace std {

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args)
{
	return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}; // namespace std
#endif

/// Main namespace
namespace camoto {

/// String creation macro (with iostream syntax)
/**
 * Useful for assigning strings in constructor initialisation lists.  Note this
 * function is wrapped around a macro called createString() which should be used
 * instead.
 *
 * @code
 * std::string str = createString("The value is " << iValue);
 * @endcode
 */
#define createString(a) \
	(static_cast<const std::ostringstream&>(std::ostringstream() << a).str())

/**

\mainpage libgamecommon

libgamecommon provides a number of utility functions for working with binary
files.

\section libgamecommon_structure Classes

<ul>
	<li>
		stream_seg - transparently add and remove chunks of data in the
		middle of a stream
	</li><li>
		stream_sub - make a C++ iostream appear as a subsection of a larger stream
	</li>
</ul>

\section libgamecommon_example Examples

\subsection libgamecommon_endian Reading endian-specific data

@code
using namespace camoto;

stream::file_sptr myfile(new stream::file());
myfile->open("test.dat");

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

\subsection libgamecommon_substream Creating a substream

@code
using namespace camoto;

stream::file_sptr myfile(new stream::file());
myfile->open("test.dat");

// Create a stream starting 10 bytes into test.dat and 15 bytes long.
stream::sub_sptr sub(new stream::sub());
sub->open(myfile, 10, 15, NULL);

sub->seekp(0, stream::start);
sub->write(...); // data is written at offset 10 in test.dat
@endcode

\subsection libgamecommon_segmented_stream Creating a segmented_stream

See the segmented_stream test code.

\section libgamecommon_info More information

Additional information is available from the Camoto homepage
<http://www.shikadi.net/camoto>.

**/

} // namespace camoto

#endif // _CAMOTO_UTIL_HPP_
