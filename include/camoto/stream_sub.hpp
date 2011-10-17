/**
 * @file   stream_sub.hpp
 * @brief  Provide a stream limited to a subsection within another stream.
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

#ifndef _CAMOTO_STREAM_SUB_HPP_
#define _CAMOTO_STREAM_SUB_HPP_

#include <camoto/stream.hpp>

namespace camoto {
namespace stream {

/// Substream parts in common with read and write
class sub_core {

	protected:
		stream::pos start;      ///< Offset into parent stream
		stream::len stream_len; ///< Length of substream
		stream::pos offset;     ///< Current pointer position

		/// Common seek function for reading and writing.
		/**
		 * @copydetails input::seekg()
		 */
		void seek(stream::delta off, seek_from from)
			throw (seek_error);

		/// Move the substream's start point within the parent stream.
		/**
		 * @param off
		 *   Distance to move the stream.  Negative values move closer to
		 *   the start of the parent stream.
		 */
		void relocate(stream::delta off)
			throw ();

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
		void resize(stream::len len)
			throw ();

		/// Get the current offset into the parent stream.
		/**
		 * @return Current offset, relative to start of parent stream, where first
		 *   byte in the substream sits.
		 */
		stream::pos get_offset()
			throw ();
};

/// Read-only stream to access a section within another stream.
class input_sub: virtual public input,
                 virtual protected sub_core
{
	public:
		virtual stream::len try_read(uint8_t *buffer, stream::len len)
			throw ();

		virtual void seekg(stream::delta off, seek_from from)
			throw (seek_error);

		virtual stream::pos tellg() const
			throw (seek_error);

		virtual stream::pos size() const
			throw (seek_error);

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
		void open(input_sptr parent, stream::pos start, stream::len len)
			throw ();

		using sub_core::relocate;
		using sub_core::resize;
		using sub_core::get_offset;

	protected:
		input_sptr in_parent; ///< Parent stream for reading
};

/// Shared pointer to a readable substream.
typedef boost::shared_ptr<input_sub> input_sub_sptr;

/// Write-only stream to access a section within another stream.
class output_sub: virtual public output,
                  virtual protected sub_core
{
	public:
		virtual stream::len try_write(const uint8_t *buffer, stream::len len)
			throw (write_error);

		virtual void seekp(stream::delta off, seek_from from)
			throw (seek_error);

		virtual stream::pos tellp() const
			throw (seek_error);

		virtual void truncate(stream::pos size)
			throw (write_error);

		virtual void flush()
			throw ();

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
		 */
		void open(output_sptr parent, stream::pos start, stream::len len,
			fn_truncate fn_resize)
			throw ();

		using sub_core::relocate;
		using sub_core::resize;
		using sub_core::get_offset;

	protected:
		/// Callback to alert parent stream we want to change size.
		fn_truncate fn_resize;
		output_sptr out_parent; ///< Parent stream for writing
};

/// Shared pointer to a writable substream.
typedef boost::shared_ptr<output_sub> output_sub_sptr;

/// Read/write stream accessing a section within another stream.
class sub: virtual public inout,
           virtual public input_sub,
           virtual public output_sub
{
	public:
		sub()
			throw ();

		/// Map onto a subsection of another stream.
		/**
		 * @copydetails output_sub::open()
		 */
		void open(inout_sptr parent, stream::pos start, stream::len len,
			fn_truncate fn_resize)
			throw ();

		using sub_core::relocate;
		using sub_core::resize;
		using sub_core::get_offset;
};

/// Shared pointer to a readable and writable substream.
typedef boost::shared_ptr<sub> sub_sptr;

} // namespace stream
} // namespace camoto

#endif // _CAMOTO_STREAM_SUB_HPP_
