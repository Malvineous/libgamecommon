/**
 * @file   stream_memory.cpp
 * @brief  Provide a stream implementation around a C++ vector.
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
#include <camoto/stream_memory.hpp>
#include <camoto/util.hpp>

namespace camoto {
namespace stream {

memory_core::memory_core()
	:	offset(0)
{
}

memory_core::~memory_core()
{
}

void memory_core::seek(stream::delta off, seek_from from)
{
	stream::pos baseOffset;
	std::vector<uint8_t>::size_type vectorSize = this->data.size();
	switch (from) {
		case cur:
			baseOffset = this->offset;
			break;
		case end:
			baseOffset = vectorSize;
			break;
		default:
			baseOffset = 0;
			break;
	}
	if ((off < 0) && (baseOffset < (unsigned)(off * -1))) {
		throw seek_error("Cannot seek back past start of memory");
	}
	baseOffset += off;
	if (baseOffset > vectorSize) {
		throw seek_error(createString("Cannot seek beyond end of memory (offset "
			<< baseOffset << " > length " << vectorSize << ")"));
	}
	this->offset = baseOffset;
	return;
}


input_memory::input_memory()
{
}

stream::len input_memory::try_read(uint8_t *buffer, stream::len len)
{
	stream::pos done = this->offset + len;
	stream::len size = this->data.size();
	stream::len amt;
	if (done > size) amt = size - this->offset;
	else amt = len;
	if (amt > 0) {
		// Don't do a zero-read past the last element, because the vector will throw
		// an error trying to retrieve the element just past the end.
		memcpy(buffer, &this->data[this->offset], amt);
		this->offset += amt;
	}
	return amt;
}

void input_memory::seekg(stream::delta off, seek_from from)
{
	this->seek(off, from);
	return;
}

stream::pos input_memory::tellg() const
{
	return this->offset;
}

stream::len input_memory::size() const
{
	return this->data.size();
}


output_memory::output_memory()
{
}

stream::len output_memory::try_write(const uint8_t *buffer, stream::len len)
{
	stream::pos done = this->offset + len;
	stream::len size = this->data.size();
	if (done > size) {
		this->data.resize(done);
	} else if (size == 0) {
		// Empty write to an empty vector
		return 0;
	}

	memcpy(&this->data[this->offset], buffer, len);
	this->offset += len;
	return len;
}

void output_memory::seekp(stream::delta off, seek_from from)
{
	this->seek(off, from);
	return;
}

stream::pos output_memory::tellp() const
{
	return this->offset;
}

void output_memory::truncate(stream::pos size)
{
	// Nothing is cached by try_write(), so there's nothing we need to flush
	// before the truncate.
	//this->flush();

	try {
		this->data.resize(size);
		this->seek(size, stream::start);
	} catch (const seek_error& e) {
		throw write_error("Unable to seek to EOF after truncate: " + e.get_message());
	}
	return;
}

void output_memory::flush()
{
	return;
}


memory::memory()
{
}

} // namespace stream
} // namespace camoto
