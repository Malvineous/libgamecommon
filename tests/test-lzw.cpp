/**
 * @file   test-lzw.cpp
 * @brief  Test code for LZW compression/decompression filters.
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

#include <boost/test/unit_test.hpp>

#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <iostream>
#include <iomanip>
#include <vector>

#include <camoto/debug.hpp>
#include <camoto/lzw.hpp>
#include <camoto/bitstream.hpp>

#include "tests.hpp"

using namespace camoto;

struct lzw_decomp_sample: public default_sample {

	iostream_sptr pin;
	std::stringstream out;

	lzw_decomp_sample() :
		pin(new std::stringstream())
	{
	}

	boost::test_tools::predicate_result is_equal(const std::string& strExpected)
	{
		// See if the stringstream now matches what we expected
		return this->default_sample::is_equal(strExpected, out.str());
	}

};

BOOST_FIXTURE_TEST_SUITE(lzw_decomp_suite, lzw_decomp_sample)

BOOST_AUTO_TEST_CASE(lzw_decomp_read)
{
	BOOST_TEST_MESSAGE("Decompress some LZW data");

	bitstream_sptr in(new bitstream(pin, bitstream::bigEndian));
	in->write(9, 'H');
	in->write(9, 'e');    // 0x101 -> He
	in->write(9, 'l');    // 0x102 -> el
	in->write(9, 'l');    // 0x103 -> ll
	in->write(9, 'o');    // 0x104 -> lo
	in->write(9, ' ');    // 0x105 -> o 
	in->write(9, 'h');    // 0x106 ->  h
	in->write(9, 0x102);  // 0x107 -> he
	in->write(9, 0x104);  // 0x108 -> ell
	in->write(9, 0x106);  // 0x109 -> lo
	in->write(9, 0x108);  // 0x10a ->  he
	in->write(9, 'o');
	in->write(9, '.');
	in->write(9, 0x100);

	io::filtering_istream inf;
	inf.push(lzw_decompress_filter(9, 9, 0x101, 0x100, 0, LZW_BIG_ENDIAN | LZW_EOF_PARAM_VALID));
	inf.push(*pin);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal("Hello hello hello."),
		"Decompressing LZW data failed");
}

BOOST_AUTO_TEST_CASE(lzw_decomp_bitlength_expand)
{
	BOOST_TEST_MESSAGE("Codeword bit length expansion when LZW decompressing");

	bitstream_sptr in(new bitstream(pin, bitstream::bigEndian));
	for (int i = 0; i < 256; i++) {
		in->write(9, 'A');
	}
	// Codeword will have just expanded to 10 bits
	in->write(10, 'B');
	in->write(10, 0x100);

	io::filtering_istream inf;
	inf.push(lzw_decompress_filter(9, 10, 0x101, 0x100, 0, LZW_BIG_ENDIAN | LZW_EOF_PARAM_VALID));
	inf.push(*pin);

	boost::iostreams::copy(inf, out);

	BOOST_CHECK_MESSAGE(is_equal(std::string(256, 'A') + "B"),
		"Codeword bit length expansion when LZW decompressing failed");
}

BOOST_AUTO_TEST_SUITE_END()
