/**
 * @file   test-stream_string.cpp
 * @brief  Test code for string stream class.
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
#include <unistd.h>
#include <errno.h>

#include <camoto/stream_string.hpp>

#include "tests.hpp"

using namespace camoto;

BOOST_FIXTURE_TEST_SUITE(stream_string_suite, default_sample)

BOOST_AUTO_TEST_CASE(write)
{
	BOOST_TEST_MESSAGE("Write string");

	stream::output_string_sptr out;

	out.reset(new stream::output_string());
	out->write("abcdefghijklmno");
	out->seekp(4, stream::start);
	out->write(" is a test");
	out->flush();
	std::string result = out->str();
	out.reset();

	BOOST_CHECK_MESSAGE(is_equal("abcd is a testo", result),
		"Error writing data to new string");
}

BOOST_AUTO_TEST_CASE(read_existing)
{
	BOOST_TEST_MESSAGE("Read existing string");

	stream::input_string_sptr in;
	std::string val;
	const std::string src("123456790");

	in.reset(new stream::input_string());
	in->open(&src);
	BOOST_REQUIRE_NO_THROW(
		val = in->read(5);
	);

	BOOST_CHECK_MESSAGE(is_equal("12345", val),
		"Error reading data from existing string");
}

BOOST_AUTO_TEST_CASE(write_existing)
{
	BOOST_TEST_MESSAGE("Write existing string");

	stream::output_string_sptr out;

	std::string src("1234567890");

	out.reset(new stream::output_string());
	out->open(&src);
	out->seekp(2, stream::start);
	out->write("abc");
	out->flush();
	out.reset();

	BOOST_CHECK_MESSAGE(is_equal("12abc67890", src),
		"Error writing data to existing string");
}

BOOST_AUTO_TEST_CASE(readwrite)
{
	BOOST_TEST_MESSAGE("Read+write new string");

	stream::string_sptr f;
	std::string val;

	f.reset(new stream::string());
	f->write("abcdefghij");
	f->seekp(4, stream::start);
	f->write("12345");
	f->seekp(2, stream::start);
	BOOST_REQUIRE_NO_THROW(
		val = f->read(5);
	);
	BOOST_CHECK_MESSAGE(is_equal("cd123", val),
		"Error reading back data just written to string");

	f->flush();

	BOOST_CHECK_MESSAGE(is_equal("abcd12345j", f->str()),
		"Error getting underlying string data just written");

	f.reset();
}

BOOST_AUTO_TEST_CASE(readwrite_existing)
{
	BOOST_TEST_MESSAGE("Read+write existing string");

	stream::string_sptr f;
	std::string val;
	std::string src("1234567890");

	f.reset(new stream::string());

	// This next call must go to output_string::open() and not
	// input_string::open().
	f->open(&src);

	// This should be an error as it will try to call input_string::open()
	//const std::string test = src;
	//f->open(&test);

	f->seekp(4, stream::start);
	f->write("12345");
	f->seekp(2, stream::start);
	BOOST_REQUIRE_NO_THROW(
		val = f->read(5);
	);
	BOOST_CHECK_MESSAGE(is_equal("34123", val),
		"Error reading back data just written to existing string");

	f->flush();

	BOOST_CHECK_MESSAGE(is_equal("1234123450", f->str()),
		"Error getting underlying string data just written to existing string");

	f.reset();
}

BOOST_AUTO_TEST_CASE(expand)
{
	BOOST_TEST_MESSAGE("Expand string");

	stream::string_sptr f;

	f.reset(new stream::string());
	f->write("1234567890");
	BOOST_REQUIRE_EQUAL(f->size(), 10);

	f->write("abcde");
	BOOST_REQUIRE_EQUAL(f->size(), 15);

	f->truncate(8);
	f->write("zyx");
	BOOST_REQUIRE_EQUAL(f->size(), 11);

	f->flush();
	f.reset();
}

BOOST_AUTO_TEST_SUITE_END()
