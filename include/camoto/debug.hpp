/**
 * @file   debug.hpp
 * @brief  Helper functions to assist with debugging.
 *
 * Copyright (C) 2009 Adam Nielsen <malvineous@shikadi.net>
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

// Compile in debugging output?
//#define DEBUG

#ifndef _CAMOTO_DEBUG_HPP_
#define _CAMOTO_DEBUG_HPP_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define CLR_NORM   "\e[22;39m"
#define CLR_GREY   "\e[1;30m"
#define CLR_RED    "\e[1;31m"
#define CLR_GREEN  "\e[1;32m"
#define CLR_YELLOW "\e[1;33m"
#define CLR_MAG    "\e[1;35m"
#define CLR_CYAN   "\e[1;36m"
#define CLR_WHITE  "\e[1;37m"

#endif // _CAMOTO_DEBUG_HPP_
