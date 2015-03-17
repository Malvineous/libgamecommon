/**
 * @file  camoto/stream.hpp
 * @brief Base stream implementation.
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

#ifndef _CAMOTO_STREAM_HPP_
#define _CAMOTO_STREAM_HPP_

#include <cstring>
#include <stdint.h>
#include <camoto/error.hpp>

namespace camoto {
namespace stream {

/// Buffer size to use in copy() and move().
#define BUFFER_SIZE 4096

/// Signed integer data type.  Internal use only.
typedef long long signed_int_type;

/// Unsigned integer data type.  Internal use only.
typedef unsigned long long unsigned_int_type;

/// Stream offset
typedef unsigned_int_type pos;

/// Length of data
typedef unsigned_int_type len;

/// Positive or negative amount from some other point
typedef signed_int_type delta;

/// Base exception for stream functions.
class DLL_EXPORT error: public camoto::error
{
	public:
		/// Constructor.
		/**
		 * @param msg
		 *   Error description for UI messages.
		 */
		error(const std::string& msg);
};

/// Could not read data from stream.
class DLL_EXPORT read_error: public error
{
	public:
		/// Constructor.
		/**
		 * @param msg
		 *   Error description for UI messages.
		 */
		read_error(const std::string& msg);
};

/// Could not write data to stream.
class DLL_EXPORT write_error: public error
{
	public:
		/// Constructor.
		/**
		 * @param msg
		 *   Error description for UI messages.
		 */
		write_error(const std::string& msg);
};

/// Tried to seek before file start or after file end.
class DLL_EXPORT seek_error: public error
{
	public:
		/// Constructor.
		/**
		 * @param msg
		 *   Error description for UI messages.
		 */
		seek_error(const std::string& msg);
};

/// Not all the expected data could be written to the stream.
class DLL_EXPORT incomplete_write: public write_error
{
	public:
		/// Number of bytes written before the error.
		stream::len bytes_written;

		/// Constructor.
		/**
		 * @param written
		 *   Number of bytes that were actually written.
		 */
		incomplete_write(stream::len written);
};

/// Not all the requested data could be read from the stream.
class DLL_EXPORT incomplete_read: public read_error
{
	public:
		/// Number of bytes written before the error.
		stream::len bytes_read;

		/// Constructor.
		/**
		 * @param read
		 *   Number of bytes that were actually read.
		 */
		incomplete_read(stream::len read);
};

/// Origin of a seek operation.
enum seek_from {
	start, ///< Move from the beginning of the stream
	cur,   ///< Move from the current location of the stream's pointer
	end    ///< Move from the end of the stream
};

/// Base stream interface for reading data.
/**
 * @post A newly created stream's seek pointer is always at the start (offset 0).
 */
class DLL_EXPORT input
{
	public:
		virtual ~input() { };

		/// Read some bytes from the stream if possible.
		/**
		 * If not all of the requested bytes could be read then whatever could
		 * be will be returned (a "partial read".)  If no bytes could be read at
		 * all (e.g. seek position is at the end of the file) the return value
		 * is 0.
		 *
		 * @param buffer
		 *   Pointer to memory where data will be stored.
		 *
		 * @param len
		 *   Number of bytes to read from the stream.
		 *
		 * @return Number of bytes read.  Always <= len, 0 at EOF.
		 *
		 * @throw read_error
		 *   The data could not be read due to some reason other than EOF.
		 *
		 * @throw filter_error
		 *   There was an error decoding the data required to perform this
		 *   operation.
		 */
		virtual stream::len try_read(uint8_t *buffer, stream::len len) = 0;

		/// Read the given number of bytes from the stream.
		/**
		 * If not all the data could be read, an exception will be thrown.
		 *
		 * @param buffer
		 *   Pointer to memory where data will be stored.
		 *
		 * @param len
		 *   Number of bytes to read from the stream.
		 *
		 * @throw incomplete_read
		 *   If not all the data could be read, most likely due to reaching EOF.
		 *
		 * @throw read_error
		 *   The data could not be read due to some other reason.
		 *
		 * @throw filter_error
		 *   There was an error decoding the data required to perform this
		 *   operation.
		 */
		void read(uint8_t *buffer, stream::len len);

		/// Convenience function.
		/// @copydoc read(uint8_t *, stream::len)
		void read(char *buffer, stream::len len);

