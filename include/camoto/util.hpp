/**
 * @file   util.hpp
 * @brief  Utility functions used across the libraries.
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

#ifndef _CAMOTO_UTIL_HPP_
#define _CAMOTO_UTIL_HPP_

#include <boost/shared_ptr.hpp>
#include <iostream>
#include <sstream>

#define TOSTRING_X(x)  #x
#define TOSTRING(x)    TOSTRING_X(x)

/// Main namespace
namespace camoto {

/// String creation macro (with iostream syntax)
/**
 * Useful for assigning strings in constructor initialisation lists.  Note this
 * function is wrapped around a macro called createString() which should be used
 * instead.
 *
 * @code
 * std::string str = createString("The value is " << iValue);
 * @endcode
 *
 * @see createString
 */
inline boost::shared_ptr<std::ostringstream> _createStream(void)
{
  return boost::shared_ptr<std::ostringstream>(new std::ostringstream);
}

/// Wrapper around _createStream()
/**
 * @example createString std::string str = createString("The value is " << iValue);
 */
#define createString(a) \
  (static_cast<const std::ostringstream&>(*camoto::_createStream().get() << a).str())
// The first variable after the ostringstream constructor always gets printed
// as a number, so we need to add some null-output value instead - ios::dec
// does the job nicely, and allows 'a' to start off with something that isn't a
// number (e.g. a char.)

} // namespace camoto

#endif // _CAMOTO_UTIL_HPP_
