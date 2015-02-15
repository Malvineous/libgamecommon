/**
 * @file   test-iff.cpp
 * @brief  Test code for IFF reader/writer.
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

#include <camoto/iostream_helpers.hpp>
#include <camoto/util.hpp>
#include <camoto/iff.hpp>

#include "tests.hpp"

using namespace camoto;

BOOST_FIXTURE_TEST_SUITE(iff_suite, string_sample)

#define RIFF_CONTENT makeString( \
	"RIFF" "\x5C\x00\x00\x00" "test" \
	"one " "\x07\x00\x00\x00" \
		"abcdefg" "\x00" \
	"LIST" "\x1C\x00\x00\x00" "demo" \
		"dem1" "\x03\x00\x00\x00" \
			"aaa" "\x00" \
		"dem2" "\x04\x00\x00\x00" \
			"bbbb" \
	"two " "\x06\x00\x00\x00" \
		"hijklm" \
	"two " "\x02\x00\x00\x00" \
		"no" \
	"two " "\x03\x00\x00\x00" \
		"pqr" "\x00" \
)

BOOST_AUTO_TEST_CASE(riff_read)
{
	BOOST_TEST_MESSAGE("Read a RIFF file");

	this->in->write(RIFF_CONTENT);

	IFFReader iff(this->in, IFF::Filetype_RIFF);

	IFF::fourcc type;
	iff.open("RIFF", &type);
	BOOST_REQUIRE_EQUAL(type, "test");

	std::vector<IFF::fourcc> chunks = iff.list();
	BOOST_REQUIRE_EQUAL(chunks[0], "one ");
	BOOST_REQUIRE_EQUAL(chunks[1], "LIST");
	BOOST_REQUIRE_EQUAL(chunks[2], "two ");

	iff.open("LIST", &type);
	BOOST_REQUIRE_EQUAL(type, "demo");

	stream::len lenDem2 = iff.seek("dem2");
	std::string contentDem2;
	*this->in >> fixedLength(contentDem2, lenDem2);
	BOOST_REQUIRE_EQUAL(contentDem2, "bbbb");

	iff.root();
	iff.open("RIFF", &type);
	stream::len lenTwo3 = iff.seek(3);
	std::string contentTwo3;
	*this->in >> fixedLength(contentTwo3, lenTwo3);
	BOOST_REQUIRE_EQUAL(contentTwo3, "no");
}

BOOST_AUTO_TEST_CASE(riff_write)
{
	BOOST_TEST_MESSAGE("Write a RIFF file");

	IFFWriter iff(this->out, IFF::Filetype_RIFF);
	iff.begin("RIFF", "test");
		iff.begin("one ");
		this->out.write("abcdefg");
		iff.end();
		iff.begin("LIST", "demo");
			iff.begin("dem1");
			this->out.write("aaa");
			iff.end();
			iff.begin("dem2");
			this->out.write("bbbb");
			iff.end();
		iff.end();
		iff.begin("two ");
		this->out.write("hijklm");
		iff.end();
		iff.begin("two ");
		this->out.write("no");
		iff.end();
		iff.begin("two ");
		this->out.write("pqr");
		iff.end();
	iff.end();

	BOOST_CHECK_MESSAGE(is_equal(RIFF_CONTENT),
		"Writing RIFF file failed");
}

BOOST_AUTO_TEST_CASE(riff_read_missing_pad)
{
	BOOST_TEST_MESSAGE("Read a padded RIFF file with a missing pad byte");

	this->in->write(makeString(
		"RIFF" "\x17\x00\x00\x00" "test"
		"one " "\x03\x00\x00\x00" "aaa"
		// missing final pad byte
	));

	IFFReader iff(this->in, IFF::Filetype_RIFF);

	IFF::fourcc type;
	iff.open("RIFF", &type);
	BOOST_REQUIRE_EQUAL(type, "test");

	std::vector<IFF::fourcc> chunks = iff.list();
	BOOST_REQUIRE_EQUAL(chunks[0], "one ");

	stream::len lenOne = iff.seek("one ");
	std::string contentOne;
	*this->in >> fixedLength(contentOne, lenOne);
	BOOST_REQUIRE_EQUAL(contentOne, "aaa");
}

#define IFF_CONTENT makeString( \
	"FORM" "\x00\x00\x00\x46" "test" \
	"one " "\x00\x00\x00\x07" \
		"abcdefg" "\x00" \
	"LIST" "\x00\x00\x00\x1C" "demo" \
		"dem1" "\x00\x00\x00\x03" \
			"aaa" "\x00" \
		"dem2" "\x00\x00\x00\x04" \
			"bbbb" \
	"two " "\x00\x00\x00\x06" \
		"hijklm" \
)

BOOST_AUTO_TEST_CASE(iff_read)
{
	BOOST_TEST_MESSAGE("Read an IFF file");

	this->in->write(IFF_CONTENT);

	IFFReader iff(this->in, IFF::Filetype_IFF);

	IFF::fourcc type;
	iff.open("FORM", &type);
	BOOST_REQUIRE_EQUAL(type, "test");

	std::vector<IFF::fourcc> chunks = iff.list();
	BOOST_REQUIRE_EQUAL(chunks[0], "one ");
	BOOST_REQUIRE_EQUAL(chunks[1], "LIST");
	BOOST_REQUIRE_EQUAL(chunks[2], "two ");

	// Make sure out-of-range seek request fails
	BOOST_CHECK_THROW(iff.seek(4), stream::error);

	iff.open("LIST", &type);
	BOOST_REQUIRE_EQUAL(type, "demo");

	stream::len lenDem2 = iff.seek("dem2");
	std::string contentDem2;
	*this->in >> fixedLength(contentDem2, lenDem2);
	BOOST_REQUIRE_EQUAL(contentDem2, "bbbb");
}

BOOST_AUTO_TEST_CASE(iff_write)
{
	BOOST_TEST_MESSAGE("Write an IFF file");

	IFFWriter iff(this->out, IFF::Filetype_IFF);
	iff.begin("FORM", "test");
		iff.begin("one ");
		this->out.write("abcdefg");
		iff.end();
		iff.begin("LIST", "demo");
			iff.begin("dem1");
			this->out.write("aaa");
			iff.end();
			iff.begin("dem2");
			this->out.write("bbbb");
			iff.end();
		iff.end();
		iff.begin("two ");
		this->out.write("hijklm");
		iff.end();
	iff.end();

	BOOST_CHECK_MESSAGE(is_equal(IFF_CONTENT),
		"Writing IFF file failed");
}

#define RIFF_CONTENT_UNPADDED makeString( \
	"RIFF" "\x59\x00\x00\x00" "test" \
	"one " "\x07\x00\x00\x00" \
		"abcdefg" \
	"LIST" "\x1B\x00\x00\x00" "demo" \
		"dem1" "\x03\x00\x00\x00" \
			"aaa" \
		"dem2" "\x04\x00\x00\x00" \
			"bbbb" \
	"two " "\x06\x00\x00\x00" \
		"hijklm" \
	"two " "\x02\x00\x00\x00" \
		"no" \
	"two " "\x03\x00\x00\x00" \
		"pqr" \
)

BOOST_AUTO_TEST_CASE(riff_read_unpadded)
{
	BOOST_TEST_MESSAGE("Read an unpadded RIFF file");

	this->in->write(RIFF_CONTENT_UNPADDED);

	IFFReader iff(this->in, IFF::Filetype_RIFF_Unpadded);

	IFF::fourcc type;
	iff.open("RIFF", &type);
	BOOST_REQUIRE_EQUAL(type, "test");

	std::vector<IFF::fourcc> chunks = iff.list();
	BOOST_REQUIRE_EQUAL(chunks[0], "one ");
	BOOST_REQUIRE_EQUAL(chunks[1], "LIST");
	BOOST_REQUIRE_EQUAL(chunks[2], "two ");

	iff.open("LIST", &type);
	BOOST_REQUIRE_EQUAL(type, "demo");

	stream::len lenDem2 = iff.seek("dem2");
	std::string contentDem2;
	*this->in >> fixedLength(contentDem2, lenDem2);
	BOOST_REQUIRE_EQUAL(contentDem2, "bbbb");

	iff.root();
	iff.open("RIFF", &type);
	stream::len lenTwo3 = iff.seek(3);
	std::string contentTwo3;
	*this->in >> fixedLength(contentTwo3, lenTwo3);
	BOOST_REQUIRE_EQUAL(contentTwo3, "no");
}

BOOST_AUTO_TEST_CASE(riff_write_unpadded)
{
	BOOST_TEST_MESSAGE("Write a RIFF file");

	IFFWriter iff(this->out, IFF::Filetype_RIFF_Unpadded);
	iff.begin("RIFF", "test");
		iff.begin("one ");
		this->out.write("abcdefg");
		iff.end();
		iff.begin("LIST", "demo");
			iff.begin("dem1");
			this->out.write("aaa");
			iff.end();
			iff.begin("dem2");
			this->out.write("bbbb");
			iff.end();
		iff.end();
		iff.begin("two ");
		this->out.write("hijklm");
		iff.end();
		iff.begin("two ");
		this->out.write("no");
		iff.end();
		iff.begin("two ");
		this->out.write("pqr");
		iff.end();
	iff.end();

	BOOST_CHECK_MESSAGE(is_equal(RIFF_CONTENT_UNPADDED),
		"Writing RIFF file failed");
}

BOOST_AUTO_TEST_SUITE_END()
