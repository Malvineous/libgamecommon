/**
 * @file   test-stream_seg.cpp
 * @brief  Test code for segmented stream class.
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

#include <iostream>
#include <errno.h>
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>
#include <camoto/stream_seg.hpp>
#include <camoto/stream_string.hpp>
#include <camoto/stream_sub.hpp>
#include <camoto/util.hpp> // std::make_unique
#include "tests.hpp"

using namespace camoto;

// Allow a string constant to be passed around with embedded nulls
#define makeString(x)  std::string((x), sizeof((x)) - 1)

struct stream_seg_sample: public default_sample
{
	std::string* baseContent; // Only valid while seg is
	std::shared_ptr<stream::seg> seg;

	stream_seg_sample()
	{
		auto base = std::make_unique<stream::string>("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
		this->baseContent = &base->data;
		this->seg = std::make_shared<stream::seg>(std::move(base));

		// Make sure the data went in correctly to begin the test
		BOOST_REQUIRE(this->baseContent->compare("ABCDEFGHIJKLMNOPQRSTUVWXYZ") == 0);

		BOOST_REQUIRE_EQUAL(this->seg->tellg(), 0);
		BOOST_REQUIRE_EQUAL(this->seg->size(), 26);
	}

	boost::test_tools::predicate_result is_equal(int pos, const std::string& strExpected)
	{
		// Make sure the file offset hasn't changed after a commit (-1 is passed
		// in pos for "don't care" to skip this test.)
		if (pos >= 0) BOOST_CHECK_EQUAL(this->seg->tellp(), pos);

		// See if the stringstream now matches what we expected
		return this->default_sample::is_equal(strExpected, *this->baseContent);
	}
};

BOOST_FIXTURE_TEST_SUITE(stream_seg_suite, stream_seg_sample)

BOOST_AUTO_TEST_CASE(segstream_no_change)
{
	BOOST_TEST_MESSAGE("Flush with no change");

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(0, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
		"Flush with no change failed");
}

BOOST_AUTO_TEST_CASE(segstream_streamMove_back)
{
	BOOST_TEST_MESSAGE("Overlapping stream move backwards (segmented_stream this time)");

	stream::move(*this->seg, 10, 5, 10);

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(-1, "ABCDEKLMNOPQRSTPQRSTUVWXYZ"),
		"Overlapping stream move backwards (segmented_stream this time) failed");
}

BOOST_AUTO_TEST_CASE(segstream_streamMove_forward)
{
	BOOST_TEST_MESSAGE("Overlapping stream move forward (segmented_stream this time)");

	stream::move(*this->seg, 10, 15, 10);

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(-1, "ABCDEFGHIJKLMNOKLMNOPQRSTZ"),
		"Overlapping stream move forward (segmented_stream this time) failed");
}

BOOST_AUTO_TEST_CASE(segstream_seek_write)
{
	BOOST_TEST_MESSAGE("Seek and write");

	this->seg->seekp(5, stream::start);
	this->seg->write("123456");

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(11, "ABCDE123456LMNOPQRSTUVWXYZ"),
		"Seek and write failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_write)
{
	BOOST_TEST_MESSAGE("Insert and write into inserted space only");

	this->seg->seekp(4, stream::start);
	this->seg->insert(5);
	this->seg->write("12345");

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(9, "ABCD12345EFGHIJKLMNOPQRSTUVWXYZ"),
		"Insert and write failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_write_src3)
{
	BOOST_TEST_MESSAGE("Insert and write over into third source");

	this->seg->seekp(4, stream::start);
	this->seg->insert(4);
	this->seg->write("123456");

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(10, "ABCD123456GHIJKLMNOPQRSTUVWXYZ"),
		"Insert and write over into third source failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_within_insert)
{
	BOOST_TEST_MESSAGE("Insert within inserted segment");

	this->seg->seekp(5, stream::start);
	this->seg->insert(10);
	this->seg->write("0123456789");
	this->seg->seekp(-5, stream::cur);
	this->seg->insert(4);
	this->seg->write("!@#$");

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(14, "ABCDE01234!@#$56789FGHIJKLMNOPQRSTUVWXYZ"),
		"Insert within inserted segment failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_twice)
{
	BOOST_TEST_MESSAGE("Insert and insert again in third part");

	this->seg->seekp(5, stream::start);
	this->seg->insert(5);
	this->seg->write("12345");
	this->seg->seekp(5, stream::cur);
	this->seg->insert(5);
	this->seg->write("67890");

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(20, "ABCDE12345FGHIJ67890KLMNOPQRSTUVWXYZ"),
		"Insert and insert again in third part failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_twice_no_seek)
{
	BOOST_TEST_MESSAGE("Write into third stream then insert with no seek");

	this->seg->seekp(5, stream::start);
	this->seg->insert(4);
	this->seg->write("123456");
	this->seg->insert(4);
	this->seg->write("123456");

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(17, "ABCDE123456123456JKLMNOPQRSTUVWXYZ"),
		"Write into third stream then insert with no seek failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_at_eof)
{
	BOOST_TEST_MESSAGE("Insert at EOF");

	this->seg->seekp(0, stream::end);
	this->seg->insert(4);
	this->seg->write("1234");

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(30, "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234"),
		"Insert at EOF failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_at_eof_overwrite)
{
	BOOST_TEST_MESSAGE("Insert at EOF and overwrite");

	this->seg->seekp(0, stream::end);
	this->seg->insert(8);
	this->seg->write("12345678");
	this->seg->seekp(-8, stream::cur);
	this->seg->write("!@#$");

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(30, "ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$5678"),
		"Insert at EOF and overwrite failed");
}

BOOST_AUTO_TEST_CASE(segstream_remove_from_eof)
{
	BOOST_TEST_MESSAGE("Remove data from EOF, reducing file size");

	this->seg->seekp(21, stream::start);
	this->seg->remove(5);

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(21, "ABCDEFGHIJKLMNOPQRSTU"),
		"Remove data from EOF, reducing file size failed");
}

BOOST_AUTO_TEST_CASE(segstream_remove_write)
{
	BOOST_TEST_MESSAGE("Remove data from middle of stream, then write before it");

	this->seg->seekp(20, stream::start);
	this->seg->remove(5);
	this->seg->seekp(10, stream::start);
	this->seg->remove(5);
	this->seg->seekp(3, stream::start);
	this->seg->write("1234");

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(7, "ABC1234HIJPQRSTZ"),
		"Remove data from middle of stream, then write before it failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_remove_before)
{
	BOOST_TEST_MESSAGE("Insert block, then remove just before new block");

	this->seg->seekp(4, stream::start);
	this->seg->insert(5);
	this->seg->write("12345");
	this->seg->seekp(2, stream::start);
	this->seg->remove(2);

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(2, "AB12345EFGHIJKLMNOPQRSTUVWXYZ"),
		"Insert block, then remove just before new block failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_remove_start)
{
	BOOST_TEST_MESSAGE("Insert block, then remove start of new block");

	this->seg->seekp(4, stream::start);
	this->seg->insert(5);
	this->seg->write("12345");
	this->seg->seekp(4, stream::start);
	this->seg->remove(3);

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(4, "ABCD45EFGHIJKLMNOPQRSTUVWXYZ"),
		"Insert block, then remove start of new block failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_remove_within)
{
	BOOST_TEST_MESSAGE("Insert block, then remove within new block");

	this->seg->seekp(4, stream::start);
	this->seg->insert(5);
	this->seg->write("12345");
	this->seg->seekp(5, stream::start);
	this->seg->remove(3);

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(5, "ABCD15EFGHIJKLMNOPQRSTUVWXYZ"),
		"Insert block, then remove within new block failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_remove_entirely)
{
	BOOST_TEST_MESSAGE("Insert block, then remove around (including) new block");

	this->seg->seekp(4, stream::start);
	this->seg->insert(5);
	this->seg->write("12345");
	this->seg->seekp(2, stream::start);
	this->seg->remove(9);

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(2, "ABGHIJKLMNOPQRSTUVWXYZ"),
		"Insert block, then remove around (including) new block failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_remove_across_sources_small)
{
	BOOST_TEST_MESSAGE("Insert block, then remove across block boundary (< inserted block size)");

	this->seg->seekp(4, stream::start);
	this->seg->insert(5);
	this->seg->write("12345");
	this->seg->seekp(7, stream::start);
	this->seg->remove(4);

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(7, "ABCD123GHIJKLMNOPQRSTUVWXYZ"),
		"Insert block, then remove across block boundary (< inserted block size) failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_remove_across_sources_large)
{
	BOOST_TEST_MESSAGE("Insert block, then remove across block boundary (> inserted block size)");

	this->seg->seekp(4, stream::start);
	this->seg->insert(5);
	this->seg->write("12345");
	this->seg->seekp(7, stream::start);
	this->seg->remove(8);

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(7, "ABCD123KLMNOPQRSTUVWXYZ"),
		"Insert block, then remove across block boundary (> inserted block size) failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_remove_src3)
{
	BOOST_TEST_MESSAGE("Insert block, then remove from third source");

	this->seg->seekp(5, stream::start);
	this->seg->insert(5);
	this->seg->write("12345");
	this->seg->seekp(15, stream::start);
	this->seg->remove(6);

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(15, "ABCDE12345FGHIJQRSTUVWXYZ"),
		"Insert block, then remove from third source failed");
}

BOOST_AUTO_TEST_CASE(segstream_large_insert)
{
	BOOST_TEST_MESSAGE("Insert large block so third source is pushed past EOF");

	this->seg->seekp(20, stream::start);
	this->seg->insert(10);
	this->seg->write("1234567890");

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(30, "ABCDEFGHIJKLMNOPQRST1234567890UVWXYZ"),
		"Insert large block so third source is pushed past EOF failed");
}

BOOST_AUTO_TEST_CASE(segstream_large_insert_gap)
{
	BOOST_TEST_MESSAGE("Insert large block so third source is pushed past EOF (with gap)");

	this->seg->seekp(20, stream::start);
	this->seg->insert(15);
	this->seg->write("1234567890");

	this->seg->flush();

	BOOST_CHECK_MESSAGE(is_equal(30,
		makeString(
			"ABCDEFGHIJKLMNOPQRST1234567890\0\0\0\0\0UVWXYZ"
		)),
		"Insert large block so third source is pushed past EOF (with gap) failed");
}


bool substreamTruncate(stream::output_sub *sub, stream::pos len,
	std::weak_ptr<stream::seg> w_parent)
{
	std::shared_ptr<stream::seg> parent = w_parent.lock();
	if (!parent) return false;

	stream::pos off = sub->start();
	stream::pos orig = parent->tellp();

	stream::pos origSub = sub->tellp();
	sub->seekp(0, stream::end);
	stream::pos oldLen = sub->tellp();
	sub->seekp(origSub, stream::start);

	// Enlarge the end of the substream (in the parent)
	stream::delta delta = len - oldLen;
	try {
		if (delta < 0) {
			// TODO: untested
			parent->seekp(off + oldLen + delta, stream::start);
			parent->remove(-delta);
		} else {
			parent->seekp(off + oldLen, stream::start);
			parent->insert(delta);
		}
		// Update the substream with its new size
		sub->resize(len);
	} catch (stream::error& e) {
		std::cerr << "Error resizing substream: " << e.what() << std::endl;
		return false;
	}
	parent->seekp(orig, stream::start);
	return true;
}

// This reproduces a crash discovered by the fmt-rff-blood archive handler
BOOST_AUTO_TEST_CASE(segstream_insert_past_parent_eof)
{
	BOOST_TEST_MESSAGE("Make segstream commit past parent's EOF");

	this->seg->seekp(0, stream::start);

	auto sub = std::make_unique<stream::sub>(
		this->seg, 15, 10,
		boost::bind(substreamTruncate,
			_1, _2,
			std::weak_ptr<stream::seg>(this->seg)
		)
	);

	auto child = std::make_shared<stream::seg>(std::move(sub));

	child->seekp(8, stream::start);
	child->insert(5);
	child->flush();

	BOOST_CHECK_MESSAGE(is_equal(-1,
		makeString(
			"ABCDEFGHIJKLMNOPQRSTUVW\0\0\0\0\0XYZ"
		)),
		"Make segstream commit past parent's EOF");
}

BOOST_AUTO_TEST_CASE(segstream_insert_past_parent_eof2)
{
	BOOST_TEST_MESSAGE("Make segstream commit past parent's EOF");

	this->seg->seekp(0, stream::start);

	auto sub = std::make_unique<stream::sub>(
		this->seg, 15, 10,
		boost::bind(substreamTruncate,
			_1, _2,
			std::weak_ptr<stream::seg>(this->seg)
		)
	);

	auto child = std::make_shared<stream::seg>(std::move(sub));

	child->seekp(0, stream::start); // will seek to @15 out of 25 (15+10)
	child->insert(5);
	child->flush();

	BOOST_CHECK_MESSAGE(is_equal(-1,
		makeString(
			"ABCDEFGHIJKLMNO\0\0\0\0\0PQRSTUVWXYZ"
		)),
		"Make segstream commit past parent's EOF");
}

BOOST_AUTO_TEST_CASE(segstream_insert_c01)
{
	BOOST_TEST_MESSAGE("Insert into first source");

	BOOST_CHECKPOINT("01 About to seek");
	this->seg->seekp(4, stream::start);
	BOOST_CHECKPOINT("02 About to insert");
	this->seg->insert(5);
	BOOST_CHECKPOINT("03 About to write");
	this->seg->write("12345");

	this->seg->flush();

	BOOST_REQUIRE_MESSAGE(is_equal(9, "ABCD12345EFGHIJKLMNOPQRSTUVWXYZ"),
		"Insert into first source failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_c02)
{
	BOOST_TEST_MESSAGE("Insert into second source");

	BOOST_CHECKPOINT("01 About to seek");
	this->seg->seekp(4, stream::start);
	BOOST_CHECKPOINT("02 About to insert");
	this->seg->insert(5);
	BOOST_CHECKPOINT("03 About to write");
	this->seg->write("12345");

	// Can't check here or the commit flattens the data and the third source
	// no longer exists.
	/*BOOST_REQUIRE_MESSAGE(is_equal(9, "ABCD12345EFGHIJKLMNOPQRSTUVWXYZ"),
		"Insert into second source failed");*/

	BOOST_CHECKPOINT("04 About to seek");
	this->seg->seekp(6, stream::start);
	BOOST_CHECKPOINT("05 About to insert");
	this->seg->insert(3);
	BOOST_CHECKPOINT("06 About to write");
	this->seg->write("!@#");

	this->seg->flush();

	BOOST_REQUIRE_MESSAGE(is_equal(9, "ABCD12!@#345EFGHIJKLMNOPQRSTUVWXYZ"),
		"Insert into second source failed");
}