		/// Read the given number of bytes into a string.
		/**
		 * If not all the data could be read, an exception will be thrown.
		 *
		 * @param len
		 *   Number of bytes to read from the stream.
		 *
		 * @return A string of length \e len.
		 *
		 * @throw incomplete_read
		 *   Not all the data could be read, most likely due to reaching EOF.
		 *
		 * @throw read_error
		 *   The data could not be read due to some other reason.
		 *
		 * @throw filter_error
		 *   There was an error decoding the data required to perform this
		 *   operation.
		 */
		std::string read(stream::len len);

		/// Move the stream's read pointer.
		/**
		 * A value of 1 will seek:
		 *  - When from == start, to the second byte in the file
		 *  - When from == cur, skip over the next byte that would have been read
		 *  - When from == end, throw seek_error as it is not possible to seek past
		 *    the end of the file
		 *
		 * @param off
		 *   Signed value indicating the number of bytes to move by.  If this value
		 *   is positive, the pointer increases from the origin (away from file
		 *   offset 0).  If this value is negative, the pointer decreases (moves
		 *   toward offset 0).  The offset/origin is always >= 0.
		 *
		 * @param from
		 *   Origin of the seek.
		 *
		 * @throw seek_error
		 *   The seek failed or attempted to move past EOF or before the start of
		 *   the stream.
		 *
		 * @throw filter_error
		 *   There was an error decoding the data required to perform this
		 *   operation.
		 */
		virtual void seekg(stream::delta off, seek_from from) = 0;

		/// Get the current location of the read pointer.
		/**
		 * Passing the return value to seekg() with seek_from::start will
		 * result in the seek position remaining unchanged.
		 *
		 * @return The current seek position.  0 means at the start of the file, so
		 *   the next byte written would be at position 0 (i.e. the first byte in
		 *   the file.)
		 *
		 * @throw filter_error
		 *   There was an error decoding the data required to perform this
		 *   operation.
		 */
		virtual stream::pos tellg() const = 0;

		/// Get the current file size.
		/**
		 * Seeking to the returned value with seek_from::start is the same as
		 * seeking to offset 0 with seek_from::end, i.e. the next byte written will
		 * cause the file to be expanded if possible.
		 *
		 * @return The current file size, in bytes.
		 *
		 * @throw seek_error
		 *   The seek to EOF failed.
		 *
		 * @throw filter_error
		 *   There was an error decoding the data required to perform this
		 *   operation.
		 */
		virtual stream::len size() const = 0;
};

/// Base stream interface for writing data.
/**
 * @post A newly created stream's seek pointer is always at the start (offset
 *   0).
 */
class DLL_EXPORT output
{
	public:
		virtual ~output() { };

		/// Write as much as possible to the stream.
		/**
		 * @param buffer
		 *   Pointer to memory where data will be stored.
		 *
		 * @param len
		 *   Number of bytes to read from the stream.
		 *
		 * @return Number of bytes written.  Always <= len.  If there was
		 *   insufficient space for the write to begin at all, the return value will
		 *   be zero.
		 *
		 * @throw write_error
		 *   The write failed due to something other than EOF/insufficient space.
		 *
		 * @throw filter_error
		 *   There was an error decoding the data required to perform this
		 *   operation.
		 */
		virtual stream::len try_write(const uint8_t *buffer, stream::len len) = 0;

		/// Write all the data to the stream or throw an exception.
		/**
		 * @param buffer
		 *   Pointer to memory where data will be stored.
		 *
		 * @param len
		 *   Number of bytes to read from the stream.
		 *
		 * @post If exception was thrown, stream position has advanced by the
		 *   number of bytes in incomplete_write::written.
		 *
		 * @throw incomplete_write
		 *   Insufficient space to write all the data and the stream could not be
		 *   expanded.
		 *
		 * @throw write_error
		 *   The write failed due to something other than EOF/insufficient space.
		 *
		 * @throw filter_error
		 *   There was an error decoding the data required to perform this
		 *   operation.
		 */
		void write(const uint8_t *buffer, stream::len len);

		/// Convenience function.
		/// @copydoc write(const uint8_t *, stream::len)
		void write(const char *buffer, stream::len len);

		/// Write a string to the stream.
		/**
		 * @param buffer
		 *   Data to write.  buffer.length() bytes are written.  If a terminating
		 *   null is required, append one to the string before the call.
		 *
		 * @throw incomplete_write
		 *   Insufficient space to write all the data and the stream could not be
		 *   expanded.
		 *
		 * @throw write_error
		 *   The write failed due to something other than EOF/insufficient space.
		 *
		 * @throw filter_error
		 *   There was an error decoding the data required to perform this
		 *   operation.
		 */
		void write(const std::string& buffer);

		/// Move the stream's write pointer.
		/**
		 * @copydetails input::seekg()
		 */
		virtual void seekp(stream::delta off, seek_from from) = 0;

