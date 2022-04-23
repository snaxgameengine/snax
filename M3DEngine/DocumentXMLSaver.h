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
#include "DocumentSaver.h"

struct _xmlNode;
typedef struct _xmlNode xmlNode;

struct _xmlDoc;
typedef struct _xmlDoc xmlDoc;

typedef unsigned char xmlChar;

#define __BAD_CAST (xmlChar *)

namespace m3d
{


class M3DENGINE_API DocumentXMLSaver : public DocumentSaver
{
private:
	xmlDoc *_doc;
	xmlNode *_node;

	void _free();
	bool _writeData(const xmlChar *data);
	bool _writeData(String data); // data should be UTF8!

	template<typename T>
	bool _writeData(const T *data, uint32 size);

	DocumentXMLSaver(DocumentXMLSaver *parent);

public:
	DocumentXMLSaver();
	~DocumentXMLSaver();

	bool IsBinary() const override { return false; }

	bool Initialize() override;

	bool SaveToFile(Path fileName, DocumentEncryptionLevel encryptionLevel = DocumentEncryptionLevel::ENCLEVEL0, Guid signature = NullGUID) override;
	bool SaveToMemory(DataBuffer &databuffer, DocumentEncryptionLevel encryptionLevel = DocumentEncryptionLevel::ENCLEVEL0, Guid signature = NullGUID) override;

	bool SaveChips(const ChipPtrByChipIDMap&chips) override;

	bool PushGroup(DocumentTags::Tag group) override;
	bool PopGroup(DocumentTags::Tag group) override;

	bool WriteData(String data) override { return _writeData(data); }
	bool WriteData(bool data) override { return _writeData(data ? MTEXT("true") : MTEXT("false")); }
	bool WriteData(float32 data) override { return _writeData(__BAD_CAST strUtils::fromNum(data).c_str()); }
	bool WriteData(float64 data) override { return _writeData(__BAD_CAST strUtils::fromNum(data).c_str()); }
	bool WriteData(int8 data) override { return _writeData(__BAD_CAST strUtils::fromNum((int16)data).c_str()); }
	bool WriteData(uint8 data) override { return _writeData(__BAD_CAST strUtils::fromNum((uint16)data).c_str()); }
	bool WriteData(int16 data) override { return _writeData(__BAD_CAST strUtils::fromNum(data).c_str()); }
	bool WriteData(uint16 data) override { return _writeData(__BAD_CAST strUtils::fromNum(data).c_str()); }
	bool WriteData(int32 data) override { return _writeData(__BAD_CAST strUtils::fromNum(data).c_str()); }
	bool WriteData(uint32 data) override { return _writeData(__BAD_CAST strUtils::fromNum(data).c_str()); }
	bool WriteData(int64 data) override { return _writeData(__BAD_CAST strUtils::fromNum(data).c_str()); }
	bool WriteData(uint64 data) override { return _writeData(__BAD_CAST strUtils::fromNum(data).c_str()); }
	bool WriteData(const void *data, uint32 size) override;
	bool WriteData(const float32 *data, uint32 size) override;
	bool WriteData(const float64 *data, uint32 size) override;
	bool WriteData(const int8 *data, uint32 numberOfEntries) override;
	bool WriteData(const uint8 *data, uint32 numberOfEntries) override;
	bool WriteData(const int16 *data, uint32 numberOfEntries) override;
	bool WriteData(const uint16 *data, uint32 numberOfEntries) override;
	bool WriteData(const int32 *data, uint32 size) override;
	bool WriteData(const uint32 *data, uint32 size) override;

};




}