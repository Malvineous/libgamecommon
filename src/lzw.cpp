/**
 * @file   lzw.cpp
 * @brief  Boost iostream filter for compressing and decompressing data using
 *         a few variants of the LZW algorithm.
 *
 * Copyright (C) 2010 Adam Nielsen <malvineous@shikadi.net>
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

#include <camoto/lzw.hpp>

namespace camoto {

ECorruptedData::ECorruptedData(const std::string& msg)
	throw () :
		msg(msg)
{
}

ECorruptedData::~ECorruptedData()
	throw ()
{
}

const char *ECorruptedData::what() const
	throw ()
{
	return this->msg.c_str();
}

CodeString::CodeString(byte newByte, unsigned pI):
	prefixIndex(pI), first(~0U),
	nextLeft(~0U), nextRight(~0U),
	k(newByte)
{
}

void Dictionary::fillDecodedString(unsigned code)
	throw (ECorruptedData)
{
	decodedString.clear();
	while (code != ~0U) {
		if (code >= table.size()) throw ECorruptedData("LZW data is corrupted - "
			"codeword was larger than the number of entries in the dictionary!");
		const CodeString& cs = table[code];
		decodedString.push_back(cs.k);

		// Make sure this codeword's prefix is not itself, otherwise we'll get
		// stuck in an infinite loop!
		if (cs.prefixIndex == code) throw ECorruptedData("LZW data is corrupted - "
			"codeword's prefix is itself, cannot continue as this would cause an "
			"infinite loop!");
		code = cs.prefixIndex;
	}
}

Dictionary::Dictionary(unsigned maxBits, unsigned codeStart) :
	table(1<<maxBits),
	codeStart(codeStart), newCodeStringIndex(codeStart)
{
	for(unsigned i = 0; i < codeStart; ++i)
		table[i].k = i;
}

void Dictionary::decode(unsigned oldCode, unsigned code,
	std::deque<byte>& outStream)
{
	const bool exists = code < newCodeStringIndex;

	if (exists) fillDecodedString(code);
	else fillDecodedString(oldCode);

	for (size_t i = decodedString.size(); i > 0;) {
		outStream.push_back(decodedString[--i]);
	}
	if (!exists) outStream.push_back(decodedString.back());

	if (newCodeStringIndex < table.size()) {
		if (newCodeStringIndex == oldCode) {
			std::cerr << "LZW Warning: Tried to set prefix for codeword 0x"
				<< std::hex << newCodeStringIndex << " to itself!  This "
				"will cause an error if this codeword is ever used."
				<< std::endl;
		}
		table[newCodeStringIndex].prefixIndex = oldCode;
		table[newCodeStringIndex++].k = decodedString.back();
	} // else dictionary is full, don't add anything to it
}

unsigned Dictionary::size() const
{
	return newCodeStringIndex;
}

void Dictionary::reset()
{
	newCodeStringIndex = codeStart;
	for(unsigned i = 0; i < codeStart; ++i)
		table[i] = CodeString(i);
}


lzw_decompress_filter::lzw_decompress_filter(int initialBits, int maxBits,
	int firstCode, int eofCode, int resetCode, int flags) :
	maxBits(maxBits),
	flags(flags),
	initialBits(initialBits),
	eofCode(eofCode),
	resetCode(resetCode),
	dictionary(maxBits, firstCode),
	data(((flags & LZW_BIG_ENDIAN) != LZW_BIG_ENDIAN) ? bitstream::littleEndian : bitstream::bigEndian)
{
	if (this->flags & LZW_NO_BITSIZE_RESET) {
		// If LZW_NO_BITSIZE_RESET is *not* set these things will be done in
		// resetDictionary() instead.
		this->currentBits = initialBits;
		this->recalcCodes();
	}
	this->resetDictionary();
}

void lzw_decompress_filter::resetDictionary()
{
	this->dictionary.reset();
	if (!(this->flags & LZW_NO_BITSIZE_RESET)) {
		// Only reset the bit length with the dictionary if wanted
		this->currentBits = this->initialBits;
		this->recalcCodes();
	}
	this->isDictReset = true;
	return;
}

void lzw_decompress_filter::recalcCodes()
{
	// Work out the maximum codeword value that will be possible at the current
	// bit length.
	int actualMaxCode = (1 << this->currentBits) - 1;
	this->maxCode = actualMaxCode;

	// Take any reserved codewords into account so we increase the bit depth
	// at the correct time.
	if (this->flags & LZW_EOF_PARAM_VALID) {
		if (this->eofCode < 1) {
			// If the eofCode is zero or negative, take it as an index back from
			// the last codeword, with 0 being the largest possible codeword.
			this->curEOFCode = actualMaxCode + this->eofCode;
			this->maxCode--;
		} else this->curEOFCode = this->eofCode;
	}

	if (this->flags & LZW_RESET_PARAM_VALID) {
		if (this->resetCode < 1) {
			// If the resetCode is zero or negative, take it as an index back from
			// the last codeword, with 0 being the largest possible codeword.
			this->curResetCode = actualMaxCode + this->resetCode;
			this->maxCode--;
		} else this->curResetCode = this->resetCode;
	}
	return;
}

void lzw_decompress_filter::fillBuffer(fn_getnextchar cbNext)
{
	if ((this->flags & LZW_EOF_PARAM_VALID) && (this->code == this->curEOFCode)) return;

getNextCodeword:
	int bitsRead = this->data.read(cbNext, this->currentBits, &this->code);
	if (bitsRead < 0) return;//bitsRead; // EOF or wouldblock
	if (bitsRead < this->currentBits) {
		// TODO: save bits and append next time?  How to test this?
		// EOF
		return;
	}
	assert(bitsRead > 0);

	if ((this->flags & LZW_EOF_PARAM_VALID) && (this->code == this->curEOFCode)) return;

	if (this->isDictReset) {
		// When the dictionary is empty, the next "codeword" is always the first
		// byte for the first dictionary entry, which also means it's the first
		// output byte too (once it's truncated to eight bits.)
		this->buffer.push_back(this->code);
		this->oldCode = this->code;
		this->isDictReset = false;
		goto getNextCodeword;
	}

	// See if the code we just got is a special one that will reset the dictionary
	if ((this->flags & LZW_RESET_PARAM_VALID) && (this->code == this->curResetCode)) {
		this->resetDictionary();
		goto getNextCodeword;
	}

	this->dictionary.decode(this->oldCode, this->code, this->buffer);

	if (this->dictionary.size() > this->maxCode) {
		if (this->currentBits == this->maxBits) {
			if (this->flags & LZW_RESET_FULL_DICT) this->resetDictionary();
		} else {
			++this->currentBits;
			this->recalcCodes();
		}
	}

	this->oldCode = this->code;
	return;
}

} // namespace camoto
