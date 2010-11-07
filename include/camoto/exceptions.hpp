/**
 * @file   exceptions.hpp
 * @brief  Exception types used across the library.
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _CAMOTO_EXCEPTIONS_HPP_
#define _CAMOTO_EXCEPTIONS_HPP_

#include <exception>
#include <string>

namespace camoto {

/// Exception thrown when invalid data is encountered during decompression.
class ECorruptedData: virtual public std::exception {
	protected:
		std::string msg;

	public:
		ECorruptedData(const std::string& msg)
			throw ();

		~ECorruptedData()
			throw ();

		virtual const char *what() const
			throw ();
};

} // namespace camoto

#endif // _CAMOTO_EXCEPTIONS_HPP_
