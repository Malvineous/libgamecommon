/**
 * @file   tests.cpp
 * @brief  Test code core.
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

#define BOOST_TEST_MODULE libgamecommon
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <boost/algorithm/string.hpp> // for case-insensitive string compare
#include <iostream>
#include <iomanip>

#include <camoto/debug.hpp>
#include <camoto/util.hpp>
#include "tests.hpp"

using namespace camoto;

default_sample::default_sample()
	: outputWidth(32)
{
}

void default_sample::printNice(boost::test_tools::predicate_result& res,
	const std::string& s, const std::string& diff)
{
	const char *c = CLR_YELLOW;
	res.message() << c;
	std::ostringstream text;
	text << c;
	unsigned int len = s.length();
	for (unsigned int i = 0; i < len; i++) {
		if ((i > 0) && (i % this->outputWidth == 0)) {
			res.message() << ' ' << text.str();
			res.message() << CLR_NORM << "\n" << std::setfill('0') << std::setw(3)
				<< std::hex << i << ": " << c;
			text.str("");
			text.seekp(0, std::ios::beg);
			res.message() << c;
			text << c;
		}
		if ((i >= diff.length()) || (s[i] != diff[i])) {
			if (c != CLR_MAG) {
				c = CLR_MAG;
				res.message() << c;
				text << c;
			}
		} else {
			if (c != CLR_YELLOW) {
				c = CLR_YELLOW;
				res.message() << c;
				text << c;
			}
		}
		if ((s[i] < 32) || (s[i] == 127)) {
			text << '.';
		} else {
			text << s[i];
		}
		res.message() << std::setfill('0') << std::setw(2)
			<< std::hex << (int)((uint8_t)s[i]) << ' ';
	}

	// If the last row was only a partial one, pad it out and write the text side
	if (len > 0) {
		for (int i = ((len - 1) % this->outputWidth) + 1; i < this->outputWidth; i++) {
			res.message() << "   ";
		}
		res.message() << ' ' << text.str();
	}
	return;
}

void default_sample::print_wrong(boost::test_tools::predicate_result& res,
	const std::string& strExpected, const std::string& strResult)
{
	res.message() << "\nExp: ";
	this->printNice(res, strExpected, strResult);
	res.message() << CLR_NORM "\n\nGot: ";
	this->printNice(res, strResult, strExpected);
	res.message() << CLR_NORM "\n";

	return;
}

boost::test_tools::predicate_result default_sample::is_equal(
	const std::string& strExpected, const std::string& strCheck)
{
	if (strExpected.compare(strCheck)) {
		boost::test_tools::predicate_result res(false);
		this->print_wrong(res, strExpected, strCheck);
		return res;
	}

	return true;
}
