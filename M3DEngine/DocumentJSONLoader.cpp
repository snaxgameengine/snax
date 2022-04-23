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
#include "DocumentJSONLoader.h"
#include "M3DCore/Base64.h"
#include "M3DCore/DataBuffer.h"
#include "Class.h"
#include <limits>
#include "rapidjson/filereadstream.h"

using namespace m3d;

extern int32 __encLevel2Mode; // 0: No pref, 1: Not allowed, 2: Allowed with right signature

DocumentJSONLoader::DocumentJSONLoader()
{
}

DocumentJSONLoader::~DocumentJSONLoader()
{
	_free();
}

bool DocumentJSONLoader::_open()
{
	if (!_doc.IsObject())
		return false;

	if (__encLevel2Mode == 2) {
		msg(FATAL, MTEXT("Can't load this document."));
		return false; // A level2 doc already loaded. Can't load other levels (to prevent access from 3rd-party docs) 
	}

	__encLevel2Mode = 1; // Block level2 docs.

	rapidjson::Value::ConstMemberIterator node = _doc.FindMember(DocumentTags::TagStr_json[DocumentTags::Document]);
	if (node == rapidjson::Value::ConstMemberIterator()) {}
	if (node == _doc.MemberEnd()) 
	{
		_free();
		msg(FATAL, MTEXT("Invalid document content."));
		return false;
	}

	_nodeStack.push_back(Node(DocumentTags::Document, node));

	return _init();
}

void DocumentJSONLoader::_free()
{
	_nodeStack.clear();
	_doc = rapidjson::Document();
}

bool DocumentJSONLoader::OpenFile(Path fileName)
{
	_free();

	FILE* fp = fopen(fileName.AsString().c_str(), "rb"); // non-Windows use "w"
	if (!fp)
		return false;

	char readBuffer[65536];
	rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));

	_doc.ParseStream(is);

	fclose(fp);

	return _open();
}

bool DocumentJSONLoader::OpenMemory(DataBuffer &&data)
{
	_free();

	if (data.getBufferSize() == 0)
		return false;

	String str((const Char*)data.getConstBuffer(), data.getBufferSize());

	_doc.Parse(str.c_str());
	if (!_doc.IsObject())
	{
		msg(FATAL, MTEXT("Failed to open document from memory."));
		return false;
	}

	return _open();
}

bool DocumentJSONLoader::Reset()
{
	if (!DocumentLoader::Reset())
		return false;
	_nodeStack.clear();
	return _open(); // Ready to read file again!
}

bool DocumentJSONLoader::VerifyGroup(DocumentTags::Tag group)
{
	return _nodeStack.empty() ? false : _nodeStack.back().group == group;
}

bool DocumentJSONLoader::EnterGroup(DocumentTags::Tag group)
{
	if (_nodeStack.empty())
		return false;

	if (_nodeStack.back().GetValue()->IsObject()) 
	{
		rapidjson::Value::ConstMemberIterator node = _nodeStack.back().GetValue()->FindMember(DocumentTags::TagStr_json[group]);
		if (node == _nodeStack.back().GetValue()->MemberEnd())
			return false;
		_nodeStack.push_back(Node(group, node));
	}
	else if (_nodeStack.back().GetValue()->IsArray()) // Array of arrays are not supported, and not something we encounter in our files!
	{ 
		assert(_nodeStack.back().node != rapidjson::Value::ConstMemberIterator() && _nodeStack.back().node->value.IsArray());
		if (_nodeStack.back().group == DocumentTags::Content) 
		{
			return EnterGroup(group, DocumentTags::objectType, DocumentTags::TagStr_json[group]);
		}
		else {
			if (_nodeStack.back().itr == _nodeStack.back().node->value.GetArray().End())
				_nodeStack.back().itr = _nodeStack.back().node->value.GetArray().Begin();
			else
				_nodeStack.back().itr++;
			while (_nodeStack.back().itr != _nodeStack.back().node->value.GetArray().End() && !_nodeStack.back().itr->IsObject())
				_nodeStack.back().itr++;
			if (_nodeStack.back().itr == _nodeStack.back().node->value.GetArray().End())
				return false;
			_nodeStack.push_back(Node(group, _nodeStack.back().itr));
		}
	}
	else
	{
		return false; // Invalid
	}

	return true;
}

