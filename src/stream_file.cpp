/**
 * @file   stream_file.cpp
 * @brief  Stream implementation for accessing local files.
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

#include <errno.h>
#include <string.h>
#include <camoto/stream_file.hpp>

namespace camoto {
namespace stream {

input_sptr open_stdin()
	throw ()
{
	input_file_sptr f(new input_file());
	f->handle = stdin;
	f->close = false;
	return f;
}

output_sptr open_stdout()
	throw ()
{
	output_file_sptr f(new output_file());
	f->handle = stdout;
	f->close = false;
	return f;
}

file_core::file_core()
	throw () :
		handle(NULL),
		close(false)
{
}

void file_core::seek(stream::delta off, seek_from from)
	throw (seek_error)
{
	int whence;
	switch (from) {
		case cur: whence = SEEK_CUR; break;
		case end: whence = SEEK_END; break;
		default: whence = SEEK_SET; break;
	}
	if (fseek(this->handle, off, whence) < 0) {
		throw seek_error(strerror(errno));
	}
	return;
}

stream::pos file_core::tell() const
	throw (seek_error)
{
	long p = ftell(this->handle);
	if (p < 0) {
		throw seek_error(strerror(errno));
	}
	return p;
}


input_file::input_file()
	throw ()
{
}

input_file::~input_file()
	throw ()
{
	if (this->close) {
		fclose(this->handle);
		this->close = false;
	}
}

stream::len input_file::try_read(uint8_t *buffer, stream::len len)
	throw ()
{
	return fread(buffer, 1, len, this->handle);
}

void input_file::seekg(stream::delta off, seek_from from)
	throw (seek_error)
{
	this->seek(off, from);
	return;
}

stream::pos input_file::tellg() const
	throw (seek_error)
{
	return this->tell();
}

stream::pos input_file::size() const
	throw (seek_error)
{
	long start = ftell(this->handle);

	fseek(this->handle, 0, SEEK_END);
	stream::pos len = ftell(this->handle);

	fseek(this->handle, start, SEEK_SET);
	return len;
}

void input_file::open(const char *filename)
	throw (open_error)
{
	this->handle = fopen(filename, "rb");
	if (this->handle == NULL) throw open_error(strerror(errno));
	this->close = true;
	// no need to seek, fopen("rb") positions file pointer at start
	return;
}


output_file::output_file()
	throw ()
{
}

output_file::~output_file()
	throw ()
{
	if (this->close) {
		fclose(this->handle);
		this->close = false;
	}
}

stream::len output_file::try_write(const uint8_t *buffer, stream::len len)
	throw ()
{
	return fwrite(buffer, 1, len, this->handle);
}

void output_file::seekp(stream::delta off, seek_from from)
	throw (seek_error)
{
	this->seek(off, from);
	return;
}

stream::pos output_file::tellp() const
	throw (seek_error)
{
	return this->tell();
}

void output_file::truncate(stream::pos size)
	throw (seek_error)
{
	this->flush();
	int fd = fileno(this->handle);
#ifdef __WIN32
	if (chsize(fd, size) < 0) {
		throw seek_error(strerror(errno));
	}
#else
	if (ftruncate(fd, size) < 0) {
		throw seek_error(strerror(errno));
	}
#endif
	// Have to seek last as the file might not have been large enough earlier
	this->seekp(size, start);
	return;
}

void output_file::flush()
	throw (write_error)
{
	if (fflush(this->handle) < 0) {
		throw write_error(strerror(errno));
	}
	return;
}

void output_file::open(const char *filename)
	throw (open_error)
{
	// We have to open the file in read/write even though we aren't reading,
	// because none of the other options allow us to seek around and overwrite
	// arbitrary points in the file.
	//
	// We also use this function as file::open() which *must* open in
	// for read+write.
	this->handle = fopen(filename, "r+b");
	if (this->handle == NULL) throw open_error(strerror(errno));
	this->close = true;
	this->seek(0, start);
	return;
}

void output_file::create(const char *filename)
	throw (open_error)
{
	// We have to open the file in read/write even though we aren't reading,
	// because none of the other options allow us to seek around and overwrite
	// arbitrary points in the file.
	//
	// We also use this function as file::create() which *must* open in
	// for read+write.
	this->handle = fopen(filename, "w+b");
	if (this->handle == NULL) throw open_error(strerror(errno));
	this->close = true;
	this->seek(0, start);
	return;
}


file::file()
	throw ()
{
}

} // namespace stream
} // namespace camoto
