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
#include "DocumentJSONSaver.h"
#include "M3DCore/Base64.h"
#include "M3DCore/DataBuffer.h"
#include "Class.h"
#include "M3DCore/SlimRWLock.h"
#include <rapidjson/filewritestream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>

using namespace m3d;


DocumentJSONSaver::DocumentJSONSaver() 
{
}

DocumentJSONSaver::DocumentJSONSaver(DocumentJSONSaver* parent) : DocumentSaver(parent)
{
}

DocumentJSONSaver::~DocumentJSONSaver()
{
	_free();
}

void DocumentJSONSaver::_free()
{
	_nodeStack.clear();
	_doc = rapidjson::Document();
}

bool DocumentJSONSaver::Initialize()
{
	_free();

	_nodeStack.push_back(Node(rapidjson::kObjectType, DocumentTags::Document));

	return true;
}

bool DocumentJSONSaver::SaveToFile(Path fileName, DocumentEncryptionLevel encryptionLevel, Guid signature)
{
	if (encryptionLevel != DocumentEncryptionLevel::ENCLEVEL0 || signature != NullGUID)
		return false; // Encryption not supported!

	if (!_finalize())
		return false;

	if (!PopGroup(DocumentTags::Document))
		return false;

	FILE* fp = fopen(fileName.AsString().c_str(), "wb"); // non-Windows use "w"
	if (!fp)
		return false;


	char writeBuffer[65536];
	rapidjson::FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));

	rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
	writer.SetIndent(' ', 2);
	writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
	bool result = _doc.Accept(writer);

	fclose(fp);

	return result;
}

bool DocumentJSONSaver::SaveToMemory(DataBuffer& databuffer, DocumentEncryptionLevel encryptionLevel, Guid signature)
{
	if (encryptionLevel != DocumentEncryptionLevel::ENCLEVEL0 || signature != NullGUID)
		return false; // Encryption not supported!

	if (!_finalize())
		return false;

	if (!PopGroup(DocumentTags::Document))
		return false;

	rapidjson::StringBuffer buffer;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
	writer.SetIndent(' ', 2);
	writer.SetFormatOptions(rapidjson::PrettyFormatOptions::kFormatSingleLineArray);
	bool result = _doc.Accept(writer);
	if (!result)
		return false;
	const char* data = buffer.GetString();
	size_t dataSize = buffer.GetSize();
	databuffer.setBufferData((const m3d::uint8*)data, dataSize);

	return true;
}

bool DocumentJSONSaver::SaveChips(const ChipPtrByChipIDMap& chips)
{
	bool ok = true;
	ok = ok && PushGroup(DocumentTags::Chips);

	if (!ok)
		return false;

	if (IsUsingMultithreading()) // Parallel execution of chip serialization?
	{ 
		struct ChipNode
		{
			Chip* chip;
			rapidjson::Document doc;
		};

		// Create sequential list of all chips!
		List<ChipNode> chipNodes;
		chipNodes.resize(chips.size());
		uint32 count = 0;
		for (const auto& n : chips) {
			chipNodes[count].chip = n.second;
			count++;
		}
		DocumentJSONSaver* saver = this;

		SlimRWLock lock;

		// Save the chips using parallel algorithm. 
		concurrency::parallel_for(0u, count, [&](uint32 i) {
			DocumentJSONSaver worker(saver);
			if (worker.SaveChip(chipNodes[i].chip, &chips)) 
			{
				rapidjson::Document doc;
				if (worker._doc.IsObject())
					doc = std::move(worker._doc);
				if (doc.IsObject()) 
				{
					chipNodes[i].doc = std::move(doc);
					lock.AquireWriteLock();
					GetChipTypes().insert(worker.GetChipTypes().begin(), worker.GetChipTypes().end());
					lock.ReleaseWriteLock();
				}
			}
			});

		// Go through list and add all nodes to the document in sequential order.
		for (uint32 i = 0; i < count; i++) 
		{
			rapidjson::Value node = chipNodes[i].doc.GetObjectA();
			if (node.IsObject()) {
				rapidjson::Value::ConstMemberIterator itr = node.FindMember(DocumentTags::TagStr_json[DocumentTags::Chip]);
				if (itr != node.MemberEnd()) {
					rapidjson::Value v;
					v.CopyFrom(itr->value, _doc.GetAllocator(), false); /// TODO: THIS COPY IS NOT OPTIMAL. IS THERE ANOTHER WAY??
					_nodeStack.back().value.PushBack(v, _doc.GetAllocator());
				}
				else
					ok = false;
			}
			else
				ok = false; // SaveChip(...) failed!
		}
	}
	else {
		for (const auto& n : chips) 
		{
			ok = ok && SaveChip(n.second, &chips);
		}
	}

	ok = PopGroup(DocumentTags::Chips) && ok;

	return ok;
}

