/**
 * @file  camoto/stream_sub.hpp
 * @brief Provide a stream limited to a subsection within another stream.
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

#ifndef _CAMOTO_STREAM_SUB_HPP_
#define _CAMOTO_STREAM_SUB_HPP_

#include <camoto/stream.hpp>

namespace camoto {
namespace stream {

class output_sub;

/// Callback function for changing the size of an output substream.
typedef boost::function<void(output_sub*, len)> fn_truncate_sub;

/// Substream parts in common with read and write
class DLL_EXPORT sub_core
{
	protected:
		sub_core(pos start, len len);

		/// Common seek function for reading and writing.
		/**
		 * @copydetails input::seekg()
		 */
		void seek(stream::delta off, seek_from from);

	public:
		/// Move the substream's start point within the parent stream.
		/**
		 * @param off
		 *   Distance to move the stream.  Negative values move closer to
		 *   the start of the parent stream.
		 */
		void relocate(stream::delta off);

		/// Alter the size of the substream without affecting any data.
		/**
		 * This function should only be called by the creator of the stream if the
		 * parent stream has been modified outside of the substream.
		 *
		 * Normally output_sub::truncate() would be used to correctly resize the
		 * substream.  If in doubt, don't use this function!
		 *
		 * @param len
		 *   New size of the substream.
		 */
		void resize(stream::len len);

		/// Get the current offset into the parent stream.
		/**
		 * @return Current offset, relative to start of parent stream, where first
		 *   byte in the substream sits.
		 */
		stream::pos start();

		stream::pos stream_start; ///< Offset into parent stream
		stream::len stream_len;   ///< Length of substream
		stream::pos offset;       ///< Current pointer position
};

/// Read-only stream to access a section within another stream.
class DLL_EXPORT input_sub:
	virtual public input,
	virtual public sub_core
{
	public:
		/// Map onto a subsection of another stream.
		/**
		 * @param parent
		 *   Parent stream supplying the data.
		 *
		 * @param start
		 *   Offset of data (relative to start of \e parent) to appear at substream
		 *   offset 0.
		 *
		 * @param len
		 *   Length of data to make available in the substream.  \e start + \e len
		 *   must be <= parent->size().
		 */
		input_sub(std::shared_ptr<input> parent, pos start, len len);

		virtual stream::len try_read(uint8_t *buffer, stream::len len);
		virtual void seekg(stream::delta off, seek_from from);
		virtual stream::pos tellg() const;
		virtual stream::len size() const;

	protected:
		std::shared_ptr<input> in_parent; ///< Parent stream for reading
};

/// Write-only stream to access a section within another stream.
class DLL_EXPORT output_sub:
	virtual public output,
	virtual public sub_core
{
	public:
		/// Map onto a subsection of another stream.
		/**
		 * @param parent
		 *   Parent stream supplying the data.
		 *
		 * @param start
		 *   Offset of data (relative to start of \e parent) to appear at substream
		 *   offset 0.
		 *
		 * @param len
		 *   Length of data to make available in the substream.  \e start + \e len
		 *   must be <= parent->size().
		 *
		 * @param fn_resize
		 *   Callback function to notify parent if and when the substream is trying
		 *   to enlarge itself.  If this parameter is NULL, or the callback function
		 *   returns false to indicate the resize was not performed, the substream's
		 *   write function will throw incomplete_write (or try_write() will return
		 *   only a partial write.)
		 *
		 * @note If using boost::bind to generate a function pointer for fn_resize,
		 *   be sure to use std::weak_ptr<> in place of any std::shared_ptr<>
		 *   parameters containing the substream.  If this is not done, the
		 *   substream will hold a shared_ptr<> to itself, meaning it will never be
		 *   deleted and it will result in a memory leak.
		 *   If in doubt, wrap whatever you want to do in a free function that only
		 *   takes weak_ptr<> arguments.
		 */
		output_sub(std::shared_ptr<output> parent, pos start, len len,
			fn_truncate_sub fn_resize);

		virtual stream::len try_write(const uint8_t *buffer, stream::len len);
		virtual void seekp(stream::delta off, seek_from from);
		virtual stream::pos tellp() const;
		virtual void truncate(stream::pos size);
		virtual void flush();

	protected:
		/// Parent stream for writing.
		std::shared_ptr<output> out_parent;

		/// Callback to alert parent stream we want to change size.
		fn_truncate_sub fn_resize;
};

/// Read/write stream accessing a section within another stream.
class DLL_EXPORT sub:
	virtual public inout,
	virtual public input_sub,
	virtual public output_sub
{
	public:
		/// Map onto a subsection of another stream.
		/**
		 * @copydetails output_sub::open()
		 */
		sub(std::shared_ptr<inout> parent, pos start, len len,
			fn_truncate_sub fn_resize);
};

} // namespace stream
} // namespace camoto

#endif // _CAMOTO_STREAM_SUB_HPP_
