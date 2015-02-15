/**
 * @file  camoto/byteorder.hpp
 * @brief Standard C and C++ implementations of byte order functions.
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
 *
 **********************************
 *
 * This file aims to provide cross-platform functions for dealing with
 * endianness issues, as well as providing a few helper functions for making
 * code easier to read.  By default it is standard C, but certain preprocessor
 * tokens can be defined to include C++ enhancements.
 *
 * Just including this file with no #define statements will get you these
 * standard functions, which all take an endian-specific value and return the
 * host version of it.
 *
 * uint16_t be16toh(uint16_t x) - take big-endian 16-bit, return host
 * uint32_t be32toh(uint32_t x) - take big-endian 32-bit, return host
 * uint64_t be64toh(uint64_t x) - take big-endian 64-bit, return host
 *
 * uint16_t le16toh(uint16_t x) - take little-endian 16-bit, return host
 * uint32_t le32toh(uint32_t x) - take little-endian 32-bit, return host
 * uint64_t le64toh(uint64_t x) - take little-endian 64-bit, return host
 *
 * uint16_t htobe16(uint16_t x) - take host, return big-endian 16-bit
 * uint32_t htobe32(uint32_t x) - take host, return big-endian 32-bit
 * uint64_t htobe64(uint64_t x) - take host, return big-endian 64-bit
 *
 * uint16_t htole16(uint16_t x) - take host, return little-endian 16-bit
 * uint32_t htole32(uint32_t x) - take host, return little-endian 32-bit
 * uint64_t htole64(uint64_t x) - take host, return little-endian 64-bit
 *
 * Further #defines will provide additional functionality:
 *
 * #define BYTEORDER_USE_IOSTREAMS
 *   iostream functions will be available for reading and writing values to
 *   streams, for example:
 *
 *   // Write four bytes containing the little endian representation of 123
 *   file << u32le(123)
 *
 *   // Read four bytes, convert from little endian to host
 *   file >> u32le(var)
 *
 * #define BYTEORDER_PROVIDE_TYPED_FUNCTIONS
 *   Provide typed wrappers around the named functions, which are useful for
 *   generic programming, for example:
 *
 *   uint32_t var = host_from<uint32_t, little_endian>(123);
 *
 *   T var = host_from<T, E>(value);   // inside a template
 */

#ifndef _BYTEORDER_H_
#define _BYTEORDER_H_

#include <stdint.h>

#if defined(linux) || defined(__posix) || defined(le16toh) || defined(_BSD_SOURCE)

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif
#include <endian.h>

// Default OS (no specific functions)
#else

#define ___swab16(x) \
	((uint16_t)( \
		(((uint16_t)(x) & (uint16_t)0x00ffU) << 8) | \
		(((uint16_t)(x) & (uint16_t)0xff00U) >> 8) ))

#define ___swab32(x) \
	((uint32_t)( \
		(((uint32_t)(x) & (uint32_t)0x000000ffUL) << 24) | \
		(((uint32_t)(x) & (uint32_t)0x0000ff00UL) <<  8) | \
		(((uint32_t)(x) & (uint32_t)0x00ff0000UL) >>  8) | \
		(((uint32_t)(x) & (uint32_t)0xff000000UL) >> 24) ))

#define ___swab64(x) \
	((uint64_t)( \
		(uint64_t)(((uint64_t)(x) & (uint64_t)0x00000000000000ffULL) << 56) | \
		(uint64_t)(((uint64_t)(x) & (uint64_t)0x000000000000ff00ULL) << 40) | \
		(uint64_t)(((uint64_t)(x) & (uint64_t)0x0000000000ff0000ULL) << 24) | \
		(uint64_t)(((uint64_t)(x) & (uint64_t)0x00000000ff000000ULL) <<  8) | \
		(uint64_t)(((uint64_t)(x) & (uint64_t)0x000000ff00000000ULL) >>  8) | \
		(uint64_t)(((uint64_t)(x) & (uint64_t)0x0000ff0000000000ULL) >> 24) | \
		(uint64_t)(((uint64_t)(x) & (uint64_t)0x00ff000000000000ULL) >> 40) | \
		(uint64_t)(((uint64_t)(x) & (uint64_t)0xff00000000000000ULL) >> 56) ))

// Little endian
#if defined(WIN32) || defined(__WIN32__) || defined(__CYGWIN32__)

