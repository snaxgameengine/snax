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
#include "DocumentBINLoader.h"
#include "M3DCore/XTEA.h"
#include "Engine.h" // for messages only!
#include "M3DCore/Util.h"
#include "M3DCore/Scrambler.h"
#include "magic_enum.hpp"

using namespace magic_enum::bitwise_operators; // enum bitwise operators!

using namespace m3d;


#define HEADER_SIZE 48
#define ENCRYPTED_HEADER_SIZE 32


typedef Guid (*UnlockLevel2Key)();

UnlockLevel2Key __unlockLevel2Key;

// TODO: Must think about multithreading here!
// Note: We should not be able to load a level2 doc in the same application as a level0/1 doc. In addition the level2 signature must match.
// This is to prevent 3rd party malicious documents to get access to level2 documents.
Guid __level2Signature = NullGUID;
int32 __encLevel2Mode = 0; // 0: No pref, 1: Not allowed, 2: Allowed with right signatur


void DocumentLoader::SetKey(void *key) { if (__unlockLevel2Key == nullptr) __unlockLevel2Key = reinterpret_cast<UnlockLevel2Key>(key); }


DocumentBINLoader::DocumentBINLoader()
{
	_lastGroup = -1;
}

DocumentBINLoader::~DocumentBINLoader()
{
}

void DocumentBINLoader::_clear()
{
	_data.clear();
	_groupStack.clear();
	_lastGroup = -1;
}

bool DocumentBINLoader::OpenFile(Path fileName)
{
	DataBuffer db;
	if (!LoadDataBuffer(fileName, db)) {
		msg(FATAL, MTEXT("Could not open document \'") + fileName.AsString() + MTEXT("\'."));
		return false;
	}

	if (!OpenMemory(std::move(db))) {
		msg(FATAL, MTEXT("Failed to read document \'") + fileName.AsString() + MTEXT("\'."));
		return false;
	}

	return true;
}

