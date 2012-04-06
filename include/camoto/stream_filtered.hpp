/**
 * @file   stream_filtered.hpp
 * @brief  Pass read/write operations through a filter to modify the data.
 *
 * Copyright (C) 2010-2012 Adam Nielsen <malvineous@shikadi.net>
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
#include <camoto/stream_string.hpp>

namespace camoto {
namespace stream {

/// Read-only stream applying a filter to another read-only stream.
class input_filtered: virtual public input_string
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
		void open(input_sptr parent, filter_sptr read_filter)
			throw (filter_error, read_error);
};

/// Shared pointer to a readable filtered stream.
typedef boost::shared_ptr<input_filtered> input_filtered_sptr;

/// Write-only stream applying a filter to another write-only stream.
class output_filtered: virtual public output_string
{
	public:
		virtual void truncate(stream::pos size)
			throw (write_error);

		virtual void flush()
			throw (write_error);

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
		void open(output_sptr parent, filter_sptr write_filter, fn_truncate resize)
			throw ();

	protected:
		filter_sptr write_filter; ///< Filter to pass data through
		output_sptr out_parent;   ///< Parent stream for writing
		fn_truncate fn_resize;    ///< Size-change notification function
		bool done_filter;         ///< Set to true once filter has been run once
};

/// Shared pointer to a writable filtered stream.
typedef boost::shared_ptr<output_filtered> output_filtered_sptr;

/// Read/write stream applying a filter to another read/write stream.
class filtered: virtual public inout,
                virtual public input_filtered,
                virtual public output_filtered
{
	public:
		filtered()
			throw ();

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
		void open(inout_sptr parent, filter_sptr read_filter,
			filter_sptr write_filter, fn_truncate resize)
			throw (filter_error, read_error);
};

/// Shared pointer to a readable and writable filtered stream.
typedef boost::shared_ptr<filtered> filtered_sptr;

} // namespace stream
} // namespace camoto

#endif // _CAMOTO_STREAM_FILTERED_HPP_
