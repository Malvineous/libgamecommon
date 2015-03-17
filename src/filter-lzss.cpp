/**
 * @file  filter-lzss.cpp
 * @brief Filter for compressing and decompressing data using LZSS.
 *
 * Copyright (C) 2010-2015 Adam Nielsen <malvineous@shikadi.net>
 *
 * LZW algorithm based on GPL code by Juha Nieminen
 *   http://warp.povusers.org/EfficientLZW/
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
#include <functional>
#include <iostream>
#include <camoto/filter-lzss.hpp>

/// How many bytes should be left in reserve
/**
 * This is required to avoid trying to read or write the next codeword, and
 * running out of space in the middle of it, truncating the codeword.  Setting
 * this to 2 means there will always be enough room for a 16-bit codeword.
 * Since all the LZW code so far only goes up to 12-bit codewords this should
 * cover everything.
 */
#define LZSS_LEFTOVER_BYTES 2

namespace camoto {

filter_lzss_decompress::filter_lzss_decompress(bitstream::endian endian, unsigned int sizeLength,
			unsigned int sizeDistance)
	:	data(endian),
		sizeLength(sizeLength),
		sizeDistance(sizeDistance),
		state(State::S0_READ_FLAG),
		maxDistance(1 << sizeDistance),
		window(new uint8_t[maxDistance]),
		posWindow(0),
		lzssLength(0)
{
}

void filter_lzss_decompress::reset(stream::len lenInput)
{
	this->data.flushByte();
	this->state = State::S0_READ_FLAG;
	this->posWindow = 0;
	this->lzssLength = 0;
	return;
}

void filter_lzss_decompress::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len r = 0, w = 0;
	fn_getnextchar cbNext = std::bind(bitstreamFilterNextChar, &in, lenIn, &r,
		std::placeholders::_1);

	// While there's more space to write, and either more data to read or
	// more data to write
	while (              // while there is...
		(w < *lenOut)      // more space to write into, and
		&& (
			(r < *lenIn)     // more data to read, or
			|| (this->lzssLength) // more data to write, and
		)
	) {
		bool needMoreData = false;
		unsigned int bitsRead, code;

		switch (this->state) {

			case State::S0_READ_FLAG:
				bitsRead = this->data.read(cbNext, 1, &code);
				if (bitsRead == 0) {
					needMoreData = true;
					break;
				}
				if (code == 0) {
					this->state = State::S1_COPY_BYTE;
				} else {
					this->state = State::S2_READ_LEN;
				}
				break;

			case State::S1_COPY_BYTE:
				bitsRead = this->data.read(cbNext, 8, &code);
				if (bitsRead != 8) {
					needMoreData = true;
					break;
				}
				*out++ = code;
				w++;
				this->window[this->posWindow++] = code;
				this->posWindow %= this->maxDistance;
				this->state = State::S0_READ_FLAG;
				break;

			case State::S2_READ_LEN:
				bitsRead = this->data.read(cbNext, this->sizeLength, &code);
				if (bitsRead != this->sizeLength) {
					needMoreData = true;
					break;
				}
				this->lzssLength = 2 + code;
				this->state = State::S3_READ_DIST;
				break;

			case State::S3_READ_DIST:
				bitsRead = this->data.read(cbNext, this->sizeDistance, &code);
				if (bitsRead != this->sizeDistance) {
					needMoreData = true;
					break;
				}
				this->lzssDistance = 1 + code;
				this->state = State::S4_COPY_REF;
				break;

			case State::S4_COPY_REF:
				if (this->lzssLength == 0) {
					// Copy complete
					this->state = State::S0_READ_FLAG;
					break;
				}

				*out = this->window[(this->maxDistance + this->posWindow - this->lzssDistance) % this->maxDistance];
				this->window[this->posWindow++] = *out++;
				this->posWindow %= this->maxDistance;
				w++;
				this->lzssLength--;
				break;

		}
		if (needMoreData) break;
	}

	*lenIn = r;
	*lenOut = w;
	return;
}


filter_lzss_compress::filter_lzss_compress(bitstream::endian endian, unsigned int sizeLength,
			unsigned int sizeDistance)
	:	data(endian),
		sizeLength(sizeLength),
		sizeDistance(sizeDistance)
{
}

void filter_lzss_compress::reset(stream::len lenInput)
{
	this->data.flushByte();
	return;
}

void filter_lzss_compress::transform(uint8_t *out, stream::len *lenOut,
	const uint8_t *in, stream::len *lenIn)
{
	stream::len r = 0, w = 0;
	fn_putnextchar cbNext = std::bind(bitstreamFilterPutChar, &out, lenOut, &w,
		std::placeholders::_1);
	while (
		(w + LZSS_LEFTOVER_BYTES < *lenOut) && ( // leave some leftover bytes to guarantee the codeword will be written
			(
				(r + 1 < *lenIn) || // Make sure there's at least one leftover byte
				((r < *lenIn) && (*lenIn < 2)) // unless it's the last incoming byte
			)
		)
	) {
		unsigned int bitsWritten = this->data.write(cbNext, 9, *in);

		in++;
		r++;
		if (bitsWritten < 9) {
			// EOF
			break;
		}
	}

	// Do the flush separately, in case we just ran out of output space
	if ((w < *lenOut) && (*lenIn == 0)) {
		this->data.flushByte(cbNext);
	}
	*lenIn = r;
	*lenOut = w;
	return;
}

} // namespace camoto
