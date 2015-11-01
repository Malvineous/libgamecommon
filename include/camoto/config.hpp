/**
 * @file  camoto/config.hpp
 * @brief Compiler configuration.
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

#ifndef _CAMOTO_CONFIG_HPP_
#define _CAMOTO_CONFIG_HPP_

#ifdef _MSC_VER

#ifndef DLL_IMPORT
#define DLL_IMPORT _declspec(dllimport)
#endif
#ifndef DLL_EXPORT
#define DLL_EXPORT _declspec(dllexport)
#endif

// These API flags by default import the classes.  Each macro gets overridden
// to be DLL_EXPORT instead when building each library, so that the library's
// own API gets exported into its DLL instead.

#ifndef CAMOTO_GAMECOMMON_API
#define CAMOTO_GAMECOMMON_API DLL_IMPORT
#endif

#ifndef CAMOTO_GAMEARCHIVE_API
#define CAMOTO_GAMEARCHIVE_API DLL_IMPORT
#endif

#ifndef CAMOTO_GAMEGRAPHICS_API
#define CAMOTO_GAMEGRAPHICS_API DLL_IMPORT
#endif

#ifndef CAMOTO_GAMEMAPS_API
#define CAMOTO_GAMEMAPS_API DLL_IMPORT
#endif

#ifndef CAMOTO_GAMEMUSIC_API
#define CAMOTO_GAMEMUSIC_API DLL_IMPORT
#endif

#else // !_MSC_VER

// GCC doesn't need any of this, everything is exported by default.
#define CAMOTO_GAMECOMMON_API
#define CAMOTO_GAMEARCHIVE_API
#define CAMOTO_GAMEGRAPHICS_API
#define CAMOTO_GAMEMAPS_API
#define CAMOTO_GAMEMUSIC_API

#endif // _MSC_VER

#endif // _CAMOTO_CONFIG_HPP_
