/**
 * @file   stream.cpp
 * @brief  Base stream implementation.
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

#include <cassert>
#include <camoto/stream.hpp>

namespace camoto {
namespace stream {

error::error(const std::string& msg)
	:	camoto::error(msg)
{
}

read_error::read_error(const std::string& msg)
	:	error(msg)
{
}

write_error::write_error(const std::string& msg)
	:	error(msg)
{
}

seek_error::seek_error(const std::string& msg)
	:	error(msg)
{
}

incomplete_write::incomplete_write(stream::len written)
	:	write_error("Incomplete write"),
		bytes_written(written)
{
}

incomplete_read::incomplete_read(stream::len read)
	:	read_error("Incomplete read"),
		bytes_read(read)
{
}

void input::read(uint8_t *buffer, stream::len len)
{
	stream::len r = this->try_read(buffer, len);
	assert(r <= len);
	if (r < len) {
		throw incomplete_read(r);
	}
	return;
}

void input::read(char *buffer, stream::len len)
{
	this->read((uint8_t *)buffer, len);
	return;
}

std::string input::read(stream::len len)
{
	std::string d;
	d.resize(len);
	this->read((uint8_t *)&d[0], len);
	return d;
}

void output::write(const uint8_t *buffer, stream::len len)
{
	stream::len w = this->try_write(buffer, len);
	assert(w <= len);
	if (w < len) {
		throw incomplete_write(w);
	}
	return;
}

void output::write(const char *buffer, stream::len len)
{
	this->write((const uint8_t *)buffer, len);
	return;
}

void output::write(const std::string& buffer)
{
	this->write((const uint8_t *)buffer.data(), buffer.length());
	return;
}

void output::truncate_here()
{
	try {
		stream::pos here = this->tellp();
		this->truncate(here);
	} catch (const seek_error& e) {
		throw write_error("Unable to obtain current seek position for truncate: "
			+ e.get_message());
	}
	return;
}

void copy(output& dest, input& src)
{
	uint8_t buffer[BUFFER_SIZE];
	stream::len total_written = 0;
	stream::len r;
	do {
		r = src.try_read(buffer, sizeof(buffer));
		if (r == 0) break;
		stream::len w = dest.try_write(buffer, r);
		total_written += w;
		if (w < r) {
			// Did not write the full buffer
			throw incomplete_write(total_written);
		}
	} while (r == sizeof(buffer));
	return;
}

void move(inout& data, pos from, pos to, len len)
{
	if (from == to) return; // job done, that was easy

	uint8_t buffer[BUFFER_SIZE];
	stream::len r, w, total_written = 0;
	stream::len szNext;

	stream::pos fromEnd = from + len;
	stream::pos toEnd = to + len;

	stream::len size = data.size();

	// Make sure the caller isn't trying to read or write past the end of the
	// stream (as it needs to be resized first if this is to happen.)
	assert(fromEnd <= size);
	assert(toEnd <= size);

	if (
		(from > to) || // The destination starts before the source
		(fromEnd <= to) || // The source ends before the dest starts (no overlap)
		(toEnd <= from) // The dest ends before the source starts (no overlap)
	) {
		// Moving data back towards the start of the stream, start at the beginning
		// and work towards the last block.
		do {
			// Figure out how much to read next (a full block or the last partial one)
			if (BUFFER_SIZE <= len) {
				szNext = BUFFER_SIZE;
			} else {
				szNext = len;
			}

			// Despite having separate read and write pointers, moving one affects
			// the other so we have to keep seeking all the time.
			try {
				data.seekg(from, stream::start);
				r = data.try_read(buffer, szNext);
			} catch (seek_error& e) {
				throw read_error(e.get_message());
			}

			try {
				data.seekp(to, stream::start);
				w = data.try_write(buffer, r);
			} catch (seek_error& e) {
				throw read_error(e.get_message());
			}

			from += r; to += w;
			len -= r;

			total_written += w;
			if (r != w) {
				throw incomplete_write(total_written);
			}
		} while ((r) && (szNext == BUFFER_SIZE));
	} else {
		// Moving data forwards towards the end of the stream, start at the end
		// and work back towards the first block.

		szNext = BUFFER_SIZE;
/*
		// Check to see if we'll be moving data out past the end of the stream
		if (size < toEnd) {
			// We have to move data past the end of the stream, but since we can't
			// seek past the end we need to enlarge the stream first.  An easy way
			// of doing this is to use the code above (in the other 'if' condition,
			// moving in the opposite direction) to move just the extra data from
			// start-to-end which will push out the stream to the correct size.
			stream::pos offExcess = toEnd - size;
			move(data, size - offExcess, size, offExcess);

			// Now we've moved the last offExcess bytes, so shrink the move operation
			// by that amount and continue as before.
			len -= offExcess;
		}
*/
		do {
			if (
				(fromEnd < BUFFER_SIZE)
				|| (fromEnd - BUFFER_SIZE < from)
			) {
				szNext = fromEnd - from;
				fromEnd = from;
				toEnd = to;
			} else {
				fromEnd -= BUFFER_SIZE;
				toEnd -= BUFFER_SIZE;
			}

			try {
				data.seekg(fromEnd, stream::start);
				r = data.try_read(buffer, szNext);
			} catch (seek_error& e) {
				throw read_error(e.get_message());
			}

			try {
				data.seekp(toEnd, stream::start);
				w = data.try_write(buffer, r);
			} catch (seek_error& e) {
				throw write_error(e.get_message());
			}

			total_written += w;
			if (r != w) {
				throw incomplete_write(total_written);
			}

		} while (fromEnd > from);
	}
	return;
}

} // namespace stream
} // namespace camoto
