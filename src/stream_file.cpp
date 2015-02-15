/**
 * @file   stream_file.cpp
 * @brief  Stream implementation for accessing local files.
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

#include <errno.h>
#include <string.h>
#ifndef WIN32
#include <unistd.h>
#else
#include <io.h>
#endif
#include <camoto/stream_file.hpp>
#include <camoto/util.hpp> // createString

inline std::string strerror_str(int errno2)
{
	char buf[256];
	buf[0] = 0;
	char *pbuf = buf;
	errno = 0;
#ifdef WIN32
	strerror_s(buf, sizeof(buf), errno2);
#else
	pbuf = strerror_r(errno2, buf, sizeof(buf));
#endif
	if (errno != 0) {
		return createString("[unable to get message for error code " << errno2
			<< "]");
	}
	if (pbuf[0] == 0) {
		return createString("[empty string returned with no failure code when "
			"getting message for error code " << errno2 << "]");
	}
	return std::string(pbuf);
}

#ifdef WIN32
#define unlink(x) _unlink(x)
#define fileno _fileno
#endif

namespace camoto {
namespace stream {

std::unique_ptr<input> open_stdin()
{
	std::unique_ptr<input_file> f(new input_file());
	f->handle = stdin;
	f->close = false;
	return std::move(f);
}

std::unique_ptr<output> open_stdout()
{
	std::unique_ptr<output_file> f(new output_file());
	f->handle = stdout;
	f->close = false;
	return std::move(f);
}

file_core::file_core()
	:	handle(NULL),
		close(false)
{
}

void file_core::seek(stream::delta off, seek_from from)
{
	int whence;
	switch (from) {
		case cur: whence = SEEK_CUR; break;
		case end: whence = SEEK_END; break;
		default: whence = SEEK_SET; break;
	}
	if (fseek(this->handle, off, whence) < 0) {
		throw seek_error(strerror_str(errno));
	}
	return;
}

stream::pos file_core::tell() const
{
	long p = ftell(this->handle);
	if (p < 0) {
		throw seek_error(strerror_str(errno));
	}
	return p;
}


input_file::input_file()
{
}

input_file::~input_file()
{
	if (this->close) {
		fclose(this->handle);
		this->close = false; // prevent double-close in ~output_file()
	}
}

stream::len input_file::try_read(uint8_t *buffer, stream::len len)
{
	return fread(buffer, 1, len, this->handle);
}

void input_file::seekg(stream::delta off, seek_from from)
{
	this->seek(off, from);
	return;
}

stream::pos input_file::tellg() const
{
	return this->tell();
}

stream::pos input_file::size() const
{
	long start = ftell(this->handle);

	fseek(this->handle, 0, SEEK_END);
	stream::pos len = ftell(this->handle);

	fseek(this->handle, start, SEEK_SET);
	return len;
}

void input_file::open(const char *filename)
{
	this->handle = fopen(filename, "rb");
	if (this->handle == NULL) throw open_error(strerror_str(errno));
	this->close = true;
	// no need to seek, fopen("rb") positions file pointer at start
	return;
}

void input_file::open(const std::string& filename)
{
	this->open(filename.c_str());
	return;
}


output_file::output_file()
	:	do_remove(false)
{
}

output_file::~output_file()
{
	if (this->close) {
		fclose(this->handle);
		this->close = false; // prevent double-close in ~input_file()

		// Can only delete the file if it's a real file
		if (this->do_remove) {
			unlink(this->filename.c_str());
		}
	}
}

stream::len output_file::try_write(const uint8_t *buffer, stream::len len)
{
	return fwrite(buffer, 1, len, this->handle);
}

void output_file::seekp(stream::delta off, seek_from from)
{
	this->seek(off, from);
	return;
}

stream::pos output_file::tellp() const
{
	return this->tell();
}

void output_file::truncate(stream::pos size)
{
	this->flush();
	int fd = fileno(this->handle);
#ifndef WIN32
	if (ftruncate(fd, size) < 0) {
#else
	if (_chsize(fd, size) < 0) {
#endif
		throw write_error(strerror_str(errno));
	}

	// Have to seek last as the file might not have been large enough earlier
	try {
		this->seekp(size, stream::start);
	} catch (const seek_error& e) {
		throw write_error("Unable to seek to EOF after truncate: " + e.get_message());
	}
	return;
}

void output_file::flush()
{
	if (fflush(this->handle) < 0) {
		throw write_error(strerror_str(errno));
	}
	return;
}

void output_file::open(const char *filename)
{
	this->filename = std::string(filename);
	this->open();
	return;
}

void output_file::open(const std::string& filename)
{
	this->filename = filename;
	this->open();
	return;
}

void output_file::open()
{
	// We have to open the file in read/write even though we aren't reading,
	// because none of the other options allow us to seek around and overwrite
	// arbitrary points in the file.
	//
	// We also use this function as file::open() which *must* open in
	// for read+write.
	this->handle = fopen(this->filename.c_str(), "r+b");
	if (this->handle == NULL) throw open_error(strerror_str(errno));
	this->close = true;
	this->seek(0, start);
	return;
}

void output_file::create(const char *filename)
{
	this->filename = std::string(filename);
	this->create();
	return;
}

void output_file::create(const std::string& filename)
{
	this->filename = filename;
	this->create();
	return;
}

void output_file::create()
{
	// We have to open the file in read/write even though we aren't reading,
	// because none of the other options allow us to seek around and overwrite
	// arbitrary points in the file.
	//
	// We also use this function as file::create() which *must* open in
	// for read+write.
	this->handle = fopen(this->filename.c_str(), "w+b");
	if (this->handle == NULL) throw open_error(strerror_str(errno));
	this->close = true;
	this->seek(0, start);
	return;
}

void output_file::remove()
{
	this->do_remove = true;
	return;
}


file::file()
	: isReadonly(false)
{
}

void file::open_readonly(const char *filename)
{
	this->filename = std::string(filename);
	this->open_readonly();
	return;
}

void file::open_readonly(const std::string& filename)
{
	this->filename = filename;
	this->open_readonly();
	return;
}

bool file::readonly()
{
	return this->isReadonly;
}

void file::open_readonly()
{
	this->handle = fopen(this->filename.c_str(), "rb");
	if (this->handle == NULL) throw open_error(strerror_str(errno));
	this->close = true;
	// no need to seek, fopen("rb") positions file pointer at start
	this->isReadonly = true;
	return;
}

} // namespace stream
} // namespace camoto
