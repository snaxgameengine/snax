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
#include "M3DCore/DataBuffer.h"


namespace m3d
{

// No export!
class DocumentBINLoader : public DocumentLoader
{
private:
	struct Block
	{
		DocumentTags::Tag tag;
		uint32 index;
		uint32 pos;
		uint32 size;
		List<Block> children;
		Block() {}
		Block(DocumentTags::Tag tag, uint32 index, uint32 pos, uint32 size) : tag(tag), index(index), pos(pos), size(size) {}
	};

	Block _rootBlock;
	List<Block*> _groupStack;
	uint32 _lastGroup;

	DataBuffer _data;

	void _clear();

	bool _read(uint32 pos, void *data, uint32 size) const;
	bool _read(uint32 pos, const void **data, uint32 size) const;
	bool _readHeader(uint32 &pos, DocumentTags::Tag &tag, uint32 &size) const;
	bool _readData(const void **data, uint32 &size) const;
	bool _readData(void *data, uint32 size) const;

	bool _exploreGroup(Block &group);

public:
	DocumentBINLoader();
	~DocumentBINLoader();

	bool IsBinary() const override { return true; }

	bool OpenFile(Path fileName) override;
	bool OpenMemory(DataBuffer &&data) override;

	bool Reset() override;

	bool VerifyGroup(DocumentTags::Tag group) override;
	bool EnterGroup(DocumentTags::Tag group) override;
	bool EnterGroup(DocumentTags::Tag group, DocumentTags::Tag attribute, String attrValue) override;
	bool LeaveGroup(DocumentTags::Tag group) override;

	bool ReadData(String &data) override;
	bool ReadData(bool &data) override { return _readData(&data, sizeof(bool)); }
	bool ReadData(float32 &data) override { return _readData(&data, sizeof(float32)); }
	bool ReadData(float64 &data) override { return _readData(&data, sizeof(float64)); }
	bool ReadData(int8 &data) override { return _readData(&data, sizeof(int8)); }
	bool ReadData(uint8 &data) override { return _readData(&data, sizeof(uint8)); }
	bool ReadData(int16 &data) override { return _readData(&data, sizeof(int16)); }
	bool ReadData(uint16 &data) override { return _readData(&data, sizeof(uint16)); }
	bool ReadData(int32 &data) override { return _readData(&data, sizeof(int32)); }
	bool ReadData(uint32 &data) override { return _readData(&data, sizeof(uint32)); }
	bool ReadData(int64 &data) override { return _readData(&data, sizeof(int64)); }
	bool ReadData(uint64 &data) override { return _readData(&data, sizeof(uint64)); }
	bool ReadData(void *data, uint32 size) override { return _readData(data, size); }
	bool ReadData(float32 *data, uint32 size) override { return _readData(data, size * sizeof(float32)); }
	bool ReadData(float64 *data, uint32 size) override { return _readData(data, size * sizeof(float64)); }
	bool ReadData(int8 *data, uint32 size) override { return _readData(data, size * sizeof(int8)); }
	bool ReadData(uint8 *data, uint32 size) override { return _readData(data, size * sizeof(uint8)); }
	bool ReadData(int16 *data, uint32 size) override { return _readData(data, size * sizeof(int16)); }
	bool ReadData(uint16 *data, uint32 size) override { return _readData(data, size * sizeof(uint16)); }
	bool ReadData(int32 *data, uint32 size) override { return _readData(data, size * sizeof(int32)); }
	bool ReadData(uint32 *data, uint32 size) override { return _readData(data, size * sizeof(uint32)); }
};


}