/**
 * @file  camoto/stream_string.hpp
 * @brief Provide a stream implementation around a C++ string.
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

#ifndef _CAMOTO_STREAM_STRING_HPP_
#define _CAMOTO_STREAM_STRING_HPP_

#include <string>
#include <memory>
#include <camoto/stream.hpp>

namespace camoto {
namespace stream {

/// String stream parts in common with read and write
class DLL_EXPORT string_core
{
	public:
		std::string data;  ///< String data

	protected:
		stream::pos offset;  ///< Current pointer position

		string_core(std::string data);

		/// Common seek function for reading and writing.
		/**
		 * @copydetails input::seekg()
		 */
		void seek(stream::delta off, seek_from from);
};

/// Read-only stream to access a C++ string.
class DLL_EXPORT input_string:
	virtual public input,
	virtual public string_core
{
	public:
		/// Default constructor.
		/**
		 * @note Initialises with empty string.
		 */
		input_string();
		input_string(std::string content);

		virtual stream::len try_read(uint8_t *buffer, stream::len len);
		virtual void seekg(stream::delta off, seek_from from);
		virtual stream::pos tellg() const;
		virtual stream::pos size() const;
};

/// Write-only stream to access a C++ string.
class DLL_EXPORT output_string:
	virtual public output,
	virtual public string_core
{
	public:
		/// Default constructor.
		/**
		 * @note Initialises with empty string.
		 */
		output_string();

		virtual stream::len try_write(const uint8_t *buffer, stream::len len);
		virtual void seekp(stream::delta off, seek_from from);
		virtual stream::pos tellp() const;
		virtual void truncate(stream::pos size);
		virtual void flush();
};

/// Read/write stream accessing a C++ string.
class DLL_EXPORT string:
	virtual public inout,
	virtual public input_string,
	virtual public output_string
{
	public:
		string();
		string(std::string content);
};

} // namespace stream
} // namespace camoto

#endif // _CAMOTO_STREAM_STRING_HPP_
