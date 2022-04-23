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
#include "DocumentXMLLoader.h"
#include "M3DCore/Base64.h"
#include "M3DCore/DataBuffer.h"

using namespace m3d;

extern int32 __encLevel2Mode; // 0: No pref, 1: Not allowed, 2: Allowed with right signature

static const Char* TagStrOld2[] =
{
	MTEXT("Document"),
	MTEXT("ChipGraph"),
	MTEXT("Chips"),
	MTEXT("Chip"),
	MTEXT("Connections"),
	MTEXT("Connection"),
	MTEXT("ChipData"),
	MTEXT("Data"),
	MTEXT("Inheritance"),
	MTEXT("RelationsGraph"),
	MTEXT("Ptr"),
	MTEXT("Editor"),
	MTEXT("Graph"),
	MTEXT("Folder"),
	MTEXT("SelectionCopy"),
	MTEXT("Function"),
	MTEXT("Parameter"),
	MTEXT("Comments"),
	MTEXT("Comment"),
	MTEXT("Instances"),
	MTEXT("Instance"),
	MTEXT("Publish"),
	MTEXT("File"),
	MTEXT("Rect"),
	MTEXT("Image"),
	MTEXT("Text"),
	MTEXT("PublishProfiles"),
	MTEXT("Profile"),
	MTEXT(""),MTEXT(""),MTEXT(""),MTEXT(""),
	MTEXT("name"),
	MTEXT("startcgid"),
	MTEXT("startchipid"),
	MTEXT("version"),
	MTEXT("chipgraphid"),
	MTEXT("chipid"),
	MTEXT("id"),
	MTEXT("conn"),
	MTEXT("subconn"),
	MTEXT("x"),
	MTEXT("y"),
	MTEXT("inx"),
	MTEXT("iny"),
	MTEXT("type"),
	MTEXT("access"),
	MTEXT("refreshmode"),
	MTEXT("filename"),
	MTEXT("extent"),
	MTEXT("title"),
	MTEXT("target"),
	MTEXT("filters"),
	MTEXT("compression"),
	MTEXT("copyproject"),
	MTEXT("includeall"),
	MTEXT("sx"),
	MTEXT("sy"),
	MTEXT("color"),
	MTEXT("textSize")
};

DocumentXMLLoader::DocumentXMLLoader() : _doc(nullptr), _node(nullptr), _last(nullptr)
{
}

DocumentXMLLoader::~DocumentXMLLoader()
{
	_free();
}

const Char *DocumentXMLLoader::_getTagStr(DocumentTags::Tag t) const
{
	if (GetDocumentVersion() < Version(1,2,2,0))
		return TagStrOld2[t];
	return DocumentTags::TagStr[t];
}

bool DocumentXMLLoader::_open()
{
	if (!_doc)
		return false;

	if (__encLevel2Mode == 2) {
		msg(FATAL, MTEXT("Can't load this document."));
		return false; // A level2 doc already loaded. Can't load other levels (to prevent access from 3rd-party docs) 
	}

	__encLevel2Mode = 1; // Block level2 docs.

	_node = xmlDocGetRootElement(_doc);

	if (_node == nullptr || (xmlStrcmp(_node->name, BAD_CAST DocumentTags::TagStr[DocumentTags::Document]) != 0)) {
		_free();
		msg(FATAL, MTEXT("Invalid document content."));
		return false;
	}

	return _init();
}

void DocumentXMLLoader::_free()
{
	if (_doc)
		xmlFreeDoc(_doc);
	_doc = nullptr;
	_node = _last = nullptr;
}

bool DocumentXMLLoader::OpenFile(Path fileName)
{
	_free();
	_doc = xmlReadFile(fileName.AsString().c_str(), nullptr, XML_PARSE_HUGE);
	if (!_doc) {
		msg(FATAL, MTEXT("Failed to open document \'") + fileName.AsString() + MTEXT("\'."));
		return false;
	}
	if (!_open()) {
		msg(FATAL, MTEXT("Failed to read document \'") + fileName.AsString() + MTEXT("\'."));
		return false;
	}
	return true;
}

bool DocumentXMLLoader::OpenMemory(DataBuffer &&data)
{
	_free();
	_doc = xmlParseMemory((const char *)data.getConstBuffer(), (int32)data.getBufferSize());
	if (!_doc) {
		msg(FATAL, MTEXT("Failed to open document from memory."));
		return false;
	}
	return _open();
}

bool DocumentXMLLoader::Reset()
{
	if (!DocumentLoader::Reset())
		return false;
	_node = nullptr;
	_last = nullptr;
	return _open(); // Ready to read again!
}

bool DocumentXMLLoader::VerifyGroup(DocumentTags::Tag group)
{
	return xmlStrcmp(_node->name, BAD_CAST _getTagStr(group)) == 0;
}

