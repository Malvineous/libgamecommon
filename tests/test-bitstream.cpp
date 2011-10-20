/**
 * @file   test-bitstream.cpp
 * @brief  Test code for bitstream class.
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

#include <boost/test/unit_test.hpp>

#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <boost/bind.hpp>
#include <iostream>
#include <iomanip>
#include <vector>

#include <camoto/debug.hpp>
#include <camoto/bitstream.hpp>
#include <camoto/stream_string.hpp>

#include "tests.hpp"

// mingw32 fix
#ifndef __STRING
#define __STRING(x) #x
#endif

using namespace camoto;

// These are the various numbers that are read/written at different bit
// lengths from/to the base data.

#define DATA_BYTES "\x12\x34\x56\x78\x9a"

int values_8le[] = {0x12, 0x34, 0x56, 0x78, 0x9a};

int values_8be[] = {0x12, 0x34, 0x56, 0x78, 0x9a};
#define PAD8 // Unused

// 0x12       0x34       0x56       0x78       0x9a
// 0001 0010  0011 0100  0101 0110  0111 1000  1001 1010
int values_4le[] = {0x2, 0x1, 0x4, 0x3, 0x6, 0x5, 0x8, 0x7, 0xa, 0x9};

// 0x12       0x34       0x56       0x78       0x9a
// 0001 0010  0011 0100  0101 0110  0111 1000  1001 1010
int values_4be[] = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa};
#define PAD4  // Unused

// 0x12       0x34       0x56       0x78       0x9a
// 0001 0010  0011 0100  0101 0110  0111 1000  1001 1010
int values_12le[] = {0x412, 0x563, 0xa78, 0x9};

// 0x12       0x34       0x56       0x78       0x9a
// 0001 0010  0011 0100  0101 0110  0111 1000  1001 1010 0000 0000
int values_12be[] = {0x123, 0x456, 0x789, 0xa00};
#define PAD12 "\x00"

// 0x12       0x34       0x56       0x78       0x9a
// 0001 0010  0011 0100  0101 0110  0111 1000  1001 1010
// 0 0001 0010  1 0001 1010  0 0001 0101  1 0100 1111  1001
// 0 0001 0010  01 011 0100  011 01 0110  1001 1 1000  1010
// 0x012        0x11a        0x15         0x14f        0x9
int values_9le[] = {0x012, 0x11a, 0x15, 0x14f, 0x9};

// 0x12       0x34       0x56       0x78       0x9a
// 0001 0010  0011 0100  0101 0110  0111 1000  1001 1010
// 0.001 0.010 0  0.11 01.00 01  0.1 011.0 011  1 1000 1001  1 0100 0000
// 0x024          0x0d1          0x0b3          0x189        0x140
int values_9be[] = {0x024, 0x0d1, 0x0b3, 0x189, 0x140};
#define PAD9  "\x00"

// 0x12       0x34       0x56       0x78       0x9a
// 0 0011 0100 0001 0010  1 0011 1100 0010 1011    10 0110
int values_17le[] = {0x03412, 0x13c2b, 0x26};

// 0x12       0x34       0x56       0x78       0x9a
// 0 0010 0100 0110 1000  1 0101 1001 1110 0010  0 1101 0000 0000 0000
int values_17be[] = {0x02468, 0x159e2, 0x0d000};
#define PAD17 "\x00\x00"  /// Extra bytes we don't want to pass to the read code, but the write code will output

typedef std::vector<int> intvector;

struct bitstream_read_sample: public default_sample {

	stream::string_sptr base;
	bitstream_sptr bit;
	intvector result;

	bitstream_read_sample()
		: base(new stream::string())
	{
		this->base << DATA_BYTES;
		this->bit.reset(new bitstream(this->base, bitstream::littleEndian));

		// Make sure the data went in correctly to begin the test
		BOOST_REQUIRE(this->base->str().compare(DATA_BYTES) == 0);
	}

	void printNice(boost::test_tools::predicate_result& res,
		const intvector& s, const intvector& diff)
	{
		const char *c = CLR_YELLOW;
		res.message() << c;
		for (int i = 0; i < s.size(); i++) {
			if ((i > 0) && (i % 16 == 0)) {
				res.message() << CLR_NORM << "\n" << std::setfill('0') << std::setw(3)
					<< std::hex << i << ": " << c;
			}
			if ((i >= diff.size()) || (s[i] != diff[i])) {
				if (c != CLR_MAG) {
					c = CLR_MAG;
					res.message() << CLR_MAG;
				}
			} else {
				if (c != CLR_YELLOW) {
					c = CLR_YELLOW;
					res.message() << CLR_YELLOW;
				}
			}
			res.message() << "\\x" << std::setfill('0') << std::setw(2)
				<< std::hex << s[i];
		}
		return;
	}


	void print_wrong(boost::test_tools::predicate_result& res,
		const intvector& expected, const intvector& result)
	{
		res.message() << "\nExp: ";
		this->printNice(res, expected, result);
		res.message() << CLR_NORM "\n" << "Got: ";
		this->printNice(res, result, expected);
		res.message() << CLR_NORM;

		return;
	}

	boost::test_tools::predicate_result is_equal(const intvector& expected)
	{
		if (this->result.size() != expected.size()) {
			boost::test_tools::predicate_result res(false);
			this->print_wrong(res, expected, this->result);
			return res;
		}
		intvector::iterator i = this->result.begin();
		for (intvector::const_iterator o = expected.begin(); o != expected.end();
			o++, i++
		) {
			if (*i != *o) {
				boost::test_tools::predicate_result res(false);
				this->print_wrong(res, expected, this->result);
				return res;
			}
		}
		return true;
	}

};

struct bitstream_write_sample: public default_sample {

	stream::string_sptr base;
	bitstream_sptr bit;
	intvector result;

	bitstream_write_sample()
		: base(new stream::string())
	{
		this->bit.reset(new bitstream(this->base, bitstream::littleEndian));
	}

};

#define make_vector(d)  make_vector_n((d), sizeof(d) / sizeof(int))
intvector make_vector_n(const int *v, int n)
{
	intvector out;
	for (int i = 0; i < n; i++) {
		out.push_back(v[i]);
	}
	return out;
}

BOOST_FIXTURE_TEST_SUITE(bitstream_read_suite, bitstream_read_sample)


#define READ_BITS(n) \
{ \
	int val, b; \
	while ((b = bit->read(n, &val)) == n) { \
		this->result.push_back(val); \
	} \
	if (b > 0) this->result.push_back(val); \
}

#define TEST_LE(n) \
BOOST_AUTO_TEST_CASE(bitstream_read_ ## n ## bit_le) \
{ \
	BOOST_TEST_MESSAGE("Read " __STRING(n) "-bit LE values"); \
\
	READ_BITS(n); \
\
	BOOST_CHECK_MESSAGE(is_equal(make_vector(values_ ## n ## le)), \
		"Reading " __STRING(n) "-bit LE values failed"); \
}

#define TEST_BE(n) \
BOOST_AUTO_TEST_CASE(bitstream_read_ ## n ## bit_be) \
{ \
	BOOST_TEST_MESSAGE("Read " __STRING(n) "-bit BE values"); \
\
	bit.reset(new bitstream(this->base, bitstream::bigEndian)); \
\
	READ_BITS(n); \
\
	BOOST_CHECK_MESSAGE(is_equal(make_vector(values_ ## n ## be)), \
		"Reading " __STRING(n) "-bit BE values failed"); \
}

TEST_LE(8)
TEST_BE(8)
TEST_LE(4)
TEST_BE(4)
TEST_LE(12)
TEST_BE(12)
TEST_LE(9)
TEST_BE(9)
TEST_LE(17)
TEST_BE(17)

#undef TEST_LE
#undef TEST_BE

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(bitstream_write_suite, bitstream_write_sample)

#define WRITE_BITS(n, values) \
	for (int i = 0; i < sizeof(values) / sizeof(int); i++) { \
		bit->write(n, values[i]); \
	} \
	if ((n * sizeof(values) / sizeof(int)) % 8) { \
		/* Pad up to nearest byte */ \
		bit->write(8-(n * sizeof(values) / sizeof(int)) % 8, 0); \
	}

