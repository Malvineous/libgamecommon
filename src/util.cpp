/**
 * @file  camoto/util.cpp
 * @brief Utility functions used across the libraries.
 *
 * Copyright (C) 2010-2017 Adam Nielsen <malvineous@shikadi.net>
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

#include <cctype>
#include <unistd.h>
#include <sys/stat.h>
#include <camoto/util.hpp>

namespace camoto {

bool icasecmp(const std::string& l, const std::string& r)
{
	return l.size() == r.size()
		&& equal(l.cbegin(), l.cend(), r.cbegin(),
			[](std::string::value_type l1, std::string::value_type r1) {
				return toupper(l1) == toupper(r1);
			}
		);
}

void uppercase(std::string& s)
{
	for (auto& i : s) i = toupper(i);
	return;
}

void lowercase(std::string& s)
{
	for (auto& i : s) i = tolower(i);
	return;
}

// basic filesystem implementation until C++17 is standardised
namespace filesystem {

std::vector<std::string> split_path(const std::string& path)
{
	std::vector<std::string> components;
	std::string segment;
	for (const auto& i : path) {
		if (i == separator) {
			if (!segment.empty()) components.push_back(segment);
			segment.clear();
		} else {
			segment += i;
		}
	}
	if (!segment.empty()) components.push_back(segment);
	return components;
}

path current_path()
{
	char s[256];
	getcwd(s, 256);
	return path(s);
}

void current_path(const path& p)
{
	if (chdir(p.c_str()) < 0) {
		throw filesystem_error();
	}
}

bool exists(const path& p)
{
	struct stat buf;
	if (stat(p.c_str(), &buf) < 0) return false;
	return true;
}

void create_directory(const std::string& d)
{
	if (mkdir(d.c_str(), 0755) < 0) {
		throw filesystem_error();
	}
}

} // namespace fs

} // namespace camoto