bool DocumentXMLLoader::EnterGroup(DocumentTags::Tag group)
{
	bool breakOnNull = _last == nullptr || xmlStrcmp(_last->name, BAD_CAST _getTagStr(group)) == 0;

	for (xmlNodePtr n = _last ? xmlNextElementSibling(_last) : xmlFirstElementChild(_node); true; n = xmlNextElementSibling(n)) {
		if (n == _last || n == nullptr && (breakOnNull || (n = xmlFirstElementChild(_node)) == _last))
			break;
		if (xmlStrcmp(n->name, BAD_CAST _getTagStr(group)) == 0) {
			_last = nullptr;
			_node = n;
			return true;
		}
	}
	return false;
}

bool DocumentXMLLoader::EnterGroup(DocumentTags::Tag group, DocumentTags::Tag attribute, String attrValue)
{
	for (xmlNodePtr n = _last ? xmlNextElementSibling(_last) : xmlFirstElementChild(_node); true; n = xmlNextElementSibling(n)) {
		if (n == _last || n == nullptr && (n = xmlFirstElementChild(_node)) == _last)
			break;
		if (xmlStrcmp(n->name, BAD_CAST _getTagStr(group)) == 0) {
			String a;
			if (!_getAttribute(n, attribute, a))
				continue;
			if (a == attrValue) {
				_last = nullptr;
				_node = n;
				return true;
			}
		}
    }
	return false;
}

bool DocumentXMLLoader::LeaveGroup(DocumentTags::Tag group)
{
	if (_node == xmlDocGetRootElement(_doc) || xmlStrcmp(_node->name, BAD_CAST _getTagStr(group)) != 0)
		return false;
	_last = _node;
	_node = _node->parent;
	return true;
}

bool DocumentXMLLoader::ReadData(String &data) { return _getContentOrAttribute(data); }
bool DocumentXMLLoader::ReadData(float32 &data) { return _getContentOrAttribute(data); }
bool DocumentXMLLoader::ReadData(bool &data)
{
	String tmp;
	if (!_getContentOrAttribute(tmp))
		return false;
	data = tmp == MTEXT("true");
	return true;
}
bool DocumentXMLLoader::ReadData(float64 &data) { return _getContentOrAttribute(data); }
bool DocumentXMLLoader::ReadData(int8 &data) 
{ 
	int16 tmp;
	if (!_getContentOrAttribute(tmp))
		return false;
	data = (int8)tmp;
	return true; 
}
bool DocumentXMLLoader::ReadData(uint8 &data)
{ 
	uint16 tmp;
	if (!_getContentOrAttribute(tmp))
		return false;
	data = (uint8)tmp;
	return true; 
}
bool DocumentXMLLoader::ReadData(int16 &data) { return _getContentOrAttribute(data); }
bool DocumentXMLLoader::ReadData(uint16 &data) { return _getContentOrAttribute(data); }
bool DocumentXMLLoader::ReadData(int32 &data) { return _getContentOrAttribute(data); }
bool DocumentXMLLoader::ReadData(uint32 &data) { return _getContentOrAttribute(data); }
bool DocumentXMLLoader::ReadData(int64 &data) { return _getContentOrAttribute(data); }
bool DocumentXMLLoader::ReadData(uint64 &data) { return _getContentOrAttribute(data); }