BOOST_AUTO_TEST_CASE(segstream_insert_c03)
{
	BOOST_TEST_MESSAGE("Insert into third source");

	BOOST_CHECKPOINT("01 About to seek");
	this->seg->seekp(4, stream::start);
	BOOST_CHECKPOINT("02 About to insert");
	this->seg->insert(5);
	BOOST_CHECKPOINT("03 About to write");
	this->seg->write("12345");

	// Can't check here or the commit flattens the data and the third source
	// no longer exists.
	/*BOOST_REQUIRE_MESSAGE(is_equal(9, "ABCD12345EFGHIJKLMNOPQRSTUVWXYZ"),
		"Insert into second source failed");*/

	BOOST_CHECKPOINT("04 About to seek");
	this->seg->seekp(15, stream::start);
	BOOST_CHECKPOINT("05 About to insert");
	this->seg->insert(3);
	BOOST_CHECKPOINT("06 About to write");
	this->seg->write("!@#");

	// Do it again (this time it'll be the third source's third source.)
	BOOST_CHECKPOINT("07 About to seek");
	this->seg->seekp(20, stream::start);
	BOOST_CHECKPOINT("08 About to insert");
	this->seg->insert(3);
	BOOST_CHECKPOINT("09 About to write");
	this->seg->write("$%^");

	this->seg->flush();

	BOOST_REQUIRE_MESSAGE(is_equal(23, "ABCD12345EFGHIJ!@#KL$%^MNOPQRSTUVWXYZ"),
		"Insert into third source failed");
}

