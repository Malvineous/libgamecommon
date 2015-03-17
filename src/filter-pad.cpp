/**
 * @file  filter-pad.cpp
 * @brief Filter object that adds extra data at the start of the stream.
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

#include <camoto/filter-pad.hpp>

namespace camoto {

filter_pad::filter_pad()
	:	posPadding(0)
{
}

void filter_pad::reset(stream::len lenInput)
{
	this->posPadding = 0;
	// Don't reset the pad data itself here, because reset() is called just
	// before flush(), where we actually need the pad data!
	return;
}

void filter_pad::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len w = 0;
	if (this->posPadding < this->pad.data.size()) {
		w = std::min(this->pad.data.size() - this->posPadding, *lenOut);
		memcpy(out, this->pad.data.c_str(), w);
		out += w;
		*lenOut -= w;
		this->posPadding += w;
	}

	// Copy as much data as will fit in the smallest buffer
	stream::len minAmt = (*lenOut > *lenIn) ? *lenIn : *lenOut;
	memcpy(out, in, minAmt);

	*lenIn = minAmt;
	*lenOut = w + minAmt;
	return;
}

} // namespace camoto
