// SnaX Game Engine - https://github.com/snaxgameengine/snax
// Licensed under the MIT License <http://opensource.org/licenses/MIT>.
// SPDX-License-Identifier: MIT
// Copyright (c) 2013 - 2022 Frank-Vegar Mortensen <franksvm(at)outlook(dot)com>.
//
// Permission is hereby  granted, free of charge, to any  person obtaining a copy
// of this software and associated  documentation files (the "Software"), to deal
// in the Software  without restriction, including without  limitation the rights
// to  use, copy,  modify, merge,  publish, distribute,  sublicense, and/or  sell
// copies  of  the Software,  and  to  permit persons  to  whom  the Software  is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE  IS PROVIDED "AS  IS", WITHOUT WARRANTY  OF ANY KIND,  EXPRESS OR
// IMPLIED,  INCLUDING BUT  NOT  LIMITED TO  THE  WARRANTIES OF  MERCHANTABILITY,
// FITNESS FOR  A PARTICULAR PURPOSE AND  NONINFRINGEMENT. IN NO EVENT  SHALL THE
// AUTHORS  OR COPYRIGHT  HOLDERS  BE  LIABLE FOR  ANY  CLAIM,  DAMAGES OR  OTHER
// LIABILITY, WHETHER IN AN ACTION OF  CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE  OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pch.h"
#include "DocumentBINSaver.h"
#include "M3DCore/DataBufferStream.h"
#include "Class.h"
#include "M3DCore/SlimRWLock.h"
#include "M3DCore/XTEA.h"
#include "M3DCore/Scrambler.h"
#include "Engine.h" // to get application
#include "Application.h" // to get LIC!

using namespace m3d;



// HEADER:
// AGUID 16 bytes
// Compressed payload size: 8 bytes
// Payload size: 8 bytes
// Payload crc: 4 bytes (crc is done before payload scramble)
// flags: 4 bytes (enc-level/compression flag)
// Scramble seed: 4 bytes (Scramble up to 1024 bytes of payload before encryption)
// Reserved: 4 bytes
#define HEADER_SIZE 48
// [part of payload - encrypted]:
// App Signature: 16 bytes
// License ID: 16 bytes
#define ENCRYPTED_HEADER_SIZE 32



DocumentBINSaver::DocumentBINSaver() : _size(0)
{
}

DocumentBINSaver::DocumentBINSaver(DocumentBINSaver *parent) : DocumentSaver(parent), _size(0)
{
}

DocumentBINSaver::~DocumentBINSaver()
{
}

bool DocumentBINSaver::_write(uint32 pos, const void *data, uint32 size)
{
	if (pos + size > _data.getBufferSize()) {
		_data.realloc(std::max(size_t(pos + size), _data.getBufferSize() * 2), true);
	}
	std::memcpy(_data.getBuffer() + pos, data, size);
	_size = std::max(_size, pos + size);
	return true;
}

bool DocumentBINSaver::_writeHeader(uint32 pos, DocumentTags::Tag tag, uint32 size)
{
	uint8 t(tag);
	return _write(pos, &t, 1) && _write(pos + 1, &size, 4);
}

bool DocumentBINSaver::_writeData(const void *data, uint32 size)
{
	DocumentTags::Tag t = DocumentTags::_content;
	if (IsWriteAttribute())
		t = GetAttributeTag();
	return _writeHeader(_pos(), t, size) && _write(_pos(), data, size);
}

bool DocumentBINSaver::Initialize()
{
	_data.clear();
	_data.realloc(1024*1024); // init to 1MB
	_groupStack.clear();
	_size = 0;

	// Write the document header. Content is written at save!
	Char header[HEADER_SIZE + ENCRYPTED_HEADER_SIZE];
	memset(header, 0, HEADER_SIZE + ENCRYPTED_HEADER_SIZE);
	if (!_write(0, header, HEADER_SIZE + ENCRYPTED_HEADER_SIZE))
		return false;

	if (!PushGroup(DocumentTags::Document))
		return false;

	return true;
}

bool DocumentBINSaver::SaveToFile(Path fileName, DocumentEncryptionLevel encryptionLevel, Guid signature)
{
	DataBuffer db;
	if (!SaveToMemory(db, encryptionLevel, signature))
		return false;
	std::ofstream file(fileName.AsString().c_str(), std::ios::out | std::ios::binary);
	if (!file.is_open())
		return false;
	file.write((const int8*)db.getConstBuffer(), db.getBufferSize());
	file.close();
	return true;
}

