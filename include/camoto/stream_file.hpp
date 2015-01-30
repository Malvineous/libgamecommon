/**
 * @file  camoto/stream_file.hpp
 * @brief Stream implementation for accessing local files.
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

#ifndef _CAMOTO_STREAM_FILE_HPP_
#define _CAMOTO_STREAM_FILE_HPP_

#include <stdio.h>
#include <camoto/stream.hpp>

namespace camoto {
namespace stream {

/// Get an input stream reading from standard input.
input_sptr DLL_EXPORT open_stdin();

/// Get an output stream writing to standard output.
output_sptr DLL_EXPORT open_stdout();

/// Exception thrown when a file could not be opened or created.
class DLL_EXPORT open_error: public error
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
class DLL_EXPORT file_core
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
class DLL_EXPORT input_file: virtual public input,
	virtual protected file_core
{
	public:
		/// Default constructor.
		/**
		 * @note Must call open() before any other functions.
		 */
		input_file();
		virtual ~input_file();

		virtual stream::len try_read(uint8_t *buffer, stream::len len);
		virtual void seekg(stream::delta off, seek_from from);
		virtual stream::pos tellg() const;
		virtual stream::pos size() const;

		/// Open an existing file.
		/**
		 * @param filename
		 *   Name of file to open.
		 *
		 * @throw open_error
		 *   The file could not be read or does not exist.
		 */
		void open(const char *filename);

		/// @copydoc open(const char *)
		void open(const std::string& filename);

		friend input_sptr open_stdin();
};

/// Shared pointer to a readable file.
typedef boost::shared_ptr<input_file> input_file_sptr;

/// Write-only stream to access a local file.
class DLL_EXPORT output_file: virtual public output,
	virtual protected file_core
{
	public:
		/// Default constructor.
		/**
		 * @note Must call open() or create() before any other functions.
		 */
		output_file();
		virtual ~output_file();

		virtual stream::len try_write(const uint8_t *buffer, stream::len len);
		virtual void seekp(stream::delta off, seek_from from);
		virtual stream::pos tellp() const;
		virtual void truncate(stream::pos size);
		virtual void flush();

		/// Open an existing file.
		/**
		 * @param filename
		 *   Name of file to open.
		 *
		 * @throw open_error
		 *   The file could not be read or does not exist.
		 */
		void open(const char *filename);

		/// @copydoc open(const char *)
		void open(const std::string& filename);

		/// Create a new file, or overwrite (blank out) an existing one.
		/**
		 * @param filename
		 *   Name of file to open.
		 *
		 * @throw open_error
		 *   The file could not be created (e.g. no permission).
		 */
		void create(const char *filename);

		/// @copydoc create(const char *)
		void create(const std::string& filename);

		/// Delete the file upon close.
		void remove();

		friend output_sptr open_stdout();

	protected:
		bool do_remove;        ///< Delete file on close?
		std::string filename;  ///< Copy of filename for deletion

		void open();
		void create();
};

/// Shared pointer to a writable file.
typedef boost::shared_ptr<output_file> output_file_sptr;

/// Read/write stream accessing a local file.
class DLL_EXPORT file: virtual public inout,
	virtual public input_file,
	virtual public output_file
{
	public:
		file();

		// Pick this version (as opposed to input_file::open) as it happens to
		// open the file in read/write mode.
		using output_file::open;

		/// Open an existing file in read-only mode.
		/**
		 * @param filename
		 *   Name of file to open.
		 *
		 * @throw open_error
		 *   The file could not be read or does not exist.
		 */
		void open_readonly(const char *filename);

		/// @copydoc open_readonly(const char *)
		void open_readonly(const std::string& filename);

		/// Is the file open in read-only mode?
		/**
		 * @return true if read-only, false if read-write.
		 */
		bool readonly();

	protected:
		bool isReadonly; ///< Is the file read-only?

		void open_readonly();
};

/// Shared pointer to a readable and writable file.
typedef boost::shared_ptr<file> file_sptr;

} // namespace stream
} // namespace camoto

#endif // _CAMOTO_STREAM_FILE_HPP_
