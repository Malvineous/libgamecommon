/**
 * @file   util.cpp
 * @brief  Utility functions.
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

#ifdef __WIN32
#include <windows.h> // SetEndOfFile() et al
#else
#include <unistd.h>  // truncate()
#endif

#include <camoto/util.hpp>

namespace camoto {

void stringStreamTruncate(std::stringstream *ss, int len)
{
	if (len < ss->str().length()) {
		// Shrinking
		std::string orig = ss->str();
		ss->clear(); // reset state, leave string alone
		ss->str(orig.substr(0, len)); // set new string
	} else {
		// Enlarging
		std::streamsize pos;
		// Work around C++ stringstream bug that returns invalid offset when empty.
		// https://issues.apache.org/jira/browse/STDCXX-332
		if (!ss->str().empty()) {
			ss->seekp(0, std::ios::end);
			pos = ss->tellp();
			assert(pos > 0);
		} else {
			pos = 0;
		}

		*ss << std::string(len - pos, '\0');
		assert(ss->tellp() == len);
	}
	return;
}

int truncateFromString(std::string filename, unsigned long len)
	throw ()
{
#ifdef __WIN32
	HANDLE f = CreateFile(filename.c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	SetFilePointer(f, length, NULL, FILE_BEGIN);
	BOOL b = SetEndOfFile(f);
	CloseHandle(f);
	return b ? -1 : 0;
#else
	return truncate(filename.c_str(), len);
#endif
}

} // namespace camoto