#define TEST_LE(n) \
BOOST_AUTO_TEST_CASE(bitstream_write_ ## n ## bit_le) \
{ \
	BOOST_TEST_MESSAGE("Write " __STRING(n) "-bit LE values"); \
\
	WRITE_BITS(n, values_ ## n ## le); \
	bit->flush(); \
\
	BOOST_CHECK_MESSAGE(is_equal(std::string(DATA_BYTES PAD ## n, sizeof(DATA_BYTES PAD ## n)-1), \
		this->base->str()), \
		"Writing " __STRING(n) "-bit LE values failed"); \
}

#define TEST_BE(n) \
BOOST_AUTO_TEST_CASE(bitstream_write_ ## n ## bit_be) \
{ \
	BOOST_TEST_MESSAGE("Write " __STRING(n) "-bit BE values"); \
\
	bit.reset(new bitstream(this->base, bitstream::bigEndian)); \
\
	WRITE_BITS(n, values_ ## n ## be); \
	bit->flush(); \
\
	BOOST_CHECK_MESSAGE(is_equal(std::string(DATA_BYTES PAD ## n, sizeof(DATA_BYTES PAD ## n)-1), \
		this->base->str()), \
		"Writing " __STRING(n) "-bit BE values failed"); \
}

TEST_LE(8)
TEST_BE(8)
TEST_LE(4)
TEST_BE(4)
TEST_LE(12)
TEST_BE(12)
TEST_LE(9)
TEST_BE(9)
TEST_LE(17)
TEST_BE(17)

BOOST_AUTO_TEST_CASE(bitstream_write_partial_byte)
{
	BOOST_TEST_MESSAGE("End write within a byte");

	// Write some dummy data to make sure the underlying stream is large enough,
	// and we're not trying to seek past EOF.
	this->base << "\xff";

	bit->changeEndian(bitstream::bigEndian);

	// Change the first four bits, but leave the remaining four alone.  The
	// remaining four will then need to be read, merged and written upon flush.
	bit->write(4, 0);

	bit->flush();

	BOOST_CHECK_MESSAGE(is_equal(std::string("\x0f", 1),
		this->base->str()),
		"End write within a byte failed");
}

BOOST_AUTO_TEST_CASE(bitstream_write_flush_partial_byte)
{
	BOOST_TEST_MESSAGE("Flush within a byte");

	// Write some dummy data to make sure the underlying stream is large enough,
	// and we're not trying to seek past EOF.
	this->base << "\x02";

	bit->changeEndian(bitstream::bigEndian);

	// Flush in the middle of the operation and make sure it doesn't affect
	// the stream position.
	bit->write(4, 0xd);
	bit->flush();

	BOOST_CHECK_MESSAGE(is_equal(std::string("\xd2", 1),
		this->base->str()),
		"Flush within a byte failed (flush didn't work)");

	bit->write(4, 0xd);
	bit->flush();

	BOOST_CHECK_MESSAGE(is_equal(std::string("\xdd", 1),
		this->base->str()),
		"Flush within a byte failed (flush affected stream pointer)");
}

BOOST_AUTO_TEST_CASE(bitstream_write_flushbyte)
{
	BOOST_TEST_MESSAGE("Flush within a byte multiple times");

	// Write some dummy data to make sure the underlying stream is large enough,
	// and we're not trying to seek past EOF.
	this->base->write("\xff\xff\x00\x00", 4);

	bit->changeEndian(bitstream::bigEndian);

	// Flush in the middle of the operation and make sure it doesn't affect
	// the stream position.
	bit->write(4, 0xd);
	bit->flushByte();
	bit->write(5, 0x1);
	bit->flushByte();
	bit->write(1, 0x1);
	bit->flushByte();
	bit->write(2, 0x3);
	bit->flush();
	BOOST_CHECK_MESSAGE(is_equal(std::string("\xd0\x08\x80\xc0", 4),
		this->base->str()),
		"Flush within a byte failed (flush didn't work)");
}

BOOST_AUTO_TEST_CASE(bitstream_write_flushbyte_over)
{
	BOOST_TEST_MESSAGE("Flush within a byte after writing more than one byte");

	// Write some dummy data to make sure the underlying stream is large enough,
	// and we're not trying to seek past EOF.
	this->base->write("\xff\xff\x00\x00", 4);

	bit->changeEndian(bitstream::bigEndian);

	// Flush in the middle of the operation and make sure it doesn't affect
	// the stream position.
	bit->write(4, 0xd);
	bit->flushByte();
	bit->write(10, 0x3ff);
	bit->flushByte();
	bit->write(4, 0xd);
	bit->flush();
	BOOST_CHECK_MESSAGE(is_equal(std::string("\xd0\xff\xc0\xd0", 4),
		this->base->str()),
		"Flush within a byte failed (flush didn't work)");
}

BOOST_AUTO_TEST_CASE(bitstream_write_peek_bigendian)
{
	BOOST_TEST_MESSAGE("Peek at buffered byte (big endian)");

	// Write some dummy data to make sure the underlying stream is large enough,
	// and we're not trying to seek past EOF.
	this->base->write("\xff\xff\x00\x00\x00", 5);

	bit->changeEndian(bitstream::bigEndian);

	uint8_t next, mask;

	// Flush in the middle of the operation and make sure it doesn't affect
	// the stream position.
	bit->write(4, 0xd);

	bit->peekByte(&next, &mask);
	BOOST_REQUIRE_EQUAL(next, 0xd0);
	BOOST_REQUIRE_EQUAL(mask, 0xf0);

	bit->flushByte();
	bit->write(10, 0x3ff);

	bit->peekByte(&next, &mask);
	BOOST_REQUIRE_EQUAL(next, 0xc0);
	BOOST_REQUIRE_EQUAL(mask, 0xc0);

	bit->write(6, 0x3f);

	bit->peekByte(&next, &mask);
	BOOST_REQUIRE_EQUAL(next, 0x00);
	BOOST_REQUIRE_EQUAL(mask, 0x00);

	bit->write(1, 0x01);
	bit->flushByte();
	BOOST_REQUIRE_EQUAL(next, 0x00);
	BOOST_REQUIRE_EQUAL(mask, 0x00);
}

BOOST_AUTO_TEST_CASE(bitstream_write_peek_littleendian)
{
	BOOST_TEST_MESSAGE("Peek at buffered byte (little endian)");

	// Write some dummy data to make sure the underlying stream is large enough,
	// and we're not trying to seek past EOF.
	this->base->write("\xff\xff\x00\x00\x00", 5);

	bit->changeEndian(bitstream::littleEndian);

	uint8_t next, mask;

	// Flush in the middle of the operation and make sure it doesn't affect
	// the stream position.
	bit->write(4, 0xd);

	bit->peekByte(&next, &mask);
	BOOST_REQUIRE_EQUAL(next, 0x0d);
	BOOST_REQUIRE_EQUAL(mask, 0x0f);

	bit->flushByte();
	bit->write(10, 0x3ff);

	bit->peekByte(&next, &mask);
	BOOST_REQUIRE_EQUAL(next, 0x03);
	BOOST_REQUIRE_EQUAL(mask, 0x03);

	bit->write(6, 0x3f);

	bit->peekByte(&next, &mask);
	BOOST_REQUIRE_EQUAL(next, 0x00);
	BOOST_REQUIRE_EQUAL(mask, 0x00);

	bit->write(1, 0x01);
	bit->flushByte();
	BOOST_REQUIRE_EQUAL(next, 0x00);
	BOOST_REQUIRE_EQUAL(mask, 0x00);
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_FIXTURE_TEST_SUITE(bitstream_seek_suite, bitstream_read_sample)

#define TEST_SEEK(b, d, o) \
BOOST_AUTO_TEST_CASE(bitstream_seek_ ## b ## d ## o) \
{ \
	BOOST_TEST_MESSAGE("Seek to " __STRING(o) "@" __STRING(d) " - " __STRING(b) "-bit"); \
\
	int dummy; \
	bit->read(8+3, &dummy); \
\
	this->bit->seek(o, stream::d); \
	READ_BITS(b); \
\
	BOOST_CHECK_MESSAGE(is_equal(make_vector(values_ ## b ## d ## o)), \
		"Seek to " __STRING(o) "@" __STRING(d) " - " __STRING(b) "-bit failed"); \
}

#define TEST_NEG_SEEK(b, d, o) \
BOOST_AUTO_TEST_CASE(bitstream_seek_ ## b ## d ## neg ## o) \
{ \
	BOOST_TEST_MESSAGE("Seek to -" __STRING(o) "@" __STRING(d) " - " __STRING(b) "-bit"); \
\
	int dummy; \
	bit->read(8+3, &dummy); \
\
	this->bit->seek(-o, stream::d); \
	READ_BITS(b); \
\
	BOOST_CHECK_MESSAGE(is_equal(make_vector(values_ ## b ## d ## neg ## o)), \
		"Seek to -" __STRING(o) "@" __STRING(d) " - " __STRING(b) "-bit failed"); \
}

int values_8start16[] = {0x56, 0x78, 0x9a};
TEST_SEEK(8, start, 16);

int values_8start32[] = {0x9a};
TEST_SEEK(8, start, 32);

// Start at the offset of 3 in the test, seek +5, end up at byte offset 1
int values_8cur5[] = {0x56, 0x78, 0x9a};
TEST_SEEK(8, cur, 5);

int values_8cur13[] = {0x78, 0x9a};
TEST_SEEK(8, cur, 13);

int values_8curneg3[] = {0x34, 0x56, 0x78, 0x9a};
TEST_NEG_SEEK(8, cur, 3);

int values_8curneg11[] = {0x12, 0x34, 0x56, 0x78, 0x9a};
TEST_NEG_SEEK(8, cur, 11);

int values_8endneg16[] = {0x78, 0x9a};
TEST_NEG_SEEK(8, end, 16);

int values_8endneg4[] = {0x9};
TEST_NEG_SEEK(8, end, 4);

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(bitstream_rwseek_suite, bitstream_write_sample)

BOOST_AUTO_TEST_CASE(bitstream_rw_1bit)
{
	BOOST_TEST_MESSAGE("Read/write within a byte in 1-bit stream");

	// Write some dummy data to make sure the underlying stream is large enough,
	// and we're not trying to seek past EOF.
	this->base << "\x90";

	bit->changeEndian(bitstream::bigEndian);

	int val = 0;

	bit->read(1, &val);
	BOOST_REQUIRE_EQUAL(val, 0x1);

	bit->write(2, 0);

	bit->read(1, &val);
	BOOST_REQUIRE_EQUAL(val, 0x1);

	bit->write(4, 0xf);

	bit->flush();

	BOOST_CHECK_MESSAGE(is_equal(std::string("\x9f", 1),
		this->base->str()),
		"Read/write within a byte in 1-bit stream failed");
}

BOOST_AUTO_TEST_CASE(bitstream_rwseek_8bit)
{
	BOOST_TEST_MESSAGE("Read/write/seek in 8-bit stream");

	// Write some dummy data to make sure the underlying stream is large enough,
	// and we're not trying to seek past EOF.
	bit->write(8, 0xff);
	bit->write(8, 0xfe);
	bit->write(8, 0xdc);
	bit->write(8, 0xba);
	bit->write(8, 0x98);

	int val = 0;
	int p = this->bit->seek(8, stream::start);
	BOOST_REQUIRE_EQUAL(p, 8);
	bit->read(8, &val);
	BOOST_REQUIRE_EQUAL(val, 0xfe);

	p = this->bit->seek(0, stream::start);
	BOOST_REQUIRE_EQUAL(p, 0);
	bit->write(8, 0x12);

	p = this->bit->seek(32, stream::start);
	BOOST_REQUIRE_EQUAL(p, 32);
	bit->write(8, 0x9a);

	p = this->bit->seek(16, stream::start);
	BOOST_REQUIRE_EQUAL(p, 16);
	bit->write(8, 0x56);

	p = this->bit->seek(8, stream::start);
	BOOST_REQUIRE_EQUAL(p, 8);
	bit->write(8, 0x34);

	p = this->bit->seek(24, stream::start);
	BOOST_REQUIRE_EQUAL(p, 24);
	bit->write(8, 0x78);

	p = this->bit->seek(8, stream::start);
	BOOST_REQUIRE_EQUAL(p, 8);
	bit->read(8, &val);
	BOOST_REQUIRE_EQUAL(val, 0x34);

	bit->flush();

	BOOST_CHECK_MESSAGE(is_equal(std::string(DATA_BYTES, sizeof(DATA_BYTES)-1),
		this->base->str()),
		"Read/write/seek in 8-bit stream failed");
}

BOOST_AUTO_TEST_CASE(bitstream_rwseek_9bit)
{
	BOOST_TEST_MESSAGE("Read/write/seek in 9-bit stream");

	// Write some dummy data to make sure the underlying stream is large enough,
	// and we're not trying to seek past EOF.
	bit->write(9, 0x1ff);
	bit->write(9, 0x1fe);
	bit->write(9, 0x1dc);
	bit->write(9, 0x1ba);
	bit->write(4, 0x3);

	int val = 0;
	int p = this->bit->seek(9, stream::start);
	BOOST_REQUIRE_EQUAL(p, 9);
	bit->read(9, &val);
	BOOST_REQUIRE_EQUAL(val, 0x1fe);

	p = this->bit->seek(0, stream::start);
	BOOST_REQUIRE_EQUAL(p, 0);
	bit->write(9, 0x012);

	p = this->bit->seek(36, stream::start);
	BOOST_REQUIRE_EQUAL(p, 36);
	bit->write(4, 0x9);

	p = this->bit->seek(18, stream::start);
	BOOST_REQUIRE_EQUAL(p, 18);
	bit->write(9, 0x015);

	p = this->bit->seek(9, stream::start);
	BOOST_REQUIRE_EQUAL(p, 9);
	bit->write(9, 0x11a);

	p = this->bit->seek(27, stream::start);
	BOOST_REQUIRE_EQUAL(p, 27);
	bit->write(9, 0x14f);

	p = this->bit->seek(9, stream::start);
	BOOST_REQUIRE_EQUAL(p, 9);
	bit->read(9, &val);
	BOOST_REQUIRE_EQUAL(val, 0x11a);

	bit->flush();

	BOOST_CHECK_MESSAGE(is_equal(std::string(DATA_BYTES, sizeof(DATA_BYTES)-1),
		this->base->str()),
		"Read/write/seek in 9-bit stream failed");
}

BOOST_AUTO_TEST_CASE(bitstream_rwseek_1bit)
{
	BOOST_TEST_MESSAGE("Read/write/seek in 1-bit stream");

	// Write some dummy data to make sure the underlying stream is large enough,
	// and we're not trying to seek past EOF.
	bit->write(32, 0xffffffff);
	bit->write(8, 0xff);

	int val = 0;
	int p = this->bit->seek(0, stream::start);
	BOOST_REQUIRE_EQUAL(p, 0);
	bit->write(1, 0);

	bit->read(1, &val);
	BOOST_REQUIRE_EQUAL(val, 0x1);

	bit->write(2, 0);

	bit->read(1, &val);
	BOOST_REQUIRE_EQUAL(val, 0x1);

	bit->write(5, 0);

	bit->read(1, &val);
	BOOST_REQUIRE_EQUAL(val, 0x1);

	bit->write(1, 0);

	bit->read(2, &val);
	BOOST_REQUIRE_EQUAL(val, 0x3);

	bit->write(3, 0);

	bit->read(2, &val);
	BOOST_REQUIRE_EQUAL(val, 0x3);


	bit->write(1, 0);

	bit->read(1, &val);
	BOOST_REQUIRE_EQUAL(val, 0x1);

	bit->write(1, 0);

	bit->read(1, &val);
	BOOST_REQUIRE_EQUAL(val, 0x1);

	bit->write(4, 0);

	bit->read(4, &val);
	BOOST_REQUIRE_EQUAL(val, 0xf);

	bit->write(2, 0);

	bit->read(1, &val);
	BOOST_REQUIRE_EQUAL(val, 0x1);

	bit->write(1, 0);

	bit->read(2, &val);
	BOOST_REQUIRE_EQUAL(val, 0x3);

	bit->write(2, 0);

	bit->read(1, &val);
	BOOST_REQUIRE_EQUAL(val, 0x1);

	bit->flush();

	BOOST_CHECK_MESSAGE(is_equal(std::string(DATA_BYTES, sizeof(DATA_BYTES)-1),
		this->base->str()),
		"Read/write/seek in 1-bit stream failed");
}

int putNextChar(stream::output_sptr src, uint8_t out)
	throw (stream::write_error)
{
	return src->try_write(&out, 1);
}

BOOST_AUTO_TEST_CASE(bitstream_writeonly)
{
	BOOST_TEST_MESSAGE("Write only without stream, make sure correct bytes come out");

	bit.reset(new bitstream(bitstream::bigEndian));

	fn_putnextchar cbNext = boost::bind(putNextChar, this->base, _1);

	bit->write(cbNext, 1, 0);
	bit->write(cbNext, 1, 0);
	bit->write(cbNext, 1, 0);
	bit->write(cbNext, 1, 1);
	bit->write(cbNext, 1, 1);
	bit->write(cbNext, 1, 0);
	bit->write(cbNext, 1, 0);
	bit->write(cbNext, 1, 0);

	bit->write(cbNext, 1, 1);
	bit->write(cbNext, 1, 1);
	bit->write(cbNext, 1, 1);
	bit->write(cbNext, 1, 0);
	bit->write(cbNext, 1, 0);
	bit->write(cbNext, 1, 1);
	bit->write(cbNext, 1, 1);
	bit->write(cbNext, 1, 1);

	bit->flushByte(cbNext);

	BOOST_CHECK_MESSAGE(is_equal(std::string("\x18\xE7", 2),
		this->base->str()),
		"Write only without stream failed");
}

BOOST_AUTO_TEST_CASE(bitstream_write_partial)
{
	BOOST_TEST_MESSAGE("Write only without stream, partial");

	bit.reset(new bitstream(bitstream::bigEndian));

	fn_putnextchar cbNext = boost::bind(putNextChar, this->base, _1);

	bit->write(cbNext, 1, 0);
	bit->write(cbNext, 1, 0);
	bit->write(cbNext, 1, 0);
	bit->write(cbNext, 1, 1);
	bit->write(cbNext, 1, 1);
	bit->write(cbNext, 1, 0);
	bit->write(cbNext, 1, 0);
	bit->write(cbNext, 1, 0);

	bit->write(cbNext, 1, 1);
	bit->write(cbNext, 1, 1);
	bit->write(cbNext, 1, 1);
	bit->write(cbNext, 1, 0);
	bit->write(cbNext, 1, 0);
	bit->write(cbNext, 1, 1);

	bit->flushByte(cbNext);

	BOOST_CHECK_MESSAGE(is_equal(std::string("\x18\xE4", 2),
		this->base->str()),
		"Write partial without stream failed");
}

BOOST_AUTO_TEST_SUITE_END()
