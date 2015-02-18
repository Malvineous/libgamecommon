/**
 * @file   stream_seg.cpp
 * @brief  Segmented stream implementation providing the ability to insert and
 *         remove blocks of data at any point in the underlying stream, shifting
 *         data around as necessary.
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

#include <cassert>
#include <cstring>
#include <errno.h>
#include <camoto/stream_seg.hpp>
#include <camoto/util.hpp>

namespace camoto {
namespace stream {

seg::seg()
{
}

seg::seg(std::unique_ptr<inout> parent)
	:	parent(std::move(parent)),
		off_parent(0),
		off_endparent(this->parent->size()),
		offset(0)
{
	assert(this->parent);
	this->parent->seekp(0, stream::start);
}

stream::len seg::try_read(uint8_t *buffer, stream::len len)
{
	// Make sure open() has been called
	assert(this->parent);

	// Read the first stream
	stream::pos lenReadFirst; // How much we read from the first stream
	stream::pos lenRemaining = len; // How much left to read from second/third streams

	// This is the "reported" end of the stream (what we tell anyone using this
	// class.)
	stream::pos lenEntireFirst = this->off_endparent - this->off_parent;

	if (this->offset < lenEntireFirst) {
		// Some of the read will happen in the first stream
		stream::pos lenFirst;
		if (this->offset + len > lenEntireFirst) {
			lenFirst = lenEntireFirst - this->offset;
			lenRemaining -= lenFirst;
		} else {
			lenFirst = lenRemaining;
			lenRemaining = 0;
		}
		this->parent->seekg(this->offset + this->off_parent, stream::start);
		lenReadFirst = this->parent->try_read(buffer, lenFirst);
		this->offset += lenReadFirst;
		if (lenReadFirst < lenFirst) {
			// Didn't read the full amount from the first stream for some reason,
			// this shouldn't happen unless there's a major problem with the
			// underlying stream.
			return lenReadFirst;
		}
		buffer += lenReadFirst;
	} else {
		lenReadFirst = 0;
	}

	// Read the second stream (the vector)
	stream::pos lenReadSecond;
	stream::len offSecondEnd = lenEntireFirst + this->vcSecond.size();
	if ((lenRemaining > 0) && (this->offset < offSecondEnd)) {
		// Some of the read will happen in the second stream
		stream::pos lenSecond;
		if (this->offset + lenRemaining > offSecondEnd) {
			lenSecond = offSecondEnd - this->offset;
		} else {
			lenSecond = lenRemaining;
		}
		stream::pos offSecond = this->offset - lenEntireFirst;
		assert(offSecond < this->vcSecond.size());
		memcpy(buffer, &this->vcSecond[offSecond], lenSecond);
		this->offset += lenSecond;
		buffer += lenSecond;
		lenReadSecond = lenSecond;
		lenRemaining -= lenSecond;
	} else {
		lenReadSecond = 0;
		// lenRemaining remains unchanged as == len
	}

	// Read the third stream (the child segmented_stream_device)
	stream::pos lenReadThird;
	if ((lenRemaining > 0) && (this->psegThird)) {
		// Some of the read will happen in the third stream
		lenReadThird = this->psegThird->try_read(buffer, lenRemaining);
		this->offset += lenReadThird;
	} else {
		lenReadThird = 0;
		// lenRemaining remains unchanged
	}

	// Return the number of bytes read
	return lenReadFirst + lenReadSecond + lenReadThird;
}

void seg::seekg(stream::delta off, seek_from from)
{
	// Calculate stream size
	stream::pos lenFirst = this->off_endparent - this->off_parent;
	stream::pos lenTotal = lenFirst;
	lenTotal += this->vcSecond.size();
	stream::pos offSecondEnd = lenTotal;
	if (this->psegThird) lenTotal += this->psegThird->size();

	stream::pos baseOffset;
	switch (from) {
		case cur:
			baseOffset = this->offset;
			break;
		case end:
			baseOffset = lenTotal;
			break;
		default:
			baseOffset = 0;
			break;
	}
	if ((off < 0) && (baseOffset < (unsigned)(off * -1))) {
		throw seek_error("Cannot seek back past start of segstream");
	}
	baseOffset += off;
	if (baseOffset > lenTotal) {
		throw seek_error(createString("Cannot seek beyond end of segstream (offset "
			<< baseOffset << " > length " << lenTotal << ")"));
	}
	this->offset = baseOffset;

	// The seek pointer can't be updated here, because it's shared by all the
	// descendent psegThird elements.

	// But we can let the third source know where we'll come in when we read
	// straight through later.
	if (this->psegThird) {
		if (this->offset >= offSecondEnd) {
			this->psegThird->seekg(this->offset - offSecondEnd, stream::start);
		} else {
			this->psegThird->seekg(0, stream::start);
		}
	}
	return;
}

stream::pos seg::tellg() const
{
	return this->offset;
}

stream::len seg::size() const
{
	// Make sure open() has been called
	assert(this->parent);

	std::streamsize lenTotal = this->off_endparent - this->off_parent
		+ this->vcSecond.size();
	if (this->psegThird) lenTotal += this->psegThird->size();
	return lenTotal;
}

stream::len seg::try_write(const uint8_t *buffer, stream::len len)
{
	// Make sure open() has been called
	assert(this->parent);

	// Write to the first stream
	stream::pos lenWroteFirst; // How much we wrote to the first source
	stream::pos lenRemaining = len; // How much left to write to the second/third sources
	stream::pos lenEntireFirst = this->off_endparent - this->off_parent;

	if (this->offset < lenEntireFirst) {
		// Some of the write will happen in the first source
		stream::pos lenFirst;
		if (this->offset + len > lenEntireFirst) {
			lenFirst = lenEntireFirst - this->offset;
			lenRemaining -= lenFirst;
		} else {
			lenFirst = lenRemaining;
			lenRemaining = 0;
		}
		this->parent->seekp(this->offset + this->off_parent, stream::start);
		lenWroteFirst = this->parent->try_write(buffer, lenFirst);
		this->offset += lenWroteFirst;
		buffer += lenWroteFirst;
		if (lenWroteFirst < lenFirst) {
			// Didn't write the full amount from the first source for some reason
			return lenWroteFirst;
		}
	} else {
		lenWroteFirst = 0;
	}

	// Write to the second stream (the vector)
	stream::pos lenWroteSecond;
	stream::len offSecondEnd = lenEntireFirst + this->vcSecond.size();
	if ((lenRemaining > 0) && (this->offset < offSecondEnd)) {
		// Some of the write will happen in the second source
		stream::pos lenSecond;
		if (this->offset + lenRemaining > offSecondEnd) {
			// The write will go past the end of the second source
			lenSecond = offSecondEnd - this->offset;
			lenRemaining -= lenSecond;
		} else {
			lenSecond = lenRemaining;
			lenRemaining = 0;
		}
		stream::pos offSecond = this->offset - lenEntireFirst;
		assert(offSecond + lenSecond <= this->vcSecond.size());
		memcpy(&this->vcSecond[offSecond], buffer, lenSecond);
		this->offset += lenSecond;
		buffer += lenSecond;
		lenWroteSecond = lenSecond;
	} else {
		lenWroteSecond = 0;
		// lenRemaining remains unchanged as == len
	}

	// Write to the third source (the child segmented_stream_device)
	stream::len lenWroteThird;
	if ((lenRemaining > 0) && (this->psegThird)) {
		// Some of the write will happen in the third stream
		// No need to seek here, the segstream will do it when it realises the
		// write is in its own first data source.
		lenWroteThird = this->psegThird->try_write(buffer, lenRemaining);
		this->offset += lenWroteThird;
	} else {
		lenWroteThird = 0;
		// lenRemaining remains unchanged
	}

	// Return the number of bytes written
	return lenWroteFirst + lenWroteSecond + lenWroteThird;
}

void seg::seekp(stream::delta off, seek_from from)
{
	this->seekg(off, from);
	return;
}

stream::pos seg::tellp() const
{
	return this->tellg();
}

void seg::truncate(stream::pos size)
{
	try {
		stream::len total = this->size();
		if (size < total) {
			this->seekp(size, stream::start);
			this->remove(total - size);
		} else if (size > total) {
			this->seekp(total, stream::start);
			this->insert(size - total);
		}
		this->flush();
	} catch (const seek_error& e) {
		throw write_error("Unable to seek during truncate: " + e.get_message());
	}
	return;
}

void seg::flush()
{
	// Make sure open() has been called
	assert(this->parent);

	stream::len plenStream = this->parent->size();
	stream::len lenTotal = this->size();
	if (plenStream < lenTotal) {
		// When we're finished the underlying stream will be larger, so make sure
		// it's big enough to hold the extra data.
		this->parent->truncate(lenTotal);

		// Make sure the stream expanded
		// TODO: Should this be replaced by an exception?  Running out of disk
		// space could trigger it.
		plenStream = this->parent->size();
		//this->parent->seekp(0, std::ios::end);
		//stream::pos plenStream = this->parent->tellp();

		// Ensure this isn't a broken stringstream
		assert(plenStream > 0);

		// Ensure the truncate works properly
		assert(plenStream == lenTotal);
	}

	// Call private commit()
	this->commit(0);

	// Safety checks
	assert(this->off_parent == 0);
	assert(this->vcSecond.empty());
	assert(this->psegThird == NULL);

	// Make sure the original calculation of the final size matches what we've
	// ended up with after the 'flattening' operation.
	assert(this->off_endparent == lenTotal);

	// Now that the data has been committed to the underlying stream, we only
	// have a single source (confirmed above), which should hold all our data.
	// This check makes sure the stream isn't too small, because if it is we've
	// lost some data off the end!
	// This can also happen (in the testing code) when something tries to seek
	// past the end of a stringstream and then perform a write.
	assert(plenStream >= this->off_endparent);

	if (plenStream > this->off_endparent) {
		// Cut any excess off the end
		this->parent->truncate(this->off_endparent);
	}

	// Sanity check to make sure the truncate worked
	plenStream = this->parent->size();
	assert(plenStream == this->off_endparent);

	this->parent->flush();
	return;
}

void seg::insert(stream::len lenInsert)
{
	stream::pos lenFirst = this->off_endparent - this->off_parent;
	if (this->offset < lenFirst) {
		// The extra data is to be inserted within the first source
		// TESTED BY: segstream_insert_c01

		this->split();
		// Make our second source lenInsert bytes long so it will become the newly
		// inserted block of data.
		this->vcSecond.resize(lenInsert);
		assert(this->vcSecond.size() == lenInsert);
	} else {
		stream::len offSecondEnd = lenFirst + this->vcSecond.size();
		if (this->offset <= offSecondEnd) {
			// Extra data is to be inserted in the middle of the second source
			// TESTED BY: segstream_insert_c02

			this->vcSecond.insert(this->vcSecond.begin() + (this->offset
				- lenFirst), lenInsert, '\0');
		} else {
			// Extra data is to be inserted in the third source
			// TESTED BY: segstream_insert_c03

			// Rebase the offset so that zero is the start of the child segstream
			assert(this->psegThird);
			this->psegThird->insert(lenInsert);
		}
	}
	return;
}

void seg::remove(stream::len lenRemove)
{
	if (lenRemove == 0) return;

	stream::pos lenFirst = this->off_endparent - this->off_parent;
	if (this->offset < lenFirst) {
		// The data to be removed is contained (or at least starts) in the first
		// source.
		if (this->offset + lenRemove >= lenFirst) {
			// The block to remove goes past the end of the first data source, so
			// we can just trim the first block, leaving the remainder to be handled
			// below.
			// TESTED BY: segstream_remove_c04
			lenRemove -= lenFirst - this->offset;
			lenFirst = this->offset;
			this->off_endparent = this->off_parent + lenFirst;

		} else if (this->offset == 0) {
			// The remove is contained entirely within the first block, starting
			// at the beginning.  So cut data off the start of the block.
			// TESTED BY: segstream_remove_c01
			this->off_parent += lenRemove;
			assert(this->off_parent <= this->off_endparent);
			return;
		} else {
			// The remove is contained entirely within the first block, so we'll have
			// to split the first block and remove the data from the front of the
			// new third block.
			// TESTED BY: segstream_remove_c02
			this->split();
			this->psegThird->off_parent += lenRemove;
			assert(this->psegThird->off_parent < this->psegThird->off_endparent);
			return;
		}
	} // else none of the remove is contained in the first source

	if (lenRemove == 0) return; // No more data to remove

	// This can't be possible, otherwise we haven't removed data from the first
	// source when we should have.
	assert(this->offset >= lenFirst);

	stream::len lenSecond = this->vcSecond.size();
	stream::pos offSecondEnd = lenFirst + lenSecond;
	if (this->offset < offSecondEnd) {
		// There is some data to remove from the second source

		if (this->offset == lenFirst) {
			// The block to remove crosses the start of the second source, so we can
			// just truncate data off the front.
			if (lenRemove >= lenSecond) {
				// The block to remove also reaches or crosses the end of the second
				// source, i.e. the entire second source is to be removed.
				// TESTED BY: segstream_remove_c05
				this->vcSecond.clear();
				lenRemove -= lenSecond;  // in case there's any leftovers
			} else {
				// Just some data off the front is to go
				// TESTED BY: segstream_remove_c06
				this->vcSecond.erase(this->vcSecond.begin(),
					this->vcSecond.begin() + lenRemove);
				lenRemove = 0;
			}
		} else {
			// The remove doesn't start until somewhere in the middle of the second
			// source.
			stream::pos offCropStart = this->offset - lenFirst;
			auto itCropStart = this->vcSecond.begin() + offCropStart;
			auto itCropEnd = [&] {
				if (offCropStart + lenRemove >= lenSecond) {
					// It goes past the end though, so truncate some data off the end of
					// the second source
					// TESTED BY: segstream_remove_c07
					lenRemove -= lenSecond - offCropStart;
					return this->vcSecond.end();
				} else {
					// Removal is contained entirely within the second source
					// TESTED BY: segstream_remove_c08
					auto ret = itCropStart + lenRemove;
					lenRemove = 0;
					return ret;
				}
			}();
			this->vcSecond.erase(itCropStart, itCropEnd);
		}
	}

	// TESTED BY: segstream_remove_from_eof
	if (lenRemove == 0) return; // No more data to remove

	// If we've gotten this far there's still some data to remove from
	// the third source.  We must have a third source, otherwise the caller is
	// trying to remove too much data.
	// TESTED BY: segstream_remove_c03
	assert(this->psegThird);
	this->psegThird->remove(lenRemove);

	return;
}

void seg::split()
{
	assert(this->offset < (this->off_endparent - this->off_parent));

	// Create child segstream
	std::unique_ptr<seg> psegNew(new seg);

	psegNew->offset = 0;
	// Copy parent to segstream's parent
	psegNew->parent = parent;
	// The new one ends where we used to end
	psegNew->off_endparent = this->off_endparent;
	// It starts at the current file pointer
	psegNew->off_parent = this->off_parent + this->offset;
	// And we now end at the current file pointer
	this->off_endparent = psegNew->off_parent;
	// Move our vcSecond to child segstream's vcSecond
	psegNew->vcSecond = this->vcSecond;
	this->vcSecond.clear();
	// Move our psegThird to child segstream's psegThird
	psegNew->psegThird = std::move(this->psegThird); // possibly NULL

	// Make child segstream our psegThird, assuming no exceptions have been thrown
	this->psegThird = std::move(psegNew);
	return;
}

void seg::commit(stream::pos poffWriteFirst)
{
	assert(this->off_parent <= this->off_endparent);

	stream::pos lenFirst = this->off_endparent - this->off_parent;
	stream::len lenSecond = this->vcSecond.size();
	stream::pos poffWriteSecond = poffWriteFirst + lenFirst;
	stream::pos poffWriteThird = poffWriteSecond + lenSecond;

	if (this->off_parent > poffWriteFirst) {
		// There's data off the front that needs to be trimmed, so move the
		// first source back a bit.
		stream::move(*this->parent, this->off_parent, poffWriteFirst, lenFirst);

		this->off_parent = poffWriteFirst;
		this->off_endparent = poffWriteFirst + lenFirst;

		if (this->psegThird) this->psegThird->commit(poffWriteThird);

	} else if (this->off_parent < poffWriteFirst) {
		// Data has been inserted before us, so we need to push the first source
		// further into the file a bit.

		// Before we do that, we need to make sure the third source has been
		// moved out of the way or we'll overwrite it!
		if (this->psegThird) this->psegThird->commit(poffWriteThird);

		// Then move the first source forward a bit
		stream::move(*this->parent, this->off_parent, poffWriteFirst, lenFirst);

		this->off_parent = poffWriteFirst;
		this->off_endparent = poffWriteFirst + lenFirst;

	} else {
		// First source isn't moving, so (possibly) make room for the second
		// source and commit the third source.

		// poffWriteSecond doesn't need to change

		// Write out the third source straight after where the second one will end.
		if (this->psegThird) this->psegThird->commit(poffWriteThird);

	}

	// Write out the second source to the underlying stream
	if (lenSecond) {
		this->parent->seekp(poffWriteSecond, stream::start);
		this->parent->try_write(&this->vcSecond[0], lenSecond);
		this->vcSecond.clear();
		this->off_endparent += lenSecond;
	}

	if (this->psegThird) {
		this->off_endparent += this->psegThird->size();
		this->psegThird.reset();
	}

	return;
}

} // namespace stream
} // namespace camoto