BOOST_AUTO_TEST_CASE(segstream_remove_c01)
{
	BOOST_TEST_MESSAGE("Remove from start of first source");

	BOOST_CHECKPOINT("01 About to seek");
	this->seg->seekp(0, stream::start);
	BOOST_CHECKPOINT("02 About to remove");
	this->seg->remove(5);

	this->seg->flush();

	BOOST_REQUIRE_MESSAGE(is_equal(0,
		makeString(
			"FGHIJKLMNOPQRSTUVWXYZ"
		)),
		"Remove from start of first source failed");

	BOOST_CHECKPOINT("03 About to remove");
	this->seg->remove(5);

	this->seg->flush();

	BOOST_REQUIRE_MESSAGE(is_equal(0,
		makeString(
			"KLMNOPQRSTUVWXYZ"
		)),
		"Second removal from start of first source failed");
}

BOOST_AUTO_TEST_CASE(segstream_remove_c02)
{
	BOOST_TEST_MESSAGE("Remove data from middle of stream");

	BOOST_CHECKPOINT("01 About to seek");
	this->seg->seekp(20, stream::start);
	BOOST_CHECKPOINT("02 About to remove");
	this->seg->remove(5);

	this->seg->flush();

	BOOST_REQUIRE_MESSAGE(is_equal(20, "ABCDEFGHIJKLMNOPQRSTZ"),
		"Remove data from middle of stream failed");

	BOOST_CHECKPOINT("03 About to seek");
	this->seg->seekp(5, stream::start);
	BOOST_CHECKPOINT("04 About to remove");
	this->seg->remove(6);

	this->seg->flush();

	BOOST_REQUIRE_MESSAGE(is_equal(5, "ABCDELMNOPQRSTZ"),
		"Remove data from middle of stream failed");
}

