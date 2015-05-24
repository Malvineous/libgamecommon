/**
 * @file   suppitem.cpp
 * @brief  Helper functions for supplementary item handling.
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

#include <camoto/suppitem.hpp>

namespace camoto {

std::string suppToString(SuppItem s)
{
	switch (s) {
		case SuppItem::Dictionary:  return "Dictionary";
		case SuppItem::FAT:         return "FAT";
		case SuppItem::Palette:     return "Palette";
		case SuppItem::Instruments: return "Instruments";
		case SuppItem::Layer1:      return "Layer1";
		case SuppItem::Layer2:      return "Layer2";
		case SuppItem::Layer3:      return "Layer3";
		case SuppItem::Extra1:      return "Extra1";
		case SuppItem::Extra2:      return "Extra2";
		case SuppItem::Extra3:      return "Extra3";
		case SuppItem::Extra4:      return "Extra4";
		case SuppItem::Extra5:      return "Extra5";
		case SuppItem::MaxValue:    return "<SuppItem out of range: MaxValue>";
	}
	return "<SuppItem out of range>";
}

} // namespace camoto