bool DocumentJSONSaver::PushGroup(DocumentTags::Tag group)
{
	if (IsWriteAttribute())
		return false; // Invalid operation

	if (group == DocumentTags::Chips || group == DocumentTags::Connections || group == DocumentTags::Inheritance || group == DocumentTags::Classes || group == DocumentTags::Parameters || group == DocumentTags::Content)
		_nodeStack.push_back(Node(rapidjson::kArrayType, group));
	else
		_nodeStack.push_back(Node(rapidjson::kObjectType, group));

	return true;
}

bool DocumentJSONSaver::PopGroup(DocumentTags::Tag group)
{
	if (IsWriteAttribute() || _nodeStack.empty())
		return false; // Invalid operation

	Node node = std::move(_nodeStack.back());
	if (node.value.IsObject())
		std::sort(node.value.MemberBegin(), node.value.MemberEnd(), [](const rapidjson::Value::Member& lhs, const rapidjson::Value::Member& rhs) { return (strcmp(lhs.name.GetString(), rhs.name.GetString()) < 0); });
	_nodeStack.pop_back();
	if (_nodeStack.empty()) 
	{
		if (!_doc.IsObject())
			_doc.SetObject();
		_doc.AddMember(rapidjson::Value(node.id.c_str(), _doc.GetAllocator()), node.value, _doc.GetAllocator());
	}
	else {
		if (_nodeStack.back().value.GetType() == rapidjson::kObjectType)
		{
			_nodeStack.back().value.AddMember(rapidjson::Value(node.id.c_str(), _doc.GetAllocator()), node.value, _doc.GetAllocator());
		}
		else if (_nodeStack.back().value.GetType() == rapidjson::kArrayType) 
		{
			if (_nodeStack.back().group == DocumentTags::Content)
				node.value.AddMember(rapidjson::Value(DocumentTags::TagStr_json[DocumentTags::objectType], _doc.GetAllocator()), rapidjson::Value(DocumentTags::TagStr_json[node.group], _doc.GetAllocator()), _doc.GetAllocator());
			_nodeStack.back().value.PushBack(node.value, _doc.GetAllocator());
		}
		else
			return false;
	}

	return true;
}

bool DocumentJSONSaver::WriteData(String data) 
{ 
	if (IsWriteAttribute()) 
	{
		if (_nodeStack.back().group == DocumentTags::Data)
			_nodeStack.back().id = data;
		else
			_nodeStack.back().value.AddMember(rapidjson::Value(DocumentTags::TagStr_json[GetAttributeTag()], _doc.GetAllocator()), rapidjson::Value(data.c_str(), _doc.GetAllocator()), _doc.GetAllocator());
	}
	else
	{
		if (_nodeStack.back().group == DocumentTags::Data || _nodeStack.back().group == DocumentTags::Comment || _nodeStack.back().group == DocumentTags::Description)
			_nodeStack.back().value.SetString(data.c_str(), data.length(), _doc.GetAllocator());
		else
			_nodeStack.back().value.AddMember(rapidjson::Value(DocumentTags::TagStr_json[DocumentTags::Content], _doc.GetAllocator()), rapidjson::Value(data.c_str(), _doc.GetAllocator()), _doc.GetAllocator());
	}
	return true; 
}