bool DocumentJSONLoader::EnterGroup(DocumentTags::Tag group, DocumentTags::Tag attribute, String attrValue)
{
	if (_nodeStack.empty())
		return false;

	if (_nodeStack.back().GetValue()->IsObject()) 
	{
		rapidjson::Value::ConstMemberIterator jtr = _nodeStack.back().GetValue()->FindMember(attrValue.c_str());
		if (jtr == _nodeStack.back().GetValue()->MemberEnd()) 
			return false;
		_nodeStack.push_back(Node(group, jtr));
	}
	else if (_nodeStack.back().GetValue()->IsArray())  // Array of arrays are not supported, and not something we encounter in our files!
	{ 
		assert(_nodeStack.back().node != rapidjson::Value::ConstMemberIterator() && _nodeStack.back().node->value.IsArray());

		if (attribute != _nodeStack.back().searchAttr || attrValue != _nodeStack.back().searchAttrValue) 
		{
			_nodeStack.back().itr = _nodeStack.back().node->value.GetArray().Begin();
			_nodeStack.back().searchAttr = attribute;
			_nodeStack.back().searchAttrValue = attrValue;
		}
		else if (_nodeStack.back().itr != _nodeStack.back().node->value.GetArray().End())
		{
			_nodeStack.back().itr++;
		}

		for (; _nodeStack.back().itr != _nodeStack.back().node->value.GetArray().End(); _nodeStack.back().itr++) 
		{
			if (_nodeStack.back().itr->IsObject()) 
			{
				rapidjson::Value::ConstMemberIterator jtr = _nodeStack.back().itr->FindMember(DocumentTags::TagStr_json[attribute]);
				if (jtr != _nodeStack.back().itr->MemberEnd()) 
				{
					if (jtr->value.IsString() && String(jtr->value.GetString(), jtr->value.GetStringLength()) == attrValue)
						break;
				}
			}
		}

		if (_nodeStack.back().itr == _nodeStack.back().node->value.GetArray().End())
			return false;
		_nodeStack.push_back(Node(group, _nodeStack.back().itr));
	}
	else
	{
		return false; // Invalid
	}

	return true;
}

bool DocumentJSONLoader::LeaveGroup(DocumentTags::Tag group)
{
	if (_nodeStack.empty())
		return false;
	_nodeStack.pop_back();
	return true;
}

bool DocumentJSONLoader::ReadData(String& data)
{
	if (_nodeStack.empty())
		return false;
	if (IsReadAttribute()) 
	{
		rapidjson::Value::ConstMemberIterator attrNode = _nodeStack.back().GetValue()->FindMember(DocumentTags::TagStr_json[GetAttributeTag()]);
		if (attrNode == _nodeStack.back().GetValue()->MemberEnd())
			return false;
		if (!attrNode->value.IsString())
			return false;
		data = String(attrNode->value.GetString(), attrNode->value.GetStringLength());
	}
	else 
	{
		if (_nodeStack.back().group == DocumentTags::Data || _nodeStack.back().group == DocumentTags::Comment || _nodeStack.back().group == DocumentTags::Description) 
		{
			if (!_nodeStack.back().GetValue()->IsString())
				return false;
			data = String(_nodeStack.back().GetValue()->GetString(), _nodeStack.back().GetValue()->GetStringLength());
		}
		else 
		{
			rapidjson::Value::ConstMemberIterator attrNode = _nodeStack.back().GetValue()->FindMember(DocumentTags::TagStr_json[DocumentTags::Content]);
			if (attrNode == _nodeStack.back().GetValue()->MemberEnd())
				return false;
			if (!attrNode->value.IsString())
				return false;
			data = String(attrNode->value.GetString(), attrNode->value.GetStringLength());
		}
	}
	return true;
}

template<typename T, typename S>
bool DocumentJSONLoader::_readData(T& data)
{
	S temp;
	if (_nodeStack.empty())
		return false;
	if (IsReadAttribute()) 
	{
		rapidjson::Value::ConstMemberIterator attrNode = _nodeStack.back().GetValue()->FindMember(DocumentTags::TagStr_json[GetAttributeTag()]);
		if (attrNode == _nodeStack.back().GetValue()->MemberEnd())
			return false;
		if (!attrNode->value.Is<S>())
			return false;
		temp = attrNode->value.Get<S>();
	}
	else {
		if (_nodeStack.back().group == DocumentTags::Data) 
		{
			if (!_nodeStack.back().GetValue()->Is<S>())
				return false;
			temp = _nodeStack.back().GetValue()->Get<S>();
		}
		else {
			rapidjson::Value::ConstMemberIterator attrNode = _nodeStack.back().GetValue()->FindMember(DocumentTags::TagStr_json[DocumentTags::Content]);
			if (attrNode == _nodeStack.back().GetValue()->MemberEnd())
				return false;
			if (!attrNode->value.Is<S>())
				return false;
			temp = attrNode->value.Get<S>();
		}
	}
	if (temp < std::numeric_limits<T>::lowest() || temp > std::numeric_limits<T>::max())
		return false;
	data = (T)temp;
	return true;
}

