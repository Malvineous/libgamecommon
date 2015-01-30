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
#include <camoto/stream.hpp>

namespace camoto {
namespace stream {

/// String stream parts in common with read and write
class DLL_EXPORT string_core
{
	protected:
		boost::shared_ptr<std::string> data;   ///< String data
		stream::pos offset;  ///< Current pointer position

		string_core();

		/// Common seek function for reading and writing.
		/**
		 * @copydetails input::seekg()
		 */
		void seek(stream::delta off, seek_from from);

		/// Get access to the underlying storage.
		/**
		 * @return Reference to the underlying string.
		 */
		boost::shared_ptr<std::string> str();
};

/// Read-only stream to access a C++ string.
class DLL_EXPORT input_string:
	virtual public input,
	virtual protected string_core
{
	public:
		/// Default constructor.
		/**
		 * @note Initialises with empty string.
		 */
		input_string();

		virtual stream::len try_read(uint8_t *buffer, stream::len len);
		virtual void seekg(stream::delta off, seek_from from);
		virtual stream::pos tellg() const;
		virtual stream::pos size() const;

		/// Wrap around an existing string.
		/**
		 * @param src
		 *   Reference to string.  The reference must remain valid as long
		 *   as the input_string instance exists.
		 *
		 * @see str();
		 */
		void open(boost::shared_ptr<std::string> src);

		using string_core::str;
};

/// Shared pointer to a readable string.
typedef boost::shared_ptr<input_string> input_string_sptr;

/// Write-only stream to access a C++ string.
class DLL_EXPORT output_string:
	virtual public output,
	virtual protected string_core
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

		/// Wrap around an existing string.
		/**
		 * @param src
		 *   Shared pointer to the new string.  The reference must remain valid as long
		 *   as the input_string instance exists.
		 *
		 * @see str();
		 */
		void open(boost::shared_ptr<std::string> src);

		using string_core::str;
};

/// Shared pointer to a writable string.
typedef boost::shared_ptr<output_string> output_string_sptr;

/// Read/write stream accessing a C++ string.
class DLL_EXPORT string:
	virtual public inout,
	virtual public input_string,
	virtual public output_string
{
	public:
		string();

		// Pick this version (as opposed to input_string::open) as it will
		// "open" the string in read/write mode.
		using output_string::open;

		using string_core::str;
};

/// Shared pointer to a readable and writable string.
typedef boost::shared_ptr<string> string_sptr;

} // namespace stream
} // namespace camoto

#endif // _CAMOTO_STREAM_STRING_HPP_