bool DocumentBINLoader::OpenMemory(DataBuffer &&data)
{
	_clear();

	_data = std::move(data);

	uint64 size = 0, compressedSize = 0;
	uint32 reserved = 0;
	uint32 stored_checksum = 0;
	uint32 flags = 0;
	uint32 scrambleSeed = 0;
	Guid signature = NullGUID;
	Guid licenseID = NullGUID;

	// ID to quickly test if document is of our type.
	static const Guid DOC_IDENT = { 0xdc595d41, 0xc4cf, 0x4c52, { 0xb1, 0x98, 0x71, 0x9f, 0x6a, 0x1a, 0x5f, 0x46 } };

	Guid ident;
	if (_data.getBufferSize() < HEADER_SIZE || !_read(0, &ident, 16) || ident != DOC_IDENT) {
		_clear();
		msg(FATAL, MTEXT("Document being loaded does not appear to be valid."));
		return false;
	}

	if (!(_read(16, &compressedSize, 8) && _read(24, &size, 8) && _read(32, &stored_checksum, 4) && _read(36, &flags, 4) && _read(40, &scrambleSeed, 4) && _read(44, &reserved, 4))) {
		_clear();
		msg(FATAL, MTEXT("Failed reading document header."));
		return false;
	}

	if (HEADER_SIZE + compressedSize != _data.getBufferSize()) {
		_clear();
		msg(FATAL, MTEXT("Document size is invalid."));
		return false;
	}

	bool isCompressed = (flags & 0x80000000) != 0;
	DocumentEncryptionLevel encLevel = (DocumentEncryptionLevel)(flags & (uint32)(DocumentEncryptionLevel::ENCLEVEL0 | DocumentEncryptionLevel::ENCLEVEL1 | DocumentEncryptionLevel::ENCLEVEL2));

	if (encLevel != DocumentEncryptionLevel::ENCLEVEL0 && encLevel != DocumentEncryptionLevel::ENCLEVEL1 && encLevel != DocumentEncryptionLevel::ENCLEVEL2) {
		_clear();
		msg(FATAL, MTEXT("Invalid document header."));
		return false; // Invalid encryption level.
	}

	if (encLevel == DocumentEncryptionLevel::ENCLEVEL2) {
		static const Guid UNLOCK_LEVEL2_KEY = { 0x066d4ff4, 0xdab0, 0x4c47, { 0xa9, 0x4a, 0xfa, 0x59, 0xee, 0x11, 0x5d, 0xec } };

		if (__encLevel2Mode == 1 || !__unlockLevel2Key || (*__unlockLevel2Key)() != UNLOCK_LEVEL2_KEY) {
			_clear();
			msg(FATAL, MTEXT("This document is encrypted and can't be opened."));
			return false;
		}
	}

	// Decrypt!
	if (encLevel != DocumentEncryptionLevel::ENCLEVEL0) {
		static const Guid LEVEL1_KEY = { 0xe3aaf9d5, 0x679a, 0x4aba, { 0x80, 0xd9, 0x74, 0x85, 0xa7, 0x14, 0xfb, 0x15 } }; // Can be used in editor, but not viewed!
		static const Guid LEVEL2_KEY = { 0x9a27a634, 0xd738, 0x4622, { 0x9b, 0xe0, 0xf9, 0x16, 0x84, 0xbb, 0xa5, 0x25 } }; // Can not be used in editor!

		Guid key = encLevel == DocumentEncryptionLevel::ENCLEVEL1 ? LEVEL1_KEY : LEVEL2_KEY;

		uint64 *K1 = (uint64*)&key;
		K1[0] += size;
		K1[1] -= size;
		uint32 *K2 = (uint32*)&key;
		K2[0] += stored_checksum;
		K2[1] -= stored_checksum;
		K2[2] += stored_checksum;
		K2[3] -= stored_checksum;

		XTEA::dec(_data.getBuffer() + HEADER_SIZE, compressedSize, (const uint32*)&key);
	}

	// Deshuffle!
	Scrambler scr(scrambleSeed * 13);
	scr.Descramble(_data.getBuffer() + HEADER_SIZE, std::min(compressedSize, 1024ull));

	// Decompress!
	if (isCompressed) {
		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		int32 ret = inflateInit(&strm);
		if (ret != Z_OK) {
			_clear();
			msg(FATAL, MTEXT("Failed to decompress document."));
			return false;
		}
		DataBuffer db(HEADER_SIZE + size);
		strm.avail_in = (uint32)compressedSize;
		strm.next_in = (Bytef*)_data.getConstBuffer() + HEADER_SIZE;
		strm.next_out = (Bytef*)db.getBuffer() + HEADER_SIZE;
		strm.avail_out = (uint32)size;
		while (ret == Z_OK && strm.avail_out > 0) {
			ret = inflate(&strm, Z_FINISH);
		}
		inflateEnd(&strm);
		if (ret != Z_STREAM_END || strm.avail_out != 0) {
			_clear();
			msg(FATAL, MTEXT("Failed to decompress."));
			return false;
		}
		std::memcpy(db.getBuffer(), _data.getConstBuffer(), HEADER_SIZE); // Copy header.
		_data = std::move(db);
	}

    boost::crc_32_type crc;
	crc.process_bytes(_data.getConstBuffer() + HEADER_SIZE, (size_t)size);
	uint32 checksum = crc.checksum();
	if (checksum != stored_checksum) {
		_clear();
		msg(FATAL, MTEXT("Document checksum mismatch."));
		return false;
	}
	
	if (!(_read(HEADER_SIZE, &signature, 16) && _read(HEADER_SIZE + 16, &licenseID, 16))) {
		_clear();
		msg(FATAL, MTEXT("Failed reading encrypted document header."));
		return false;
	}

	if (encLevel == DocumentEncryptionLevel::ENCLEVEL2) {
		if (__encLevel2Mode == 2 && __level2Signature != signature) {
			_clear();
			msg(FATAL, MTEXT("Invalid document signature."));
			return false;
		}
		__encLevel2Mode = 2;
		__level2Signature = signature;
	}
	else if (signature != NullGUID) {
		_clear();
		msg(FATAL, MTEXT("Invalid document header."));
		return false;
	}

	uint32 pos = HEADER_SIZE + ENCRYPTED_HEADER_SIZE;
	DocumentTags::Tag tag;
	uint32 s = 0;

	if (!_readHeader(pos, tag, s)) {
		_clear();
		msg(FATAL, MTEXT("Document parsing error."));
		return false;
	}

//	Block g = {tag, -1, pos, s};
	_rootBlock = Block(tag, -1, pos, s);
	if (!_exploreGroup(_rootBlock)) {
		_clear();
		msg(FATAL, MTEXT("Document parsing error."));
		return false;
	}

	_groupStack.push_back(&_rootBlock);

	_setEncryptionLevel(DocumentEncryptionLevel(encLevel));

	return _init();
}

bool DocumentBINLoader::Reset()
{
	if (!DocumentLoader::Reset())
		return false;
	_lastGroup = -1;
	while (_groupStack.size() > 1)
		_groupStack.pop_back();
	return true;
}

