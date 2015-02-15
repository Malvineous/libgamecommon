/**
 * @file   test-stream_file.cpp
 * @brief  Test code for file stream class.
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

#include <memory>
#include <iostream>
#include <errno.h>
#include <boost/test/unit_test.hpp>
#include <camoto/stream_file.hpp>
#include "tests.hpp"

#ifdef WIN32
#define unlink(x) _unlink(x)
#endif

using namespace camoto;

#define TEST_FILE "_test.$"

struct AtExit {
	~AtExit()
	{
		// Remove the test file when the tests are done
		if (unlink(TEST_FILE) < 0) {
			std::cerr << "Could not remove test file \"" TEST_FILE << "\": "
				<< strerror(errno) << std::endl;
		}
	}
};

AtExit atex;

BOOST_FIXTURE_TEST_SUITE(stream_file_suite, default_sample)

BOOST_AUTO_TEST_CASE(create)
{
	BOOST_TEST_MESSAGE("Create file");

	std::shared_ptr<stream::output_file> out;
	std::shared_ptr<stream::input_file> in;
	std::string val;

	out.reset(new stream::output_file());
	out->create(TEST_FILE);
	out->write("abcdefghijklmno");
	out->seekp(4, stream::start);
	out->write(" is a test");
	out->flush();
	out.reset();

	in.reset(new stream::input_file());
	in->open(TEST_FILE);
	BOOST_REQUIRE_NO_THROW(
		val = in->read(15);
	);

	BOOST_CHECK_MESSAGE(is_equal("abcd is a testo", val),
		"Error reading back data from new file");

	out.reset(new stream::output_file());
	out->open(TEST_FILE);
	out->seekp(8, stream::start);
	out->write("12345");
	out->flush();
	out.reset();

	in.reset(new stream::input_file());
	in->open(TEST_FILE);
	BOOST_REQUIRE_NO_THROW(
		val = in->read(15);
	);

	BOOST_CHECK_MESSAGE(is_equal("abcd is 12345to", val),
		"Error reading back data from new file");

	out.reset(new stream::output_file());
	out->open(TEST_FILE);
	out->seekp(2, stream::start);
	out->write("xy");
	out->truncate_here();
	out.reset();

	in.reset(new stream::input_file());
	in->open(TEST_FILE);
	BOOST_REQUIRE_EQUAL(in->size(), 4);
	BOOST_REQUIRE_NO_THROW(
		val = in->read(4);
	);

	BOOST_CHECK_MESSAGE(is_equal("abxy", val),
		"Error reading back data from new file");
}

BOOST_AUTO_TEST_CASE(readwrite)
{
	BOOST_TEST_MESSAGE("Read+write file");

	std::shared_ptr<stream::file> f;
	std::string val;

	f.reset(new stream::file());
	f->create(TEST_FILE); // should blank out existing file
	f->write("zyxwvu");
	f->seekp(4, stream::start);
	f->write("12345");
	f->seekp(2, stream::start);
	BOOST_REQUIRE_NO_THROW(
		val = f->read(5);
	);
	BOOST_CHECK_MESSAGE(is_equal("xw123", val),
		"Error reading back data just written to file");

	f->flush();
	f.reset();
}

BOOST_AUTO_TEST_CASE(expand)
{
	BOOST_TEST_MESSAGE("Expand file");

	std::shared_ptr<stream::file> f;

	f.reset(new stream::file());
	f->create(TEST_FILE);
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
