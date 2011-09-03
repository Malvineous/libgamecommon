/**
 * @file   filteredstream.hpp
 * @brief  C++ iostream providing transparent seekable access to a filtered
 *         stream via an in-memory buffer.
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

#ifndef _CAMOTO_FILTEREDSTREAM_HPP_
#define _CAMOTO_FILTEREDSTREAM_HPP_

#include <vector>
#include <sstream>
#include <iosfwd>                           // streamsize, seekdir
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/categories.hpp>   // seekable_device_tag
#include <boost/iostreams/positioning.hpp>  // stream_offset
#include <boost/iostreams/filtering_stream.hpp>

#include <camoto/types.hpp>
#include <camoto/exceptions.hpp>

namespace camoto {

namespace io = boost::iostreams;

/// filtering_stream that can have shared stream pointers pushed onto it
template <typename T, typename X>
class filtered_Xstream: virtual public X {
	protected:
		typedef boost::shared_ptr<T> Xstream_sptr; ///< Shared pointer to stream type
		std::vector<Xstream_sptr> streams;         ///< List of active streams

	public:

		/// Flush on destruct
		~filtered_Xstream()
		{
			// Flush and close stream before destruction.  Without this
			// boost::iostreams will flush *after* destruction causing a segfault!
			this->reset();
		}

		/// Add a shared pointer to the filter chain.
		/**
		 * @param stream
		 *   Stream to add
		 */
		void pushShared(Xstream_sptr stream)
		{
			this->streams.push_back(stream); // mark as in-use
			return this->push(*stream);
		}
};

typedef filtered_Xstream<std::iostream, io::filtering_stream<io::seekable> > filtered_iostream;
typedef filtered_Xstream<std::istream, io::filtering_istream > filtered_istream;
typedef filtered_Xstream<std::ostream, io::filtering_ostream > filtered_ostream;

typedef boost::shared_ptr<filtered_istream> filtered_istream_sptr;
typedef boost::shared_ptr<filtered_ostream> filtered_ostream_sptr;
typedef boost::shared_ptr<filtered_iostream> filtered_iostream_sptr;

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

		/// Output filter to pass in-memory data through on flush().
		filtered_ostream_sptr outFilter;

		/// In-memory copy of data (after it has passed through inFilter)
		boost::shared_ptr<std::stringstream> cache;

		/// Should flush() do anything?  Set to true if stream is written to.
		bool needFlush;

	public:
		typedef char char_type;
		struct category: io::seekable_device_tag, io::flushable_tag { };

		/// Create a filteredstream out of the given stream
		/**
		 * @param parent
		 *   Parent stream, where the data comes from
		 *
		 * @param iOffset
		 *   Offset into the parent stream where the filteredstream starts
		 *
		 * @param iLength Size of filteredstream in bytes
		 */
		filteredstream_device(iostream_sptr parent, filtered_istream_sptr inFilter,
			filtered_ostream_sptr outFilter)
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

		/// Write out the cached data to the underlying stream, passing it through
		/// the output filter.
		bool flush();

		/// Adjust (truncate) the stream size.
		/**
		 * @param newSize
		 *   New length of the in-memory cache.
		 */
		void setSize(io::stream_offset newSize)
			throw (std::ios::failure);

};

/// C++ iostream class for applying filters to a stream without seeking.
/**
 * This is the actual Boost iostream class.
 *
 * All these functions (including c'tors) call their equivalents in the
 * filteredstream_device class.  See the comments in the class above.
 */
class filteredstream: public io::stream<filteredstream_device>
{
	public:
		filteredstream(iostream_sptr parent,
			filtered_istream_sptr inFilter, filtered_ostream_sptr outFilter)
			throw (std::exception);

		filteredstream(const filteredstream_device& orig)
			throw (std::exception);

		/// See filteredstream_device::setSize()
		void setSize(io::stream_offset newSize);
};

/// Shared pointer to filteredstream
typedef boost::shared_ptr<filteredstream> filteredstream_sptr;

} // namespace camoto

#endif // _CAMOTO_FILTEREDSTREAM_HPP_
