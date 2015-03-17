/**
 * @file   bitstream.cpp
 * @brief  iostream wrapper reading and writing a number of bits rather than
 *         bytes.
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

#include <cassert>
#include <camoto/bitstream.hpp>

/// origBufByte value: The last operation was a write, so the bufByte wasn't updated
#define WASNT_BUFFERED  -1
/// origBufByte value: This is the first read operation, don't write out the bufByte
#define INITIAL_VALUE   -2

namespace camoto {

int bitstreamFilterNextChar(const uint8_t **in, stream::len *lenIn, stream::len *r, uint8_t *out)
{
	if (*r < *lenIn) {
		*out = **in; // "read" byte
		(*in)++;     // increment read buffer
		(*r)++;      // increment read count
		return 1;    // return number of bytes read
	}
	return 0; // EOF
}

int bitstreamFilterPutChar(uint8_t **out, const stream::len *lenOut, stream::len *w, uint8_t in)
{
	if (*w < *lenOut) {
		**out = in;  // "write" byte
		(*out)++;    // increment write buffer
		(*w)++;      // increment write count
		return 1;    // return number of bytes written
	}
	return 0; // EOF
}

bitstream::bitstream(std::shared_ptr<stream::inout> parent,
	bitstream::endian endianType)
	:	parent(std::move(parent)),
		offset(0),
		curBitPos(8), // 8 means update bufByte on next operation
		bufByte(0),
		origBufByte(INITIAL_VALUE),
		endianType(endianType)
{
}

bitstream::bitstream(bitstream::endian endianType)
	:	parent(),
		offset(0),
		curBitPos(8), // 8 means update bufByte on next operation
		bufByte(0),
		origBufByte(INITIAL_VALUE),
		endianType(endianType)
{
}

bitstream::~bitstream()
{
}

int bitstream::read(unsigned int bits, unsigned int *out)
{
	assert(this->parent);
	return this->read(NULL, bits, out);
}

int bitstream::read(fn_getnextchar fnNextChar, unsigned int bits, unsigned int *out)
{
	*out = 0;
	int bitsread = 0;

	// This check is out here so that read(0) will cause it to run.  This is used
	// by flush().
	if (this->origBufByte == WASNT_BUFFERED) {
		// We're half way through a byte, but we crossed into this byte during a
		// write operation so the value was never read in (because if we kept
		// writing we would never have needed it.)  But now we do need it, so we
		// have to read it in and merge it in with whatever has been written so
		// far, so that we don't lose any data previously written.
		stream::pos r;
		uint8_t b;
		if (fnNextChar == NULL) {
			this->parent->seekg(this->offset, stream::start);
			r = this->parent->try_read(&b, 1);
		} else {
			r = fnNextChar(&b);
		}
		if (r == 0) {
			// EOF
			if (bitsread == 0) return r;
			if (this->endianType == bitstream::bigEndian) {
				// Big endian needs to be padded with zero data first
				// to make the output number correct.
				*out <<= bits;
				bitsread += bits;
			}
			return bitsread;
		}
		this->origBufByte = b;
		this->offset++;

		// Combine the bits already written with those just read.
		int mask;
		if (this->endianType == bitstream::littleEndian) {
			mask = 0xff << this->curBitPos;
		} else {
			mask = 0xff >> this->curBitPos;
		}
		this->bufByte = (this->bufByte & ~mask) | (this->origBufByte & mask);
	}

	while (bits > 0) {
		// If the bit buffer is empty, refill it.
		if (this->curBitPos == 8) {
			if (this->parent) this->writeBufByte();

			stream::pos r;
			if (fnNextChar == NULL) {
				this->parent->seekg(this->offset, stream::start);
				r = this->parent->try_read(&this->bufByte, 1);
			} else {
				r = fnNextChar(&this->bufByte);
			}
			if (r == 0) {
				// EOF
				if (bitsread == 0) return r;
				if (this->endianType == bitstream::bigEndian) {
					// Big endian needs to be padded with zero data first
					// to make the output number correct.
					*out <<= bits;
					bitsread += bits;
				}
				return bitsread;
			}
			this->origBufByte = this->bufByte;
			this->offset++;
			this->curBitPos = 0;
		}

		// Read at most whatever is left in the buffer, which will always be
		// eight bits or less.
		unsigned int bufBitsRemaining = 8 - this->curBitPos;
		unsigned int bitsNow = (bits > bufBitsRemaining) ? bufBitsRemaining : bits;

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

		bits -= bitsNow;
		bitsread += bitsNow;
		this->curBitPos += bitsNow;
		assert(this->curBitPos <= 8); // 8 means next byte will be read later
	}
	return bitsread;
}

int bitstream::write(unsigned int bits, unsigned int in)
{
	assert(this->parent);
	return this->write(NULL, bits, in);
}

int bitstream::write(fn_putnextchar fnNextChar, unsigned int bits, unsigned int in)
{
	// Make sure the number being written can actually fit in this many bits.
	assert((bits == 32) || (in < (1u << bits)));

	unsigned int bitswritten = 0;
	while (bitswritten < bits) {

		// If the bit buffer is full, write out the byte.
		if (this->curBitPos == 8) {
			if (fnNextChar == NULL) {
				this->writeBufByte();
			} else {
				if (
					(this->origBufByte != INITIAL_VALUE) && // if not first read
					(this->bufByte != this->origBufByte)    // and bufbyte has been modified
				) {
					int r = fnNextChar(this->bufByte);
					//this->offset++; // unused here (no seeking allowed)
					if (r <= 0) {
						// EOF or WOULD_BLOCK
						if (bitswritten) return bitswritten;
						return r;
					}
				}
			}
			this->origBufByte = WASNT_BUFFERED;
			this->curBitPos = 0;

			// Make sure a subsequent flushByte() call pads out the byte with zeroes
			// instead of the same bits as the current byte.
			this->bufByte = 0;
		}

		// Write at most whatever space is left in the buffer, which will
		// always be eight bits or less.
		unsigned int bufBitsRemaining = 8 - this->curBitPos;
		unsigned int bitsNow = (bits-bitswritten > bufBitsRemaining)
			? bufBitsRemaining : bits-bitswritten;

		int writeVal, writeMask;
		if (this->endianType == bitstream::littleEndian) {
			// Extract the next bits we will be writing.
			writeMask = (~(0xff << bitsNow)) & 0xff;
			writeMask <<= this->curBitPos;
			writeVal = (in << this->curBitPos) & writeMask;

			in >>= bitsNow;
		} else {
			// Extract the next bits we will be writing.

			unsigned int mask = (bits == 32) ? 0xFFFFFFFF : (1 << bits)-1; // could be up to sizeof(int)
			// Isolate the bits in the input data (in) that we are interested in.
			// These might be the upper two bits in a 9-bit number, for instance.
			writeMask = ~(mask >> bitsNow) & mask;
			writeVal = in & writeMask;

			// Shift the number left as far as we can without cutting it off.
			writeVal >>= bits - bitsNow;
			writeMask>>= bits - bitsNow;

			// Now we have the bits we're interested in as a raw value, i.e. the
			// bits have been shifted down so if we'd been looking at the top two
			// bits, now we have a two-bit number.

			// Shift the number up to where it will be written in the current
			// cached byte.
			writeVal <<= 8-bitsNow-this->curBitPos;
			writeMask<<= 8-bitsNow-this->curBitPos;

			in = (in << bitsNow) & mask;
		}

		assert(writeVal < 256); // make sure we're not writing more than 8 bits
		this->bufByte = (this->bufByte & ~writeMask) | writeVal;

		bitswritten += bitsNow;
		this->curBitPos += bitsNow;
		assert(this->curBitPos <= 8); // 8 means next byte will be read later
	}
	return bitswritten;
}

stream::pos bitstream::seek(stream::delta off, stream::seek_from way)
{
	assert(this->parent);
	this->flush();

	int bitOffset = off % 8;
	switch (way) {
		case stream::end:
			this->parent->seekg(off / 8, stream::end);
			this->offset = this->parent->tellg();
			break;
		case stream::cur:
			// Undo the effect of reading the latest cache byte
			if (this->origBufByte >= 0) this->offset--;

			this->offset += off / 8;
			bitOffset += this->curBitPos;// += bitOffset;
			/*if (bitOffset < 0) {
				this->offset--;
				bitOffset += 8;
			} else */if (bitOffset > 7) {
				this->offset++;
				bitOffset -= 8;
			}
			break;
		default: // stream::start
			this->offset = off / 8;
			break;
	}
	if (bitOffset < 0) {
		this->offset--;
		bitOffset += 8;
	}

	// Reset the caches so that the next read operation will start afresh
	this->origBufByte = INITIAL_VALUE;
	this->curBitPos = 8;

	// Read in the last few bits if the seek destination is inside a byte
	unsigned int origOffset = this->offset;
	unsigned int dummy;
	this->read(bitOffset, &dummy);

	return origOffset * 8 + (this->curBitPos % 8);
}

