/**
 * @file   test-stream_sub.cpp
 * @brief  Test code for substream class.
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
#include <errno.h>
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <camoto/stream_sub.hpp>
#include <camoto/stream_string.hpp>
#include "tests.hpp"

using namespace camoto;

/// Truncate function to adjust a stream::string so it can hold an enlarging stream::sub
/**
 * We use weak_ptr<> here instead of the usual shared_ptr<> because if
 * boost::bind is used to pass this function as the truncate function, then the
 * substream will end up holding a reference to itself (in the bound pointer to
 * the truncate function) causing the shared_ptr to never get deleted and leak
 * memory.
 */
void ss_resize(std::weak_ptr<stream::output> w_base,
	stream::output_sub *sub, stream::len len)
{
	std::shared_ptr<stream::output> base = w_base.lock();
	if (!base) return;
	base->truncate(sub->sub_start() + len);
	sub->resize(len);
	return;
}

struct stream_sub_sample: public default_sample {

	std::shared_ptr<stream::string> base;
	std::shared_ptr<stream::sub> sub;

	stream_sub_sample()
		:	base(new stream::string()),
			sub()
	{
		this->base->write("ABCDEFGHIJKLMNOPQRSTUVWXYZ");

		// Make sure the data went in correctly to begin the test
		//BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
		//	"Error creating fresh string for testing");
		BOOST_REQUIRE(this->base->data.compare("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == 0);
	}

	boost::test_tools::predicate_result is_equal(const char *cExpected)
	{
		assert(this->sub);

		// See if the stringstream now matches what we expected
		std::string strExpected = cExpected;
		this->sub->seekg(0, stream::start);
		stream::string got;
		stream::copy(got, *this->sub);
		return this->default_sample::is_equal(strExpected, got.data);
	}

};

BOOST_FIXTURE_TEST_SUITE(stream_sub_suite, stream_sub_sample)

BOOST_AUTO_TEST_CASE(read)
{
	BOOST_TEST_MESSAGE("Create substream with size and offset");

	this->sub = std::make_shared<stream::sub>(
		std::dynamic_pointer_cast<stream::inout>(this->base),
		5, 6, stream::fn_truncate_sub());

	BOOST_CHECK_MESSAGE(this->sub->sub_start(), 5);
	BOOST_CHECK_MESSAGE(this->sub->size(), 6);

	BOOST_CHECK_MESSAGE(is_equal("FGHIJK"),
		"Substream creation with size and offset failed");
}

BOOST_AUTO_TEST_CASE(read_write)
{
	BOOST_TEST_MESSAGE("Create substream and read what was written");

	this->sub = std::make_shared<stream::sub>(
		std::dynamic_pointer_cast<stream::inout>(this->base),
		2, 10, boost::bind(ss_resize,
		std::weak_ptr<stream::output>(this->base),
		_1, _2));
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

	this->sub = std::make_shared<stream::sub>(
		std::dynamic_pointer_cast<stream::inout>(this->base),
		2, 4, boost::bind(ss_resize,
		std::weak_ptr<stream::output>(this->base),
		_1, _2));

	this->sub->relocate(8);
	this->sub->resize(16); // can't read past end of stream!

	BOOST_CHECK_MESSAGE(is_equal("KLMNOPQRSTUVWXYZ"),
		"Move substream's offset failed");
}

BOOST_AUTO_TEST_CASE(relocate_to_start)
{
	BOOST_TEST_MESSAGE("Move substream's offset to start of parent");

	this->sub = std::make_shared<stream::sub>(
		std::dynamic_pointer_cast<stream::inout>(this->base),
		2, 4, boost::bind(ss_resize,
		std::weak_ptr<stream::output>(this->base),
		_1, _2));

	BOOST_CHECK_MESSAGE(is_equal("CDEF"),
		"Open substream failed");

	this->sub->relocate(-2);

	BOOST_CHECK_MESSAGE(is_equal("ABCD"),
		"Move substream's offset to start of parent failed");
}

BOOST_AUTO_TEST_CASE(write_then_move)
{
	BOOST_TEST_MESSAGE("Move substream's offset after writing");
	this->sub = std::make_shared<stream::sub>(
		std::dynamic_pointer_cast<stream::inout>(this->base),
		0, 16, boost::bind(ss_resize,
		std::weak_ptr<stream::output>(this->base),
		_1, _2));

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

void noResize(stream::output_sub *sub, stream::len len)
{
	throw stream::write_error("Resize refused for testing purposes");
}

BOOST_AUTO_TEST_CASE(write_past_eof_fixed)
{
	BOOST_TEST_MESSAGE("Write past substream's fixed EOF");

	this->sub = std::make_shared<stream::sub>(
		std::dynamic_pointer_cast<stream::inout>(this->base),
		0, 26, noResize);
	this->sub->seekp(20, stream::start);
	stream::len w = sub->try_write((const uint8_t *)"1234567890", 10);

	BOOST_CHECK_MESSAGE(w == 6,
		"Write past fixed-size substream's EOF failed (too many/few bytes written)");

	// Flush the changes to the underlying stream
	this->sub->flush();

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOPQRST123456"),
		"Write past fixed-size substream's EOF");
}

void doResize(stream::output_sub& sub, stream::len len)
{
	// Since we're writing to a string stream we don't have to bother resizing the
	// underlying stream - just notify the substream the resize has been done.  As
	// long as we're not seeking into the new space in the test, the string stream
	// will enlarge automatically.
	sub.resize(len);
	return;
}

BOOST_AUTO_TEST_CASE(write_past_eof_expand)
{
	BOOST_TEST_MESSAGE("Write past substream's expandable EOF");

	this->sub = std::make_shared<stream::sub>(
		std::dynamic_pointer_cast<stream::inout>(this->base),
		0, 26, boost::bind(&stream::sub::resize, _1, _2));
	this->sub->seekp(20, stream::start);

	stream::len w;
	BOOST_REQUIRE_NO_THROW(
		w = sub->try_write((const uint8_t *)"1234567890", 10)
	);

	BOOST_CHECK_MESSAGE(w == 10,
		"Write past expanding substream's EOF failed (too many/few bytes written)");

	// Flush the changes to the underlying stream
	this->sub->flush();

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOPQRST1234567890"),
		"Write past expanding substream's EOF");
}

BOOST_AUTO_TEST_SUITE_END()
