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
#include "DocumentXMLSaver.h"
#include "M3DCore/Base64.h"
#include "M3DCore/DataBuffer.h"
#include "Class.h"
#include "M3DCore/SlimRWLock.h"


using namespace m3d;

void __xmlFree(void *data) { xmlFree(data); }

DocumentXMLSaver::DocumentXMLSaver() : _doc(nullptr), _node(nullptr)
{
	}

DocumentXMLSaver::DocumentXMLSaver(DocumentXMLSaver *parent) : DocumentSaver(parent), _doc(nullptr), _node(xmlNewNode(nullptr, BAD_CAST "tmp parent"))
{
}

DocumentXMLSaver::~DocumentXMLSaver() 
{
	_free();
}

void DocumentXMLSaver::_free()
{
	if (_doc)
		xmlFreeDoc(_doc);
	else if (_node)
		xmlFreeNode(_node);
	_doc = nullptr;
	_node = nullptr;
}

bool DocumentXMLSaver::Initialize()
{
	_free();

	_doc = xmlNewDoc(BAD_CAST "1.0");
	if (!_doc)
		return false;
	_node = xmlNewNode(nullptr, BAD_CAST DocumentTags::TagStr[DocumentTags::Document]);
	if (_node) {
	    xmlDocSetRootElement(_doc, _node);
		return true;
	}
	return false;
}

bool DocumentXMLSaver::SaveToFile(Path fileName, DocumentEncryptionLevel encryptionLevel, Guid signature)
{
	if (encryptionLevel != DocumentEncryptionLevel::ENCLEVEL0 || signature != NullGUID)
		return false; // Encryption not supported!
	
	if (!_finalize())
		return false;

	if (!_doc || _node != xmlDocGetRootElement(_doc))
		return false;

	return xmlSaveFormatFile(fileName.AsString().c_str(), _doc, 1) > 0;
}

bool DocumentXMLSaver::SaveToMemory(DataBuffer &databuffer, DocumentEncryptionLevel encryptionLevel, Guid signature)
{
	if (encryptionLevel != DocumentEncryptionLevel::ENCLEVEL0 || signature != NullGUID)
		return false; // Encryption not supported!

	if (!_finalize())
		return false;

	if (!_doc || _node != xmlDocGetRootElement(_doc))
		return false;

	xmlChar *data = 0;
	int32 size = 0;
	xmlDocDumpFormatMemory(_doc, &data, &size, 1);
	databuffer.setBufferData(data, size, &__xmlFree);
	return true;
}

bool DocumentXMLSaver::SaveChips(const ChipPtrByChipIDMap&chips)
{
	bool ok = true;
	ok = ok && PushGroup(DocumentTags::Chips);

	if (!ok)
		return false;
	
	if (IsUsingMultithreading()) { // Parallel execution of chip serialization?
		struct ChipNode
		{
			Chip *chip;
			xmlNodePtr node;
		};

		// Create sequential list of all chips!
		List<ChipNode> chipNodes;
		chipNodes.resize(chips.size());
		uint32 count = 0;
		for (const auto &n : chips) {
			chipNodes[count].chip = n.second;
			chipNodes[count].node = nullptr;
			count++;
		}
		DocumentXMLSaver *saver = this;

		SlimRWLock lock;

		// Save the chips using parallel algorithm. 
		concurrency::parallel_for(0u, count, [&](uint32 i) {
			DocumentXMLSaver worker(saver);
			if (worker.SaveChip(chipNodes[i].chip, &chips)) {
				xmlNodePtr node = worker._node ? xmlFirstElementChild(worker._node) : nullptr;
				if (node) {
					xmlUnlinkNode(node);
					chipNodes[i].node = node;
					lock.AquireWriteLock();
					GetChipTypes().insert(worker.GetChipTypes().begin(), worker.GetChipTypes().end());
					lock.ReleaseWriteLock();
				}
			}
		});

		// Go through list and add all nodes to the document in sequential order.
		for (uint32 i = 0; i < count; i++) {
			xmlNodePtr node = chipNodes[i].node;
			if (node != nullptr) {
				if (xmlAddChild(_node, node) == nullptr) {
					xmlFreeNode(node);
					ok = false; // Failed to add a node to the document for some reason!
				}
			}
			else
				ok = false; // SaveChip(...) failed!
		}
	}
	else {
		for (const auto &n : chips) {
			ok = ok && SaveChip(n.second, &chips);
		}
	}

	ok = PopGroup(DocumentTags::Chips) && ok;

	return ok;
}

bool DocumentXMLSaver::PushGroup(DocumentTags::Tag group)
{
	if (/*!_doc ||*/ IsWriteAttribute())
		return false;
	xmlNodePtr newNode = xmlNewNode(0, BAD_CAST DocumentTags::TagStr[group]);
	if (!newNode)
		return false;

	if (!xmlAddChild(_node, newNode)) {
		xmlFreeNode(newNode);
		return false;
	}

	_node = newNode;
	return true;
}

bool DocumentXMLSaver::PopGroup(DocumentTags::Tag group)
{
	if (/*!_doc ||*/ IsWriteAttribute() || /*_node == xmlDocGetRootElement(_doc)*/ _node->parent == nullptr || xmlStrcmp(_node->name, BAD_CAST DocumentTags::TagStr[group]) != 0)
		return false;

	_node = _node->parent;

	return true;
}

