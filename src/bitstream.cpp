/**
 * @file   bitstream.cpp
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

/**
 * @todo Add a flush function for when the last bits written don't end on a
 * byte boundary
 *
 * @todo Implement seek code (w/ tests)
 */

#include <boost/bind.hpp>
#include <camoto/bitstream.hpp>

namespace camoto {

namespace io = boost::iostreams;

bitstream::bitstream(iostream_sptr parent, bitstream::endian endianType)
	throw () :
		parent(parent),
		endianType(endianType),
		curBitPos(0),
		bufByte(0),
		bufBits(0)
{
}

bitstream::bitstream(bitstream::endian endianType)
	throw () :
		endianType(endianType),
		curBitPos(0),
		bufByte(0),
		bufBits(0)
{
}

bitstream::~bitstream()
	throw ()
{
}

int bitstream::nextCharFromParent(uint8_t *out)
{
	this->parent->read((char *)out, 1);
	return this->parent->gcount();
}

int bitstream::nextCharToParent(uint8_t in)
{
	return this->parent->rdbuf()->sputn((char *)&in, 1);
}

int bitstream::read(int bits, int *out)
	throw (std::ios::failure)
{
	return this->read(boost::bind(&bitstream::nextCharFromParent, this, _1), bits, out);
}

int bitstream::read(fn_getnextchar fnNextChar, int bits, int *out)
	throw (std::ios::failure)
{
	*out = 0;
	int bitsread = 0;
	while (bits > 0) {
		// If the bit buffer is empty, refill it.
		if (this->bufBits == 0) {
			int r = fnNextChar(&this->bufByte);
			if (r <= 0) {
				// EOF or WOULD_BLOCK
				if (bitsread == 0) return r;
				if (this->endianType == bitstream::bigEndian) {
					// Big endian needs to be padded with zero data first
					// to make the output number correct.
					*out <<= bits;
					bitsread += bits;
				}
				return bitsread;
			}
			this->bufBits = 8;
		}

		// Read at most whatever is left in the buffer, which will always be
		// eight bits or less.
		int bitsNow = (bits > this->bufBits) ? this->bufBits : bits;

		// Figure out which bits in the buffered byte we're interested in
		int exval;
		if (this->endianType == bitstream::littleEndian) {
			uint8_t mask = ~(0xFF << bitsNow);
			exval = (bufByte >> curBitPos) & mask;
		} else {
			uint8_t mask = ~(0xFF >> bitsNow);
			exval = (bufByte << curBitPos) & mask;
			exval >>= 8 - bitsNow;
		}

		// Work out where they have to go in the current number
		if (this->endianType == bitstream::littleEndian) {
			// Shift the exval up to the top of the number.
			*out |= exval << bitsread;
		} else {
			// Shift the previously-read value up out of the way and write exval
			// into the (now empty) lower bits.
			*out <<= bitsNow;
			*out |= exval;
		}
		this->bufBits -= bitsNow;
		bits -= bitsNow;
		bitsread += bitsNow;
		this->curBitPos = (this->curBitPos + bitsNow) % 8;
	}
	return bitsread;
}

int bitstream::write(int bits, int in)
	throw (std::ios::failure)
{
	return this->write(boost::bind(&bitstream::nextCharToParent, this, _1), bits, in);
}

int bitstream::write(fn_putnextchar fnNextChar, int bits, int in)
	throw (std::ios::failure)
{
	// Make sure the number being written can actually fit in this many bits.
	assert(in < (1 << bits));

	int bitswritten = 0;
	while (bitswritten < bits) {

		// Read at most whatever space is left in the buffer, which will
		// always be eight bits or less.
		int bitsNow = (bits-bitswritten > (8 - this->bufBits)) ? (8 - this->bufBits) : bits-bitswritten;


		int writeVal;
		if (this->endianType == bitstream::littleEndian) {
			// Extract the next bits we will be writing.
			writeVal = (in & ~(0xff << bitsNow)) & 0xff;
			writeVal <<= this->bufBits;

			in >>= bitsNow;
		} else {
			// Extract the next bits we will be writing.
			int mask = (1 << bits)-1;
			writeVal = in & ~(mask >> bitsNow);

			// Shift the number left as far as we can without cutting it off.
			writeVal >>= bits - bitsNow;
			writeVal <<= 8-bitsNow-this->bufBits;

			in = (in << bitsNow) & mask;
		}

		assert(writeVal < 256); // make sure we're not writing more than 8 bits
		this->bufByte |= writeVal;

		assert(this->bufBits < 8);

		this->bufBits += bitsNow;
		//bits -= bitsNow;
		bitswritten += bitsNow;
		this->curBitPos = (this->curBitPos + bitsNow) % 8;

		// If the bit buffer is full, write out the byte.
		if (this->bufBits == 8) {
			int r = fnNextChar(this->bufByte);
			if (r <= 0) {
				// EOF or WOULD_BLOCK
				if (bitswritten) return bitswritten;
				return r;
			}
			this->bufBits = 0;
			this->bufByte = 0;
		}
	}
	return bitswritten;
}

io::stream_offset bitstream::seek(io::stream_offset off, std::ios_base::seekdir way)
	throw (std::ios::failure)
{
	this->parent->seekg(off / 8, way);
	switch (way) {
		case std::ios::beg:
			//this->parent->seekg(off / 8, std::ios::beg);
			this->curBitPos = off % 8;
			break;
		case std::ios::end:
			//this->parent->seekg(off / 8, std::ios::end);
			this->curBitPos = (8 - (off % 8)) % 8;
			break;
		case std::ios::cur:
			//this->parent->seekg(off / 8, std::ios::cur);
			this->curBitPos += (off % 8);
			break;
	}
	io::stream_offset byte = this->parent->tellg();
	return (byte * 8) + this->curBitPos;
}

} // namespace camoto
