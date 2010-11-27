/**
 * @file   metadata.hpp
 * @brief  Interface for reading and writing metadata about a file.
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

#ifndef _CAMOTO_METADATA_HPP_
#define _CAMOTO_METADATA_HPP_

#include <string>
#include <vector>
#include <camoto/types.hpp>

namespace camoto {

/// Interface for reading and writing metadata about a file.
/**
 * This interface will usually be inherited by some other class, and the
 * functions will then be overridden to provide access to the metadata in that
 * file format.
 *
 * The metadata functions all have no-op defaults, they only need to be
 * overridden for tileset formats that have metadata.
 */
class Metadata {

	public:

		/// Metadata item types.
		enum MetadataType {
			Description,     ///< File description
			PaletteFilename, ///< Palette filename
		};

		/// Vector of metadata item types.
		typedef std::vector<MetadataType> MetadataTypes;


		/// Get a list of supported metadata elements that can be set.
		/**
		 * Some tileset formats have room for additional data, such as a palette
		 * filename.  This function is used to obtain a list of the
		 * metadata elements supported by the current tileset.  Not every tileset
		 * supports all the metadata types, and any optional elements will be
		 * included in this list (but getMetadata() may return an empty string for
		 * those.)
		 *
		 * Note to tileset format implementors: There is a default implementation
		 * of this function which returns an empty vector.  Thus this only needs
		 * to be overridden if the tileset format does actually support metadata.
		 *
		 * @return std::vector of \ref E_METADATA items.
		 */
		virtual MetadataTypes getMetadataList() const
			throw ();

		/// Get the value of a metadata element.
		/**
		 * Returns the value of a metadata item reported to exist by
		 * getMetadataList().
		 *
		 * Note to tileset format implementors: There is a default implementation
		 * of this function which always throws an exception.  Thus this only needs
		 * to be overridden if the tileset format does actually support metadata.
		 *
		 * @param  item Item to retrieve.  Must have been included in the list
		 *         returned by getMetadataList().
		 * @return A string containing the metadata (may be empty.)
		 */
		virtual std::string getMetadata(MetadataType item) const
			throw (std::ios::failure);

		/// Change the value of a metadata element.
		/**
		 * Only elements returned by getMetadataList() can be changed.
		 *
		 * Note to tileset format implementors: There is a default implementation
		 * of this function which always throws an exception.  Thus this only needs
		 * to be overridden if the tileset format does actually support metadata.
		 *
		 * @param  item Item to set.  Must have been included in the list returned
		 *         by getMetadataList().
		 * @param  value The value to set.  Passing an empty string will remove
		 *         the metadata element if possible, otherwise it will be set to
		 *         a blank.
		 */
		virtual void setMetadata(MetadataType item, const std::string& value)
			throw (std::ios::failure);

};

} // namespace camoto

#endif // _CAMOTO_METADATA_HPP_
