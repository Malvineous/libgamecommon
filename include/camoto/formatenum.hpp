/**
 * @file  camoto/formatenum.hpp
 * @brief File format enumeration interface.
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

#ifndef _CAMOTO_FORMATENUM_HPP_
#define _CAMOTO_FORMATENUM_HPP_

#include <memory>
#include <vector>

namespace camoto {

template <class T>
class FormatEnumerator {
	public:
		// This is a shared_ptr because the class is polymorphic and we don't want
		// it to be sliced if it's copied.
		typedef typename std::shared_ptr<const T> handler_t;
		typedef typename std::vector<handler_t> handler_list_t;

		static const handler_list_t formats();

		static std::shared_ptr<const T> byCode(const std::string& code)
		{
			for (const auto& i : FormatEnumerator<T>::formats()) {
				if (code.compare(i->code()) == 0) return i;
			}
			return nullptr;
		};

		template <class A>
		static void addFormat(handler_list_t& list)
		{
			list.emplace_back(std::make_shared<const A>());
			return;
		}

		template <class A, class B, class... Args>
		static void addFormat(handler_list_t& list)
		{
			FormatEnumerator<T>::addFormat<A>(list);
			FormatEnumerator<T>::addFormat<B, Args...>(list);
			return;
		}
};

} // namespace camoto

#endif // _CAMOTO_FORMATENUM_HPP_
