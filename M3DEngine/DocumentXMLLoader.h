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

#pragma once

#include "Exports.h"
#include "DocumentLoader.h"

struct _xmlNode;
typedef struct _xmlNode xmlNode;

struct _xmlDoc;
typedef struct _xmlDoc xmlDoc;

typedef unsigned char xmlChar;


namespace m3d
{

class M3DENGINE_API DocumentXMLLoader : public DocumentLoader
{
private:
	xmlDoc *_doc;
	xmlNode *_node;
	xmlNode *_last;

	bool _open();
	void _free();

	bool _getAttribute(xmlNode* node, DocumentTags::Tag name, String &value); // Data comes as UTF8!
	bool _getContentOrAttribute(String &str); // Data comes as UTF8!
	xmlChar *_getContentOrAttribute();

	template<typename T>
	bool _getContentOrAttribute(T &data);

	template<typename T>
	bool _readData(T *data, uint32 size);

	const Char *_getTagStr(DocumentTags::Tag t) const;

public:
	DocumentXMLLoader();
	~DocumentXMLLoader();

	bool IsBinary() const override { return false; }

	bool OpenFile(Path fileName) override;
	bool OpenMemory(DataBuffer &&data) override;

	bool Reset();

	bool VerifyGroup(DocumentTags::Tag group);
	bool EnterGroup(DocumentTags::Tag group);
	bool EnterGroup(DocumentTags::Tag group, DocumentTags::Tag attribute, String attrValue);
	bool LeaveGroup(DocumentTags::Tag group);

	bool ReadData(String &data);
	bool ReadData(bool &data);
	bool ReadData(float32 &data);
	bool ReadData(float64 &data);
	bool ReadData(int8 &data);
	bool ReadData(uint8 &data);
	bool ReadData(int16 &data);
	bool ReadData(uint16 &data);
	bool ReadData(int32 &data);
	bool ReadData(uint32 &data);
	bool ReadData(int64 &data);
	bool ReadData(uint64 &data);
	bool ReadData(void *data, uint32 size);
	bool ReadData(float32 *data, uint32 size);
	bool ReadData(float64 *data, uint32 size);
	bool ReadData(int8 *data, uint32 size);
	bool ReadData(uint8 *data, uint32 size);
	bool ReadData(int16 *data, uint32 size);
	bool ReadData(uint16 *data, uint32 size);
	bool ReadData(int32 *data, uint32 size);
	bool ReadData(uint32 *data, uint32 size);

};

}