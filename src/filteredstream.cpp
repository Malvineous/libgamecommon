/**
 * @file   filteredstream.cpp
 * @brief  Class declaration for a C++ iostream exposing a limited section
 *         of another C++ iostream.
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

#include <boost/iostreams/copy.hpp>
#include <camoto/filteredstream.hpp>
#include <camoto/debug.hpp>

namespace camoto {

filteredstream_device::filteredstream_device(iostream_sptr parent,
	filtering_istream_sptr inFilter, filtering_ostream_sptr outFilter)
	throw (std::exception) :
		parent(parent),
		outFilter(outFilter),
		cache(new std::stringstream)
{
	assert(this->parent);
	this->outFilter->push(*this->parent);
	inFilter->push(*this->parent);
	boost::iostreams::copy(*inFilter, *this->cache);
	this->cache->seekg(0, std::ios::beg);
	assert(this->cache->good());
}

filteredstream_device::filteredstream_device(const filteredstream_device& orig)
	throw () :
		parent(orig.parent),
		outFilter(orig.outFilter),
		cache(orig.cache)
{
}

filteredstream_device::~filteredstream_device()
	throw ()
{
	// TODO: This should really be in some sort of a 'flush' function
	/*std::cerr << "Flushing filteredstream" << std::endl;
	this->cache->seekg(0, std::ios::beg);
	this->outFilter->seekp(0, std::ios::beg);
	boost::iostreams::copy(*this->cache, *this->outFilter);*/
}

std::streamsize filteredstream_device::read(char_type *s, std::streamsize n)
{
	assert(this->cache->good());
	return this->cache->rdbuf()->sgetn(s, n);
}

std::streamsize filteredstream_device::write(const char_type *s, std::streamsize n)
{
	return this->cache->rdbuf()->sputn(s, n);
}

io::stream_offset filteredstream_device::seek(io::stream_offset off,
	std::ios_base::seekdir way)
{
	this->cache->seekg(off, way);
	this->cache->seekp(off, way);
	return this->cache->tellg();
}


filteredstream::filteredstream(iostream_sptr parent,
	filtering_istream_sptr inFilter, filtering_ostream_sptr outFilter)
	throw (std::exception) :
		io::stream<filteredstream_device>(parent, inFilter, outFilter)
{
}

filteredstream::filteredstream(const filteredstream_device& orig)
	throw () :
		io::stream<filteredstream_device>(orig)
{
}

} // namespace camoto
