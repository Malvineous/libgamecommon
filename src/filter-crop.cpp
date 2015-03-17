/**
 * @file  filter-crop.cpp
 * @brief Filter object that drops bytes from the start of a stream.
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

#include <camoto/filter-crop.hpp>

namespace camoto {

filter_crop::filter_crop(stream::pos start)
	:	start(start),
		cropped(false)
{
}

void filter_crop::reset(stream::len lenInput)
{
	this->cropped = false;
	return;
}

void filter_crop::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len r = 0;
	if (!this->cropped) {
		if (*lenIn < this->start) {
			// Not enough space to read
			*lenOut = 0;
			*lenIn = 0;
			return;
		}
		*lenIn -= this->start;
		in += this->start;
		r += this->start;
		this->cropped = true;
	}

	// Copy as much data as will fit in the smallest buffer
	stream::len minAmt = (*lenOut > *lenIn) ? *lenIn : *lenOut;
	memcpy(out, in, minAmt);

	*lenIn = minAmt + r;
	*lenOut = minAmt;
	return;
}

} // namespace camoto
