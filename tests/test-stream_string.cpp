/**
 * @file   test-stream_string.cpp
 * @brief  Test code for string stream class.
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

#include <errno.h>
#include <boost/test/unit_test.hpp>
#include <camoto/stream_string.hpp>
#include "tests.hpp"

using namespace camoto;

BOOST_FIXTURE_TEST_SUITE(stream_string_suite, default_sample)

BOOST_AUTO_TEST_CASE(write)
{
	BOOST_TEST_MESSAGE("Write string");

	stream::output_string out;

	out.write("abcdefghijklmno");
	out.seekp(4, stream::start);
	out.write(" is a test");
	out.flush();

	BOOST_CHECK_MESSAGE(is_equal("abcd is a testo", out.data),
		"Error writing data to new string");
}

BOOST_AUTO_TEST_CASE(read_existing)
{
	BOOST_TEST_MESSAGE("Read existing string");

	stream::input_string in("123456790");
	std::string val;

	BOOST_REQUIRE_NO_THROW(
		val = in.read(5);
	);

	BOOST_CHECK_MESSAGE(is_equal("12345", val),
		"Error reading data from existing string");
}

BOOST_AUTO_TEST_CASE(write_existing)
{
	BOOST_TEST_MESSAGE("Write existing string");

	stream::output_string out;
	out.data = "1234567890";

	out.seekp(2, stream::start);
	out.write("abc");
	out.flush();

	BOOST_CHECK_MESSAGE(is_equal("12abc67890", out.data),
		"Error writing data to existing string");
}

BOOST_AUTO_TEST_CASE(readwrite)
{
	BOOST_TEST_MESSAGE("Read+write new string");

	stream::string f;
	std::string val;

	f.write("abcdefghij");
	f.seekp(4, stream::start);
	f.write("12345");
	f.seekp(2, stream::start);
	BOOST_REQUIRE_NO_THROW(
		val = f.read(5);
	);
	BOOST_CHECK_MESSAGE(is_equal("cd123", val),
		"Error reading back data just written to string");

	f.flush();

	BOOST_CHECK_MESSAGE(is_equal("abcd12345j", f.data),
		"Error getting underlying string data just written");
}

BOOST_AUTO_TEST_CASE(readwrite_existing)
{
	BOOST_TEST_MESSAGE("Read+write existing string");

	stream::string f("1234567890");
	std::string val;

	f.seekp(4, stream::start);
	f.write("12345");
	f.seekp(2, stream::start);
	BOOST_REQUIRE_NO_THROW(
		val = f.read(5);
	);
	BOOST_CHECK_MESSAGE(is_equal("34123", val),
		"Error reading back data just written to existing string");

	f.flush();

	BOOST_CHECK_MESSAGE(is_equal("1234123450", f.data),
		"Error getting underlying string data just written to existing string");
}

BOOST_AUTO_TEST_CASE(readwrite_dynamic)
{
	BOOST_TEST_MESSAGE("Read+write dynamic string");

	auto f = std::make_shared<stream::string>();
	std::string val;

	*f << "hello";
	f->flush();
	f->seekg(0, stream::start);
	BOOST_REQUIRE_NO_THROW(
		val = f->read(5);
	);
	BOOST_CHECK_MESSAGE(is_equal("hello", val),
		"Error reading back data just written to dynamic string");

	BOOST_CHECK_MESSAGE(is_equal("hello", f->data),
		"Error getting underlying string data just written to dynamic instance");
}

BOOST_AUTO_TEST_CASE(expand)
{
	BOOST_TEST_MESSAGE("Expand string");

	stream::string f;

	f.write("1234567890");
	f.flush();
	BOOST_REQUIRE_EQUAL(f.size(), 10);
	BOOST_REQUIRE_EQUAL(f.data.length(), 10);

	f.write("abcde");
	f.flush();
	BOOST_REQUIRE_EQUAL(f.size(), 15);
	BOOST_REQUIRE_EQUAL(f.data.length(), 15);

	f.truncate(8);
	f.write("zyx");
	f.flush();
	BOOST_REQUIRE_EQUAL(f.size(), 11);
	BOOST_REQUIRE_EQUAL(f.data.length(), 11);
}

BOOST_AUTO_TEST_SUITE_END()
