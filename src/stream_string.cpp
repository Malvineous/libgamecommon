/**
 * @file   stream_string.cpp
 * @brief  Provide a stream implementation around a C++ string.
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
#include <string.h>
#include <camoto/stream_string.hpp>
#include <camoto/util.hpp>

namespace camoto {
namespace stream {

string_core::string_core()
	:	data(new std::string()),
		offset(0)
{
}

void string_core::seek(stream::delta off, seek_from from)
{
	stream::pos baseOffset;
	std::string::size_type stringSize = this->data->length();
	switch (from) {
		case cur:
			baseOffset = this->offset;
			break;
		case end:
			baseOffset = stringSize;
			break;
		default:
			baseOffset = 0;
			break;
	}
	if ((off < 0) && (baseOffset < (unsigned)(off * -1))) {
		throw seek_error("Cannot seek back past start of string");
	}
	baseOffset += off;
	if (baseOffset > stringSize) {
		throw seek_error(createString("Cannot seek beyond end of string (offset "
			<< baseOffset << " > length " << stringSize << ")"));
	}
	this->offset = baseOffset;
	return;
}

boost::shared_ptr<std::string> string_core::str()
{
	return this->data;
}


input_string::input_string()
{
}

stream::len input_string::try_read(uint8_t *buffer, stream::len len)
{
	assert(this->data);

	stream::pos done = this->offset + len;
	stream::pos size = this->data->length();
	stream::len amt;
	if (done > size) amt = size - this->offset;
	else amt = len;
	memcpy(buffer, this->data->data() + this->offset, amt);
	this->offset += amt;
	return amt;
}

void input_string::seekg(stream::delta off, seek_from from)
{
	this->seek(off, from);
	return;
}

stream::pos input_string::tellg() const
{
	return this->offset;
}

stream::pos input_string::size() const
{
	return this->data->length();
}

void input_string::open(boost::shared_ptr<std::string> src)
{
	this->data = src;
	this->offset = 0;
	return;
}


output_string::output_string()
{
}

stream::len output_string::try_write(const uint8_t *buffer, stream::len len)
{
	assert(this->data);

	stream::pos done = this->offset + len;
	stream::pos size = this->data->length();
	if (done > size) {
		this->data->resize(done);
	} else if (size == 0) {
		// Empty write to an empty string
		return 0;
	}

	// Make sure our final assumed memory at least matches the real one.
	//assert(this->data->data() + done - 1 == &this->data->at(done - 1));

	memcpy(&this->data->at(0) + this->offset, buffer, len);
	this->offset += len;
	return len;
}

void output_string::seekp(stream::delta off, seek_from from)
{
	this->seek(off, from);
	return;
}

stream::pos output_string::tellp() const
{
	return this->offset;
}

void output_string::truncate(stream::pos size)
{
	this->flush();
	try {
		this->data->resize(size);
		this->seek(size, stream::start);
	} catch (const seek_error& e) {
		throw write_error("Unable to seek to EOF after truncate: " + e.get_message());
	}
	return;
}

void output_string::flush()
{
	return;
}

void output_string::open(boost::shared_ptr<std::string> src)
{
	this->data = src;
	this->offset = 0;
	return;
}


string::string()
{
}

} // namespace stream
} // namespace camoto
