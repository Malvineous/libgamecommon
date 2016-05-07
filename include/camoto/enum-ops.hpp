/**
 * @file  camoto/enum-ops.hpp
 * @brief Generic enum class operator implementations.
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

#ifndef _CAMOTO_ENUM_OPS_HPP_
#define _CAMOTO_ENUM_OPS_HPP_

/// Implement standard bitwise operators for the given enum class type.
#define IMPLEMENT_ENUM_OPERATORS(T) \
inline T operator| (T a, T b) { \
	return static_cast<T>( \
		static_cast<unsigned int>(a) | static_cast<unsigned int>(b) \
	); \
} \
\
inline T operator|= (T& a, const T& b) { \
	a = a | b; \
	return a; \
} \
\
inline bool operator& (T a, T b) { \
	return \
		static_cast<unsigned int>(a) & static_cast<unsigned int>(b) \
	; \
}

#endif // _CAMOTO_ENUM_OPS_HPP_