		/// Get the current location of the write pointer.
		/**
		 * Passing the return value to seekp() with seek_from::start will
		 * result in the seek position remaining unchanged.
		 *
		 * @return The current seek position.  0 means at the start of the file, so
		 *   the next byte written would be at position 0 (i.e. the first byte in
		 *   the file.)
		 *
		 * @throw filter_error
		 *   There was an error decoding the data required to perform this
		 *   operation.
		 */
		virtual stream::pos tellp() const = 0;

		/// Set the current stream size.
		/**
		 * @param size
		 *   New stream size.  This value will become the position the seek pointer
		 *   is moved to when seeking to the end of the stream.
		 *
		 * @note You should call flush() immediately after this function to fill up
		 *   the allocated data.  In very rare cases (mostly involving filters) you
		 *   may need to flush() before truncate() however as best-practice, always
		 *   call flush() after every truncate() or truncate_here() call.
		 *
		 * @throw write_error
		 *   The truncate could not be performed.
		 *
		 * @throw filter_error
		 *   There was an error decoding the data required to perform this
		 *   operation.
		 *
		 * @post The seek position is moved to the end of the stream (at offset
		 *   \e size)
		 */
		virtual void truncate(stream::pos size) = 0;

		/// Set the stream size to end at the current position.
		/**
		 * @see truncate()
		 *
		 * This function is just shorthand for calling truncate() with the current
		 * write position.
		 *
		 * @throw write_error
		 *   The truncate could not be performed.
		 *
		 * @throw filter_error
		 *   There was an error decoding the data required to perform this
		 *   operation.
		 */
		virtual void truncate_here();

		/// Commit all changes to the underlying storage medium.
		/**
		 * @throw write_error
		 *   The data could not be flushed.  The stream is in an indeterminate
		 *   state.
		 *
		 * @throw filter_error
		 *   There was an error decoding the data required to perform this
		 *   operation.
		 */
		virtual void flush() = 0;
};

/// Base stream interface for reading and writing data.
class DLL_EXPORT inout: virtual public input, virtual public output {
};

/// Output stream that automatically expands as needed.
/**
 * A normal stream::output will return an incomplete write once data has reached
 * the end of the available space, and requires an advance call to truncate() to
 * ensure there is enough data available to be written.
 *
 * The stream::expanding_output will handle this automatically, and will enlarge
 * itself as required in order to fit each write operation.
 *
 * Users should still either call truncate(0) before starting, or call
 * truncate_here() at the end, in case the stream started off much larger than
 * the amount of data that was written.
 */
class DLL_EXPORT expanding_output: virtual public output {
};

/// Base stream interface for reading and writing data.
class DLL_EXPORT expanding_inout: virtual public inout, virtual public expanding_output {
};

/// Copy one stream into another.
/**
 * @param dest
 *   Target stream to write data into, beginning at the current seek position.
 *
 * @param src
 *   Source stream to read data from, beginning from the current seek position.
 *
 * @throw read_error
 *   Data could not be read from src.
 *
 * @throw write_error
 *   Data could not be written to dest.
 *
 * @throw incomplete_write
 *  Not all data could fit into dest.
 *
 * @throw filter_error
 *   There was an error decoding the data required to perform this
 *   operation.
 */
void DLL_EXPORT copy(output& dest, input& src);

/// Copy possibly overlapping data from one position in a stream to another.
/**
 * @param data
 *   Target stream to move data around in.
 *
 * @param from
 *   Offset from the start of the stream where the data will be read from.
 *
 * @param to
 *   Offset from the start of the stream where the data will be written to.
 *
 * @param len
 *   Number of bytes to move.
 *
 * @throw read_error
 *   Data could not be read from src.
 *
 * @throw write_error
 *   Data could not be written to dest.
 *
 * @throw incomplete_write
 *  Not all data could fit into dest.
 *
 * @throw filter_error
 *   There was an error decoding the data required to perform this
 *   operation.
 *
 * @post \e len bytes of data that existed starting at position \e from before
 *   this function call are now at location \e to.  They may no longer exist at
 *   location \e from, if the regions overlapped.
 */
void DLL_EXPORT move(inout& data, pos from, pos to, len len);

/// iostream-style output function for char strings
inline output& operator << (output& s, const char *d) {
	s.write((const uint8_t *)d, strlen(d));
	return s;
}

/// iostream-style output function for strings
inline output& operator << (output& s, const std::string& d) {
	s.write(d);
	return s;
}

} // namespace stream
} // namespace camoto

#endif // _CAMOTO_STREAM_HPP_
