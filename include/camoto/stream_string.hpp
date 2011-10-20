/**
 * @file   stream_string.hpp
 * @brief  Provide a stream implementation around a C++ string.
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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
class string_core {

	protected:
		std::string *data;   ///< String data
		bool free;           ///< Delete \e data when done?
		stream::pos offset;  ///< Current pointer position

		string_core()
			throw ();

		~string_core()
			throw ();

		/// Common seek function for reading and writing.
		/**
		 * @copydetails input::seekg()
		 */
		void seek(stream::delta off, seek_from from)
			throw (seek_error);

		/// Get access to the underlying storage.
		/**
		 * @return Reference to the underlying string.
		 */
		std::string& str()
			throw ();
};

/// Read-only stream to access a C++ string.
class input_string: virtual public input,
                    virtual protected string_core
{
	public:
		/// Default constructor.
		/**
		 * @note Initialises with empty string.
		 */
		input_string()
			throw ();

		virtual stream::len try_read(uint8_t *buffer, stream::len len)
			throw ();

		virtual void seekg(stream::delta off, seek_from from)
			throw (seek_error);

		virtual stream::pos tellg() const
			throw (seek_error);

		virtual stream::pos size() const
			throw (seek_error);

		/// Wrap around an existing string.
		/**
		 * @param src
		 *   Reference to string.  The reference must remain valid as long
		 *   as the input_string instance exists.
		 *
		 * @see str();
		 */
		void open(const std::string *src)
			throw ();

		using string_core::str;
};

/// Shared pointer to a readable string.
typedef boost::shared_ptr<input_string> input_string_sptr;

/// Write-only stream to access a C++ string.
class output_string: virtual public output,
                     virtual protected string_core
{
	public:
		/// Default constructor.
		/**
		 * @note Initialises with empty string.
		 */
		output_string()
			throw ();

		virtual stream::len try_write(const uint8_t *buffer, stream::len len)
			throw ();

		virtual void seekp(stream::delta off, seek_from from)
			throw (seek_error);

		virtual stream::pos tellp() const
			throw (seek_error);

		virtual void truncate(stream::pos size)
			throw (write_error);

		virtual void flush()
			throw (write_error);

		/// Wrap around an existing string.
		/**
		 * @param src
		 *   Reference to string.  The reference must remain valid as long
		 *   as the input_string instance exists.
		 *
		 * @see str();
		 */
		void open(std::string *src)
			throw ();

		using string_core::str;
};

/// Shared pointer to a writable string.
typedef boost::shared_ptr<output_string> output_string_sptr;

/// Read/write stream accessing a C++ string.
class string: virtual public inout,
              virtual public input_string,
              virtual public output_string
{
	public:
		string()
			throw ();

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