bool DocumentBINLoader::_read(uint32 pos, void *data, uint32 size) const
{
	if (pos + size > _data.getBufferSize())
		return false;
	std::memcpy(data, _data.getConstBuffer() + pos, size);
	return true;
}

bool DocumentBINLoader::_read(uint32 pos, const void **data, uint32 size) const
{
	if (pos + size > _data.getBufferSize())
		return false;
	*data = _data.getConstBuffer() + pos;
	return true;
}

bool DocumentBINLoader::_readHeader(uint32 &pos, DocumentTags::Tag &tag, uint32 &size) const
{
	uint8 t;
	if (!_read(pos, &t, 1) || !_read(pos + 1, &size, 4)) 
		return false;
	tag = DocumentTags::Tag(t);
	pos += 5;
	return true;
}

bool DocumentBINLoader::_readData(const void **data, uint32 &size) const
{
	DocumentTags::Tag tag = DocumentTags::_content;
	if(IsReadAttribute())
		tag = GetAttributeTag();
	const List<Block> &l = _groupStack.back()->children;
	for (uint32 i = 0; i < l.size(); i++)
		if (l[i].tag == tag)
			return _read(l[i].pos, data, size = l[i].size);
	return false;
}

bool DocumentBINLoader::_readData(void *data, uint32 size) const
{
	const void *d = nullptr;
	uint32 s = 0;
	if (!_readData(&d, s) || s != size)
		return false;
	if (s > 0)
		std::memcpy(data, d, size);
	return true;
}

bool DocumentBINLoader::_exploreGroup(Block &group)
{
	if (group.children.empty()) { // Note: content may already have been explored!
		for (uint32 pos = group.pos, size = 0; pos < group.pos + group.size; pos += size) {
			DocumentTags::Tag tag;
			if (!_readHeader(pos, tag, size))
				return false; // Note: this indicates a serious error reading data buffer!
			//Block b = {tag, uint32(group.children.size()), pos, size};
			Block b(tag, uint32(group.children.size()), pos, size);
			group.children.push_back(b);
			assert(pos + size <= group.pos + group.size);
		}
	}
	return true;
}

bool DocumentBINLoader::VerifyGroup(DocumentTags::Tag group)
{
	return _groupStack.size() > 0 && _groupStack.back()->tag == group;
}

bool DocumentBINLoader::EnterGroup(DocumentTags::Tag group)
{
	bool breakOnEnd = _lastGroup == -1 || _groupStack.back()->children[_lastGroup].tag == group;
	for (uint32 i = 0, j = uint32(_groupStack.back()->children.size()), k = _lastGroup + 1; breakOnEnd ? k < j : i < j; i++, k++) {
		if (_groupStack.back()->children[k % j].tag == group) {
			_lastGroup = -1;
			_groupStack.push_back(&_groupStack.back()->children[k % j]);
			// Explore content of group
			if (!_exploreGroup(*_groupStack.back()))
				return false; // Note: this indicates a serious error reading data buffer!
			return true;
		}
	}
	return false; // Note: this just indicates group not found!
}

bool DocumentBINLoader::EnterGroup(DocumentTags::Tag group, DocumentTags::Tag attribute, String attrValue)
{
	for (uint32 i = 0, j = uint32(_groupStack.back()->children.size()), k = _lastGroup + 1; i < j; i++, k++) {
		Block &g = _groupStack.back()->children[k % j];
		if (g.tag == group) {
			if (!_exploreGroup(g))
				return false; // Note: this indicates a serious error reading data buffer!
			for (uint32 p = 0; p < g.children.size(); p++) {
				if (g.children[p].tag == attribute) {
					const Char *buff = nullptr;
					if (!_read(g.children[p].pos, (const void**)&buff, g.children[p].size))
						return false; // Note: this indicates a serious error reading data buffer!
					if (String(buff, g.children[p].size) == attrValue) { // Note: this is only valid if the attribute was saved as a string!!
						_lastGroup = -1;
						_groupStack.push_back(&g);//  Block(g)); // Copy contructor neccesarry because of list reallocation!
						// Content is already explored!
						return true;
					}
				}
			}
		}
	}
	return false; // Note: this just indicates group not found!
}

bool DocumentBINLoader::LeaveGroup(DocumentTags::Tag group)
{
	if (_groupStack.size() < 2 || _groupStack.back()->tag != group)
		return false;
	_lastGroup = _groupStack.back()->index;
	_groupStack.pop_back();
	return true;
}

bool DocumentBINLoader::ReadData(String &data)
{
	const Char *tmp = nullptr;
	uint32 s = 0;
	if (!_readData((const void**)&tmp, s))
		return false;
	data = String(tmp, s);
	return true;
}
