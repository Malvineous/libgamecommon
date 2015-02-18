/**
 * @file  camoto/filter.hpp
 * @brief Base filter implementation.
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

#ifndef _CAMOTO_FILTER_HPP_
#define _CAMOTO_FILTER_HPP_

#include <exception>
#include <string>
#include <stdint.h>
#include <camoto/stream.hpp>

#ifndef DLL_EXPORT
#define DLL_EXPORT
#endif

namespace camoto {

/// Could not read data from stream.
class DLL_EXPORT filter_error: public stream::error
{
	public:
		/// Constructor.
		/**
		 * @param msg
		 *   Error description for UI messages.
		 */
		filter_error(const std::string& msg);
};

/// Base filter interface.
/**
 * The filter converts data in one format to data in another.  For example one
 * filter could compress data with a given algorithm and a different filter
 * could decompress that data back into its original form.  In this case the
 * input and output data will be different sizes.
 *
 * Alternatively an XOR filter could encrypt data and in this case the input
 * and output data would be the same size.  Since the XOR encryption algorithm
 * is symmetric, the same filter could be used to decrypt the data as well.
 */
class DLL_EXPORT filter
{
	public:
		/// Reset this filter back to its initial state.
		/**
		 * Often a filter will need to be called multiple times, such as saving some
		 * data, making more changes, then saving again.  This function will be
		 * called before each read or write operation begins, to ensure the filter
		 * is set back to its initial state.
		 *
		 * @param lenInput
		 *   Length of the input data stream.  This value is often used when
		 *   compressing data, and the length of the decompressed data must be
		 *   written at the start of the output (compressed) stream.
		 */
		virtual void reset(stream::len lenInput) = 0;

		/// Process some bytes through this filter.
		/**
		 * A block of input data is supplied, and this function transforms it as
		 * appropriate and writes the output data into a second block.  For ease of
		 * writing filters, not all the input data has to be read, and the output
		 * buffer need only be partially filled.  The function will be called
		 * repeatedly until it signals that it has not consumed any input data and
		 * also not produced any output data.
		 *
		 * @param out
		 *   Pointer to memory where post-processed data will be stored.
		 *
		 * @param lenOut
		 *   Initially, the size of \e out (i.e. the maximum number of output
		 *   bytes.)  On return, this is set to the number of bytes that were
		 *   placed into \e out.  This will always be >= MIN_BUFFER_SIZE.
		 *
		 * @param in
		 *   Pointer to where the data needing processing is to be read from.
		 *
		 * @param lenIn
		 *   Initially, the size of \e in (i.e. the maximum number of input
		 *   bytes that can be read.)  On return, this is set to the number of
		 *   input bytes that were read.  Any remaining (unread) bytes will be
		 *   passed in on the next call.
		 *
		 *   The function can be called with this value set to zero if all the
		 *   input data has been consumed but the filter has not indicated EOF
		 *   (e.g. if it is still writing out decompressed data).
		 *
		 * @note You must signal the end of the data stream by setting lenOut
		 *   and lenIn to zero on return.  If this is not done the filter will
		 *   be called in an endless loop!
		 *
		 * @note \e in will be populated with at least a few hundred bytes, and
		 *   only less than this when reaching the end of the incoming data.
		 *   Therefore if there is not enough incoming data to produce a single
		 *   byte of output data, a filter_error should be thrown to indicate
		 *   the input data was truncated.
		 *
		 * @throw filter_error
		 *   The data was corrupted and could not be filtered.
		 */
		virtual void transform(uint8_t *out, stream::len *lenOut, const uint8_t *in,
			stream::len *lenIn) = 0;
};

} // namespace camoto

#endif // _CAMOTO_FILTER_HPP_
