/**
 * @file   test-filter-crop.cpp
 * @brief  Test code for ignoring the unfiltered size field in output data.
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
#include <camoto/filter-crop.hpp>

#include "tests.hpp"

using namespace camoto;

BOOST_FIXTURE_TEST_SUITE(crop_comp_suite, string_sample)

BOOST_AUTO_TEST_CASE(crop_comp_write)
{
	BOOST_TEST_MESSAGE("Read data with unfiltered size at start");

	*this->in << makeString("\x00\x00\x00\x05" "Hello");

	auto algo = std::make_shared<filter_crop>(4);
	auto f = std::make_shared<stream::filtered>(
		this->in, algo, algo, stream::fn_notify_prefiltered_size()
	);

	f->seekg(0, stream::start);
	stream::copy(this->out, *f);

	BOOST_CHECK_MESSAGE(is_equal("Hello"),
		"Reading data with unfiltered size at start failed");
}

BOOST_AUTO_TEST_SUITE_END()
