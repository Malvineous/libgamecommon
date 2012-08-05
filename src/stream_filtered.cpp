/**
 * @file   stream_filtered.cpp
 * @brief  Pass read/write operations through a filter to modify the data.
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

#include <camoto/stream_filtered.hpp>

namespace camoto {
namespace stream {

void input_filtered::open(input_sptr parent, filter_sptr read_filter)
	throw (filter_error, read_error)
{
	assert(parent);
	assert(read_filter);

	// Seek to the start here, because we will have to do the same when the time
	// comes to write the change, so seeking here will make it obvious if the
	// offset is wrong.
	try {
		parent->seekg(0, stream::start);
	} catch (const seek_error&) {
		// Just ignore it, the stream might not be seekable (e.g. stdin)
	}

	// Read and filter the entire input into an in-memory buffer
	uint8_t bufIn[BUFFER_SIZE];
	uint8_t bufOut[BUFFER_SIZE];
	stream::len lenIn, lenOut;
	stream::len lenRead, lenLeftover = 0;
	do {
		lenOut = BUFFER_SIZE;
		lenRead = parent->try_read(bufIn + lenLeftover, BUFFER_SIZE - lenLeftover);
		assert(lenRead <= BUFFER_SIZE - lenLeftover);
		lenRead += lenLeftover;
		lenIn = lenRead;
		read_filter->transform(bufOut, &lenOut, bufIn, &lenIn);
		assert(lenIn <= BUFFER_SIZE);  // sanity check
		assert(lenOut <= BUFFER_SIZE); // sanity check
		lenLeftover = lenRead - lenIn;
		if (lenLeftover) {
			// Not all input data was processed, keep the leftovers
			memmove(bufIn, bufIn + lenIn, lenLeftover);
		}
		this->data->append((char *)bufOut, lenOut);
	} while ((lenIn != 0) || (lenOut != 0));
	return;
}

stream::len output_filtered::try_write(const uint8_t *buffer, stream::len len)
	throw ()
{
if (this->done_filter) std::cerr
<< "NOTE: Writing to previously flushed filtered stream." << std::endl;
	// Data has changed, make sure we flush it
	this->done_filter = false;

	return this->output_string::try_write(buffer, len);
}

void output_filtered::truncate(stream::pos size)
	throw (write_error)
{
	// The input side (decompressed, if we're a compressor filter) wants to be
	// truncated.  We'll have to ignore this, because at this point we don't know
	// where the output side (compressed data) should end.  We may not have even
	// run the filter yet!
	//
	// We perform the actual truncate of the parent stream in flush(), after we've
	// performed the filtering operation.
	//
	// Since the docs say truncate() does an implicit flush(), we'd better do it
	// all now anyway though.
	this->flush();
	return;
}

void output_filtered::flush()
	throw (write_error)
{
	if (this->done_filter) {
		std::cout << "WARNING: Tried to flush a filtered stream twice, ignoring "
			"second flush to avoid additional call to filter." << std::endl;
		return;
	}
	this->done_filter = true;

	this->output_string::flush();

	const uint8_t *bufIn = (const uint8_t *)this->data->c_str();
	uint8_t bufOut[BUFFER_SIZE];
	stream::len lenRemaining = this->data->length();
	stream::len lenIn, lenOut;

	// Notify the owner what the unfiltered size is
	if (this->fn_resize) this->fn_resize(lenRemaining);

	// Filter the in-memory buffer and write it out to the parent stream
	this->out_parent->seekp(0, stream::start);
	do {
		lenIn = lenRemaining;
		lenOut = BUFFER_SIZE;
		try {
			this->write_filter->transform(bufOut, &lenOut, bufIn, &lenIn);
		} catch (const filter_error& e) {
			throw write_error("Filter error: " + e.get_message());
		}

		// Sanity checks: Make sure the filter didn't write more data that it was
		// allowed to.
		assert(lenOut <= BUFFER_SIZE);
		// Make sure the filter didn't read more data than we gave it.
		assert(lenIn <= lenRemaining);

		bufIn += lenIn;
		lenRemaining -= lenIn;
		this->out_parent->write(bufOut, lenOut);
	} while ((lenIn != 0) && (lenOut != 0));

	this->out_parent->truncate_here();
	return;
}

void output_filtered::open(output_sptr parent, filter_sptr write_filter,
	fn_truncate resize)
	throw ()
{
	assert(parent);
	assert(write_filter);

	this->out_parent = parent;
	this->write_filter = write_filter;
	this->fn_resize = resize;
	this->done_filter = false;
	return;
}


filtered::filtered()
	throw ()
{
}

void filtered::open(inout_sptr parent, filter_sptr read_filter,
	filter_sptr write_filter, fn_truncate resize)
	throw (filter_error, read_error)
{
	this->input_filtered::open(parent, read_filter);
	this->output_filtered::open(parent, write_filter, resize);
	return;
}

} // namespace stream
} // namespace camoto
