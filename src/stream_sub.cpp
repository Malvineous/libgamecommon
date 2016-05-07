/**
 * @file   stream_sub.cpp
 * @brief  Provide a stream limited to a subsection within another stream.
 *
 * Copyright (C) 2010-2016 Adam Nielsen <malvineous@shikadi.net>
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
#include <cstring>
#include <errno.h>
#include <camoto/stream_sub.hpp>
#include <camoto/util.hpp>

namespace camoto {
namespace stream {

sub_core::sub_core(pos start, len len)
	:	offset(0),
		stream_start(start),
		stream_len(len)
{
}

sub_core::~sub_core()
{
}

void sub_core::seek(stream::delta off, seek_from from)
{
	stream::pos baseOffset;
	switch (from) {
		case cur:
			baseOffset = this->offset;
			break;
		case end:
			baseOffset = this->sub_size();
			break;
		default:
			baseOffset = 0;
			break;
	}
	if ((off < 0) && (baseOffset < (unsigned)(off * -1))) {
		throw seek_error("Cannot seek back past start of substream");
	}
	baseOffset += off;
	if (baseOffset > this->sub_size()) {
		throw seek_error(createString("Cannot seek beyond end of substream (offset "
			<< baseOffset << " > length " << this->sub_size() << ")"));
	}
	this->offset = baseOffset;
	return;
}

void sub_core::relocate(stream::delta off)
{
	// Don't seek past the start of the parent stream
	if (off < 0) assert(this->stream_start >= (unsigned)(off * -1));

	// Don't seek beyond the end of the parent stream
	//assert(this->stream_start + off + this->stream_len < this->parent->size());
	// Can't do this as we don't have access to any parent stream here

	this->stream_start += off;
	return;
}

void sub_core::resize(stream::len len)
{
	this->stream_len = len;

	// Clip pointer if the resize has made it go past EOF
	if (this->offset > this->stream_len) this->offset = this->stream_len;
	return;
}

stream::pos sub_core::sub_start() const
{
	return this->stream_start;
}

stream::len sub_core::sub_size() const
{
	return this->stream_len;
}


input_sub::input_sub(std::shared_ptr<input> parent, pos start, len len)
	:	sub_core(start, len),
		in_parent(parent)
{
}

stream::len input_sub::try_read(uint8_t *buffer, stream::len len)
{
	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->sub_size());

	if (this->offset >= this->sub_size()) return 0; // EOF

	// Make sure we can't read past the end of the file
	if ((this->offset + len) > this->sub_size()) {
		len = this->sub_size() - this->offset;
	}

	this->in_parent->seekg(this->sub_start() + this->offset, stream::start);

	stream::len r = this->in_parent->try_read(buffer, len);
	assert(r <= len);
	this->offset += r;

	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->sub_size());

	return r;
}

void input_sub::seekg(stream::delta off, seek_from from)
{
	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->sub_size());

	this->seek(off, from);

	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->sub_size());
	return;
}

stream::pos input_sub::tellg() const
{
	return this->offset;
}

stream::len input_sub::size() const
{
	return this->sub_size();
}


output_sub::output_sub(std::shared_ptr<output> parent, pos start, len len,
	fn_truncate_sub fn_resize)
	:	sub_core(start, len),
		out_parent(parent),
		fn_resize(fn_resize)
{
}

stream::len output_sub::try_write(const uint8_t *buffer, stream::len len)
{
	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->sub_size());

	if ((this->offset + len) > this->sub_size()) {
		// Stream is too small to accommodate entire write, attempt to enlarge
		// Don't call truncate() because we don't want the pointer moved
		try {
			if (this->fn_resize) {
				this->fn_resize(this, this->offset + len);
				if ((this->offset + len) > this->sub_size()) {
					// Truncate failed, reduce write to available space
					len = this->sub_size() - this->offset;
				}
			} else {
				std::cerr << "[stream::sub::try_write] No truncate function, cannot "
					"enlarge substream.  Doing a partial write." << std::endl;
			}
		} catch (const write_error&) {
			// Truncate failed, reduce write to available space
			len = this->sub_size() - this->offset;
		}
	}

	try {
		this->out_parent->seekp(this->sub_start() + this->offset, stream::start);
	} catch (const seek_error&) {
		return 0;
	}
	stream::pos w = this->out_parent->try_write(buffer, len);
	this->offset += w;

	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->sub_size());

	return w;
}

void output_sub::seekp(stream::delta off, seek_from from)
{
	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->sub_size());

	this->seek(off, from);

	// Make sure we didn't somehow end up past the end of the stream
	assert(this->offset <= this->sub_size());
	return;
}

stream::pos output_sub::tellp() const
{
	return this->offset;
}

void output_sub::truncate(stream::pos size)
{
	if (this->sub_size() == size) return; // nothing to do
	if (!this->fn_resize) {
		throw write_error("Cannot truncate substream, no callback function was "
			"provided to notify the substream owner.");
	}

	this->fn_resize(this, size);
	try {
		this->seekp(size, stream::start);
	} catch (const seek_error& e) {
		throw write_error("Truncate callback silently failed!  Unable to seek to "
			"EOF after truncate: " + e.get_message());
	}
	return;
}

void output_sub::flush()
{
	this->out_parent->flush();
	return;
}


sub::sub(std::shared_ptr<inout> parent, pos start, len len,
	fn_truncate_sub fn_resize)
	:	sub_core(start, len),
		input_sub(parent, start, len),
		output_sub(parent, start, len, fn_resize)
{
}

} // namespace stream
} // namespace camoto
