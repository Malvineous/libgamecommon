/**
 * @file   test-filter-pad.cpp
 * @brief  Test code for adding extra bytes at the start of the output data.
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

#include <camoto/stream_filtered.hpp>
#include <camoto/util.hpp>
#include <camoto/filter-pad.hpp>
#include <camoto/iostream_helpers.hpp>

#include "tests.hpp"

using namespace camoto;

BOOST_FIXTURE_TEST_SUITE(pad_comp_suite, string_sample)

BOOST_AUTO_TEST_CASE(pad_write)
{
	BOOST_TEST_MESSAGE("Write data with padding at start");

	auto algo = std::make_shared<filter_pad>();
	auto f = std::make_shared<stream::filtered>(
		this->in, algo, algo, stream::fn_notify_prefiltered_size()
	);

	f->seekp(0, stream::start);
	f->write("Hello", 5);
	f->truncate_here();
	algo->pad << u32le(0x1234);
	f->flush();

	this->in->seekg(0, stream::start);
	stream::copy(this->out, *this->in);

	BOOST_CHECK_MESSAGE(is_equal(makeString("\x34\x12\x00\x00" "Hello")),
		"Writing data with padding at start failed");

	BOOST_REQUIRE_EQUAL(this->in->tellg(), 5 + 4);
}

BOOST_AUTO_TEST_SUITE_END()
