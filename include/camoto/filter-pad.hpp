/**
 * @file  camoto/filter-pad.hpp
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

#ifndef _CAMOTO_FILTER_PAD_HPP_
#define _CAMOTO_FILTER_PAD_HPP_

#include <camoto/filter.hpp>
#include <camoto/stream_string.hpp>

namespace camoto {

class DLL_EXPORT filter_pad: public filter
{
	public:
		/// Constructor.
		filter_pad();

		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut, const uint8_t *in,
			stream::len *lenIn);

		/// Data to insert at start of output upon flush()
		stream::string pad;

	protected:
		stream::len len; ///< Number of bytes to insert
		stream::pos posPadding; ///< How many bytes of pad we have written
};

} // namespace camoto

#endif // _CAMOTO_FILTER_PAD_HPP_
