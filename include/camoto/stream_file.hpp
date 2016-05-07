/**
 * @file  camoto/stream_file.hpp
 * @brief Stream implementation for accessing local files.
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

#ifndef _CAMOTO_STREAM_FILE_HPP_
#define _CAMOTO_STREAM_FILE_HPP_

#include <memory>
#include <stdio.h>
#include <camoto/stream.hpp>

namespace camoto {
namespace stream {

/// Get an input stream reading from standard input.
std::unique_ptr<stream::input> CAMOTO_GAMECOMMON_API open_stdin();

/// Get an output stream writing to standard output.
std::unique_ptr<stream::output> CAMOTO_GAMECOMMON_API open_stdout();

/// Exception thrown when a file could not be opened or created.
class CAMOTO_GAMECOMMON_API open_error: public error
{
	public:
		/// Constructor.
		/**
		 * @param msg
		 *   Error description for UI messages.
		 */
		open_error(const std::string& msg)
			: error(msg)
		{
		}
};

/// File stream parts in common with read and write
class CAMOTO_GAMECOMMON_API file_core
{
	protected:
		FILE *handle;  ///< stdio file handle
		bool close;    ///< Do we need to close \e handle ?

		file_core();

		/// Common seek function for reading and writing.
		/**
		 * @copydetails input::seekg()
		 */
		void seek(stream::delta off, seek_from from);

		/// Common function for obtaining current seek position.
		stream::pos tell() const;
};

/// Read-only stream to access a local file.
class CAMOTO_GAMECOMMON_API input_file: virtual public input,
	virtual protected file_core
{
	public:
		/// Open an existing file.
		/**
		 * @param filename
		 *   Name of file to open.
		 *
		 * @throw open_error
		 *   The file could not be read or does not exist.
		 */
		input_file(const std::string& filename);
		virtual ~input_file();

		virtual stream::len try_read(uint8_t *buffer, stream::len len);
		virtual void seekg(stream::delta off, seek_from from);
		virtual stream::pos tellg() const;
		virtual stream::len size() const;

		friend std::unique_ptr<stream::input> CAMOTO_GAMECOMMON_API open_stdin();

	protected:
		input_file();
};

/// Write-only stream to access a local file.
class CAMOTO_GAMECOMMON_API output_file: virtual public output,
	virtual protected file_core
{
	public:
		/// Open an existing file, create a new file, or overwrite (blank out) an
		/// existing one.
		/**
		 * @param filename
		 *   Name of file to open.
		 *
		 * @param create
		 *   false to open an existing file for read/write, true to create the file
		 *   (create it if it doesn't exist, or truncate/blank out the file if it
		 *   does exist.)
		 *
		 * @throw open_error
		 *   The file could not be read or does not exist.
		 */
		output_file(const std::string& filename, bool create);
		virtual ~output_file();

		virtual stream::len try_write(const uint8_t *buffer, stream::len len);
		virtual void seekp(stream::delta off, seek_from from);
		virtual stream::pos tellp() const;
		virtual void truncate(stream::pos size);
		virtual void flush();

		/// Delete the file upon close.
		void remove();

		friend std::unique_ptr<stream::output> CAMOTO_GAMECOMMON_API open_stdout();

	protected:
		bool do_remove;        ///< Delete file on close?
		std::string filename;  ///< Copy of filename for deletion

		output_file(); // used by open_stdout()
		void open();
		void create();
};

/// Read/write stream accessing a local file.
class CAMOTO_GAMECOMMON_API file: virtual public inout,
	virtual public input_file,
	virtual public output_file
{
	public:
		file() = delete;
		file(const std::string& filename, bool create);
};

} // namespace stream
} // namespace camoto

#endif // _CAMOTO_STREAM_FILE_HPP_