bool DocumentBINSaver::SaveToMemory(DataBuffer &databuffer, DocumentEncryptionLevel encryptionLevel, Guid signature)
{
	if (!_finalize())
		return false;

	if (!PopGroup(DocumentTags::Document))
		return false; // Document error!
	if (!_groupStack.empty())
		return false; // Document error!

	if (encryptionLevel != DocumentEncryptionLevel::ENCLEVEL0 && encryptionLevel != DocumentEncryptionLevel::ENCLEVEL1 && encryptionLevel != DocumentEncryptionLevel::ENCLEVEL2)
		return false; // Invalid encryption level.

	if (encryptionLevel != DocumentEncryptionLevel::ENCLEVEL2 && signature != NullGUID)
		return false; // Signature not supported for other than LEVEL2
	
	// (May meed to write some padding bytes here to get total size right?)

	uint64 size = _pos() - HEADER_SIZE; // Size of payload.
	uint64 compressedSize = size; // Size of payload actually written to data buffer.
	uint32 reserved = 0;

	uint32 flags = (uint32)encryptionLevel;

	srand((uint32)time(nullptr));
	uint32 scrambleSeed = (rand() << 16) | rand();

	// Save the license ID with the document to have a (hidden) way of identifying who created the document. Only save this for encrypted documents!
	Guid licenseID = NullGUID;
//	if (encryptionLevel != ENCLEVEL0)
//		licenseID = engine->GetApplication()->GetLID();

	// Header (part of payload) to be encrypted.
	if (!(_write(HEADER_SIZE, &signature, 16), _write(HEADER_SIZE + 16, &licenseID, 16)))
		return false;

	// Calculate checksum of payload.
    boost::crc_32_type crc;
	crc.process_bytes(_data.getConstBuffer() + HEADER_SIZE, (size_t)size);
	uint32 checksum = crc.checksum();

	// ID to quickly test if document is of our type.
	static const Guid DOC_IDENT = { 0xdc595d41, 0xc4cf, 0x4c52, { 0xb1, 0x98, 0x71, 0x9f, 0x6a, 0x1a, 0x5f, 0x46 } };

	// Write header.
	if (!(_write(0, &DOC_IDENT, 16) && _write(16, &compressedSize, 8) && _write(24, &size, 8) && _write(32, &checksum, 4) && _write(36, &flags, 4) && _write(40, &scrambleSeed, 4) && _write(44, &reserved, 4)))
		return false;

	// Compress payload
	if (GetCompressionLevel() != DocumentCompressionLevel::DCL_NONE && size > 512 && size < 0xFFFFFFFF) { // Don't compress very small chunks of data! (Also make sure no data over 4GB. Does not handle it.. yet..)
		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		int32 ret = deflateInit(&strm, GetCompressionLevel() == DocumentCompressionLevel::DCL_LOW ? Z_BEST_SPEED : (GetCompressionLevel() == DocumentCompressionLevel::DCL_HIGH ? Z_BEST_COMPRESSION : Z_DEFAULT_COMPRESSION));
		if (ret == Z_OK) {
			uint32 written = 0;
			databuffer.realloc(HEADER_SIZE + std::max(1024ull, size / 3));
			strm.avail_in = (uint32)size;
			strm.next_in = (Bytef*)_data.getConstBuffer() + HEADER_SIZE; // casting to nonconst!
			while (ret == Z_OK) {
				if (databuffer.getBufferSize() - written - HEADER_SIZE < 256) { // less than 256 bytes available?
					databuffer.realloc(databuffer.getBufferSize() * 2, true); // Double buffer size!
				}
				uint32 available = uint32(databuffer.getBufferSize() - written - HEADER_SIZE);
				strm.avail_out = available;
				strm.next_out = (Bytef*)(databuffer.getBuffer() + written + HEADER_SIZE);
				ret = deflate(&strm, Z_FINISH);
				written += (available - strm.avail_out);
			}
			deflateEnd(&strm);
			if (ret == Z_STREAM_END) {
				flags |= 0x80000000; // Indicate compression in high order bit!
				compressedSize = written;
				if (!(_write(16, &compressedSize, 8) && _write(36, &flags, 4))) { // Write compressed size and flags to header again.
					databuffer.clear(); // clear any data written to out buffer.
					return false;
				}
				std::memcpy(databuffer.getBuffer(), _data.getConstBuffer(), HEADER_SIZE); // Copy header to output buffer.
				databuffer.realloc(compressedSize + HEADER_SIZE, true);
			}
		}
	}

	if ((flags & 0x80000000) == 0) {
		databuffer.setBufferData(_data.getConstBuffer(), HEADER_SIZE + size); // No compression, but from now on we use this buffer!
	}

	// Shuffle the first 1024 bytes after the scrambleSeed.
	Scrambler scr(scrambleSeed * 13);
	scr.Scramble(databuffer.getBuffer() + HEADER_SIZE, std::min(compressedSize, 1024ull));

	if (encryptionLevel != DocumentEncryptionLevel::ENCLEVEL0) {
		static const Guid LEVEL1_KEY = { 0xe3aaf9d5, 0x679a, 0x4aba, { 0x80, 0xd9, 0x74, 0x85, 0xa7, 0x14, 0xfb, 0x15 } }; // Can be used in editor, but not viewed!
		static const Guid LEVEL2_KEY = { 0x9a27a634, 0xd738, 0x4622, { 0x9b, 0xe0, 0xf9, 0x16, 0x84, 0xbb, 0xa5, 0x25 } }; // Can not be used in editor!

		Guid key = encryptionLevel == DocumentEncryptionLevel::ENCLEVEL1 ? LEVEL1_KEY : LEVEL2_KEY;

		// Alter the key based on content!
		uint64 *K1 = (uint64*)&key;
		K1[0] += size;
		K1[1] -= size;
		uint32 *K2 = (uint32*)&key;
		K2[0] += checksum;
		K2[1] -= checksum;
		K2[2] += checksum;
		K2[3] -= checksum;

		XTEA::enc(databuffer.getBuffer() + HEADER_SIZE, compressedSize, (const uint32*)&key); // Encrypt (compressed) payload.
	}

	return true;
}

