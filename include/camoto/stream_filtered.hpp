/**
 * @file  camoto/stream_filtered.hpp
 * @brief Pass read/write operations through a filter to modify the data.
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

#ifndef _CAMOTO_STREAM_FILTERED_HPP_
#define _CAMOTO_STREAM_FILTERED_HPP_

#include <camoto/filter.hpp>
#include <camoto/stream_memory.hpp>

namespace camoto {
namespace stream {

class output_filtered;

/// Truncate function callback (to truncate a stream::output)
/**
 * This function is called with an integer parameter when a stream
 * needs to be shrunk or enlarged to the given size.
 *
 * Since no other data can be passed with this function call, usually
 * boost::bind is used to create a "wrapping" around some other function with
 * more parameters.
 *
 * The function signature is:
 * @code
 * void fnTruncate(stream::output_filtered* output, stream::len new_length);
 * @endcode
 *
 * This example uses boost::bind to package up a call to the Linux
 * truncate() function (which requires both a filename and size) such that
 * the filename is supplied in advance and not required when the \e fn_truncate
 * call is made.
 *
 * @code
 * fn_truncate fnTruncate = boost::bind<void>(truncate, "graphics.dat", _2);
 * // later...
 * fnTruncate(123);  // calls truncate("graphics.dat", 123)
 * @endcode
 *
 * This callback is used in cases where both a file's compressed and
 * decompressed size are stored.  Here the callback will be notified with the
 * prefiltered size during the flush() call, with the postfiltered size
 * being the amount of data that was actually written to the stream.
 *
 * The callback should throw stream::write_error if the operation did not
 * succeed.
 */
typedef boost::function<void(stream::output_filtered*, stream::len)> fn_truncate_filter;

/// Read-only stream applying a filter to another read-only stream.
class DLL_EXPORT input_filtered: virtual public input_memory
{
	public:
		/// Apply a filter to the given stream.
		/**
		 * As data is read from this stream (the input_filtered instance), data is
		 * in turn read from \e parent and passed through \e read_filter before
		 * being returned to the caller.
		 *
		 * @param parent
		 *   Parent stream supplying the data.
		 *
		 * @param read_filter
		 *   Filter to process data.
		 */
		input_filtered(std::shared_ptr<input> parent,
			std::shared_ptr<filter> read_filter);

		virtual stream::len try_read(uint8_t *buffer, stream::len len);
		virtual void seekg(stream::delta off, seek_from from);
		virtual stream::pos tellg() const;
		virtual stream::len size() const;

		/// A partial write is about to occur, ensure the unfiltered data is present.
		/**
		 * When opening a read/write stream, the data is not populated
		 * automatically to avoid unnecessary computation in case it is going to be
		 * overwritten.
		 *
		 * This function is overridden in a read/write stream so that the original
		 * data is populated before any read or write occurs.
		 */
		virtual void populate() const;

		/// Non-const version of populate() that actually does the work.
		void realPopulate();

	protected:
		std::shared_ptr<input> in_parent;   ///< Parent stream for reading
		std::shared_ptr<filter> read_filter; ///< Filter to pass data through
		bool populated; ///< Has the input data been run through the filter yet?
};

/// Write-only stream applying a filter to another write-only stream.
class DLL_EXPORT output_filtered: virtual public output_memory
{
	public:
		/// Apply a filter to the given stream.
		/**
		 * As data is written to this stream (the output_filtered instance), the
		 * data is first passed through \e write_filter before being written to
		 * \e parent.
		 *
		 * @param parent
		 *   Parent stream to write the processed data to.
		 *
		 * @param write_filter
		 *   Filter to process data.
		 *
		 * @param resize
		 *   Notification function called when the stream is resized.  This function
		 *   doesn't have to do anything (and can be NULL) but it is used in cases
		 *   where a game archive stores both a file's compressed and decompressed
		 *   size.  Here the callback will be notified of the decompressed size
		 *   during the flush() call, while parent->truncate() will be called with
		 *   the compressed size.
		 */
		output_filtered(std::shared_ptr<output> parent,
			std::shared_ptr<filter> write_filter, fn_truncate_filter resize);

		virtual stream::len try_write(const uint8_t *buffer, stream::len len);
		virtual void seekp(stream::delta off, seek_from from);
		virtual stream::pos tellp() const;
		virtual void flush();

		/// A partial write is about to occur, ensure the unfiltered data is present.
		/**
		 * When opening a read/write stream, the data is not populated
		 * automatically to avoid unnecessary computation in case it is going to be
		 * overwritten.
		 *
		 * This function is overridden in a read/write stream so that the original
		 * data is populated before any read or write occurs.
		 */
		virtual void populate() const;

	protected:
		std::shared_ptr<output> out_parent;   ///< Parent stream for writing
		std::shared_ptr<filter> write_filter; ///< Filter to pass data through
		fn_truncate_filter fn_resize;         ///< Size-change notification callback
		bool done_filter;                     ///< true once filter has run once
};

/// Read/write stream applying a filter to another read/write stream.
class DLL_EXPORT filtered:
	virtual public inout,
	virtual public input_filtered,
	virtual public output_filtered
{
	public:
		/// Apply a filter to the given stream.
		/**
		 * As data is read from this stream (the filtered instance), data is
		 * in turn read from \e parent and passed through \e read_filter before
		 * being returned to the caller.  Likewise any data written is passed
		 * through \e write_filter before being written to \e parent.
		 *
		 * Internally this is implemented by reading the entire contents of
		 * \e parent in the constructor, passing it through \e read_filter and
		 * storing it in memory.  Data is then read from and written to this
		 * memory buffer.  On flush(), the data is passed through \e write_filter
		 * and written back to the parent stream.
		 *
		 * @param parent
		 *   Parent stream supplying the data.
		 *
		 * @param read_filter
		 *   Filter to process data.
		 *
		 * @param write_filter
		 *   Filter to process data.
		 *
		 * @param resize
		 *   Notification function called when the stream is resized.  This function
		 *   doesn't have to do anything (and can be NULL) but it is used in cases
		 *   where a game archive stores both a file's compressed and decompressed
		 *   size.  Here the callback will be notified of the decompressed size
		 *   during the flush() call.
		 */
		filtered(std::shared_ptr<inout> parent,
			std::shared_ptr<filter> read_filter, std::shared_ptr<filter> write_filter,
			fn_truncate_filter resize);

		virtual void truncate(stream::len size);

		virtual void populate() const;
};

} // namespace stream
} // namespace camoto

#endif // _CAMOTO_STREAM_FILTERED_HPP_
