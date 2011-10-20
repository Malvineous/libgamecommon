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
	throw ()
{
	assert(parent);
	assert(read_filter);

	// Seek to the start here, because we will have to do the same when the time
	// comes to write the change, so seeking here will make it obvious if the
	// offset is wrong.
	parent->seekg(0, stream::start);

	// Read and filter the entire input into an in-memory buffer
	uint8_t bufIn[BUFFER_SIZE];
	uint8_t bufOut[BUFFER_SIZE];
	stream::len lenIn, lenOut;
	stream::len lenRead, lenLeftover = 0;
	do {
		lenOut = BUFFER_SIZE;
		lenRead = parent->try_read(bufIn + lenLeftover, BUFFER_SIZE - lenLeftover);
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
	} while ((lenIn != 0) && (lenOut != 0));
	return;
}

void output_filtered::flush()
	throw (write_error)
{
	this->out_parent->seekp(0, stream::start);

	// Filter the in-memory buffer and write it out to the parent stream
	const uint8_t *bufIn = (const uint8_t *)this->data->c_str();
	uint8_t bufOut[BUFFER_SIZE];
	stream::len lenRemaining = this->data->length();
	stream::len lenIn, lenOut;
	do {
		lenIn = lenRemaining;
		lenOut = BUFFER_SIZE;
		this->write_filter->transform(bufOut, &lenOut, bufIn, &lenIn);

		// Sanity checks: Make sure the filter didn't write more data that it was
		// allowed to.
		assert(lenOut <= BUFFER_SIZE);
		// Make sure the filter didn't read more data than we gave it.
		assert(lenIn <= lenRemaining);

		bufIn += lenIn;
		lenRemaining -= lenIn;
		this->out_parent->write(bufOut, lenOut);
	} while ((lenIn != 0) && (lenOut != 0));

	this->output_string::flush();
	return;
}

void output_filtered::open(output_sptr parent, filter_sptr write_filter)
	throw ()
{
	assert(parent);
	assert(write_filter);

	this->out_parent = parent;
	this->write_filter = write_filter;
	return;
}


filtered::filtered()
	throw ()
{
}

void filtered::open(inout_sptr parent, filter_sptr read_filter, filter_sptr write_filter)
	throw ()
{
	this->input_filtered::open(parent, read_filter);
	this->output_filtered::open(parent, write_filter);
	return;
}

} // namespace stream
} // namespace camoto
