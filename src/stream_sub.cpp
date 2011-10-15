/**
 * @file   stream_sub.cpp
 * @brief  Provide a stream limited to a subsection within another stream.
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

#include <errno.h>
#include <string.h>
#include <camoto/stream_sub.hpp>

namespace camoto {
namespace stream {

void sub_core::seek(stream::delta off, seek_from from)
	throw (seek_error)
{
	stream::pos newOffset;
	switch (from) {
		case cur:
			newOffset = this->offset + off;
			break;
		case end:
			newOffset = this->offset + this->stream_len + off;
			break;
		default:
			newOffset = off;
			break;
	}
	if (newOffset > this->stream_len) {
		throw seek_error("Cannot seek beyond end of substream");
	}
	if (newOffset < 0) {
		throw seek_error("Cannot seek back past start of substream");
	}
	this->offset = newOffset;
	return;
}

void sub_core::relocate(stream::delta off)
	throw ()
{
	// Don't seek past the start of the parent stream
	if (off < 0) assert(this->start > off * -1);

	// Don't seek beyond the end of the parent stream
	//assert(this->start + off + this->stream_len < this->parent->size());
	// Can't do this as we don't have access to any parent stream here

	this->start += off;
	return;
}

void sub_core::resize(stream::len len)
	throw ()
{
	this->stream_len = len;

	// Clip pointer if the resize has made it go past EOF
	if (this->offset > this->stream_len) this->offset = this->stream_len;
	return;
}


stream::len input_sub::try_read(uint8_t *buffer, stream::len len)
	throw ()
{
	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->stream_len);

	if (this->offset >= this->stream_len) return 0; // EOF

	// Make sure we can't read past the end of the file
	if ((this->offset + len) > this->stream_len) {
		len = this->stream_len - this->offset;
	}

	this->in_parent->seekg(this->start + this->offset, stream::start);

	stream::len r = this->in_parent->try_read(buffer, len);
	this->offset += r;

	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->stream_len);

	return r;
}

void input_sub::seekg(stream::delta off, seek_from from)
	throw (seek_error)
{
	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->stream_len);

	this->seek(off, from);

	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->stream_len);
	return;
}

stream::pos input_sub::tellg() const
	throw (seek_error)
{
	return this->offset;
}

stream::pos input_sub::size() const
	throw (seek_error)
{
	return this->stream_len;
}

void input_sub::open(input_sptr parent, stream::pos start, stream::len len)
	throw ()
{
	this->in_parent = parent;
	this->start = start;
	this->stream_len = len;
	this->offset = 0;
	return;
}


stream::len output_sub::try_write(const uint8_t *buffer, stream::len len)
	throw ()
{
	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->stream_len);

	if ((this->offset + len) > this->stream_len) {
		// Stream is too small to accommodate entire write, attempt to enlarge
		this->flush();
		// Don't call truncate() because we don't want the pointer moved
		if (!this->fn_resize(this->offset + len)) {
			// Truncate failed, reduce write to available space
			len = this->stream_len - this->offset;
		}
	}

	this->out_parent->seekp(this->start + this->offset, stream::start);
	stream::pos w = this->out_parent->try_write(buffer, len);
	this->offset += w;

	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->stream_len);

	return w;
}

void output_sub::seekp(stream::delta off, seek_from from)
	throw (seek_error)
{
	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->stream_len);

	this->seek(off, from);

	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->stream_len);
	return;
}

stream::pos output_sub::tellp() const
	throw (seek_error)
{
	return this->offset;
}

void output_sub::truncate(stream::pos size)
	throw (seek_error)
{
	assert(this->fn_resize);

	this->flush();
	if (!this->fn_resize(size)) {
		throw seek_error("Resize callback could not resize the stream");
	}
	this->seekp(size, stream::start);
	return;
}

void output_sub::flush() const
	throw ()
{
	this->out_parent->flush();
	return;
}

void output_sub::open(output_sptr parent, stream::pos start, stream::len len,
	fn_truncate fn_resize)
	throw ()
{
	this->out_parent = parent;
	this->start = start;
	this->stream_len = len;
	this->fn_resize = fn_resize;
	this->offset = 0;
	return;
}


sub::sub()
	throw ()
{
}

void sub::open(inout_sptr parent, stream::pos start, stream::len len,
	fn_truncate fn_resize)
	throw ()
{
	this->in_parent = parent;
	this->out_parent = parent;
	this->start = start;
	this->stream_len = len;
	this->fn_resize = fn_resize;
	this->offset = 0;
	return;
}

} // namespace stream
} // namespace camoto