#define le16toh(x)  (x)
#define le32toh(x)  (x)
#define le64toh(x)  (x)

#define htole16(x)  (x)
#define htole32(x)  (x)
#define htole64(x)  (x)

#define be16toh(x)  ___swab16(x)
#define be32toh(x)  ___swab32(x)
#define be64toh(x)  ___swab64(x)

#define htobe16(x)  ___swab16(x)
#define htobe32(x)  ___swab32(x)
#define htobe64(x)  ___swab64(x)

// Big endian
#else

#define le16toh(x)  ___swab16(x)
#define le32toh(x)  ___swab32(x)
#define le64toh(x)  ___swab64(x)

#define htole16(x)  ___swab16(x)
#define htole32(x)  ___swab32(x)
#define htole64(x)  ___swab64(x)

#define be16toh(x)  (x)
#define be32toh(x)  (x)
#define be64toh(x)  (x)

#define htobe16(x)  (x)
#define htobe32(x)  (x)
#define htobe64(x)  (x)

#endif

#endif

// Create some strongly-typed functions that call the correct endian conversion
// routines based on the given C++ type.

#if defined BYTEORDER_PROVIDE_TYPED_FUNCTIONS || defined BYTEORDER_USE_IOSTREAMS

struct big_endian { };
struct little_endian { };

template <typename T, class E> T host_from(T value);
template <typename T, class E> T host_to(T value);

template <> inline  uint8_t host_from< uint8_t, little_endian>( uint8_t value) { return value; }
template <> inline  uint8_t host_to  < uint8_t, little_endian>( uint8_t value) { return value; }
template <> inline uint16_t host_from<uint16_t, little_endian>(uint16_t value) { return le16toh(value); }
template <> inline uint16_t host_to  <uint16_t, little_endian>(uint16_t value) { return htole16(value); }
template <> inline uint32_t host_from<uint32_t, little_endian>(uint32_t value) { return le32toh(value); }
template <> inline uint32_t host_to  <uint32_t, little_endian>(uint32_t value) { return htole32(value); }
template <> inline uint64_t host_from<uint64_t, little_endian>(uint64_t value) { return le64toh(value); }
template <> inline uint64_t host_to  <uint64_t, little_endian>(uint64_t value) { return htole64(value); }

template <> inline  uint8_t host_from< uint8_t, big_endian>( uint8_t value) { return value; }
template <> inline  uint8_t host_to  < uint8_t, big_endian>( uint8_t value) { return value; }
template <> inline uint16_t host_from<uint16_t, big_endian>(uint16_t value) { return be16toh(value); }
template <> inline uint16_t host_to  <uint16_t, big_endian>(uint16_t value) { return htobe16(value); }
template <> inline uint32_t host_from<uint32_t, big_endian>(uint32_t value) { return be32toh(value); }
template <> inline uint32_t host_to  <uint32_t, big_endian>(uint32_t value) { return htobe32(value); }
template <> inline uint64_t host_from<uint64_t, big_endian>(uint64_t value) { return be64toh(value); }
template <> inline uint64_t host_to  <uint64_t, big_endian>(uint64_t value) { return htobe64(value); }

template <> inline  int8_t host_from< int8_t, little_endian>( int8_t value) { return value; }
template <> inline  int8_t host_to  < int8_t, little_endian>( int8_t value) { return value; }
template <> inline int16_t host_from<int16_t, little_endian>(int16_t value) { return (int16_t)le16toh((uint16_t)value); }
template <> inline int16_t host_to  <int16_t, little_endian>(int16_t value) { return (int16_t)htole16((uint16_t)value); }
template <> inline int32_t host_from<int32_t, little_endian>(int32_t value) { return (int32_t)le32toh((uint32_t)value); }
template <> inline int32_t host_to  <int32_t, little_endian>(int32_t value) { return (int32_t)htole32((uint32_t)value); }
template <> inline int64_t host_from<int64_t, little_endian>(int64_t value) { return (int64_t)le64toh((uint64_t)value); }
template <> inline int64_t host_to  <int64_t, little_endian>(int64_t value) { return (int64_t)htole64((uint64_t)value); }

