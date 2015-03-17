/**
 * @file   test-lzw.cpp
 * @brief  Test code for LZW compression/decompression filters.
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

#include <boost/test/unit_test.hpp>

#include <camoto/lzw.hpp>
#include <camoto/bitstream.hpp>
#include <camoto/stream_filtered.hpp>
#include <camoto/util.hpp>

#include "tests.hpp"

using namespace camoto;

BOOST_FIXTURE_TEST_SUITE(lzw_decomp_suite, string_sample)

BOOST_AUTO_TEST_CASE(lzw_decomp_read)
{
	BOOST_TEST_MESSAGE("Decompress some LZW data");

	bitstream bit_in(this->in, bitstream::bigEndian);
	bit_in.write(9, 'H');
	bit_in.write(9, 'e');    // 0x101 -> He
	bit_in.write(9, 'l');    // 0x102 -> el
	bit_in.write(9, 'l');    // 0x103 -> ll
	bit_in.write(9, 'o');    // 0x104 -> lo
	bit_in.write(9, ' ');    // 0x105 -> o 
	bit_in.write(9, 'h');    // 0x106 ->  h
	bit_in.write(9, 0x102);  // 0x107 -> he
	bit_in.write(9, 0x104);  // 0x108 -> ell
	bit_in.write(9, 0x106);  // 0x109 -> lo
	bit_in.write(9, 0x108);  // 0x10a ->  he
	bit_in.write(9, 'o');
	bit_in.write(9, '.');
	bit_in.write(9, 0x100);
	bit_in.flush();

	auto processed = std::make_shared<stream::input_filtered>(
		this->in,
		std::make_shared<filter_lzw_decompress>(
			9, 9, 0x101, 0x100, 0, LZW_BIG_ENDIAN | LZW_EOF_PARAM_VALID
		)
	);

	stream::copy(this->out, *processed);

	BOOST_CHECK_MESSAGE(is_equal("Hello hello hello."),
		"Decompressing LZW data failed");
}

BOOST_AUTO_TEST_CASE(lzw_decomp_codeword_length)
{
	BOOST_TEST_MESSAGE("Confirm codeword length changes at correct position (eof+reset)");

	bitstream bit_in(this->in, bitstream::bigEndian);
	bit_in.write(9, 'A'); // first byte isn't assigned a codeword
	// finish at 0x1fd because 0x1fe is for reset and 0x1ff is for eof
	for (unsigned int i = 0x100; i < 0x200 - 2; i++) {
		bit_in.write(9, 'A');
	}
	bit_in.write(10, 'A'); // 256th 'A'
	bit_in.write(10, 'B'); // 257th char
	bit_in.write(10, 'B');
	bit_in.write(10, 'B');
	bit_in.write(10, 'B');
	bit_in.write(10, 0x3ff);  // EOF is max possible code

	auto processed = std::make_shared<stream::input_filtered>(
		this->in,
		std::make_shared<filter_lzw_decompress>(
			9, 14, 0x100,
			0, // EOF is max possible code
			-1, // Reset is max code minus one
			LZW_BIG_ENDIAN | LZW_EOF_PARAM_VALID | LZW_RESET_PARAM_VALID
		)
	);

	stream::copy(this->out, *processed);

	BOOST_CHECK_MESSAGE(is_equal(
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"AAAAAAAAAAAAAAAA"
		"BBBB"
	),
		"Decompressing LZW data failed");
}

BOOST_AUTO_TEST_CASE(lzw_decomp_bitlength_expand)
{
	BOOST_TEST_MESSAGE("Codeword bit length expansion when LZW decompressing");

	bitstream bit_in(this->in, bitstream::bigEndian);
	bit_in.write(9, 'A'); // first byte isn't assigned a codeword
	// start at 0x101 because 0x100 is used for EOF
	for (int i = 0x101; i < 0x200; i++) {
		bit_in.write(9, 'A');
	}
	// Codeword will have just expanded to 10 bits
	bit_in.write(10, 'B'); // 257th char
	bit_in.write(10, 0x100);

	auto processed = std::make_shared<stream::input_filtered>(
		this->in,
		std::make_shared<filter_lzw_decompress>(
			9, 10, 0x101, 0x100, 0, LZW_BIG_ENDIAN | LZW_EOF_PARAM_VALID
		)
	);

	stream::copy(this->out, *processed);

	BOOST_CHECK_MESSAGE(is_equal(std::string(256, 'A') + "B"),
		"Codeword bit length expansion when LZW decompressing failed");
}

BOOST_AUTO_TEST_CASE(lzw_decomp_reset)
{
	BOOST_TEST_MESSAGE("Dictionary reset shared with EOF codeword when LZW decompressing");

	bitstream bit_in(this->in, bitstream::bigEndian);
	bit_in.write(9, 'A'); // first byte isn't assigned a codeword
	// start at 0x101 because 0x100 is used for reset
	for (int i = 0x101; i < 0x200; i++) {
		bit_in.write(9, 'A');
	}
	// Codeword will have just expanded to 10 bits
	bit_in.write(10, 'B'); // 257th char
	bit_in.write(10, 0x100);
	bit_in.write(9, 'C');
	bit_in.write(9, 'C');
	bit_in.write(9, 0x100);

	auto processed = std::make_shared<stream::input_filtered>(
		this->in,
		std::make_shared<filter_lzw_decompress>(
			9, 10, 0x101, 0, 0x100, LZW_BIG_ENDIAN | LZW_RESET_PARAM_VALID
		)
	);

	stream::copy(this->out, *processed);

	BOOST_CHECK_MESSAGE(is_equal(std::string(256, 'A') + "BCC"),
		"Dictionary reset shared with EOF codeword when LZW decompressing failed");
}

BOOST_AUTO_TEST_CASE(lzw_decomp_dict_overflow)
{
	BOOST_TEST_MESSAGE("Decompress some LZW data with a dictionary overflow");

	bitstream bit_in(this->in, bitstream::bigEndian);
	for (int i = 0; i < (1<<8); i++) bit_in.write(9, 'a');
	for (int i = 0; i < (1<<9); i++) bit_in.write(10, 'b');
	for (int i = 0; i < (1<<10); i++) bit_in.write(11, 'c');
	for (int i = 0; i < (1<<11); i++) bit_in.write(12, 'd');
	bit_in.write(12, 'e');
	bit_in.write(12, 'e');
	bit_in.write(12, 0x100);

	auto processed = std::make_shared<stream::input_filtered>(
		this->in,
		std::make_shared<filter_lzw_decompress>(
			9, 12, 0x101, 0x100, 0, LZW_BIG_ENDIAN | LZW_EOF_PARAM_VALID
		)
	);

	stream::copy(this->out, *processed);

	BOOST_CHECK_MESSAGE(is_equal(createString(
		std::string((1 << 8), 'a') <<
		std::string((1 << 9), 'b') <<
		std::string((1 << 10), 'c') <<
		std::string((1 << 11), 'd') <<
		"ee"
		)),
		"Decompressing LZW data with a dictionary overflow failed");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(lzw_comp_suite, string_sample)

BOOST_AUTO_TEST_CASE(lzw_comp_write)
{
	BOOST_TEST_MESSAGE("Compress some LZW data");

	std::shared_ptr<stream::string> exp(new stream::string());
	bitstream bit_exp(exp, bitstream::bigEndian);
	bit_exp.write(9, 'H');
	bit_exp.write(9, 'e');
	bit_exp.write(9, 'l');
	bit_exp.write(9, 'l');
	bit_exp.write(9, 'o');
	bit_exp.write(9, ' ');
	bit_exp.write(9, 'h');
	bit_exp.write(9, 'e');
	bit_exp.write(9, 'l');
	bit_exp.write(9, 'l');
	bit_exp.write(9, 'o');
	bit_exp.write(9, ' ');
	bit_exp.write(9, 'h');
	bit_exp.write(9, 'e');
	bit_exp.write(9, 'l');
	bit_exp.write(9, 'l');
	bit_exp.write(9, 'o');
	bit_exp.write(9, '.');
	bit_exp.write(9, 0x100);
	bit_exp.flushByte();

	this->in->write("Hello hello hello.");

	auto processed = std::make_shared<stream::input_filtered>(
		this->in,
		std::make_shared<filter_lzw_compress>(
			9, 9, 0x101, 0x100, 0, LZW_BIG_ENDIAN | LZW_EOF_PARAM_VALID
		)
	);

	stream::copy(this->out, *processed);

	BOOST_CHECK_MESSAGE(is_equal(exp->data),
		"Compressing LZW data failed");
}

BOOST_AUTO_TEST_CASE(lzw_comp_write_neg_eof)
{
	BOOST_TEST_MESSAGE("Compress some LZW data with negative EOF code");

	std::shared_ptr<stream::string> exp(new stream::string());
	bitstream bit_exp(exp, bitstream::bigEndian);
	bit_exp.write(9, 'H');
	bit_exp.write(9, 'e');
	bit_exp.write(9, 'l');
	bit_exp.write(9, 'l');
	bit_exp.write(9, 'o');
	bit_exp.write(9, ' ');
	bit_exp.write(9, 'h');
	bit_exp.write(9, 'e');
	bit_exp.write(9, 'l');
	bit_exp.write(9, 'l');
	bit_exp.write(9, 'o');
	bit_exp.write(9, ' ');
	bit_exp.write(9, 'h');
	bit_exp.write(9, 'e');
	bit_exp.write(9, 'l');
	bit_exp.write(9, 'l');
	bit_exp.write(9, 'o');
	bit_exp.write(9, '.');
	bit_exp.write(9, 0x1fe);
	bit_exp.flushByte();

	this->in->write("Hello hello hello.");

	auto processed = std::make_shared<stream::input_filtered>(
		this->in,
		std::make_shared<filter_lzw_compress>(
			9, 9, 0x100, -1, 0, LZW_BIG_ENDIAN | LZW_EOF_PARAM_VALID
		)
	);

	stream::copy(this->out, *processed);

	BOOST_CHECK_MESSAGE(is_equal(exp->data),
		"Compressing LZW data failed");
}

BOOST_AUTO_TEST_CASE(lzw_comp_write_dict_grow)
{
	BOOST_TEST_MESSAGE("Compress some LZW data with a growing dictionary");

	std::shared_ptr<stream::string> exp(new stream::string());
	bitstream bit_exp(exp, bitstream::bigEndian);
	for (int i = 0; i < 256; i++) {
		bit_exp.write(9, 'a');
	}
	bit_exp.write(10, 'b');
	bit_exp.write(10, 0x100);
	bit_exp.flushByte();

	for (int i = 0; i < 256; i++) {
		this->in->write("a");
	}
	this->in->write("b");

	auto processed = std::make_shared<stream::input_filtered>(
		this->in,
		std::make_shared<filter_lzw_compress>(
			9, 12, 0x101, 0x100, 0, LZW_BIG_ENDIAN | LZW_EOF_PARAM_VALID
		)
	);

	stream::copy(this->out, *processed);

	BOOST_CHECK_MESSAGE(is_equal(exp->data),
		"Compressing LZW data with growing dictionary failed");
}

BOOST_AUTO_TEST_CASE(lzw_comp_write_dict_overflow)
{
	BOOST_TEST_MESSAGE("Compress some LZW data with an overflowing dictionary");

	std::shared_ptr<stream::string> exp(new stream::string());
	bitstream bit_exp(exp, bitstream::bigEndian);
	for (int i = 0; i < (1<<8); i++) bit_exp.write(9, 'a');
	for (int i = 0; i < (1<<9); i++) bit_exp.write(10, 'b');
	for (int i = 0; i < (1<<10); i++) bit_exp.write(11, 'c');
	for (int i = 0; i < (1<<11); i++) bit_exp.write(12, 'd');
	bit_exp.write(12, 'e');
	bit_exp.write(12, 'e');
	bit_exp.write(12, 0x100);
	bit_exp.flushByte();

	for (int i = 0; i < (1<<8); i++) this->in->write("a");
	for (int i = 0; i < (1<<9); i++) this->in->write("b");
	for (int i = 0; i < (1<<10); i++) this->in->write("c");
	for (int i = 0; i < (1<<11); i++) this->in->write("d");
	this->in->write("ee");

	auto processed = std::make_shared<stream::input_filtered>(
		this->in,
		std::make_shared<filter_lzw_compress>(
			9, 12, 0x101, 0x100, 0, LZW_BIG_ENDIAN | LZW_EOF_PARAM_VALID
		)
	);

	stream::copy(this->out, *processed);

	BOOST_CHECK_MESSAGE(is_equal(exp->data),
		"Compressing LZW data with an overflowing dictionary failed");
}

BOOST_AUTO_TEST_CASE(lzw_comp_write_dict_overflow_reset)
{
	BOOST_TEST_MESSAGE("Compress some LZW data with an autoreset dictionary");

	std::shared_ptr<stream::string> exp(new stream::string());
	bitstream bit_exp(exp, bitstream::bigEndian);
	for (int i = 0; i < (1<<8); i++) bit_exp.write(9, 'a');
	for (int i = 0; i < (1<<9); i++) bit_exp.write(10, 'b');
	for (int i = 0; i < (1<<10); i++) bit_exp.write(11, 'c');
	for (int i = 0; i < (1<<11); i++) bit_exp.write(12, 'd');
	bit_exp.write(9, 'e');
	bit_exp.write(9, 'e');
	bit_exp.write(9, 0x100);
	bit_exp.flushByte();

	for (int i = 0; i < (1<<8); i++) this->in->write("a");
	for (int i = 0; i < (1<<9); i++) this->in->write("b");
	for (int i = 0; i < (1<<10); i++) this->in->write("c");
	for (int i = 0; i < (1<<11); i++) this->in->write("d");
	this->in->write("ee");

	auto processed = std::make_shared<stream::input_filtered>(
		this->in,
		std::make_shared<filter_lzw_compress>(
			9, 12, 0x101, 0x100, 0,
			LZW_BIG_ENDIAN | LZW_EOF_PARAM_VALID | LZW_RESET_FULL_DICT
		)
	);

	stream::copy(this->out, *processed);

	BOOST_CHECK_MESSAGE(is_equal(exp->data),
		"Compressing LZW data with an autoreset dictionary failed");
}

BOOST_AUTO_TEST_CASE(lzw_comp_end_write_midbyte)
{
	BOOST_TEST_MESSAGE("Compress some LZW data and ensure it ends mid-byte");

	this->in->write("aa");

	auto processed = std::make_shared<stream::input_filtered>(
		this->in,
		std::make_shared<filter_lzw_compress>(
			9, 12, 0x101, 0x100, 0, LZW_BIG_ENDIAN | LZW_EOF_PARAM_VALID
		)
	);

	stream::copy(this->out, *processed);

	BOOST_CHECK_MESSAGE(is_equal(makeString("\x30\x98\x60\x00")),
		"Compressing LZW data ensuring it ends mid-byte failed");
}

BOOST_AUTO_TEST_SUITE_END()
