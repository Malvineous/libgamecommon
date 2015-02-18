/**
 * @file   stream_filtered.cpp
 * @brief  Pass read/write operations through a filter to modify the data.
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
#include <camoto/stream_filtered.hpp>

namespace camoto {
namespace stream {

input_filtered::input_filtered(std::shared_ptr<input> parent,
	std::shared_ptr<filter> read_filter)
	:	in_parent(parent),
		read_filter(read_filter),
		populated(false)
{
	assert(parent);
	assert(read_filter);
	assert(this->data.size() == 0);
}

stream::len input_filtered::try_read(uint8_t *buffer, stream::len len)
{
	this->populate();
	return this->input_memory::try_read(buffer, len);
}

void input_filtered::seekg(stream::delta off, seek_from from)
{
	this->populate();
	return this->input_memory::seekg(off, from);
}

stream::pos input_filtered::tellg() const
{
	this->populate();
	return this->input_memory::tellg();
}

stream::len input_filtered::size() const
{
	this->populate();
	return this->input_memory::size();
}

void input_filtered::populate() const
{
	if (this->populated) return;
	input_filtered *self = const_cast<input_filtered *>(this);
	self->realPopulate();
	return;
}

void input_filtered::realPopulate()
{
	this->populated = true;

	// Seek to the start here, because we will have to do the same when the time
	// comes to write the change, so seeking here will make it obvious if the
	// offset is wrong.
	try {
		this->in_parent->seekg(0, stream::start);
	} catch (const seek_error&) {
		// Just ignore it, the stream might not be seekable (e.g. stdin)
	}

	// Read and filter the entire input into an in-memory buffer
	uint8_t bufIn[BUFFER_SIZE];
	stream::len lenIn, lenOut;
	stream::len lenRead, lenLeftover = 0;
	stream::len lenTotalOut = 0;
	this->read_filter->reset(this->in_parent->size());
	do {
		lenOut = BUFFER_SIZE;
		lenRead = this->in_parent->try_read(bufIn + lenLeftover, BUFFER_SIZE - lenLeftover);
		assert(lenRead <= BUFFER_SIZE - lenLeftover);
		lenRead += lenLeftover;
		lenIn = lenRead;
		this->data.resize(lenTotalOut + lenOut);
		read_filter->transform((uint8_t *)(&this->data[lenTotalOut]), &lenOut, bufIn, &lenIn);
		assert(lenIn <= BUFFER_SIZE);  // sanity check
		assert(lenOut <= BUFFER_SIZE); // sanity check
		lenTotalOut += lenOut;
		lenLeftover = lenRead - lenIn;
		if (lenLeftover) {
			// Not all input data was processed, keep the leftovers
			memmove(bufIn, bufIn + lenIn, lenLeftover);
		}
	} while ((lenIn != 0) || (lenOut != 0));

	// Cut off any excess from the last read
	this->data.resize(lenTotalOut);

	return;
}


output_filtered::output_filtered(std::shared_ptr<output> parent,
	std::shared_ptr<filter> write_filter, fn_truncate_filter resize)
	:	out_parent(parent),
		write_filter(write_filter),
		fn_resize(resize),
		done_filter(false)
{
	assert(parent);
	assert(write_filter);
	return;
}

stream::len output_filtered::try_write(const uint8_t *buffer, stream::len len)
{
	this->populate();

	// Data has changed, make sure we flush it
	this->done_filter = false;

	return this->output_memory::try_write(buffer, len);
}

void output_filtered::seekp(stream::delta off, seek_from from)
{
	this->populate();
	return this->output_memory::seekp(off, from);
}

stream::pos output_filtered::tellp() const
{
	this->populate();
	return this->output_memory::tellp();
}

void output_filtered::flush()
{
	if (this->done_filter) {
		std::cout << "WARNING: Tried to flush a filtered stream twice, ignoring "
			"second flush to avoid additional call to filter." << std::endl;
		return;
	}
	this->done_filter = true;

	std::vector<uint8_t> bufOut; // data is filtered to here first
	unsigned long lenFinal = 0;

	const uint8_t *bufIn = this->data.data();
	stream::len lenRealSize = this->data.size();
	stream::len lenRemaining = lenRealSize;
	stream::len lenIn, lenOut;

	this->write_filter->reset(lenRealSize);

	// Filter the in-memory buffer and write it out to the parent stream
	do {
		lenIn = lenRemaining;
		lenOut = BUFFER_SIZE;

		// Resize the output buffer to allow a full write
		bufOut.resize(lenFinal + lenOut);

		try {
			this->write_filter->transform(&bufOut[lenFinal], &lenOut, bufIn, &lenIn);
		} catch (const filter_error& e) {
			throw write_error("Filter error: " + e.get_message());
		}
		// Keep the data that was written
		lenFinal += lenOut;

		// Sanity checks: Make sure the filter didn't write more data than it was
		// allowed to.
		assert(lenOut <= BUFFER_SIZE);
		// Make sure the filter didn't read more data than we gave it.
		assert(lenIn <= lenRemaining);
		// Make sure we didn't write past the end of the vector
		assert(lenFinal <= bufOut.size());

		bufIn += lenIn;
		lenRemaining -= lenIn;
	} while ((lenIn != 0) && (lenOut != 0));

	this->out_parent->truncate(lenFinal);
	this->out_parent->seekp(0, stream::start);
	this->out_parent->write(&bufOut[0], lenFinal);

	// Notify the owner what the unfiltered size is.  We have to do this after
	// truncate(), because truncate() sets both stored and real sizes in case
	// there are no filters active.  So once that is done, we override it and
	// set the correct real size.
	if (this->fn_resize) this->fn_resize(this, lenRealSize);

	this->out_parent->flush();

	return;
}

void output_filtered::populate() const
{
	return;
}


filtered::filtered(std::shared_ptr<inout> parent,
	std::shared_ptr<filter> read_filter, std::shared_ptr<filter> write_filter,
	fn_truncate_filter resize)
	:	input_filtered(parent, read_filter),
		output_filtered(parent, write_filter, resize)
{
}

void filtered::truncate(stream::pos size)
{
	if (size == 0) this->populated = true;
	this->output_filtered::truncate(size);
	return;
}

void filtered::populate() const
{
	this->input_filtered::populate();
	return;
}

} // namespace stream
} // namespace camoto
