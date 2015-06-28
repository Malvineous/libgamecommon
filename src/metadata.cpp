/**
 * @file   metadata.cpp
 * @brief  Interface for reading and writing metadata about a file.
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

#include <cassert>
#include <iostream>
#include <camoto/metadata.hpp>

namespace camoto {

Metadata::MetadataTypes Metadata::getMetadataList() const
{
	return {};
}

std::string Metadata::getMetadata(MetadataType item) const
{
	// This should never be called because getMetadataList() returned an empty
	// list.
	assert(false);
	throw stream::error("unsupported metadata item");
}

// Change the value of a metadata element.
void Metadata::setMetadata(MetadataType item, const std::string& value)
{
	// This should never be called because getMetadataList() returned an empty
	// list.
	assert(false);
	throw stream::error("unsupported metadata item");
}

} // namespace camoto

std::ostream& operator << (std::ostream& s, const camoto::Metadata::MetadataType& m)
{
	switch (m) {
		case camoto::Metadata::MetadataType::Description: s << "Description"; break;
		case camoto::Metadata::MetadataType::PaletteFilename: s << "Palette Filename"; break;
		case camoto::Metadata::MetadataType::Version: s << "Version"; break;
		case camoto::Metadata::MetadataType::Title: s << "Title"; break;
		case camoto::Metadata::MetadataType::Author: s << "Author"; break;
	}
	return s;
}
