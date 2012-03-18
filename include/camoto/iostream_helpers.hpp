/**
 * @file   iostream_helpers.hpp
 * @brief  Functions to get variables in and out of a stream.
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _CAMOTO_IOSTREAM_HELPERS_HPP_
#define _CAMOTO_IOSTREAM_HELPERS_HPP_

#include <iostream>
#include <exception>
#include <string.h>
#include <camoto/stream.hpp>

#define BYTEORDER_USE_IOSTREAMS
#define BYTEORDER_ISTREAM camoto::stream::input_sptr
#define BYTEORDER_OSTREAM camoto::stream::output_sptr
#define BYTEORDER_ACCESSOR ->
#define BYTEORDER_BUFFER_TYPE uint8_t *
#define BYTEORDER_PROVIDE_TYPED_FUNCTIONS
#include <camoto/byteorder.hpp>

namespace camoto {

/// @sa null_padded
struct null_padded_read {
	null_padded_read(std::string& r, stream::len len, bool chop);
	void read(stream::input_sptr s) const;

	private:
		std::string& r;
		stream::len len;
		bool chop;
};

/// @sa null_padded
struct null_padded_write {
	null_padded_write(const std::string& r, stream::len len);
	void write(stream::output_sptr s) const;

	private:
		const std::string& r;
		stream::len len;
};

/// @sa null_padded
struct null_padded_const: public null_padded_write {
	null_padded_const(const std::string& r, stream::len len);
};

/**
 * nullPadded will pad a string with nulls when writing it to a stream, e.g.
 *
 * @code
 * file << nullPadded("hello", 10);  // write 10 bytes, "hello" and five nulls
 * @endcode
 *
 * It is an error for the string ("hello") to be longer than the pad (10), this
 * will cause an assertion failure at runtime.
 *
 * It can also be used when reading from a stream, e.g.
 *
 * @code
 * file >> nullPadded(str, 10, true);
 * @endcode
 *
 * In this case 10 bytes will always be read, but they will only be stored in
 * str until the first null is encountered.  If there are no null bytes then
 * the final string will be the full 10 chars (std::string will provide the
 * null termination in this case.)
 *
 * Set the last parameter to false to avoid having the string chopped at the
 * first null.  In this example if there is a null at position 5, setting the
 * last parameter to true will read 10 bytes but return a string of length 4,
 * and setting the last parameter to false will also read 10 bytes but return
 * a string of 10 bytes too, with an embedded null.
 */
struct null_padded: public null_padded_read, public null_padded_write {
	null_padded(std::string& r, stream::len len, bool chop);
};

// If you get an error related to the next line (e.g. no match for operator >>)
// it's because you're trying to read a value into a const variable.
inline stream::input_sptr operator >> (stream::input_sptr s, const null_padded_read& n) {
	n.read(s);
	return s;
}

inline stream::output_sptr operator << (stream::output_sptr s, const null_padded_write& n) {
	n.write(s);
	return s;
}

inline null_padded nullPadded(std::string& r, int len)
{
	return null_padded(r, len, true);
}
inline null_padded_const nullPadded(const std::string& r, int len)
{
	return null_padded_const(r, len);
}
inline null_padded fixedLength(std::string& r, int len)
{
	return null_padded(r, len, false);
}

// null terminated strings

/// @sa null_terminated
struct null_terminated_read {
	null_terminated_read(std::string& r, stream::len len);
	void read(stream::input_sptr s) const;

	private:
		std::string& r;
		stream::len maxlen;
};

/// @sa null_terminated
struct null_terminated_write {
	null_terminated_write(const std::string& r, stream::len len);
	void write(stream::output_sptr s) const;

	private:
		const std::string& r;
		stream::len maxlen;
};

/// @sa null_terminated
struct null_terminated_const: public null_terminated_write {
	null_terminated_const(const std::string& r, stream::len len);
};

/**
 * nullTerminated will read a variable length null-terminated string from a
 * stream, e.g.
 *
 * @code
 * std::string str;
 * file >> nullTerminated(str, 256);
 * @endcode
 *
 * Here up to 256 characters will be read from the file.  If no terminating null
 * is encountered, the string will be 256 characters in length.
 *
 * It can also be used when writing to a stream, e.g.
 *
 * @code
 * std::string demo("hello");
 * file << nullTerminated(demo, 256);  // write 6 bytes, "hello\0"
 * @endcode
 *
 * In this case up to 256 bytes will be written, and there is guaranteed to be
 * a terminating null character.  If the string is longer than this length, it
 * is truncated (only 255 characters written) and a null will be placed as the
 * 256th character written.
 */
struct null_terminated: public null_terminated_read, public null_terminated_write {
	null_terminated(std::string& r, stream::len maxlen);
};

// If you get an error related to the next line (e.g. no match for operator >>)
// it's because you're trying to read a value into a const variable.
inline stream::input_sptr operator >> (stream::input_sptr s, const null_terminated_read& n) {
	n.read(s);
	return s;
}

inline stream::output_sptr operator << (stream::output_sptr s, const null_terminated_write& n) {
	n.write(s);
	return s;
}

inline null_terminated nullTerminated(std::string& r, int maxlen)
{
	return null_terminated(r, maxlen);
}
inline null_terminated_const nullTerminated(const std::string& r, int maxlen)
{
	return null_terminated_const(r, maxlen);
}

// uint8_t / byte iostream operators

struct number_format_u8: public number_format_read, public number_format_write {
	number_format_u8(uint8_t& r);
	void read(stream::input_sptr s) const;
	void write(stream::output_sptr s) const;

	private:
		uint8_t& r;
};

struct number_format_const_u8: public number_format_write {
	number_format_const_u8(const uint8_t& r);
	void write(stream::output_sptr s) const;

	private:
		const uint8_t& r;
};

inline number_format_u8 u8(uint8_t& r)
{
	return number_format_u8(r);
}
inline number_format_const_u8 u8(const uint8_t& r)
{
	return number_format_const_u8(r);
}

} // namespace camoto

#endif // _CAMOTO_IOSTREAM_HELPERS_HPP_
