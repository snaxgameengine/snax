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
#include "rapidjson/document.h"


namespace m3d
{

class M3DENGINE_API DocumentJSONLoader : public DocumentLoader
{
private:
	rapidjson::Document _doc;

	struct Node
	{
		DocumentTags::Tag group;
		rapidjson::Value::ConstMemberIterator node;
		rapidjson::Value::ConstValueIterator itr; // for arrays.

		DocumentTags::Tag searchAttr = DocumentTags::Document;
		String searchAttrValue;

		Node(DocumentTags::Tag group, rapidjson::Value::ConstMemberIterator node) : group(group), node(node), itr(node->value.IsArray() ? node->value.GetArray().End() : rapidjson::Value::ConstValueIterator()) {}
		Node(DocumentTags::Tag group, rapidjson::Value::ConstValueIterator itr) : group(group), node(rapidjson::Value::ConstMemberIterator()), itr(itr) {}

		const rapidjson::Value* GetValue() const { return node == rapidjson::Value::ConstMemberIterator() ? itr : &node->value; }
	};
	List<Node> _nodeStack;

	bool _open();
	void _free();

	template<typename T, typename S = T>
	bool _readData(T &data);

	template<typename T, typename S = T>
	bool _readData(T* data, uint32 size);

public:
	DocumentJSONLoader();
	~DocumentJSONLoader();

	bool IsBinary() const override { return false; }
	bool IsJson() const override { return true; }

	bool OpenFile(Path fileName) override;
	bool OpenMemory(DataBuffer&& data) override;

	bool Reset() override;

	bool VerifyGroup(DocumentTags::Tag group) override;
	bool EnterGroup(DocumentTags::Tag group) override;
	bool EnterGroup(DocumentTags::Tag group, DocumentTags::Tag attribute, String attrValue) override;
	bool LeaveGroup(DocumentTags::Tag group) override;

	bool ReadData(String& data) override;
	bool ReadData(bool &data) override { return _readData(data); }
	bool ReadData(float32 &data) override { return _readData(data); }
	bool ReadData(float64 &data) override { return _readData(data); }
	bool ReadData(int8 &data) override { return _readData<int8, int32>(data); }
	bool ReadData(uint8 &data) override { return _readData<uint8, uint32>(data); }
	bool ReadData(int16 &data) override { return _readData<int16, int32>(data); }
	bool ReadData(uint16 &data) override { return _readData<uint16, uint32>(data); }
	bool ReadData(int32 &data) override { return _readData(data); }
	bool ReadData(uint32 &data) override { return _readData(data); }
	bool ReadData(int64 &data) override { return _readData(data); }
	bool ReadData(uint64 &data) override { return _readData(data); }
	bool ReadData(void* data, uint32 size) override;
	bool ReadData(float32 *data, uint32 size) override { return _readData(data, size); }
	bool ReadData(float64 *data, uint32 size) override { return _readData(data, size); }
	bool ReadData(int8 *data, uint32 size) override { return _readData<int8, int32>(data, size); }
	bool ReadData(uint8 *data, uint32 size) override { return _readData<uint8, uint32>(data, size); }
	bool ReadData(int16 *data, uint32 size) override { return _readData<int16, int32>(data, size); }
	bool ReadData(uint16 *data, uint32 size) override { return _readData<uint16, uint32>(data, size); }
	bool ReadData(int32 *data, uint32 size) override { return _readData(data, size); }
	bool ReadData(uint32 *data, uint32 size) override { return _readData(data, size); }

	template<typename T>
	bool ReadData(T* data, uint32 size)
	{
		if (_nodeStack.empty() || _nodeStack.back().group != DocumentTags::Data)
			return false;
		bool ok = true;
		for (uint32 i = 0; i < size; i++) {
			ok = ok && EnterGroup(DocumentTags::Data);
			ok = ok && ((DocumentLoader*)this)->ReadData(data[i]);
			ok = ok && LeaveGroup(DocumentTags::Data);
		}

		return ok;
	}
};

}