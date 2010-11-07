/**
 * @file   lzw.hpp
 * @brief  Boost iostream filter for compressing and decompressing data using
 *         a few variants of the LZW algorithm.
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

#ifndef _CAMOTO_LZW_HPP_
#define _CAMOTO_LZW_HPP_

#include <vector>
#include <deque>
#include <iosfwd>                           // streamsize
#include <boost/iostreams/concepts.hpp>     // multichar_input_filter
#include <boost/iostreams/char_traits.hpp>  // EOF, WOULD_BLOCK
#include <boost/iostreams/operations.hpp>   // get
#include <boost/bind.hpp>

#include <camoto/bitstream.hpp>
#include <camoto/types.hpp>
#include <camoto/exceptions.hpp>

namespace camoto {

namespace io = boost::iostreams;

#define LZW_LITTLE_ENDIAN     0x00 ///< Read bytes in little-endian order
#define LZW_BIG_ENDIAN        0x01 ///< Read bytes in big-endian order
#define LZW_RESET_FULL_DICT   0x02 ///< Should the dict be wiped when it's full?
#define LZW_NO_BITSIZE_RESET  0x04 ///< Leave the codeword bit length unchanged after dictionary reset
#define LZW_EOF_PARAM_VALID   0x08 ///< Is the EOF parameter (to c'tor) valid?
#define LZW_RESET_PARAM_VALID 0x10 ///< Is the reset parameter (to c'tor) valid?

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
		throw (ECorruptedData);

public:
	Dictionary(unsigned maxBits, unsigned codeStart);

	void decode(unsigned oldCode, unsigned code,
		std::deque<byte>& outStream);

	unsigned size() const;

	void reset();
};

template <typename Source>
int nextSourceChar(Source& src, uint8_t *out) {
	int c = boost::iostreams::get(src);
	if (c < 0) return c;
	*out = c;
	return 1;
}

class lzw_decompress_filter: public io::multichar_input_filter {

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
		int maxCode;

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
		lzw_decompress_filter(int initialBits, int maxBits, int firstCode,
			int eofCode, int resetCode, int flags);

		void resetDictionary();
		/// Recalculate the reserved/trigger codewords.
		void recalcCodes();

		void fillBuffer(fn_getnextchar cbNext);

		template<typename Source>
		std::streamsize read(Source& src, char* s, std::streamsize n)
		{
			int r = 0;
			while (n > 0) {
				if (!this->buffer.empty()) {
					*s++ = this->buffer.front();
					this->buffer.pop_front();
					n--;
					r++;
				} else {
					fn_getnextchar cbNext = boost::bind(nextSourceChar<Source>, boost::ref(src), _1);
					this->fillBuffer(cbNext);
					if (this->buffer.empty()) {
						if (r == 0) r = -1; // EOF
						break;
					}
				}
			}
			return r;
		}

};

} // namespace camoto

#endif // _CAMOTO_LZW_HPP_
