/**
 * @file   attribute.cpp
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
#include <camoto/attribute.hpp>
#include <camoto/util.hpp>

namespace camoto {

const std::vector<Attribute>& HasAttributes::attributes() const
{
	return this->v_attributes;
}

void HasAttributes::attribute(unsigned int index, int newValue)
{
	if (index >= this->v_attributes.size()) {
		throw camoto::error(createString("Attribute index (" << index
			<< ") out of range (" << this->v_attributes.size() << " elements)."));
	}
	auto& a = this->v_attributes[index];
	switch (a.type) {
		case Attribute::Type::Integer:
			if (newValue < a.integerMinValue) {
				throw camoto::error(createString("Attribute #" << index
					<< " value is out of range - cannot set " << newValue
					<< " when minimum is " << a.integerMinValue << "."));
			}
			if (newValue > a.integerMaxValue) {
				throw camoto::error(createString("Attribute #" << index
					<< " value is out of range - cannot set " << newValue
					<< " when maximum is " << a.integerMaxValue << "."));
			}
			a.integerValue = newValue;
			break;
		case Attribute::Type::Enum:
			if (newValue < 0) {
				throw camoto::error(createString("Attribute #" << index << " value is "
					"out of range - cannot set negative values for an enum."));
			}
			if ((unsigned int)newValue >= a.enumValueNames.size()) {
				throw camoto::error(createString("Attribute #" << index
					<< " value is out of range - cannot set " << newValue
					<< " when maximum must be less than " << a.enumValueNames.size()
					<< "."));
			}
			a.enumValue = newValue;
			break;
		case Attribute::Type::Filename:
			throw camoto::error(createString("Attribute #" << index
				<< " is a filename, it cannot be set to an integer value."));
		case Attribute::Type::Text:
			throw camoto::error(createString("Attribute #" << index
				<< " is text, it cannot be set to an integer value."));
		case Attribute::Type::Image:
			a.imageIndex = newValue;
			break;
	}
	a.changed = true;
	return;
}

void HasAttributes::attribute(unsigned int index, const std::string& newValue)
{
	if (index >= this->v_attributes.size()) {
		throw camoto::error(createString("Attribute index (" << index
			<< ") out of range (" << this->v_attributes.size() << " elements)."));
	}
	auto& a = this->v_attributes[index];
	switch (a.type) {
		case Attribute::Type::Integer:
			throw camoto::error(createString("Attribute #" << index
				<< " is an integer, it cannot be set to a string value."));
		case Attribute::Type::Enum:
			throw camoto::error(createString("Attribute #" << index
				<< " is an enum, it cannot be set to a string value."));
		case Attribute::Type::Filename:
			a.filenameValue = newValue;
			break;
		case Attribute::Type::Text:
			if (
				(a.textMaxLength > 0)
				&& (newValue.length() >= a.textMaxLength)
			) {
				throw camoto::error(createString("String too long for attribute #" << index
					<< " - length is " << newValue.length() << ", must be less than "
					<< a.textMaxLength << "."));
			}
			a.textValue = newValue;
			break;
		case Attribute::Type::Image:
			throw camoto::error(createString("Attribute #" << index
				<< " is an image index, it cannot be set to a string value."));
	}
	a.changed = true;
	return;
}

} // namespace camoto

std::ostream& operator << (std::ostream& s, const camoto::Attribute& a)
{
	s << "Attribute[name=" << a.name << ", type=";
	switch (a.type) {
		case camoto::Attribute::Type::Integer:
			s << "int, range=" << a.integerMinValue << ".." << a.integerMaxValue
				<< ", val=" << a.integerValue;
			break;
		case camoto::Attribute::Type::Enum:
			s << "enum, val=" << a.enumValue << ", val_name="
				<< a.enumValueNames[a.enumValue];
			break;
		case camoto::Attribute::Type::Filename:
			s << "filename, val=" << a.filenameValue;
			break;
		case camoto::Attribute::Type::Text:
			s << "text, val=" << a.textValue;
			break;
		case camoto::Attribute::Type::Image:
			s << "image, index=" << a.imageIndex;
			break;
	}
	s << "]";
	return s;
}