BOOST_AUTO_TEST_CASE(segstream_remove_c03)
{
	BOOST_TEST_MESSAGE("Remove data within third source");

	BOOST_CHECKPOINT("01 About to seek");
	this->seg->seekp(10, stream::start);
	BOOST_CHECKPOINT("02 About to remove");
	this->seg->remove(5);

	// Can't check here or the commit flattens the data and the third source
	// no longer exists.
	/*BOOST_REQUIRE_MESSAGE(is_equal(10, "ABCDEFGHIJPQRSTUVWXYZ"),
		"Remove data from middle of first source failed");*/

	BOOST_CHECKPOINT("03 About to seek");
	this->seg->seekp(15, stream::start);
	BOOST_CHECKPOINT("04 About to remove");
	this->seg->remove(5);

	this->seg->flush();

	BOOST_REQUIRE_MESSAGE(is_equal(15, "ABCDEFGHIJPQRSTZ"),
		"Remove data within third source failed");
}

BOOST_AUTO_TEST_CASE(segstream_remove_c04)
{
	BOOST_TEST_MESSAGE("Remove data up to end of first source");

	BOOST_CHECKPOINT("01 About to seek");
	this->seg->seekp(20, stream::start);
	BOOST_CHECKPOINT("02 About to remove");
	this->seg->remove(6);

	this->seg->flush();

	BOOST_REQUIRE_MESSAGE(is_equal(20, "ABCDEFGHIJKLMNOPQRST"),
		"Remove data up to end of first source failed");

	BOOST_CHECKPOINT("03 About to seek");
	this->seg->seekp(15, stream::start);
	BOOST_CHECKPOINT("04 About to remove");
	this->seg->remove(5);

	this->seg->flush();

	BOOST_REQUIRE_MESSAGE(is_equal(15, "ABCDEFGHIJKLMNO"),
		"Second removal up to end of first source failed");
}

