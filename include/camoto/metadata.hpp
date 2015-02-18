/**
 * @file  camoto/metadata.hpp
 * @brief Interface for reading and writing metadata about a file.
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

#ifndef _CAMOTO_METADATA_HPP_
#define _CAMOTO_METADATA_HPP_

#include <string>
#include <vector>
#include <map>
#include <camoto/stream.hpp>

#ifndef DLL_EXPORT
#define DLL_EXPORT
#endif

namespace camoto {

/// Interface for reading and writing metadata about a file.
/**
 * This interface will usually be inherited by some other class, and the
 * functions will then be overridden to provide access to the metadata in that
 * file format.
 *
 * The metadata functions all have no-op defaults, they only need to be
 * overridden for file formats that have metadata.
 */
class DLL_EXPORT Metadata
{
	public:
		/// Metadata item types.
		enum MetadataType {
			Description,     ///< File description, remarks, etc.
			PaletteFilename, ///< Palette filename
			Version,         ///< Version/revision number

			Title,           ///< Song title
			Author,          ///< Composer, etc.
		};

		/// Vector of metadata item types.
		typedef std::vector<MetadataType> MetadataTypes;

		/// Map for associating strings with metadata item types.
		typedef std::map<MetadataType, std::string> TypeMap;

		/// Get a list of supported metadata elements that can be set.
		/**
		 * Some file formats have room for additional data, such as a palette
		 * filename.  This function is used to obtain a list of the
		 * metadata elements supported by the current format.  Not every format
		 * supports all the metadata types, and any optional elements will be
		 * included in this list (but getMetadata() may return an empty string for
		 * those.)
		 *
		 * Note to implementors: There is a default implementation of this function
		 * which returns an empty vector.  Thus this only needs to be overridden if
		 * the file format does actually support metadata.
		 *
		 * @return std::vector of \ref MetadataType items.
		 */
		virtual MetadataTypes getMetadataList() const;

		/// Get the value of a metadata element.
		/**
		 * Returns the value of a metadata item reported to exist by
		 * getMetadataList().
		 *
		 * Note to implementors: There is a default implementation of this function
		 * which always throws an exception.  Thus this only needs to be overridden
		 * if metadata is actually supported.
		 *
		 * @param item
		 *   Item to retrieve.  Must have been included in the list returned by
		 *   getMetadataList().
		 *
		 * @return A string containing the metadata (may be empty.)
		 */
		virtual std::string getMetadata(MetadataType item) const;

		/// Change the value of a metadata element.
		/**
		 * Only elements returned by getMetadataList() can be changed.
		 *
		 * Note to implementors: There is a default implementation of this function
		 * which always throws an exception.  Thus this only needs to be overridden
		 * if metadata is actually supported.
		 *
		 * @param item
		 *   Item to set.  Must have been included in the list returned by
		 *   getMetadataList().
		 *
		 * @param  value The value to set.  Passing an empty string will remove
		 *         the metadata element if possible, otherwise it will be set to
		 *         a blank.
		 */
		virtual void setMetadata(MetadataType item, const std::string& value);
};

/// Convert a metadata item into text.
DLL_EXPORT std::ostream& operator << (std::ostream& s,
	const camoto::Metadata::MetadataType& m);

} // namespace camoto

#endif // _CAMOTO_METADATA_HPP_