template<typename T>
bool DocumentJSONSaver::_writeData(const T& data)
{
	rapidjson::Value a(data);

	if (IsWriteAttribute()) 
	{
		_nodeStack.back().value.AddMember(rapidjson::Value(DocumentTags::TagStr_json[GetAttributeTag()], _doc.GetAllocator()), a, _doc.GetAllocator());
	}
	else 
	{
		if (_nodeStack.back().group == DocumentTags::Data) 
			_nodeStack.back().value = a;
		else 
			_nodeStack.back().value.AddMember(rapidjson::Value(DocumentTags::TagStr_json[DocumentTags::Content], _doc.GetAllocator()), a, _doc.GetAllocator());
	}

	return true;
}

template<typename T, typename S>
bool DocumentJSONSaver::_writeData(const T* data, uint32 size)
{
	rapidjson::Value a(rapidjson::kArrayType);
	for (uint32 i = 0; i < size; i++)
		a.PushBack(rapidjson::Value((S)data[i]), _doc.GetAllocator());

	if (IsWriteAttribute()) 
	{
		_nodeStack.back().value.AddMember(rapidjson::Value(DocumentTags::TagStr_json[GetAttributeTag()], _doc.GetAllocator()), a, _doc.GetAllocator());
	}
	else 
	{
		if (_nodeStack.back().group == DocumentTags::Data)
			_nodeStack.back().value = a;
		else
			_nodeStack.back().value.AddMember(rapidjson::Value(DocumentTags::TagStr_json[DocumentTags::Content], _doc.GetAllocator()), a, _doc.GetAllocator());
	}

	return true;
}

bool DocumentJSONSaver::WriteData(const void* data, uint32 size)
{
	boost::crc_32_type crc;
	crc.process_bytes(data, size);
	uint32 checksum = crc.checksum();
	uint32 flags = 0;
	DataBuffer db;

	if (GetCompressionLevel() != DocumentCompressionLevel::DCL_NONE && size > 512)  // Don't compress very small chunks of data!
	{
		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		int32 ret = deflateInit(&strm, GetCompressionLevel() == DocumentCompressionLevel::DCL_LOW ? Z_BEST_SPEED : (GetCompressionLevel() == DocumentCompressionLevel::DCL_HIGH ? Z_BEST_COMPRESSION : Z_DEFAULT_COMPRESSION));
		if (ret == Z_OK) 
		{
			uint32 written = 0;
			db.realloc(std::max(1024u, size / 3));
			strm.avail_in = size;
			strm.next_in = (Bytef*)data; // casting to nonconst!
			while (ret == Z_OK) 
			{
				if (db.getBufferSize() - written < 256)  // less than 256 bytes available?
				{
					db.realloc(db.getBufferSize() * 2, true); // Double buffer size!
				}
				uint32 available = uint32(db.getBufferSize() - written);
				strm.avail_out = available;
				strm.next_out = (Bytef*)(db.getBuffer() + written);
				ret = deflate(&strm, Z_FINISH);
				written += (available - strm.avail_out);
			}
			deflateEnd(&strm);
			if (ret == Z_STREAM_END) 
			{
				flags |= 0x00000001; // Indicate compression!
				data = db.getConstBuffer();
				size = written;
			}
		}
	}

	if ((flags & 0x00000001) == 0)  // not compressed?
	{
		db.setBufferData((const uint8*)data, size, nullptr);
	}

	const uint32 HEADER_B64_SIZE = 12;
	size_t ts = Base64::CalculateTargetLengthForEncoding(size);
	DataBuffer db64((uint32)ts + HEADER_B64_SIZE);

	uint32 header[2] = { checksum, flags };
	if (!Base64::Encode((const uint8*)header, sizeof(header), (Char*)db64.getBuffer(), HEADER_B64_SIZE + 1)) // +1 because last byte is 0 and overwritten!
		return false;

	if (!Base64::Encode((uint8*)data, size, (Char*)db64.getBuffer() + HEADER_B64_SIZE, db64.getBufferSize() - HEADER_B64_SIZE))
		return false;

	return WriteData(String((const Char*)db64.getConstBuffer()));
}
