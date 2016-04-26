/**
 * @file   test-iostream_helpers.cpp
 * @brief  Test code for the iostream helper functions.
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
#include <camoto/stream_string.hpp>
#include <camoto/iostream_helpers.hpp>

using namespace camoto;

BOOST_AUTO_TEST_SUITE(iostream_helpers)

BOOST_AUTO_TEST_CASE(null_padded_write)
{
	BOOST_TEST_MESSAGE("Write null-padded string");
	{
		stream::string content;
		content << nullPadded("AB", 4);
		BOOST_REQUIRE_EQUAL(content.data.length(), 4);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(0), 0x41);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(1), 0x42);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(2), 0x00);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(3), 0x00);
	}
}

BOOST_AUTO_TEST_CASE(null_padded_read)
{
	BOOST_TEST_MESSAGE("Read null-padded string");
	{
		stream::string content;
		content << std::string("ABC\0EFGHIJKL", 12);
		content.seekg(0, stream::start);
		std::string v;
		content >> nullPadded(v, 8);
		BOOST_CHECK(v.compare("ABC") == 0);
		// Make sure all eight characters were read, even though the ones after the
		// null were discarded.
		BOOST_REQUIRE_EQUAL(content.tellg(), 8);
	}
}

BOOST_AUTO_TEST_CASE(fixed_length_read)
{
	BOOST_TEST_MESSAGE("Fixed-length read with embedded nulls");
	{
		stream::string content;
		content << std::string("ABC\0EFGHIJKL", 12);
		content.seekg(0, stream::start);
		std::string v;
		content >> fixedLength(v, 8);
		BOOST_REQUIRE_EQUAL(v.length(), 8);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[0], 0x41);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[1], 0x42);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[2], 0x43);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[3], 0x00);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[4], 0x45);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[5], 0x46);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[6], 0x47);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[7], 0x48);
		BOOST_CHECK_EQUAL((uint8_t)v.c_str()[8], 0x00);
	}
}

BOOST_AUTO_TEST_CASE(null_terminated_write)
{
	BOOST_TEST_MESSAGE("Write null-terminated string");
	{
		stream::string content;
		content << nullTerminated("AB", 4);
		BOOST_REQUIRE_EQUAL(content.data.length(), 3);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(0), 0x41);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(1), 0x42);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(2), 0x00);
	}
}

BOOST_AUTO_TEST_CASE(null_terminated_read)
{
	BOOST_TEST_MESSAGE("Read null-terminated string");
	{
		stream::string content;
		content << std::string("ABC\0EFGHIJKL", 12);
		content.seekg(0, stream::start);
		std::string v;
		content >> nullTerminated(v, 8);
		BOOST_CHECK(v.compare("ABC") == 0);
		// Make sure only the characters up to the null were read, even though there
		// was room to read more.
		BOOST_REQUIRE_EQUAL(content.tellg(), 4);
	}
}

BOOST_AUTO_TEST_CASE(stream_write)
{
	{
		stream::string content;
		content << u16le(0x0123);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(0), 0x23);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(1), 0x01);
	}
	{
		stream::string content;
		content << u32le(0x01234567);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(0), 0x67);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(1), 0x45);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(2), 0x23);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(3), 0x01);
	}
	{
		stream::string content;
		content << u64le(0x0123456789ABCDEF);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(0), 0xEF);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(1), 0xCD);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(2), 0xAB);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(3), 0x89);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(4), 0x67);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(5), 0x45);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(6), 0x23);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(7), 0x01);
	}

	{
		stream::string content;
		content << u16be(0x0123);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(0), 0x01);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(1), 0x23);
	}
	{
		stream::string content;
		content << u32be(0x01234567);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(0), 0x01);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(1), 0x23);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(2), 0x45);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(3), 0x67);
	}
	{
		stream::string content;
		content << u64be(0x0123456789ABCDEF);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(0), 0x01);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(1), 0x23);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(2), 0x45);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(3), 0x67);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(4), 0x89);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(5), 0xAB);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(6), 0xCD);
		BOOST_CHECK_EQUAL((uint8_t)content.data.at(7), 0xEF);
	}

}

BOOST_AUTO_TEST_CASE(stream_read)
{
	{
		stream::string content;
		content << std::string("\x01\x23\x45\x67" "\x89\xAB\xCD\xEF", 8);
		content.seekg(0, stream::start);
		uint16_t v = 0;
		content >> u16le(v);
		BOOST_CHECK_EQUAL(v, 0x2301);
	}
	{
		stream::string content;
		content << "\x01\x23\x45\x67" "\x89\xAB\xCD\xEF";
		content.seekg(0, stream::start);
		uint32_t v = 0;
		content >> u32le(v);
		BOOST_CHECK_EQUAL(v, 0x67452301);
	}
	{
		stream::string content;
		content << "\x01\x23\x45\x67" "\x89\xAB\xCD\xEF";
		content.seekg(0, stream::start);
		uint64_t v = 0;
		content >> u64le(v);
		BOOST_CHECK_EQUAL(v, 0xEFCDAB8967452301);
	}

	{
		stream::string content;
		content << "\x01\x23\x45\x67" "\x89\xAB\xCD\xEF";
		content.seekg(0, stream::start);
		uint16_t v = 0;
		content >> u16be(v);
		BOOST_CHECK_EQUAL(v, 0x123);
	}
	{
		stream::string content;
		content << "\x01\x23\x45\x67" "\x89\xAB\xCD\xEF";
		content.seekg(0, stream::start);
		uint32_t v = 0;
		content >> u32be(v);
		BOOST_CHECK_EQUAL(v, 0x1234567);
	}
	{
		stream::string content;
		content << "\x01\x23\x45\x67" "\x89\xAB\xCD\xEF";
		content.seekg(0, stream::start);
		uint64_t v = 0;
		content >> u64be(v);
		BOOST_CHECK_EQUAL(v, 0x123456789ABCDEF);
	}
}

BOOST_AUTO_TEST_SUITE_END()