template<typename T, typename S>
bool __readJsonDataArray(const rapidjson::Value &v, T* data, uint32 size)
{
	if (!v.IsArray())
		return false;
	S temp;
	uint32 i = 0;
	for (rapidjson::Value::ConstValueIterator itr =v.GetArray().Begin(); itr != v.GetArray().End(); itr++)
	{
		if (i >= size)
			return false;
		if (!itr->Is<S>())
			return false;
		temp = itr->Get<S>();
		if (temp < std::numeric_limits<T>::lowest() || temp > std::numeric_limits<T>::max())
			return false;
		data[i++] = temp;
	}
	if (i != size)
		return false;
	return true;
}

template<typename T, typename S>
bool DocumentJSONLoader::_readData(T* data, uint32 size)
{
	if (IsReadAttribute()) 
	{
		rapidjson::Value::ConstMemberIterator attrNode = _nodeStack.back().GetValue()->FindMember(DocumentTags::TagStr_json[GetAttributeTag()]);
		if (attrNode == _nodeStack.back().GetValue()->MemberEnd())
			return false;
		return __readJsonDataArray<T, S>(attrNode->value, data, size);
	}
	else 
	{
		if (_nodeStack.back().group == DocumentTags::Data) 
		{
			return __readJsonDataArray<T, S>(*_nodeStack.back().GetValue(), data, size);
		}
		else 
		{
			rapidjson::Value::ConstMemberIterator attrNode = _nodeStack.back().GetValue()->FindMember(DocumentTags::TagStr_json[DocumentTags::Content]);
			if (attrNode == _nodeStack.back().GetValue()->MemberEnd())
				return false;
			return __readJsonDataArray<T, S>(attrNode->value, data, size);
		}
	}

	return false;
}

bool DocumentJSONLoader::ReadData(void* data, uint32 size)
{
	String str;
	if (!ReadData(str))
		return false;

	const uint32 HEADER_B64_SIZE = 12;

	// Check that the string is at least size of header!
	if (str.length() < HEADER_B64_SIZE) 
	{
		return false;
	}

	// Decode header!
	uint32 header[2] = { 0, 0 };
	Char headerB64[HEADER_B64_SIZE + 1];
	std::memcpy(headerB64, str.c_str(), HEADER_B64_SIZE);
	headerB64[HEADER_B64_SIZE] = 0;
	size_t headerSize = sizeof(header);
	uint32* tmp = header;
	if (!Base64::Decode(headerB64, (uint8**)&tmp, headerSize) || headerSize != sizeof(header)) 
	{
		return false;
	}
	uint32 checksum = header[0];
	uint32 flags = header[1];
	bool ok = false;
	size_t resultSize = 0;

	if (flags & 0x00000001)  // compressed?
	{
		size_t s = Base64::CalculateTargetLengthForDecoding((const Char*)str.c_str() + HEADER_B64_SIZE);
		DataBuffer db(s); // buffer to hold compressed data
		uint8* buff = db.getBuffer();
		ok = Base64::Decode((const Char*)str.c_str() + HEADER_B64_SIZE, &buff, s);
		if (!ok)
			return false;
		z_stream strm;
		strm.zalloc = Z_NULL;
		strm.zfree = Z_NULL;
		strm.opaque = Z_NULL;
		int32 ret = inflateInit(&strm);
		if (ret != Z_OK)
			return false;
		strm.avail_in = (uint32)db.getBufferSize();
		strm.next_in = (Bytef*)db.getConstBuffer();
		strm.next_out = (Bytef*)data;
		strm.avail_out = size;
		while (ret == Z_OK && strm.avail_out > 0) {
			ret = inflate(&strm, Z_FINISH);
		}
		inflateEnd(&strm);
		if (ret != Z_STREAM_END)
			return false; // decompression failed!
		resultSize = size - strm.avail_out;
	}
	else 
	{
		resultSize = size;
		ok = Base64::Decode((const Char*)str.c_str() + HEADER_B64_SIZE, (uint8**)&data, resultSize);
	}

	if (!ok || resultSize != size)
		return false;

	boost::crc_32_type crc;
	crc.process_bytes(data, size);
	uint32 checksumFromData = crc.checksum();

	return checksumFromData == checksum;
}

