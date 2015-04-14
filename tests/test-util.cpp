/**
 * @file  test-util.cpp
 * @brief Test code for utility functions.
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
#include <camoto/util.hpp>
#include "tests.hpp"

using namespace camoto;

BOOST_AUTO_TEST_SUITE(util_suite)

BOOST_AUTO_TEST_CASE(to_nearest_multiple)
{
	BOOST_TEST_MESSAGE("toNearestMultiple()");

	BOOST_REQUIRE_EQUAL(toNearestMultiple(0, 4), 0);
	BOOST_REQUIRE_EQUAL(toNearestMultiple(3, 4), 4);
	BOOST_REQUIRE_EQUAL(toNearestMultiple(4, 4), 4);
	BOOST_REQUIRE_EQUAL(toNearestMultiple(5, 4), 8);

	BOOST_REQUIRE_EQUAL(toNearestMultiple(0, 2), 0);
	BOOST_REQUIRE_EQUAL(toNearestMultiple(1, 2), 2);
	BOOST_REQUIRE_EQUAL(toNearestMultiple(2, 2), 2);
	BOOST_REQUIRE_EQUAL(toNearestMultiple(3, 2), 4);
	BOOST_REQUIRE_EQUAL(toNearestMultiple(4, 2), 4);
	BOOST_REQUIRE_EQUAL(toNearestMultiple(5, 2), 6);
}

BOOST_AUTO_TEST_SUITE_END()