bool DocumentBINSaver::SaveChips(const ChipPtrByChipIDMap&chips)
{
	bool ok = true;
	ok = ok && PushGroup(DocumentTags::Chips);

	if (!ok)
		return false;
	
	if (IsUsingMultithreading()) { // Parallel execution of chip serialization?
		struct ChipNode
		{
			Chip *chip;
			DataBuffer db;
			uint32 size;
		};

		// Create sequential list of all chips!
		List<ChipNode> chipNodes;
		chipNodes.resize(chips.size());
		uint32 count = 0;
		for (const auto &n : chips) {
			chipNodes[count].chip = n.second;
			chipNodes[count].size = 0;
			count++;
		}

		DocumentBINSaver *saver = this;

		SlimRWLock lock;

		// Save the chips using parallel algorithm. 
		concurrency::parallel_for(0u, count, [&](uint32 i) {
			DocumentBINSaver worker(saver);
			worker._data.realloc(512); // Allocate some default amount of memory for the chip!
			if (worker.SaveChip(chipNodes[i].chip, &chips)) {
				chipNodes[i].db = std::move(worker._data);
				chipNodes[i].size = worker._size;
				lock.AquireWriteLock();
				GetChipTypes().insert(worker.GetChipTypes().begin(), worker.GetChipTypes().end());
				lock.ReleaseWriteLock();
			}
		});

		DataBufferOutputStream dbs(_data, _pos());
		
		// Go through list and add all nodes to the document in sequential order.
		for (uint32 i = 0; i < count && ok; i++) {
			ChipNode &node = chipNodes[i];
			if (node.db.getBufferSize() > 0) {
				dbs.Write(node.db.getConstBuffer(), node.size);
				if (dbs.IsError())
					ok = false; // Data buffer error... Out of memory?
			}
			else 
				ok = false; // SaveChip(...) failed!
		}

		_size = (uint32)dbs.GetBufferPos();

		ok = ok && !dbs.IsError();
	}
	else {
		for (const auto &n : chips) {
			ok = ok && SaveChip(n.second, &chips);
		}
	}

	ok = PopGroup(DocumentTags::Chips) && ok;

	return ok;
}

bool DocumentBINSaver::PushGroup(DocumentTags::Tag group)
{
	_writeHeader(_pos(), group, 0); // size 0 is a placeholder for a 32-bit size written at pop!
	Block g = { group, _pos() };
	_groupStack.push_back(g);
	return true;
}

bool DocumentBINSaver::PopGroup(DocumentTags::Tag group)
{
	if (_groupStack.empty() || _groupStack.back().tag != group)
		return false;
	Block g = _groupStack.back();
	_groupStack.pop_back();
	return _writeHeader(g.pos - 5, g.tag, _pos() - g.pos); // -5 because a group header is always 5 bytes!
}