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
#include <rapidjson/document.h>



namespace m3d
{


class M3DENGINE_API DocumentJSONSaver : public DocumentSaver
{
private:
	rapidjson::Document _doc;
	struct Node
	{
		rapidjson::Value value;
		DocumentTags::Tag group;
		String id;

		Node(rapidjson::Type type, DocumentTags::Tag group) : value(type), group(group), id(DocumentTags::TagStr_json[group]) {}
	};
	List<Node> _nodeStack;
	
	void _free();

	template<typename T, typename S = T>
	bool _writeData(const T* data, uint32 size);

	template<typename T>
	bool _writeData(const T &data);


	DocumentJSONSaver(DocumentJSONSaver* parent);

public:
	DocumentJSONSaver();
	DocumentJSONSaver(const DocumentJSONSaver&) = delete;
	~DocumentJSONSaver();

	bool IsBinary() const override { return false; }
	bool IsJson() const override { return true; }

	bool Initialize() override;

	bool SaveToFile(Path fileName, DocumentEncryptionLevel encryptionLevel = DocumentEncryptionLevel::ENCLEVEL0, Guid signature = NullGUID) override;
	bool SaveToMemory(DataBuffer& databuffer, DocumentEncryptionLevel encryptionLevel = DocumentEncryptionLevel::ENCLEVEL0, Guid signature = NullGUID) override;

	bool SaveChips(const ChipPtrByChipIDMap& chips) override;

	bool PushGroup(DocumentTags::Tag group) override;
	bool PopGroup(DocumentTags::Tag group) override;

	bool WriteData(String data) override;
	bool WriteData(bool data) override { return _writeData(data); }
	bool WriteData(float32 data) override { return _writeData(data); }
	bool WriteData(float64 data) override { return _writeData(data); }
	bool WriteData(int8 data) override { return _writeData((int32)data); }
	bool WriteData(uint8 data) override { return _writeData((int32)data); }
	bool WriteData(int16 data) override { return _writeData((int32)data); }
	bool WriteData(uint16 data) override { return _writeData((uint32)data); }
	bool WriteData(int32 data) override { return _writeData(data); }
	bool WriteData(uint32 data) override { return _writeData(data); }
	bool WriteData(int64 data) override { return _writeData(data); }
	bool WriteData(uint64 data) override { return _writeData(data); }
	bool WriteData(const void* data, uint32 size) override;
	bool WriteData(const float32* data, uint32 size) override { return _writeData(data, size); }
	bool WriteData(const float64* data, uint32 size) override { return _writeData(data, size); }
	bool WriteData(const int8* data, uint32 size) override { return _writeData<int8, int32>(data, size); }
	bool WriteData(const uint8* data, uint32 size) override { return _writeData<uint8, uint32>(data, size); }
	bool WriteData(const int16* data, uint32 size) override { return _writeData<int16, int32>(data, size); }
	bool WriteData(const uint16* data, uint32 size) override { return _writeData<uint16, uint32>(data, size); }
	bool WriteData(const int32* data, uint32 size) override { return _writeData(data, size); }
	bool WriteData(const uint32* data, uint32 size) override { return _writeData(data, size); }

	template<typename T>
	bool WriteData(const T* data, uint32 size)
	{
		if (_nodeStack.empty() || _nodeStack.back().group != DocumentTags::Data)
			return false;
		_nodeStack.back().value = rapidjson::Value(rapidjson::kArrayType);
		for (uint32 i = 0; i < size; i++) {
			PushGroup(DocumentTags::Data);
			((DocumentSaver*)this)->WriteData(data[i]);
			PopGroup(DocumentTags::Data);
		}

		return true;
	}
};




}