/**
 * @file   filteredstream.cpp
 * @brief  Class declaration for a C++ iostream applying filters in-memory to
 *         avoid seeking.
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
		doFlush(false),
		parent(parent),
		outFilter(outFilter),
		cache(new std::stringstream)
{
	assert(this->parent);
	assert(inFilter);
	assert(this->outFilter);

	inFilter->push(*this->parent);
	boost::iostreams::copy(*inFilter, *this->cache);
	this->cache->seekg(0, std::ios::beg);
	this->cache->seekp(0, std::ios::beg);
	assert(this->cache->good());

	this->parent->seekp(0, std::ios::beg);
	this->outFilter->push(*this->parent);
}

filteredstream_device::filteredstream_device(const filteredstream_device& orig)
	throw () :
		doFlush(orig.doFlush),
		parent(orig.parent),
		outFilter(orig.outFilter),
		cache(orig.cache)
{
}

filteredstream_device::~filteredstream_device()
	throw ()
{
}

std::streamsize filteredstream_device::read(char_type *s, std::streamsize n)
{
	assert(this->cache->good());
	return this->cache->rdbuf()->sgetn(s, n);
}

std::streamsize filteredstream_device::write(const char_type *s, std::streamsize n)
{
	this->doFlush = true;
	return this->cache->rdbuf()->sputn(s, n);
}

io::stream_offset filteredstream_device::seek(io::stream_offset off,
	std::ios_base::seekdir way)
{
	this->cache->seekg(off, way);
	this->cache->seekp(off, way);
	return this->cache->tellg();
}

bool filteredstream_device::flush()
{
	if (this->doFlush) {

		this->cache->seekg(0, std::ios::beg);
		assert(this->cache->good());
		this->parent->seekp(0, std::ios::beg);

		// Can't use boost::iostreams::copy here as it causes a segfault for some
		// reason.  Copying the data ourselves works fine though...
		//boost::iostreams::copy(*this->cache, *this->outFilter);
		char buf[1024];
		int len;
		while ((len = this->cache->rdbuf()->sgetn(buf, 1024))) {
			this->outFilter->rdbuf()->sputn(buf, len);
		}
		this->outFilter->flush();
		this->parent->flush();
		this->doFlush = false;
	}
	return true;
}

} // namespace camoto
