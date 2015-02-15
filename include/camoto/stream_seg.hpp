/**
 * @file  camoto/stream_seg.hpp
 * @brief Segmented stream implementation providing the ability to insert and
 *        remove blocks of data at any point in the underlying stream, shifting
 *        data around as necessary.
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

#ifndef _CAMOTO_STREAM_SEG_HPP_
#define _CAMOTO_STREAM_SEG_HPP_

#include <memory>
#include <vector>
#include <camoto/stream.hpp>

namespace camoto {
namespace stream {

class seg;

/// Read/write segmented stream
/**
 * This stream sits on top of another, and allows data to be inserted and
 * removed, automatically shifting the existing data out of the way.  Data in
 * the underlying stream is only partially modified before flush() is called,
 * which performs the "heavy lifting" of relocating the data as necessary.
 *
 * @see insert() and remove()
 */
class DLL_EXPORT seg: virtual public inout
{
	public:
		/// Create a segmented stream backed onto another stream.
		/**
		 * @param parent
		 *   Parent stream supplying the data.
		 */
		seg(std::shared_ptr<inout> parent);

		virtual stream::len try_read(uint8_t *buffer, stream::len len);
		virtual void seekg(stream::delta off, seek_from from);
		virtual stream::pos tellg() const;
		virtual stream::pos size() const;
		virtual stream::len try_write(const uint8_t *buffer, stream::len len);
		virtual void seekp(stream::delta off, seek_from from);
		virtual stream::pos tellp() const;
		virtual void truncate(stream::pos size);
		virtual void flush();

		/// Insert a block of data at the pointer.
		/**
		 * The rest of the data is shifted forward, out of the way.  Seek position
		 * remains unchanged, but stream size will have enlarged by \e lenInsert
		 * bytes.
		 *
		 * @code
		 * Before: AAAABBBB
		 * After:  AAAA____BBBB
		 *             ^ Seek position, len == 4
		 * @endcode
		 *
		 * @param lenInsert
		 *   Number of bytes to insert.
		 *
		 * @note This function will call the fn_resize parameter passed to open() if
		 *   the new size is greater than the parent stream's size.
		 *
		 * @throw write_error
		 *   The parent stream could not be resized to accommodate the new block.
		 */
		void insert(stream::len lenInsert);

		/// Remove a chunk of data starting at the pointer's location.
		/**
		 * The rest of the data is shifted back.  All data from the current seek
		 * position to \e lenRemove bytes after it are lost.  The seek position
		 * remains unchanged, but the stream size will have shrunk by \e lenRemove
		 * bytes.
		 *
		 * @code
		 * Before: AAAAXXXXBBBB
		 * After:  AAAABBBB
		 *             ^ Seek position, len == 4
		 * @endcode
		 *
		 * @param lenRemove
		 *   Number of bytes to remove.
		 *
		 * @throw write_error
		 *   The parent stream could not be resized to accommodate the new block.
		 */
		void remove(stream::len lenRemove);

	protected:
		seg();

		std::shared_ptr<inout> parent;      ///< Parent stream
		stream::pos off_parent;             ///< Offset into parent stream
		stream::pos off_endparent;          ///< Offset of vcSecond
		std::vector<uint8_t> vcSecond;      ///< Data to place after parent stream
		std::unique_ptr<seg> psegThird;     ///< Data to place after vcSecond

		/// Offset into self (starts at 0)
		/**
		 * When offset == 0, the parent stream file pointer is at off_parent.
		 */
		stream::pos offset;

		/// Split the segstream at the current seek position.
		/**
		 * Upon return the first data source will only last until the current seek
		 * position, the second data source will be empty and the third data source
		 * will contain all the data that was originally after the current seek
		 * position.
		 *
		 * @code
		 * Before: AAAABBBB
		 *             ^ seek position
		 * After:  AAAABBBB
		 * first --^   ^-- third (second is empty)
		 * @endcode
		 */
		void split();

		/// Commit the data to the underlying stream.
		/**
		 * This is used by flush() to move the first segment around as necessary,
		 * then write the third segment (which often shares the same underlying
		 * stream as the first segment) and lastly writes out the second segment in
		 * the middle.  It has to be done in this order so that no data we need gets
		 * overwritten before it has been moved out of the way.
		 */
		void commit(stream::pos poffWriteFirst);
};

} // namespace stream
} // namespace camoto

#endif // _CAMOTO_STREAM_SEG_HPP_