template <> inline  int8_t host_from< int8_t, big_endian>( int8_t value) { return value; }
template <> inline  int8_t host_to  < int8_t, big_endian>( int8_t value) { return value; }
template <> inline int16_t host_from<int16_t, big_endian>(int16_t value) { return (int16_t)be16toh((uint16_t)value); }
template <> inline int16_t host_to  <int16_t, big_endian>(int16_t value) { return (int16_t)htobe16((uint16_t)value); }
template <> inline int32_t host_from<int32_t, big_endian>(int32_t value) { return (int32_t)be32toh((uint32_t)value); }
template <> inline int32_t host_to  <int32_t, big_endian>(int32_t value) { return (int32_t)htobe32((uint32_t)value); }
template <> inline int64_t host_from<int64_t, big_endian>(int64_t value) { return (int64_t)be64toh((uint64_t)value); }
template <> inline int64_t host_to  <int64_t, big_endian>(int64_t value) { return (int64_t)htobe64((uint64_t)value); }

#endif // BYTEORDER_PROVIDE_TYPED_FUNCTIONS || BYTEORDER_USE_IOSTREAMS

#ifdef BYTEORDER_USE_IOSTREAMS

#ifndef BYTEORDER_ISTREAM
#include <iostream>
#define BYTEORDER_ISTREAM std::istream&
#define BYTEORDER_OSTREAM std::ostream&
#define BYTEORDER_BUFFER_TYPE char *
#define BYTEORDER_ACCESSOR .  // as opposed to ->
#endif

struct number_format_read {
	virtual void read(BYTEORDER_ISTREAM s) const = 0;
};
struct number_format_write {
	virtual void write(BYTEORDER_OSTREAM s) const = 0;
};

template <typename T, typename E, typename I>
struct number_format: public number_format_read, public number_format_write {
	number_format(I& r)
		: r(r)
	{
	}

	void read(BYTEORDER_ISTREAM s) const
	{
		T x = 0;
		s BYTEORDER_ACCESSOR read((BYTEORDER_BUFFER_TYPE)&x, sizeof(T));
		this->r = host_from<T, E>(x);
		return;
	}

	void write(BYTEORDER_OSTREAM s) const
	{
		T x = host_to<T, E>(this->r);
		s BYTEORDER_ACCESSOR write((BYTEORDER_BUFFER_TYPE)&x, sizeof(T));
		return;
	}

	private:
		I& r;
};

template <typename T, typename E, typename I>
struct number_format_const: public number_format_write {
	number_format_const(const I& r)
		: r(r)
	{
	}

	void write(BYTEORDER_OSTREAM s) const
	{
		T x = host_to<T, E>(this->r);
		s BYTEORDER_ACCESSOR write((BYTEORDER_BUFFER_TYPE)&x, sizeof(T));
		return;
	}

	private:
		const I& r;
};

// If you get an error related to the next line (e.g. no match for operator >>)
// it's because you're trying to read a value into a const variable.
inline BYTEORDER_ISTREAM operator >> (BYTEORDER_ISTREAM s, const number_format_read& n) {
	n.read(s);
	return s;
}

inline BYTEORDER_OSTREAM operator << (BYTEORDER_OSTREAM s, const number_format_write& n) {
	n.write(s);
	return s;
}

#define DEFINE_TYPE(TYPE, NAME) \
	template <typename I> \
	inline number_format<TYPE, little_endian, I> NAME ## le(I& r) \
	{ \
		return number_format<TYPE, little_endian, I>(r); \
	} \
	\
	template <typename I> \
	inline number_format<TYPE, big_endian, I> NAME ## be(I& r) \
	{ \
		return number_format<TYPE, big_endian, I>(r); \
	} \
	template <typename I> \
	inline number_format_const<TYPE, little_endian, I> NAME ## le(const I& r) \
	{ \
		return number_format_const<TYPE, little_endian, I>(r); \
	} \
	\
	template <typename I> \
	inline number_format_const<TYPE, big_endian, I> NAME ## be(const I& r) \
	{ \
		return number_format_const<TYPE, big_endian, I>(r); \
	}

DEFINE_TYPE(uint16_t, u16);
DEFINE_TYPE(uint32_t, u32);
DEFINE_TYPE(uint64_t, u64);
DEFINE_TYPE(int16_t, s16);
DEFINE_TYPE(int32_t, s32);
DEFINE_TYPE(int64_t, s64);

#endif // BYTEORDER_USE_IOSTREAMS

#endif // _BYTEORDER_H_
