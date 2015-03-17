/**
 * @file   test-lzss.cpp
 * @brief  Test code for LZSS compression/decompression filters.
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

#include <camoto/filter-lzss.hpp>
#include <camoto/bitstream.hpp>
#include <camoto/stream_filtered.hpp>
#include <camoto/util.hpp>

#include "tests.hpp"

using namespace camoto;

BOOST_FIXTURE_TEST_SUITE(lzss_decomp_suite, string_sample)

BOOST_AUTO_TEST_CASE(lzss_decomp_read)
{
	BOOST_TEST_MESSAGE("Decompress some LZSS data");

	bitstream bit_in(this->in, bitstream::bigEndian);
	bit_in.write(9, 'H');
	bit_in.write(9, 'e');
	bit_in.write(9, 'l');
	bit_in.write(9, 'l');
	bit_in.write(9, 'o');
	bit_in.write(9, ' ');
	bit_in.write(9, 'h');
	bit_in.write(1, 1);      // Code
	bit_in.write(2, 3);      // len=3(+2)
	bit_in.write(8, 5);      // dist=5(+1) "H[ello ]h"
	bit_in.write(1, 1);      // Code
	bit_in.write(2, 3);      // len=3(+2)
	bit_in.write(8, 5);      // dist=5(+1) "Hello [hello] "
	bit_in.write(9, '.');    // "Hello hello hello" + '.'
	bit_in.flush();

	auto processed = std::make_shared<stream::input_filtered>(
		this->in,
		std::make_shared<filter_lzss_decompress>(
			bitstream::bigEndian, 2, 8
		)
	);

	stream::copy(this->out, *processed);

	BOOST_CHECK_MESSAGE(is_equal("Hello hello hello."),
		"Decompressing LZSS data failed");
}

BOOST_AUTO_TEST_CASE(lzss_decomp_read_overlap)
{
	BOOST_TEST_MESSAGE("Decompress some LZSS data with overlapping reads");

	bitstream bit_in(this->in, bitstream::bigEndian);
	bit_in.write(9, 'A');
	bit_in.write(9, 'B');
	bit_in.write(9, 'C');
	bit_in.write(1, 1);      // Code
	bit_in.write(2, 3);      // len=3(+2)
	bit_in.write(8, 2);      // dist=2(+1) "[ABC"_] -> "ABCABCAB"
	bit_in.write(1, 1);      // Code
	bit_in.write(2, 2);      // len=2(+2)
	bit_in.write(8, 2);      // dist=2(+1) "ABCAB[CAB"_]
	bit_in.flush();

	auto processed = std::make_shared<stream::input_filtered>(
		this->in,
		std::make_shared<filter_lzss_decompress>(
			bitstream::bigEndian, 2, 8
		)
	);

	stream::copy(this->out, *processed);

	BOOST_CHECK_MESSAGE(is_equal("ABCABCABCABC"),
		"Decompressing LZSS data with overlapping reads failed");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_FIXTURE_TEST_SUITE(lzss_comp_suite, string_sample)

BOOST_AUTO_TEST_CASE(lzss_comp_write)
{
	BOOST_TEST_MESSAGE("Compress some LZSS data");

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
	bit_exp.flush();

	this->in->write("Hello hello hello.");
	this->in->flush();

	auto processed = std::make_shared<stream::input_filtered>(
		this->in,
		std::make_shared<filter_lzss_compress>(
			bitstream::bigEndian, 2, 8
		)
	);

	stream::copy(this->out, *processed);

	BOOST_CHECK_MESSAGE(is_equal(exp->data),
		"Compressing LZSS data failed");
}

BOOST_AUTO_TEST_SUITE_END()