void bitstream::flush()
{
	assert(this->parent);

	if (this->origBufByte == INITIAL_VALUE) return; // nothing to flush

	if (this->curBitPos < 8) {
		// Partial byte.  Read the rest of the byte to trigger a merge.
		unsigned int dummy;
		this->read(0, &dummy);
	}

	// Write out the buf byte (if it has been changed)
	this->writeBufByte();
	this->parent->flush();
	return;
}

void bitstream::changeEndian(bitstream::endian endianType)
{
	this->endianType = endianType;
	return;
}

bitstream::endian bitstream::getEndian()
{
	return this->endianType;
}

void bitstream::flushByte()
{
	this->flushByte(NULL);
	return;
}

void bitstream::flushByte(fn_putnextchar fnNextChar)
{
	// Write out the buf byte (if it has been changed)
	if (this->parent) this->writeBufByte();
	else {
		if (
			(this->origBufByte != INITIAL_VALUE) && // if not first read
			(this->bufByte != this->origBufByte)    // and bufbyte has been modified
		) {
			fnNextChar(this->bufByte);
		}
	}
	this->origBufByte = INITIAL_VALUE;
	this->curBitPos = 8; // reset to the start of the byte boundary

	// Make sure a subsequent flushByte() call pads out the byte with zeroes
	// instead of the same bits as the current byte.
	this->bufByte = 0;
	return;
}

