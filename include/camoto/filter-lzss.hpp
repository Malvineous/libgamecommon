/**
 * @file  camoto/filter-lzss.hpp
 * @brief Filter for compressing and decompressing data using LZSS.
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

#ifndef _CAMOTO_FILTER_LZSS_HPP_
#define _CAMOTO_FILTER_LZSS_HPP_

#include <vector>
#include <deque>

#include <camoto/bitstream.hpp>
#include <camoto/filter.hpp>

#ifndef DLL_EXPORT
#define DLL_EXPORT
#endif

namespace camoto {

/// LZSS decompressor
class DLL_EXPORT filter_lzss_decompress: public filter
{
	public:
		/// LZSS decompressor.
		/**
		 * @param endian
		 *   Endian-ness of the input data (from which end are the bytes split into
		 *   bits.)
		 *
		 * @param sizeLength
		 *   Size of the LZSS length field, in bits.
		 *
		 * @param sizeDistance
		 *   Size of the LZSS distance field, in bits.
		 */
		filter_lzss_decompress(bitstream::endian endian, unsigned int sizeLength,
			unsigned int sizeDistance);

		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut, const uint8_t *in,
			stream::len *lenIn);

	protected:
		bitstream data;
		unsigned int sizeLength;   ///< Size of the length field, in bits
		unsigned int sizeDistance; ///< Size of the distance field, in bits

		enum class State {
			S0_READ_FLAG,  ///< Read the first code/flag
			S1_COPY_BYTE,  ///< Copy a literal byte
			S2_READ_LEN,   ///< Read the length of the back-reference
			S3_READ_DIST,  ///< Read the offset of the back-reference
			S4_COPY_REF,   ///< Copy the backreference to the end of the output data
		} state;
		const unsigned int maxDistance; ///< Size of sliding window, 1 << sizeDist
		std::unique_ptr<uint8_t[]> window; ///< Sliding window, maxDistance-1 elements
		unsigned int posWindow; ///< Current offset within window

		unsigned int lzssLength;   ///< Last value of the length field
		unsigned int lzssDistance; ///< Last value of the distance field
};

/// LZSS compressor
class DLL_EXPORT filter_lzss_compress: public filter
{
	public:
		/// LZSS compression constructor.
		/**
		 * This is currently just a dummy compressor.  It writes out LZSS-compatible
		 * data that can be decompressed correctly, but it doesn't actually perform
		 * any compression - so the output data will be slightly larger than the
		 * input (as each 8-bit byte is written out as a 9-bit byte.)
		 *
		 * @param endian
		 *   Endian-ness of the input data (from which end are the bytes split into
		 *   bits.)
		 *
		 * @param sizeLength
		 *   Size of the LZSS length field, in bits.
		 *
		 * @param sizeDistance
		 *   Size of the LZSS distance field, in bits.
		 */
		filter_lzss_compress(bitstream::endian endian, unsigned int sizeLength,
			unsigned int sizeDistance);

		virtual void reset(stream::len lenInput);
		virtual void transform(uint8_t *out, stream::len *lenOut, const uint8_t *in,
			stream::len *lenIn);

	protected:
		bitstream data;
		unsigned int sizeLength;   ///< Size of the length field, in bits
		unsigned int sizeDistance; ///< Size of the distance field, in bits
};

} // namespace camoto

#endif // _CAMOTO_FILTER_LZSS_HPP_