BOOST_AUTO_TEST_CASE(segstream_remove_c05)
{
	BOOST_TEST_MESSAGE("Remove entire second source");

	BOOST_CHECKPOINT("01 About to seek");
	this->seg->seekp(10, stream::start);
	BOOST_CHECKPOINT("02 About to insert");
	this->seg->insert(5);
	BOOST_CHECKPOINT("03 About to write into inserted space");
	this->seg->write("12345");

	// Can't check here or the commit flattens the data and the third source
	// no longer exists.
	/*BOOST_REQUIRE_MESSAGE(is_equal(15, "ABCDEFGHIJ12345KLMNOPQRSTUVWXYZ"),
		"Removing entire second source failed during set up phase");*/

	BOOST_CHECKPOINT("04 About to seek");
	this->seg->seekp(10, stream::start);
	BOOST_CHECKPOINT("05 About to remove");
	this->seg->remove(5);

	this->seg->flush();

	BOOST_REQUIRE_MESSAGE(is_equal(10, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"),
		"Removing entire second source failed");
}

BOOST_AUTO_TEST_CASE(segstream_remove_c06)
{
	BOOST_TEST_MESSAGE("Remove start of second source");

	BOOST_CHECKPOINT("01 About to seek");
	this->seg->seekp(10, stream::start);
	BOOST_CHECKPOINT("02 About to insert");
	this->seg->insert(5);
	BOOST_CHECKPOINT("03 About to write into inserted space");
	this->seg->write("12345");

	// Can't check here or the commit flattens the data and the third source
	// no longer exists.
	/*BOOST_REQUIRE_MESSAGE(is_equal(15, "ABCDEFGHIJ12345KLMNOPQRSTUVWXYZ"),
		"Removing entire second source failed during set up phase");*/

	BOOST_CHECKPOINT("04 About to seek");
	this->seg->seekp(10, stream::start);
	BOOST_CHECKPOINT("05 About to remove");
	this->seg->remove(3);

	this->seg->flush();

	BOOST_REQUIRE_MESSAGE(is_equal(10, "ABCDEFGHIJ45KLMNOPQRSTUVWXYZ"),
		"Removing start of second source failed");
}

BOOST_AUTO_TEST_CASE(segstream_remove_c07)
{
	BOOST_TEST_MESSAGE("Remove end of second source");

	BOOST_CHECKPOINT("01 About to seek");
	this->seg->seekp(10, stream::start);
	BOOST_CHECKPOINT("02 About to insert");
	this->seg->insert(5);
	BOOST_CHECKPOINT("03 About to write into inserted space");
	this->seg->write("12345");

	// Can't check here or the commit flattens the data and the third source
	// no longer exists.
	/*BOOST_REQUIRE_MESSAGE(is_equal(15, "ABCDEFGHIJ12345KLMNOPQRSTUVWXYZ"),
		"Removing entire second source failed during set up phase");*/

	BOOST_CHECKPOINT("04 About to seek");
	this->seg->seekp(12, stream::start);
	BOOST_CHECKPOINT("05 About to remove");
	this->seg->remove(3);

	// Do it again
	BOOST_CHECKPOINT("04 About to seek");
	this->seg->seekp(11, stream::start);
	BOOST_CHECKPOINT("05 About to remove");
	this->seg->remove(1);

	this->seg->flush();

	BOOST_REQUIRE_MESSAGE(is_equal(11, "ABCDEFGHIJ1KLMNOPQRSTUVWXYZ"),
		"Removing end of second source failed");
}

BOOST_AUTO_TEST_CASE(segstream_remove_c08)
{
	BOOST_TEST_MESSAGE("Remove middle of second source");

	BOOST_CHECKPOINT("01 About to seek");
	this->seg->seekp(10, stream::start);
	BOOST_CHECKPOINT("02 About to insert");
	this->seg->insert(5);
	BOOST_CHECKPOINT("03 About to write into inserted space");
	this->seg->write("12345");

	// Can't check here or the commit flattens the data and the third source
	// no longer exists.
	/*BOOST_REQUIRE_MESSAGE(is_equal(15, "ABCDEFGHIJ12345KLMNOPQRSTUVWXYZ"),
		"Removing entire second source failed during set up phase");*/

	BOOST_CHECKPOINT("04 About to seek");
	this->seg->seekp(11, stream::start);
	BOOST_CHECKPOINT("05 About to remove");
	this->seg->remove(2);
	// Do it again
	BOOST_CHECKPOINT("06 About to remove");
	this->seg->remove(1);

	this->seg->flush();

	BOOST_REQUIRE_MESSAGE(is_equal(11, "ABCDEFGHIJ15KLMNOPQRSTUVWXYZ"),
		"Removing middle of second source failed");
}

BOOST_AUTO_TEST_SUITE_END()
