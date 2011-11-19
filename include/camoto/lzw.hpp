/**
 * @file   lzw.hpp
 * @brief  Boost iostream filter for compressing and decompressing data using
 *         a few variants of the LZW algorithm.
 *
 * Copyright (C) 2010-2011 Adam Nielsen <malvineous@shikadi.net>
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

#ifndef _CAMOTO_LZW_HPP_
#define _CAMOTO_LZW_HPP_

#include <vector>
#include <deque>

#include <camoto/bitstream.hpp>
#include <camoto/filter.hpp>

namespace camoto {

#define LZW_LITTLE_ENDIAN     0x00 ///< Read bytes in little-endian order
#define LZW_BIG_ENDIAN        0x01 ///< Read bytes in big-endian order
#define LZW_RESET_FULL_DICT   0x02 ///< Should the dict be wiped when it's full?
#define LZW_NO_BITSIZE_RESET  0x04 ///< Leave the codeword bit length unchanged after dictionary reset
#define LZW_EOF_PARAM_VALID   0x08 ///< Is the EOF parameter (to c'tor) valid?
#define LZW_RESET_PARAM_VALID 0x10 ///< Is the reset parameter (to c'tor) valid?
#define LZW_FLUSH_ON_RESET    0x20 ///< Jump to next word boundary on reset

typedef char byte;
// The string element:
struct CodeString
{
	unsigned prefixIndex;

	// First CodeString using this CodeString as prefix:
	unsigned first;

	// Next CodeStrings using the same prefixIndex as this one:
	unsigned nextLeft, nextRight;

	byte k;

	CodeString(byte newByte = 0, unsigned pI = ~0U);
};

class Dictionary
{
	std::vector<CodeString> table;
	unsigned codeStart, newCodeStringIndex;
	std::vector<byte> decodedString;

	void fillDecodedString(unsigned code)
		throw (filter_error);

public:
	Dictionary(unsigned maxBits, unsigned codeStart);

	void decode(unsigned oldCode, unsigned code,
		std::deque<byte>& outStream);

	unsigned size() const;

	void reset();
};

class filter_lzw_decompress: public filter {

	protected:
		const unsigned int maxBits;  ///< Maximum codeword size (dictionary size limit)
		const unsigned int flags;

		/// The codeword for end-of-data.  Only used if LZW_EOF_PARAM_VALID used.
		/// Values < 1 are from the maximum possible codeword (so -1 means the EOF
		/// code is one less than the max codeword at the current bit depth.)
		int eofCode;
		/// Actual eofCode at the moment, for those codewords which change with
		/// the bit length.
		int curEOFCode;

		/// Same as eofCode but the code to reset the dictionary.  As above, only
		/// valid if LZW_RESET_PARAM_VALID included in c'tor flags.
		int resetCode;
		/// Actual eofCode at the moment, for those codewords which change with
		/// the bit length.
		int curResetCode;

		/// The maximum codeword value at the current bit length
		unsigned int maxCode;

		/// Length of initial codeword, and codeword length after a dictionary
		/// reset (unless LZW_NO_BITSIZE_RESET is given, when the codeword length
		/// is unchanged after a dictionary reset.)
		int initialBits;

		std::deque<char> buffer;
		Dictionary dictionary;
		unsigned int currentBits;     ///< Current codeword size in bits
		//unsigned int nextBitIncLimit; ///< Last codeword value before currentBits is next incremented

		bitstream data;
		bool isDictReset;      ///< Has the dict been reset but not yet inited?
		int code;              ///< Curent codeword
		int oldCode;           ///< Previous codeword

	public:

		/**
		 * @param firstCode The first valid codeword.  Will be 256 for 9-bit
		 *   codewords with no reserved values, or e.g. 258 for 9-bit codewords
		 *   with two reserved values.
		 */
		filter_lzw_decompress(int initialBits, int maxBits, int firstCode,
			int eofCode, int resetCode, int flags);

		virtual void transform(uint8_t *out, stream::len *lenOut, const uint8_t *in,
			stream::len *lenIn)
			throw (filter_error);

		void resetDictionary();

		/// Recalculate the reserved/trigger codewords.
		void recalcCodes();
};

} // namespace camoto

#endif // _CAMOTO_LZW_HPP_
