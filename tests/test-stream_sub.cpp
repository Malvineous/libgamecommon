/**
 * @file   test-stream_sub.cpp
 * @brief  Test code for substream class.
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
#include <boost/bind.hpp>
#include <unistd.h>
#include <errno.h>

#include <camoto/stream_sub.hpp>
#include <camoto/stream_string.hpp>

#include "tests.hpp"

using namespace camoto;

/// Truncate function to adjust a stream::string so it can hold an enlarging stream::sub
void ss_resize(stream::output_sptr base, stream::output_sub_sptr sub, stream::len len)
	throw (stream::write_error)
{
	base->truncate(sub->get_offset() + len);
	sub->resize(len);
}

struct stream_sub_sample: public default_sample {

	stream::string_sptr base;
	stream::sub_sptr sub;

	stream_sub_sample()
		: base(new stream::string()),
		  sub(new stream::sub())
	{
		this->base->write("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

		// Make sure the data went in correctly to begin the test
		//BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
		//	"Error creating fresh string for testing");
		BOOST_REQUIRE(this->base->str().compare("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == 0);
	}

	boost::test_tools::predicate_result is_equal(const char *cExpected)
	{
		// See if the stringstream now matches what we expected
		std::string strExpected = cExpected;
		stream::string_sptr got(new stream::string());
		this->sub->seekg(0, stream::start);
		stream::copy(got, this->sub);
		return this->default_sample::is_equal(strExpected, got->str());
	}

};

BOOST_FIXTURE_TEST_SUITE(stream_sub_suite, stream_sub_sample)

BOOST_AUTO_TEST_CASE(read)
{
	BOOST_TEST_MESSAGE("Create substream with size and offset");

	this->sub->open(this->base, 5, 6, NULL);

	BOOST_CHECK_MESSAGE(is_equal("FGHIJK"),
		"Substream creation with size and offset failed");
}

BOOST_AUTO_TEST_CASE(read_write)
{
	BOOST_TEST_MESSAGE("Create substream and read what was written");

	this->sub->open(this->base, 2, 10, boost::bind(ss_resize, this->base, this->sub, _1));
	this->sub->seekp(4, stream::cur); // initial offset must always be 0
	this->sub->write("123");
	this->sub->seekg(2, stream::start);
	std::string result = this->sub->read(6);


	BOOST_CHECK_MESSAGE(default_sample::is_equal("EF123J", result),
		"Substream could not read what was written");
}

BOOST_AUTO_TEST_CASE(change_offset)
{
	BOOST_TEST_MESSAGE("Move substream's offset");

	this->sub->open(this->base, 2, 4, boost::bind(ss_resize, this->base, this->sub, _1));

	this->sub->relocate(8);
	this->sub->resize(16); // can't read past end of stream!

	BOOST_CHECK_MESSAGE(is_equal("KLMNOPQRSTUVWXYZ"),
		"Move substream's offset failed");
}

BOOST_AUTO_TEST_CASE(write_then_move)
{
	BOOST_TEST_MESSAGE("Move substream's offset after writing");
	this->sub->open(this->base, 0, 16, boost::bind(ss_resize, this->base, this->sub, _1));

	this->sub->seekp(10, stream::start);
	this->sub->write("12345");

	// Flush the changes to the underlying stream
	this->sub->flush();
	// It would be nice to have another test without the flush, but we can't
	// guarantee the previous write statement will be cached (and that test
	// would fail if it wasn't cached.)  The behaviour is correct either way, we
	// just can't guarantee which behaviour we'll get during the test.

	this->sub->relocate(8);

	// The pointer is at substream offset 15, so the resize should clip it back
	// to offset 10.
	BOOST_REQUIRE_EQUAL(this->sub->tellp(), 15);
	this->sub->resize(10);
	BOOST_REQUIRE_EQUAL(this->sub->tellp(), 10);

	BOOST_CHECK_MESSAGE(is_equal("IJ12345PQR"),
		"Move substream's offset after write failed");
}

void noResize(stream::len len)
{
	throw stream::write_error("Resize refused for testing purposes");
}

BOOST_AUTO_TEST_CASE(write_past_eof_fixed)
{
	BOOST_TEST_MESSAGE("Write past substream's EOF");

	this->sub->open(this->base, 0, 26, noResize);
	this->sub->seekp(20, stream::start);
	stream::len w = sub->try_write((const uint8_t *)"1234567890", 10);

	BOOST_CHECK_MESSAGE(w == 6,
		"Write past fixed-size substream's EOF failed (too many/few bytes written)");

	// Flush the changes to the underlying stream
	this->sub->flush();

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOPQRST123456"),
		"Write past fixed-size substream's EOF");
}

void doResize(stream::output_sub_sptr s, stream::len len)
{
	// Since we're writing to a string stream we don't have to bother resizing the
	// underlying stream - just notify the substream the resize has been done.  As
	// long as we're not seeking into the new space in the test, the string stream
	// will enlarge automatically.
	s->resize(len);
	return;
}

BOOST_AUTO_TEST_CASE(write_past_eof_expand)
{
	BOOST_TEST_MESSAGE("Write past substream's EOF");

	stream::fn_truncate yesResize = boost::bind(doResize, this->sub, _1);

	this->sub->open(this->base, 0, 26, yesResize);
	this->sub->seekp(20, stream::start);
	stream::len w = sub->try_write((const uint8_t *)"1234567890", 10);

	BOOST_CHECK_MESSAGE(w == 10,
		"Write past expanding substream's EOF failed (too many/few bytes written)");

	// Flush the changes to the underlying stream
	this->sub->flush();

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOPQRST1234567890"),
		"Write past expanding substream's EOF");
}

BOOST_AUTO_TEST_SUITE_END()
