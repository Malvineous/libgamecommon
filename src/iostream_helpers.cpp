/**
 * @file   iostream_helpers.cpp
 * @brief  Functions to get variables in and out of a stream.
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

#include <camoto/iostream_helpers.hpp>

#ifdef DEBUG
#define BUFFER_SIZE 4
#else
#define BUFFER_SIZE 4096
#endif

#define ZEROPAD_BLOCK_SIZE  16

namespace camoto {

null_padded_read::null_padded_read(std::string& r, stream::len len, bool chop) :
	r(r),
	len(len),
	chop(chop)
{
}

void null_padded_read::read(stream::input_sptr s) const
{
	// Make the buffer the length of the whole operation
	this->r.resize(this->len);

	// Read in the whole data
	s->read((uint8_t *)&this->r[0], this->len);

	if (this->chop) {
		// Shrink the string back to the first null
		this->r.resize(strlen(this->r.c_str()));
	}
	return;
}

null_padded_write::null_padded_write(const std::string& r, stream::len len) :
	r(r),
	len(len)
{
}

void null_padded_write::write(stream::output_sptr s) const
{
	int lenData = this->r.length();
	assert(lenData <= this->len);

	// Write the content
	s->write((const uint8_t *)this->r.c_str(), lenData);

	// Pad out to the full length with nulls
	uint8_t blank[ZEROPAD_BLOCK_SIZE];
	memset(blank, 0, ZEROPAD_BLOCK_SIZE);
	int lenRemaining = this->len - lenData;
	int amt = ZEROPAD_BLOCK_SIZE;
	while (lenRemaining > 0) {
		if (lenRemaining < ZEROPAD_BLOCK_SIZE) amt = lenRemaining;
		s->write(blank, amt);
		lenRemaining -= amt;
	}
	return;
}

null_padded_const::null_padded_const(const std::string& r, stream::len len) :
	null_padded_write(r, len)
{
}

null_padded::null_padded(std::string& r, stream::len len, bool chop) :
	null_padded_read(r, len, chop),
	null_padded_write(r, len)
{
}


number_format_u8::number_format_u8(uint8_t& r) :
	r(r)
{
}

void number_format_u8::read(stream::input_sptr s) const
{
	s->read((uint8_t *)&this->r, 1);
	return;
}

void number_format_u8::write(stream::output_sptr s) const
{
	s->write((const uint8_t *)&this->r, 1);
	return;
}

number_format_const_u8::number_format_const_u8(const uint8_t& r) :
	r(r)
{
}

void number_format_const_u8::write(stream::output_sptr s) const
{
	s->write((const uint8_t *)&this->r, 1);
	return;
}

} // namespace camoto
