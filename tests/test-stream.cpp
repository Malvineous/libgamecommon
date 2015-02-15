/**
 * @file   test-stream.cpp
 * @brief  Test code for core stream class and helper functions.
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
#include <boost/bind.hpp>
#include <camoto/stream.hpp>
#include <camoto/stream_string.hpp>
#include "tests.hpp"

using namespace camoto;

struct stream_move_sample: public default_sample
{
	stream::string content;

	stream_move_sample()
	{
		this->content.write("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	}

	boost::test_tools::predicate_result is_equal(const char *cExpected)
	{
		std::string strExpected = cExpected;
		return this->default_sample::is_equal(strExpected, this->content.data);
	}

};

BOOST_FIXTURE_TEST_SUITE(stream_move_suite, stream_move_sample)

BOOST_AUTO_TEST_CASE(stream_move_fwd)
{
	BOOST_TEST_MESSAGE("Stream move forward");

	stream::move(this->content, 5, 15, 5);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOFGHIJUVWXYZ"),
		"Error in stream move forward");
}

BOOST_AUTO_TEST_CASE(stream_move_bk)
{
	BOOST_TEST_MESSAGE("Stream move backward");

	stream::move(this->content, 15, 5, 5);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEPQRSTKLMNOPQRSTUVWXYZ"),
		"Error in stream move backward");
}

BOOST_AUTO_TEST_CASE(stream_move_fwd_borderline)
{
	BOOST_TEST_MESSAGE("Stream move forward (blocks touching)");

	stream::move(this->content, 5, 10, 5);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJFGHIJPQRSTUVWXYZ"),
		"Error in stream move forward (blocks touching)");
}

BOOST_AUTO_TEST_CASE(stream_move_bk_borderline)
{
	BOOST_TEST_MESSAGE("Stream move backward (blocks touching)");

	stream::move(this->content, 10, 5, 5);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEKLMNOKLMNOPQRSTUVWXYZ"),
		"Error in stream move backward (blocks touching)");
}

BOOST_AUTO_TEST_CASE(stream_move_fwd_overlap)
{
	BOOST_TEST_MESSAGE("Overlapping stream move forward (process from end to start)");

	stream::move(this->content, 10, 15, 10);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOKLMNOPQRSTZ"),
		"Error in overlapping stream move forward (process from end to start)");
}

BOOST_AUTO_TEST_CASE(stream_move_back_overlap)
{
	BOOST_TEST_MESSAGE("Overlapping stream move backward (process from start to end)");

	stream::move(this->content, 10, 5, 10);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEKLMNOPQRSTPQRSTUVWXYZ"),
		"Error in overlapping stream move backward (process from start to end)");
}

BOOST_AUTO_TEST_CASE(stream_move_fw2)
{
	BOOST_TEST_MESSAGE("Overlapping stream double-move forwards");

	stream::move(this->content, 5, 10, 15);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJFGHIJKLMNOPQRSTZ"),
		"Error in overlapping stream double-move forwards");
}

BOOST_AUTO_TEST_CASE(stream_move_bk2)
{
	BOOST_TEST_MESSAGE("Overlapping stream double-move backwards");

	stream::move(this->content, 10, 5, 5);
	stream::move(this->content, 20, 10, 4);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEKLMNOUVWXOPQRSTUVWXYZ"),
		"Error in overlapping stream double-move backwards");
}

BOOST_AUTO_TEST_CASE(stream_move_extend)
{
	BOOST_TEST_MESSAGE("Stream move past EOF");

	this->content.truncate(30); // extend stringstream to make room

	stream::move(this->content, 5, 20, 10);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOPQRSTFGHIJKLMNO"),
		"Error in stream move past EOF");
}

BOOST_AUTO_TEST_CASE(stream_move_extend_overlap)
{
	BOOST_TEST_MESSAGE("Overlapping stream move past EOF");

	this->content.truncate(35); // extend stringstream to make room

	stream::move(this->content, 5, 15, 20);

	BOOST_CHECK_MESSAGE(is_equal("ABCDEFGHIJKLMNOFGHIJKLMNOPQRSTUVWXY"),
		"Error in overlapping stream move past EOF");
}

BOOST_AUTO_TEST_SUITE_END() // stream_move_suite