bool DocumentXMLSaver::_writeData(String data) 
{ 
	// TODO: Need to handle invalid XML-character somehow. Illigal: x00-x08 x0B x0C x0E-x19
	return _writeData(__BAD_CAST data.c_str()); 
} 

bool DocumentXMLSaver::_writeData(const xmlChar *data)
{
	if (IsWriteAttribute())
		return xmlSetProp(_node, BAD_CAST DocumentTags::TagStr[GetAttributeTag()], data) != nullptr;
	xmlNodeAddContent(_node, data);
	return true;
}

bool DocumentXMLSaver::WriteData(const void *data, uint32 size)
{
    boost::crc_32_type crc;
	crc.process_bytes(data, size);
	uint32 checksum = crc.checksum();
	uint32 flags = 0;
	DataBuffer db;
	
	if (GetCompressionLevel() != DocumentCompressionLevel::DCL_NONE && size > 512) { // Don't compress very small chunks of data!
		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		int32 ret = deflateInit(&strm, GetCompressionLevel() == DocumentCompressionLevel::DCL_LOW ? Z_BEST_SPEED : (GetCompressionLevel() == DocumentCompressionLevel::DCL_HIGH ? Z_BEST_COMPRESSION : Z_DEFAULT_COMPRESSION));
		if (ret == Z_OK) {
			uint32 written = 0;
			db.realloc(std::max(1024u, size / 3));
			strm.avail_in = size;
			strm.next_in = (Bytef*)data; // casting to nonconst!
			while (ret == Z_OK) {
				if (db.getBufferSize() - written < 256) { // less than 256 bytes available?
					db.realloc(db.getBufferSize() * 2, true); // Double buffer size!
				}
				uint32 available = uint32(db.getBufferSize() - written);
				strm.avail_out = available;
				strm.next_out = (Bytef*)(db.getBuffer() + written);
				ret = deflate(&strm, Z_FINISH);
				written += (available - strm.avail_out);
			}
			deflateEnd(&strm);
			if (ret == Z_STREAM_END) {
				flags |= 0x00000001; // Indicate compression!
				data = db.getConstBuffer();
				size = written;
			}
		}
	}

	if ((flags & 0x00000001) == 0) { // not compressed?
		db.setBufferData((const uint8*)data, size, nullptr);
	}

	const uint32 HEADER_B64_SIZE = 12;
	size_t ts = Base64::CalculateTargetLengthForEncoding(size);
	DataBuffer db64((uint32)ts + HEADER_B64_SIZE);

	uint32 header[2] = {checksum, flags};
	if (!Base64::Encode((const uint8*)header, sizeof(header), (Char*)db64.getBuffer(), HEADER_B64_SIZE + 1)) // +1 because last byte is 0 and overwritten!
		return false;

	if (!Base64::Encode((uint8*)data, size, (Char*)db64.getBuffer() + HEADER_B64_SIZE, db64.getBufferSize() - HEADER_B64_SIZE))
		return false;

	return _writeData((const xmlChar*) db64.getConstBuffer());
}

bool DocumentXMLSaver::WriteData(const float32 *data, uint32 size) { return _writeData(data, size); }
bool DocumentXMLSaver::WriteData(const float64 *data, uint32 size) { return _writeData(data, size); }
bool DocumentXMLSaver::WriteData(const int16 *data, uint32 size) { return _writeData(data, size); }
bool DocumentXMLSaver::WriteData(const uint16 *data, uint32 size) { return _writeData(data, size); }
bool DocumentXMLSaver::WriteData(const int32 *data, uint32 size) { return _writeData(data, size); }
bool DocumentXMLSaver::WriteData(const uint32 *data, uint32 size) { return _writeData(data, size); }

bool DocumentXMLSaver::WriteData(const int8 *data, uint32 size)
{
	if (size > ARRAY_LIMIT_BEFORE_BASE64)
		return WriteData((void*)data, size * sizeof(int8)); // With large number of data we are better of using base64!
	String s;
	// Allocate a reasonable size for the string!
	for (uint32 i = 0; i < size; i++) {
		s += strUtils::fromNum((int16)data[i]);
		if (i < size - 1)
			s += MTEXT(" ");
	}
	return _writeData(s);
}

bool DocumentXMLSaver::WriteData(const uint8 *data, uint32 size)
{
	if (size > ARRAY_LIMIT_BEFORE_BASE64)
		return WriteData((void*)data, size * sizeof(uint8)); // With large number of data we are better of using base64!
	String s;
	// Allocate a reasonable size for the string!
	for (uint32 i = 0; i < size; i++) {
		s += strUtils::fromNum((uint16)data[i]);
		if (i < size - 1)
			s += MTEXT(" ");
	}
	return _writeData(s);
}

template<typename T>
bool DocumentXMLSaver::_writeData(const T *data, uint32 size)
{
	if (size > ARRAY_LIMIT_BEFORE_BASE64)
		return WriteData((void*)data, size * sizeof(T)); // With large number of data we are better of using base64!
	String s;
	// Allocate a reasonable size for the string!
	for (uint32 i = 0; i < size; i++) {
		s += strUtils::fromNum(data[i]);
		if (i < size - 1)
			s += MTEXT(" ");
	}
	return _writeData(s);
}