bool DocumentXMLLoader::ReadData(void *data, uint32 size)
{
	xmlChar *c = _getContentOrAttribute();
	if (!c)
		return false;

	// TODO: Remove this old version stuff.......
	if (GetDocumentVersion() == VERSION1) {
		size_t s = size;
		bool ok = Base64::Decode((const Char*)c, (uint8**)&data, s);
		xmlFree(c);
		return ok && s == size;
	}

	const uint32 HEADER_B64_SIZE = 12;

	// Check that the string is at least size of header!
	for (uint32 i = 0; i < HEADER_B64_SIZE; i++) {
		if (c[i] == '\0') {
			xmlFree(c);
			return false;
		}
	}

	// Decode header!
	uint32 header[2] = {0, 0};
	Char headerB64[HEADER_B64_SIZE + 1];
	std::memcpy(headerB64, c, HEADER_B64_SIZE);
	headerB64[HEADER_B64_SIZE] = 0;
	size_t headerSize = sizeof(header);
	uint32 *tmp = header;
	if (!Base64::Decode(headerB64, (uint8**)&tmp, headerSize) || headerSize != sizeof(header)) {
		xmlFree(c);
		return false;
	}
	uint32 checksum = header[0];
	uint32 flags = header[1];
	bool ok = false;
	size_t resultSize = 0;

	if (flags & 0x00000001) { // compressed?
		size_t s = Base64::CalculateTargetLengthForDecoding((const Char*)c + HEADER_B64_SIZE);
		DataBuffer db(s); // buffer to hold compressed data
		uint8* buff = db.getBuffer();
		ok = Base64::Decode((const Char*)c + HEADER_B64_SIZE, &buff, s);
		xmlFree(c);
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
	else {
		resultSize = size;
		ok = Base64::Decode((const Char*)c + HEADER_B64_SIZE, (uint8**)&data, resultSize);
		xmlFree(c);
	}

	if (!ok || resultSize != size)
		return false;

    boost::crc_32_type crc;
	crc.process_bytes(data, size);
	uint32 checksumFromData = crc.checksum();

	return checksumFromData == checksum;
}

bool DocumentXMLLoader::ReadData(float32 *data, uint32 size) { return _readData(data, size); }
bool DocumentXMLLoader::ReadData(float64 *data, uint32 size) { return _readData(data, size); }
bool DocumentXMLLoader::ReadData(int16 *data, uint32 size) { return _readData(data, size); }
bool DocumentXMLLoader::ReadData(uint16 *data, uint32 size) { return _readData(data, size); }
bool DocumentXMLLoader::ReadData(int32 *data, uint32 size) { return _readData(data, size); }
bool DocumentXMLLoader::ReadData(uint32 *data, uint32 size) { return _readData(data, size); }

bool DocumentXMLLoader::ReadData(int8 *data, uint32 size)
{
	if (size > ARRAY_LIMIT_BEFORE_BASE64)
		return ReadData((void*)data, size * sizeof(int8));

	xmlChar *c = _getContentOrAttribute();

	if (!c)
		return false;

	bool ok = true;
	int16 f;
	uint32 count = 0;
	for (Char *co = 0, *token = strtok_s((Char*)c, MTEXT(" "), &co); token; token = strtok_s(0, MTEXT(" "), &co), count++) {
		if (!(ok = (strUtils::toNum(String(token), f) && count < size)))
			break;
		data[count] = (int8)f;
	}
	xmlFree(c);
	return ok && size == count;
}

bool DocumentXMLLoader::ReadData(uint8 *data, uint32 size)
{
	if (size > ARRAY_LIMIT_BEFORE_BASE64)
		return ReadData((void*)data, size * sizeof(uint8));

	xmlChar *c = _getContentOrAttribute();

	if (!c)
		return false;

	bool ok = true;
	uint16 f;
	uint32 count = 0;
	for (Char*co = 0, *token = strtok_s((Char*)c, MTEXT(" "), &co); token; token = strtok_s(0, MTEXT(" "), &co), count++) {
		if (!(ok = (strUtils::toNum(String(token), f) && count < size)))
			break;
		data[count] = (uint8)f;
	}
	xmlFree(c);
	return ok && size == count;
}

bool DocumentXMLLoader::_getAttribute(xmlNode* node, DocumentTags::Tag name, String &value)
{
	xmlChar *a = xmlGetProp(node, BAD_CAST _getTagStr(name));
	if (!a)
		return false;

	value = (const Char*) a;

	xmlFree(a);
	return true;
}

xmlChar *DocumentXMLLoader::_getContentOrAttribute()
{
	if (IsReadAttribute())
		return  xmlGetProp(_node, BAD_CAST _getTagStr(GetAttributeTag()));
	return xmlNodeGetContent(_node);
}

bool DocumentXMLLoader::_getContentOrAttribute(String &str)
{
	str.clear();
	if (IsReadAttribute())
		return _getAttribute(_node, GetAttributeTag(), str);
	xmlChar *s = xmlNodeGetContent(_node);
	if (!s)
		return true;
	str = (const Char*)s;
	xmlFree(s);
	return true;
}

template<typename T>
bool DocumentXMLLoader::_getContentOrAttribute(T &data)
{
	String tmp;
	return _getContentOrAttribute(tmp) ? (tmp.find(MTEXT("0x")) == 0 ? strUtils::toNum(tmp, data, strUtils::NumBase::HEX) : strUtils::toNum(tmp, data)) : false;
}

template<typename T>
bool DocumentXMLLoader::_readData(T *data, uint32 size)
{
	if (size > ARRAY_LIMIT_BEFORE_BASE64)
		return ReadData((void*)data, size * sizeof(T));

	xmlChar *c = _getContentOrAttribute();

	if (!c)
		return false;

	bool ok = true;
	T f;
	uint32 count = 0;
	for (Char *co = 0, *token = strtok_s((Char*)c, MTEXT(" "), &co); token; token = strtok_s(0, MTEXT(" "), &co), count++) {
		if (!(ok = (strUtils::toNum(String(token), f) && count < size)))
			break;
		data[count] = f;
	}
	xmlFree(c);
	return ok && size == count;
}
