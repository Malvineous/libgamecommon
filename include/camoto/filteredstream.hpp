/**
 * @file   filteredstream.hpp
 * @brief  C++ iostream providing transparent seekable access to a filtered
 *         stream via an in-memory buffer. LZW compression and decompression
 *         via a memory buffer and LZW filters.
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

#ifndef _CAMOTO_FILTEREDSTREAM_HPP_
#define _CAMOTO_FILTEREDSTREAM_HPP_

#include <sstream>
#include <iosfwd>                           // streamsize, seekdir
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>   // seekable_device_tag
#include <boost/iostreams/positioning.hpp>  // stream_offset
#include <boost/iostreams/filtering_stream.hpp>

#include <camoto/types.hpp>

namespace camoto {

namespace io = boost::iostreams;

typedef boost::shared_ptr<io::filtering_istream> filtering_istream_sptr;
typedef boost::shared_ptr<io::filtering_ostream> filtering_ostream_sptr;

/// boost::iostream class for wrapping iostream filters around another iostream,
/// using memory as a buffer so that the data can be seeked and read/written
/// at random.
/**
 * This class is never used directly, rather the filteredstream class acts as a
 * wrapper around it.
 *
 * @see filteredstream
 */
class filteredstream_device {
	private:
		/// Parent stream, where the actual data is read from and written to.
		iostream_sptr parent;
		filtering_ostream_sptr outFilter;

		boost::shared_ptr<std::stringstream> cache;

		/// Current seek position.
		io::stream_offset iCurPos;

	public:
		typedef char char_type;
		typedef io::seekable_device_tag category;

		/// Create a filteredstream out of the given stream
		/**
		 * @param psParent Parent stream, where the data comes from
		 * @param iOffset Offset into the parent stream where the filteredstream starts
		 * @param iLength Size of filteredstream in bytes
		 */
		filteredstream_device(iostream_sptr parent, filtering_istream_sptr inFilter,
			filtering_ostream_sptr outFilter)
			throw (std::exception);

		filteredstream_device(const filteredstream_device&)
			throw ();

		~filteredstream_device()
			throw ();

		/// boost::iostream callback function
		std::streamsize read(char_type *s, std::streamsize n);

		/// boost::iostream callback function
		std::streamsize write(const char_type *s, std::streamsize n);

		/// boost::iostream callback function
		io::stream_offset seek(io::stream_offset off, std::ios_base::seekdir way);

	friend class filteredstream;

};


/// C++ iostream class for accessing a portion of another stream.
class filteredstream: public io::stream<filteredstream_device>
{
	public:
		/// @copydoc filteredstream_device::filteredstream_device()
		filteredstream(iostream_sptr parent, filtering_istream_sptr inFilter,
			filtering_ostream_sptr outFilter)
			throw (std::exception);

		filteredstream(const filteredstream_device& orig)
			throw ();
};

typedef boost::shared_ptr<filteredstream> filteredstream_sptr;

} // namespace camoto

#endif // _CAMOTO_FILTEREDSTREAM_HPP_
