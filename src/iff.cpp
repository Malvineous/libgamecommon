/**
 * @file   iff.cpp
 * @brief  RIFF/IFF/RIFX reader/writer.
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

#include <camoto/util.hpp>
#include <camoto/iostream_helpers.hpp>
#include <camoto/iff.hpp>

namespace camoto {

IFFReader::IFFReader(std::shared_ptr<stream::input> iff_ptr, Filetype filetype)
	:	iff_ptr(iff_ptr),
		iff(*iff_ptr),
		filetype(filetype)
{
	this->root();
}

IFFReader::IFFReader(stream::input& iff, Filetype filetype)
	:	iff_ptr(),
		iff(iff),
		filetype(filetype)
{
	this->root();
}

void IFFReader::root()
{
	this->iff.seekg(0, stream::start);
	this->loadChunks(this->iff.size());
	return;
}

std::vector<IFFReader::fourcc> IFFReader::list()
{
	std::vector<fourcc> names;
	for (const auto& i : this->chunks) {
		names.push_back(i.name);
	}
	return names;
}

stream::len IFFReader::seek(const fourcc& name)
{
	for (const auto& i : this->chunks) {
		if (name.compare(i.name) == 0) {
			this->iff.seekg(i.start, stream::start);
			return i.len;
		}
	}
	throw stream::error(createString("IFF: Could not find chunk " << name));
}

stream::len IFFReader::seek(unsigned int index)
{
	if (index >= this->chunks.size()) {
		throw stream::error(createString("IFF: Chunk #" << index
			<< " is out of range (max chunk is #" << this->chunks.size() << ")"));
	}
	Chunk& chunk = this->chunks[index];
	this->iff.seekg(chunk.start, stream::start);
	return chunk.len;
}

stream::len IFFReader::open(const fourcc& name, fourcc *type)
{
	stream::len len = this->seek(name);
	this->iff >> fixedLength(*type, 4);
	this->loadChunks(len - 4);
	return len;
}

stream::len IFFReader::open(unsigned int index, fourcc *type)
{
	stream::len len = this->seek(index);
	this->iff >> fixedLength(*type, 4);
	this->loadChunks(len - 4);
	return len;
}

void IFFReader::loadChunks(stream::len lenChunk)
{
	this->chunks.clear();
	while (lenChunk > 8) {
		lenChunk -= 8; // ID and chunk size fields
		Chunk c;
		c.start = this->iff.tellg() + 8;
		this->iff >> fixedLength(c.name, 4);
		switch (this->filetype) {
			case Filetype_RIFF_Unpadded:
			case Filetype_RIFF:
				this->iff >> u32le(c.len);
				break;
			case Filetype_IFF_Unpadded:
			case Filetype_IFF:
				this->iff >> u32be(c.len);
				break;
		}

		unsigned int pad;
		switch (this->filetype) {
			case Filetype_RIFF:
			case Filetype_IFF:
				pad = (c.len % 2) ? 1 : 0;
				break;
			case Filetype_RIFF_Unpadded:
			case Filetype_IFF_Unpadded:
				pad = 0;
				break;
		}

		stream::len lenPaddedSub = c.len + pad;
		if (lenChunk < c.len) c.len = lenChunk; // truncated
		if (lenChunk < lenPaddedSub) lenPaddedSub = lenChunk; // final pad truncated
		this->chunks.push_back(c);
		lenChunk -= lenPaddedSub;
		this->iff.seekg(lenPaddedSub, stream::cur);
	}
	return;
}


IFFWriter::IFFWriter(std::shared_ptr<stream::output> iff_ptr, Filetype filetype)
	:	iff_ptr(iff_ptr),
		iff(*iff_ptr),
		filetype(filetype)
{
}

IFFWriter::IFFWriter(stream::output& iff, Filetype filetype)
	:	iff_ptr(),
		iff(iff),
		filetype(filetype)
{
}

void IFFWriter::begin(const fourcc& name)
{
	this->chunk.push_back(this->iff.tellp());
	this->iff
		<< nullPadded(name, 4)
		<< nullPadded("", 4)
	;
	return;
}

void IFFWriter::begin(const fourcc& name, const fourcc& type)
{
	this->chunk.push_back(this->iff.tellp());
	this->iff
		<< nullPadded(name, 4)
		<< nullPadded("", 4)
		<< nullPadded(type, 4)
	;
	return;
}

void IFFWriter::end()
{
	stream::pos orig = this->iff.tellp();
	stream::pos start = this->chunk.back();
	this->chunk.pop_back();
	stream::len lenChunk = orig - (start + 8);

	switch (this->filetype) {
		case Filetype_RIFF_Unpadded:
		case Filetype_IFF_Unpadded:
			break;
		case Filetype_RIFF:
		case Filetype_IFF:
			if (orig % 2) {
				// Pad to even byte boundary
				this->iff.write("", 1);
				orig++;
			}
			break;
	}

	this->iff.seekp(start + 4, stream::start);
	switch (this->filetype) {
		case Filetype_RIFF_Unpadded:
		case Filetype_RIFF:
			this->iff << u32le(lenChunk);
			break;
		case Filetype_IFF_Unpadded:
		case Filetype_IFF:
			this->iff << u32be(lenChunk);
			break;
	}
	this->iff.seekp(orig, stream::start);
	return;
}

} // namespace camoto
