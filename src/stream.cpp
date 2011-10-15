/**
 * @file   stream.cpp
 * @brief  Base stream implementation.
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

#include <camoto/stream.hpp>

namespace camoto {
namespace stream {

error::error(const std::string& msg)
	throw () :
		msg(msg)
{
}

error::~error()
	throw ()
{
}

const char *error::what()
	throw ()
{
	return this->msg.c_str();
}

read_error::read_error(const std::string& msg)
	throw () :
		error(msg)
{
}

write_error::write_error(const std::string& msg)
	throw () :
		error(msg)
{
}

seek_error::seek_error(const std::string& msg)
	throw () :
		error(msg)
{
}

incomplete_write::incomplete_write(stream::len written)
	throw () :
		write_error("Incomplete write"),
		bytes_written(written)
{
}

incomplete_read::incomplete_read(stream::len read)
	throw () :
		read_error("Incomplete read"),
		bytes_read(read)
{
}

void input::read(uint8_t *buffer, stream::len len)
	throw (incomplete_read, read_error)
{
	stream::len r = this->try_read(buffer, len);
	if (r < len) {
		throw incomplete_read(r);
	}
	return;
}

std::string input::read(stream::len len)
	throw (incomplete_read, read_error)
{
	std::string d;
	d.resize(len);
	this->read((uint8_t *)&d[0], len);
	return d;
}

void output::write(const uint8_t *buffer, stream::len len)
	throw (incomplete_write, write_error)
{
	stream::len w = this->try_write(buffer, len);
	if (w < len) {
		throw incomplete_write(w);
	}
	return;
}

void output::write(const std::string& buffer)
	throw (incomplete_write, write_error)
{
	this->write((const uint8_t *)buffer.data(), buffer.length());
	return;
}

void output::truncate_here()
	throw (seek_error)
{
	stream::pos here = this->tellp();
	this->truncate(here);
	return;
}

void copy(output_sptr dest, input_sptr src)
	throw (read_error, write_error, incomplete_write)
{
	uint8_t buffer[4096];
	stream::len total_written = 0;
	stream::len r;
	do {
		r = src->try_read(buffer, sizeof(buffer));
		if (r == 0) break;
		stream::len w = dest->try_write(buffer, r);
		total_written += w;
		if (w < r) {
			// Did not write the full buffer
			throw incomplete_write(total_written);
		}
	} while (r == sizeof(buffer));
	return;
}

} // namespace stream
} // namespace camoto
