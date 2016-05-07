/**
 * @file  camoto/attribute.hpp
 * @brief Interface for reading and writing flags and metadata about a file.
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

#ifndef _CAMOTO_ATTRIBUTE_HPP_
#define _CAMOTO_ATTRIBUTE_HPP_

#include <string>
#include <vector>
#include <map>
#include <camoto/config.hpp>
#include <camoto/stream.hpp>

namespace camoto {

/// Standard value for Attribute::name for a title.
#define CAMOTO_ATTRIBUTE_TITLE "Title"

/// Standard value for Attribute::name for an author/artist/composer.
#define CAMOTO_ATTRIBUTE_AUTHOR "Author"

/// Standard value for Attribute::name for a comment/description.
#define CAMOTO_ATTRIBUTE_COMMENT "Comment"

/// Attribute attached to this map.
/**
 * Attributes are configuration options that apply to particular map files,
 * such as a default background colour or which song to play as background
 * music in the level.
 *
 * Attributes should reflect data contained in the map file itself, so for
 * example, if the map file doesn't store some value that controls which
 * tileset is used to draw the level, then the tileset filename shouldn't be
 * exposed as an attribute (because if it was changed, the new value
 * couldn't be saved back into the map file.)
 *
 * Attributes should reflect properties of the map that the user can and may
 * wish to change.
 */
struct Attribute
{
	enum class Type {
		Integer,         ///< One number within a given range
		Enum,            ///< One choice from a list of static values
		Filename,        ///< A filename of the given file type
		Text,            ///< A text string
		Image,           ///< One image from a list of images
	};
	Type type;         ///< What type this attribute is
	std::string name;  ///< Short name of this attribute
	std::string desc;  ///< Description of this attribute
	bool changed;      ///< Has this value been changed since it was populated?

	int integerValue;    ///< Integer type: current value
	int integerMinValue; ///< Integer type: minimum allowed value (set min and max to 0 for unlimited)
	int integerMaxValue; ///< Integer type: maximum allowed value (set min and max to 0 for unlimited)

	unsigned int enumValue;                  ///< Enum type: current value
	std::vector<std::string> enumValueNames; ///< Enum type: permitted values

	/// Filename type: current filename
	/**
	 * Filenames should be specified here as map attributes (as opposed to
	 * supplementary items) if the files are not required to load the map.
	 *
	 * Parts of the actual map (like layer data or sprite positions) should
	 * be listed as supp data because the map will be incomplete if those
	 * files are not available, but things like tileset filenames are not
	 * required to load the map (e.g. if all you want to do is find out the
	 * map dimensions) so those optional files should be listed as attributes.
	 */
	std::string filenameValue;

	/// Filename type: valid filenames
	/**
	 * Any files that match this specification will be listed as valid choices
	 * for this attribute value.  An empty vector means there is no restriction
	 * on filename.  Example entries are "*.txt" and "demo*"
	 */
	std::vector<std::string> filenameSpec;

	std::string textValue;      ///< Text type: the text value
	unsigned int textMaxLength; ///< Text type: maximum string length, in chars, 0 is no limit

	/// Image-enum type: current index into a vector supplied in some other manner
	unsigned int imageIndex;
};

/// Class inherited by anything that needs to get/set Attribute values.
/**
 * This interface will usually be inherited by some other class, and the
 * functions will then be overridden to provide access to the data in that
 * file format.
 *
 * The functions all have no-op defaults, they only need to be overridden for
 * file formats that have special requirements.
 */
class CAMOTO_GAMECOMMON_API HasAttributes
{
	public:
		/// Get a copy of the attributes for this map.
		/**
		 * @note As the returned value is a copy, any changes will not affect the
		 *   map.
		 *
		 * @return Copy of the map's attributes.  The indices into this vector are
		 *   used for the index parameter in an attribute() call, to change the
		 *   value of the attribute.
		 */
		virtual const std::vector<Attribute>& attributes() const;

		/// Change one of the map's integer/enum attributes.
		/**
		 * @param index
		 *   Index into vector returned by attributes().  0 is the first item.
		 *
		 * @param newValue
		 *   The new value to set for the attribute.
		 *
		 * @note This function performs validation, such as ensuring the value is
		 *   within range.
		 *
		 * @throws camoto::error if the value could not be set, such as it was out
		 *   of range or for the wrong data type.
		 */
		virtual void attribute(unsigned int index, int newValue);

		/// Change one of the map's string/filename attributes.
		/**
		 * @param index
		 *   Index into vector returned by attributes().  0 is the first item.
		 *
		 * @param newValue
		 *   The new value to set for the attribute.
		 *
		 * @throws camoto::error if the value could not be set, such as it was out
		 *   of range or for the wrong data type.
		 */
		virtual void attribute(unsigned int index, const std::string& newValue);

	protected:
		/// Attribute list.
		/**
		 * The child class should add items to this vector when the format is read,
		 * and read from this vector when the format is written.
		 */
		std::vector<Attribute> v_attributes;
};

} // namespace camoto

/// Convert a metadata item into text.
CAMOTO_GAMECOMMON_API std::ostream& operator << (std::ostream& s,
	const camoto::Attribute& a);

#endif // _CAMOTO_ATTRIBUTE_HPP_
