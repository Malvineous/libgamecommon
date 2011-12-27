/**
 * @file   error.hpp
 * @brief  Base error exception.
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

#ifndef _CAMOTO_ERROR_HPP_
#define _CAMOTO_ERROR_HPP_

#include <exception>
#include <string>

namespace camoto {

/// Base exception for stream functions.
class error: public std::exception
{
	public:
		/// Constructor.
		/**
		 * @param msg
		 *   Error description for UI messages.
		 */
		error(const std::string& msg)
			throw ();

		~error()
			throw ();

		/// Standard error retrieval function.
		virtual const char *what() const
			throw ();

		/// Get the error message as a string instead.
		std::string get_message() const
			throw ();

	protected:
		std::string msg;  ///< Detailed error message for UI
};

} // namespace camoto

#endif // _CAMOTO_ERROR_HPP_
