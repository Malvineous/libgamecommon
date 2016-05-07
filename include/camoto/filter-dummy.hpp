/**
 * @file  camoto/filter-dummy.hpp
 * @brief Filter object that doesn't alter the data.
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

#ifndef _CAMOTO_FILTER_DUMMY_HPP_
#define _CAMOTO_FILTER_DUMMY_HPP_

#include <camoto/filter.hpp>

namespace camoto {

class CAMOTO_GAMECOMMON_API filter_dummy: public filter
{
	public:
		filter_dummy();

		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut, const uint8_t *in,
			stream::len *lenIn);
};

} // namespace camoto

#endif // _CAMOTO_FILTER_DUMMY_HPP_
