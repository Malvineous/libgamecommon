/**
 * @file   bitstream.hpp
 * @brief  iostream wrapper reading and writing a number of bits rather than
 *         bytes.
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _CAMOTO_BITSTREAM_HPP_
#define _CAMOTO_BITSTREAM_HPP_

#include <iosfwd>                           // streamsize, seekdir
#include <boost/iostreams/positioning.hpp>  // stream_offset
#include <boost/function.hpp>

#include <camoto/types.hpp>

namespace camoto {

namespace io = boost::iostreams;

typedef boost::function<int (uint8_t *)> fn_getnextchar;
typedef boost::function<int (uint8_t)> fn_putnextchar;

/// Class for reading and writing to iostreams at the bit level rather than
/// the byte level.
class bitstream {
	private:
		/// Parent stream, where the actual data is read from and written to.
		iostream_sptr parent;

		/// Current seek position within the current byte (0-7)
		uint8_t curBitPos;
		uint8_t bufByte;
		uint8_t bufBits;

	public:

		/// Byte order in the bitstream
		enum endian {
			littleEndian,
			bigEndian,
		} endianType;

		/// Create a bitstream out of the given stream
		/**
		 * @param parent  Parent stream, where the data comes from
		 * @param isLittleEndian  Whether the bytes are in little or big endian
		 *   order.  To represent 0x102 as a nine-bit number, in little endian it
		 *   would be written out as 02 80, in big endian it would be written as
		 *   81 00.
		 */
		bitstream(iostream_sptr parent, endian endianType)
			throw ();

		/// Stream-less constructor for using variant of read() with stream as
		/// a parameter.
		bitstream(endian et)
			throw ();

		~bitstream()
			throw ();

		/// Callback function which will read in the next char from %parent.
		int nextCharFromParent(uint8_t *out);

		/// Callback function which will write the given char to %parent.
		int nextCharToParent(uint8_t in);

		/// Read some bits in from the stream.
		int read(int bits, int *out)
			throw (std::ios::failure);

		/// Read some bits from a particular stream.
		/**
		 * @note The internal state is not stream-specific, so if you pass different
		 *   streams to this function the results will be quite strange.  This
		 *   function is only intended to be used in Boost iostream filters, which
		 *   supply the source stream as a parameter to the read() function rather
		 *   that at object construction time.
		 */
		int read(fn_getnextchar fnNextChar, int bits, int *out)
			throw (std::ios::failure);

		/// Write some bits out to the stream.
		int write(int bits, int in)
			throw (std::ios::failure);

		/// Write some bits to a particular stream.
		/**
		 * @note The internal state is not stream-specific, so if you pass different
		 *   streams to this function the results will be quite strange.  This
		 *   function is only intended to be used in Boost iostream filters, which
		 *   supply the source stream as a parameter to the read() function rather
		 *   that at object construction time.
		 */
		int write(fn_putnextchar fnNextChar, int bits, int in)
			throw (std::ios::failure);

		/// Seek to a given bit position within the stream.
		/**
		 * @param off  Bit offset (e.g. 0 == first byte, 8 == second byte)
		 * @param way  Seek direction - std::ios::beg/cur/end
		 * @return Current offset (in bits from start of file)
		 */
		io::stream_offset seek(io::stream_offset off, std::ios_base::seekdir way)
			throw (std::ios::failure);

};

typedef boost::shared_ptr<bitstream> bitstream_sptr;

} // namespace camoto

#endif // _CAMOTO_BITSTREAM_HPP_
