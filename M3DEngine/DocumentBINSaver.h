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
#include "M3DCore/DataBuffer.h"

namespace m3d
{

// No export!
class DocumentBINSaver : public DocumentSaver
{
private:
	struct Block
	{
		DocumentTags::Tag tag;
		uint32 pos;
	};

	List<Block> _groupStack;

	DataBuffer _data;
	uint32 _size;

	uint32 _pos() const { return _size; }
	bool _write(uint32 pos, const void *data, uint32 size);
	bool _writeHeader(uint32 pos, DocumentTags::Tag tag, uint32 size);
	bool _writeData(const void *data, uint32 size);


	DocumentBINSaver(DocumentBINSaver *parent);

public:
	DocumentBINSaver();
	~DocumentBINSaver();

	bool IsBinary() const override { return true; }

	bool Initialize() override;

	bool SaveToFile(Path fileName, DocumentEncryptionLevel encryptionLevel = DocumentEncryptionLevel::ENCLEVEL0, Guid signature = NullGUID) override;
	bool SaveToMemory(DataBuffer &databuffer, DocumentEncryptionLevel encryptionLevel = DocumentEncryptionLevel::ENCLEVEL0, Guid signature = NullGUID) override;

	bool SaveChips(const ChipPtrByChipIDMap &chips) override;

	bool PushGroup(DocumentTags::Tag group) override;
	bool PopGroup(DocumentTags::Tag group) override;

	bool WriteData(String data) override { return _writeData(data.c_str(), uint32(data.size() * sizeof(Char))); }
	bool WriteData(bool data) override { return _writeData(&data, sizeof(bool)); }
	bool WriteData(float32 data) override { return _writeData(&data, sizeof(float32)); }
	bool WriteData(float64 data) override { return _writeData(&data, sizeof(float64)); }
	bool WriteData(int8 data) override { return _writeData(&data, sizeof(int8)); }
	bool WriteData(uint8 data) override { return _writeData(&data, sizeof(uint8)); }
	bool WriteData(int16 data) override { return _writeData(&data, sizeof(int16)); }
	bool WriteData(uint16 data) override { return _writeData(&data, sizeof(uint16)); }
	bool WriteData(int32 data)  override { return _writeData(&data, sizeof(int32)); }
	bool WriteData(uint32 data)  override { return _writeData(&data, sizeof(uint32)); }
	bool WriteData(int64 data) override { return _writeData(&data, sizeof(int64)); }
	bool WriteData(uint64 data) override { return _writeData(&data, sizeof(uint64)); }
	bool WriteData(const void *data, uint32 size) override { return _writeData(data, size); }
	bool WriteData(const float32 *data, uint32 size) override { return _writeData(data, size * sizeof(float32)); }
	bool WriteData(const float64 *data, uint32 size) override { return _writeData(data, size * sizeof(float64)); }
	bool WriteData(const int8 *data, uint32 size) override { return _writeData(data, size * sizeof(int8)); }
	bool WriteData(const uint8 *data, uint32 size) override { return _writeData(data, size * sizeof(uint8)); }
	bool WriteData(const int16 *data, uint32 size) override { return _writeData(data, size * sizeof(int16)); }
	bool WriteData(const uint16 *data, uint32 size) override { return _writeData(data, size * sizeof(uint16)); }
	bool WriteData(const int32 *data, uint32 size) override { return _writeData(data, size * sizeof(int32)); }
	bool WriteData(const uint32 *data, uint32 size) override { return _writeData(data, size * sizeof(uint32)); }
};




}