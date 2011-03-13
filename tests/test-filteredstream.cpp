/**
 * @file   test-filteredstream.cpp
 * @brief  Test code for filteredstream class.
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
#include <boost/iostreams/copy.hpp>
#include <iostream>

#include <camoto/filteredstream.hpp>

#include "tests.hpp"

using namespace camoto;

BOOST_FIXTURE_TEST_SUITE(filteredstream_suite, default_sample)

BOOST_AUTO_TEST_CASE(filteredstream_read)
{
	BOOST_TEST_MESSAGE("Read from filteredstream");

	iostream_sptr in(new std::stringstream);
	*in << "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	filtered_istream_sptr pinf(new filtered_istream());
	filtered_ostream_sptr poutf(new filtered_ostream());
	iostream_sptr dec(new filteredstream(in, pinf, poutf));
	BOOST_REQUIRE_EQUAL(dec->good(), true);

	dec->seekg(10, std::ios::beg);
	BOOST_REQUIRE_EQUAL(dec->good(), true);

	char buf[10];
	dec->read(buf, 10);
	BOOST_REQUIRE_EQUAL(dec->gcount(), 10);

	BOOST_CHECK_MESSAGE(is_equal("KLMNOPQRST", std::string(buf, 10)),
		"Read from filteredstream failed");
}

BOOST_AUTO_TEST_CASE(filteredstream_write)
{
	BOOST_TEST_MESSAGE("Write to filteredstream");

	std::stringstream *ss = new std::stringstream;
	iostream_sptr in(ss);
	*in << "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	filtered_istream_sptr pinf(new filtered_istream());
	filtered_ostream_sptr poutf(new filtered_ostream());
	iostream_sptr dec(new filteredstream(in, pinf, poutf));
	BOOST_REQUIRE_EQUAL(dec->good(), true);

	dec->seekp(10, std::ios::beg);
	BOOST_REQUIRE_EQUAL(dec->good(), true);

	dec->write("1234567890", 10);
	dec->flush();

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJ1234567890UVWXYZ", ss->str()),
		"Write to filteredstream failed");
}

BOOST_AUTO_TEST_CASE(filteredstream_remember)
{
	BOOST_TEST_MESSAGE("Remember underlying stream used by filteredstream");

	std::stringstream *ss = new std::stringstream;
	iostream_sptr in(ss);
	*in << "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	filtered_iostream_sptr dec(new filtered_iostream());
	dec->pushShared(in);

	BOOST_REQUIRE_EQUAL(dec->good(), true);
	// Release the original stream, now that "dec" should have a copy of it
	in.reset();
	BOOST_REQUIRE_EQUAL(dec->good(), true);

	dec->seekp(10, std::ios::beg);
	BOOST_REQUIRE_EQUAL(dec->good(), true);

	dec->write("1234567890", 10);
	dec->flush();

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJ1234567890UVWXYZ", ss->str()),
		"Remembering underlying stream used by filteredstream failed");
}

// filteredstream_remember must come before filteredstream_read_write or it
// won't always fail

BOOST_AUTO_TEST_CASE(filteredstream_read_write)
{
	BOOST_TEST_MESSAGE("Write to filteredstream");

	std::stringstream *ss = new std::stringstream;
	iostream_sptr in(ss);
	*in << "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	filtered_iostream_sptr dec(new filtered_iostream());
	dec->pushShared(in);
	BOOST_REQUIRE_EQUAL(dec->good(), true);

	dec->seekp(10, std::ios::beg);
	BOOST_REQUIRE_EQUAL(dec->good(), true);

	dec->write("1234567890", 10);
	dec->flush();

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJ1234567890UVWXYZ", ss->str()),
		"Write to filteredstream failed");

	dec->seekg(5, std::ios::beg);
	BOOST_REQUIRE_EQUAL(dec->good(), true);

	char buf[10];
	dec->read(buf, 10);
	BOOST_REQUIRE_EQUAL(dec->gcount(), 10);

	BOOST_CHECK_MESSAGE(is_equal("FGHIJ12345", std::string(buf, 10)),
		"Read from filteredstream failed");
}

BOOST_AUTO_TEST_CASE(filteredstream_pushed_stream_write)
{
	BOOST_TEST_MESSAGE("Filter data into a stream that was pushed on");

	std::stringstream out;

	boost::shared_ptr<io::filtering_ostream> poutf(new io::filtering_ostream());

	poutf->push(out);

	poutf->write("\x00\x01\x02\x03\xFF\xFF\xFF\xFF", 8);
	poutf->flush();

	BOOST_CHECK_MESSAGE(is_equal(makeString("\x00\x01\x02\x03\xFF\xFF\xFF\xFF"),
		out.str()),
		"Filter data into a stream that was pushed on");
}

BOOST_AUTO_TEST_SUITE_END()
