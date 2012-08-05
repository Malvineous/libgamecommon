/**
 * @file   filter_dummy.cpp
 * @brief  Filter object that doesn't alter the data.
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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

#include <camoto/filter_dummy.hpp>

namespace camoto {

filter_dummy::filter_dummy()
	throw ()
{
}

void filter_dummy::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
	throw (filter_error)
{
	// Copy as much data as will fit in the smallest buffer
	stream::len minAmt = (*lenOut > *lenIn) ? *lenIn : *lenOut;
	memcpy(out, in, minAmt);
	*lenIn = minAmt;
	*lenOut = minAmt;
	return;
}

} // namespace camoto
