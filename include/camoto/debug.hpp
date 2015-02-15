/**
 * @file  camoto/debug.hpp
 * @brief Helper functions to assist with debugging.
 *
 * Copyright (C) 2009-2015 Adam Nielsen <malvineous@shikadi.net>
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

// Compile in debugging output?
//#define DEBUG

#ifndef _CAMOTO_DEBUG_HPP_
#define _CAMOTO_DEBUG_HPP_

#include <iostream>
#include <iomanip>
#include <camoto/stream.hpp>

namespace camoto {

#define CLR_NORM   "\033[22;39m"
#define CLR_GREY   "\033[1;30m"
#define CLR_RED    "\033[1;31m"
#define CLR_GREEN  "\033[1;32m"
#define CLR_YELLOW "\033[1;33m"
#define CLR_MAG    "\033[1;35m"
#define CLR_CYAN   "\033[1;36m"
#define CLR_WHITE  "\033[1;37m"

/// Dump a stream to stdout in human-readable form.
/**
 * @param data
 *   Stream to dump.
 *
 * @param start
 *   Index of first byte to dump, default is from the beginning of the stream.
 *
 * @param end
 *   Index of last byte to dump, default is to dump until EOF.  Specify 0 to
 *   dump until EOF.
 *
 * @param width
 *   Width of hex dump (in source bytes) before it wraps to the next line.
 *
 * @param hexOnly
 *   Only use hex codes.  Default is false which displays ASCII-printable
 *   characters.  Set to true to only display hex numbers.
 *
 * @post Stream is unchanged - read pointer is restored to where it
 *   was when this function was called.
 */
inline void hexdumpStream(stream::input& data, stream::len start = 0,
	stream::len end = 0, unsigned int width = 16, bool hexOnly = false)
{
	stream::pos orig = data.tellg();
	stream::len lenData;
	if (end > 0) lenData = std::min(end, data.size());
	else lenData = data.size();
	if (lenData < start) {
		std::cout << "hexdumpStream(): Cannot dump stream, end pos " << end
			<< " is before start pos " << start << std::endl;
		return;
	}
	lenData -= start;
	data.seekg(start, stream::start);
	std::string buf;
	try {
		buf = data.read(lenData);
	} catch (const stream::read_error&) {
		std::cout << "hexdumpStream(): read error" << std::endl;
		data.seekg(orig, stream::start);
		return;
	}
	for (stream::len i = 0; i < lenData; i++) {
		if (i % width == 0) {
			std::cout << CLR_NORM;
			if (i > 0) {
				std::cout << "\n";
			}
			std::cout << std::setfill('0') << std::setw(3)
				<< std::hex << i << ": " << CLR_GREEN;
		}
		if (hexOnly) {
			std::cout << "\\x" << std::setfill('0') << std::setw(2)
				<< std::hex << (int)((uint8_t)buf[i]);
		} else {
			if ((buf[i] < 32) || (buf[i] == 127)) {
				std::cout << std::setfill('0') << std::setw(2)
					<< std::hex << (int)((uint8_t)buf[i]) << ' ';
			} else {
				std::cout << '_' << buf[i] << ' ';
			}
		}
	}
	std::cout << CLR_NORM << std::endl;
	data.seekg(orig, stream::start);
	return;
}

} // namespace camoto

#endif // _CAMOTO_DEBUG_HPP_