void bitstream::writeBufByte()
{
	assert(this->parent);
	if (
		(this->origBufByte != INITIAL_VALUE) && // if not first read
		(this->bufByte != this->origBufByte)    // and bufbyte has been modified
	) {
		// The currently buffered byte has been modified and needs to be
		// written back to the stream, or the current byte buffer was never
		// read in (origBufByte == -1, which means the byte buffer was filled
		// entirely by writes, so we'll just write them out now.)

		if (this->origBufByte >= 0) {
			// Since we know the original value of this byte (before any writes
			// may have changed it) that means we read it in earlier.  So we
			// need to adjust the seek position now, to make sure it goes into
			// the right spot when we write out the new value.
			this->offset--;
		}
		// else the previous operation should have been a write, so we don't
		// need to perform a seek (since C++ seems to require a seek when switching
		// between read and write operations on the same stream.)

		// Write the updated byte to the parent stream
		this->parent->seekp(this->offset, stream::start);
		this->parent->write(&this->bufByte, 1);
		this->offset++;
		this->origBufByte = this->bufByte; // bufByte now matches on-disk version
	} // else no modification, or the prev byte hadn't been cached
	return;
}

void bitstream::peekByte(uint8_t *buf, uint8_t *mask)
{
	*buf = (this->curBitPos == 8) ? 0x00 : this->bufByte;
	if (this->endianType == bitstream::littleEndian) {
		*mask = ~(0xFF << (this->curBitPos % 8));
	} else {
		*mask = ~(0xFF >> (this->curBitPos % 8));
	}
	return;
}

} // namespace camoto
