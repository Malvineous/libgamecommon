/**
 * @file  camoto/bitstream.hpp
 * @brief iostream wrapper reading and writing a number of bits rather than
 *        bytes.
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

#ifndef _CAMOTO_BITSTREAM_HPP_
#define _CAMOTO_BITSTREAM_HPP_

#include <memory>
#include <boost/function.hpp>
#include <camoto/stream.hpp>

#ifndef DLL_EXPORT
#define DLL_EXPORT
#endif

namespace camoto {

typedef boost::function<int (uint8_t*)> fn_getnextchar;
typedef boost::function<int (uint8_t)> fn_putnextchar;

/// Class for reading and writing to iostreams at the bit level rather than
/// the byte level.
class DLL_EXPORT bitstream
{
	private:
		/// Parent stream, where the actual data is read from and written to.
		std::shared_ptr<stream::inout> parent;

		/// Current offset into parent stream.
		/**
		 * It would be nicer not to have to use this, but the logic gets so
		 * complicated when the C++ standard requires a seek operation before
		 * switching between reading and writing, and we don't have the current
		 * offset.
		 */
		stream::pos offset;

		/// Current seek position within bufByte (0-7), or 8 if bufByte has been
		/// entirely used and must be updated on the next read/write.
		uint8_t curBitPos;

		/// This contains the currently buffered byte, i.e. when the read/write
		/// operation ends within a byte, this contains the value of that byte.
		uint8_t bufByte;

		/// The value of bufByte as read from the parent stream
		/**
		 * If this is different to bufByte then a write operation occurred and
		 * bufByte must be written back to the parent stream.  If this is
		 * WASNT_BUFFERED (-1) then the bufByte was never read in, which would
		 * happen when the last operation was a write operation and it went over a
		 * byte boundary.  If this is INITIAL_VALUE (-2) then it's the first read
		 * operation and no writes have occurred.
		 */
		int origBufByte;

	public:

		/// Byte order in the bitstream
		enum endian {
			littleEndian,    ///< Little-endian (Intel)
			bigEndian,       ///< Big-endian (Motorola)
		} endianType;

		/// Create a bitstream out of the given stream
		/**
		 * @param parent
		 *   Parent stream, where the data comes from.
		 *
		 * @param endianType
		 *   Whether the bytes are in little or big endian order.  To represent
		 *   0x102 as a nine-bit number, in little endian it would be written out
		 *   as 02 01, in big endian it would be written as 81 00.
		 *
		 *   Number: 0x102 (100000010)
		 *   Little: 02 01 (00000010 00000001)
		 *      Big: 81 00 (10000001 0)
		 *
		 * In other words, reading one bit at a time, the most-significant-bit (MSB)
		 * of each byte is read first in big-endian order, while the LSB is read
		 * first in little-endian order.
		 */
		bitstream(std::shared_ptr<stream::inout> parent, endian endianType);

		/// Stream-less constructor for using variant of read() with stream as
		/// a parameter.
		/**
		 * @param endianType
		 *   Endian type.
		 */
		bitstream(endian endianType);

		/// Destructor.
		~bitstream();

		/// Read some bits in from the stream.
		/**
		 * @param bits
		 *   Number of bits to read, e.g. 8 to read a byte.
		 *
		 * @param out
		 *   Where to store the value read.
		 *
		 * @return The number of bits actually read.  Will be the same as the bits
		 *   parameter if the read completed fully.
		 */
		int read(unsigned int bits, unsigned int *out);

		/// Read some bits from a particular stream.
		/**
		 * This function is only intended to be used in Boost iostream filters,
		 * which supply the source stream as a parameter to the read() function
		 * rather than at object construction time.
		 *
		 * @note The internal state is not stream-specific, so if you pass different
		 *   streams to this function the results will be quite strange.
		 *
		 * @note Obviously the seek() function cannot be used in combination with
		 *   this particular function.
		 *
		 * @param fnNextChar
		 *   The function to call to read the next byte.
		 *
		 * @param bits
		 *   Number of bits to read, e.g. 8 to read a byte.
		 *
		 * @param out
		 *   Where to store the value read.
		 *
		 * @return The number of *bits* read, or < 0 on error (e.g. EOF/-1)
		 */
		int read(fn_getnextchar fnNextChar, unsigned int bits, unsigned int *out);

		/// Write some bits out to the stream.
		/**
		 * @param bits
		 *   Number of bits to write, e.g. 8 to write a byte.
		 *
		 * @param in
		 *   The value to write.  This must be small enough to fit in the number of
		 *   bits being written, otherwise an assertion failure will result.
		 *
		 * @return The number of bits written.  Will be the same as the bits
		 *   parameter if the write completed fully.
		 */
		int write(unsigned int bits, unsigned int in);

		/// Write some bits to a particular stream.
		/**
		 * This function is only intended to be used in Boost iostream filters,
		 * which supply the source stream as a parameter to the write() function
		 * rather than at object construction time.
		 *
		 * @note The internal state is not stream-specific, so if you pass different
		 *   streams to this function the results will be quite strange.
		 *
		 * @note Obviously the seek() function cannot be used in combination with
		 *   this particular function.
		 *
		 * @param fnNextChar
		 *   The function to call to write the next byte.
		 *
		 * @param bits
		 *   Number of bits to write, e.g. 8 to write a byte.
		 *
		 * @param in
		 *   The value to write.  This must be small enough to fit in the number of
		 *   bits being written, otherwise an assertion failure will result.
		 *
		 * @return The number of *bits* written, or < 0 on error (e.g. EOF/-1)
		 */
		int write(fn_putnextchar fnNextChar, unsigned int bits, unsigned int in);

		/// Seek to a given bit position within the stream.
		/**
		 * @note This only works with the read() and write() function which do NOT
		 *   take an fnNextChar parameter.
		 *
		 * @param off
		 *   Bit offset (e.g. 0 == first byte, 8 == second byte), can be negative
		 *   if \e way is not stream::start.
		 *
		 * @param way
		 *   Seek direction - stream::start/cur/end
		 *
		 * @return Current offset (in bits from start of file)
		 */
		stream::pos seek(stream::delta off, stream::seek_from way);

		/// Write out any partially written byte to the underlying stream.
		/**
		 * @note Uses this->parent, so it only works with the read() and write()
		 *   functions which do NOT take an fnNextChar parameter.
		 */
		void flush();

		/// Alter the endian type without affecting the current seek position.
		/**
		 * @param endianType
		 *   New endian type.
		 */
		void changeEndian(endian endianType);

		/// Get the current endian setting.
		/**
		 * @return Current endian type.
		 */
		endian getEndian();

		/// Flush the byte currently cached.
		/**
		 * This will cause the next read operation to start at the following byte
		 * boundary.
		 */
		void flushByte();

		/// Flush the byte currently cached, ensuring it is written out.
		/**
		 * This will cause the next write operation to start at the following byte
		 * boundary.
		 *
		 * @param fnNextChar
		 *   The function to call to write the next byte.
		 */
		void flushByte(fn_putnextchar fnNextChar);

		/// Write bufByte out to the parent stream if it has changed.
		/**
		 * @note Uses this->parent, so it only works with the read() and write()
		 *   functions which do NOT take an fnNextChar parameter.
		 */
		void writeBufByte();

		/// Peek at the next byte that will be written out.
		/**
		 * @param buf
		 *   Where to store the byte.
		 *
		 * @param mask
		 *   Mask of valid bits, e.g. 0xF0 indicates only the four most-significant
		 *   bits in \e buf are valid.
		 */
		void peekByte(uint8_t *buf, uint8_t *mask);
};

} // namespace camoto

#endif // _CAMOTO_BITSTREAM_HPP_
