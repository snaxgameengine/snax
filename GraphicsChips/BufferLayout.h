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
#include "M3DCore/MString.h"
#include "M3DCore/Containers.h"
#include <d3d12.h>
#include <DirectXMath.h>


namespace m3d
{


class GRAPHICSCHIPS_API BufferLayout
{
public:
	struct Item
	{
		enum Type { VALUE, VECTOR, MATRIX, VALUE_ARRAY, VECTOR_ARRAY, MATRIX_ARRAY };
		enum struct DataType { BOOL, INT, UINT, FLOAT, DOUBLE };
		Type type;
		DataType dt;
		UINT rows; // Number of rows (for matrices, 1 for other numeric, 0 if not applicable)
		UINT columns; // Number of columns (for vectors & matrices, 1 for other numeric, 0 if not applicable)
		UINT elements; // Number of elements (0 if not an array)
		UINT64 offset; // Offset from the start of structure
		UINT size; // Number of bytes for this item
		String name; // Case sensitive!

		// Example:
		// float3x2: 3 rows & 2 columns.
		// float3x2: 3x float2: Size will be 28 as second column starts a new vector at 16 bytes.
		// double3x2: 3x double2 Size will be 56 bytes as second column starts a new vector at 32 bytes.

		Item() : type(VALUE), dt(DataType::BOOL), rows(0), columns(0), elements(0), offset(0), size(0) {}
	};

	BufferLayout();
	~BufferLayout();

	// Ex:
	// Value specularStrength
	// Vector diffuseColor : offset(64)
	// Matrix skinning[32]
	bool Init(String config, String *err = nullptr);

	UINT64 GetBufferSize() const;
	String GetConfig() const;
	String GetStruct(String name) const;

	const List<Item> &GetItems() const { return _items; }

	bool IsSubsetOf(const BufferLayout &bl, String &msg) const;
	bool IsCompatible(const BufferLayout &bl) const;
	bool IsEqual(const BufferLayout &bl) const;

	BufferLayout Merge(const BufferLayout &bl) const;

	uint32 GetNumberOf(Item::Type type) const;
	String GetNameOf(Item::Type type, uint32 idx) const;
	uint32 GetElementsInArray(Item::Type type, uint32 idx) const;

	uint32 GetNumValues() const { return GetNumberOf(Item::VALUE); }
	String GetValueName(uint32 idx) const { return GetNameOf(Item::VALUE, idx); }
	void SetValue(uint32 idx, BYTE *buffer, const double *v) const;

	uint32 GetNumVectors() const { return GetNumberOf(Item::VECTOR); }
	String GetVectorName(uint32 idx) const { return GetNameOf(Item::VECTOR, idx); }
	void SetVector(uint32 idx, BYTE *buffer, const XMFLOAT4 *v) const;

	uint32 GetNumMatrices() const { return GetNumberOf(Item::MATRIX); }
	String GetMatrixName(uint32 idx) const { return GetNameOf(Item::MATRIX, idx); }
	void SetMatrix(uint32 idx, BYTE *buffer, const XMFLOAT4X4 *m) const;

	uint32 GetNumValueArrays() const { return GetNumberOf(Item::VALUE_ARRAY); }
	String GetValueArrayName(uint32 idx) const { return GetNameOf(Item::VALUE_ARRAY, idx); }
	uint32 GetValueArraySize(uint32 idx) const { return GetElementsInArray(Item::VALUE_ARRAY, idx); }
	void SetValueArray(uint32 idx, BYTE *buffer, uint32 count, const double *v) const;

	uint32 GetNumVectorArrays() const { return GetNumberOf(Item::VECTOR_ARRAY); }
	String GetVectorArrayName(uint32 idx) const { return GetNameOf(Item::VECTOR_ARRAY, idx); }
	uint32 GetVectorArraySize(uint32 idx) const { return GetElementsInArray(Item::VECTOR_ARRAY, idx); }
	void SetVectorArray(uint32 idx, BYTE *buffer, uint32 count, const XMFLOAT4 *v) const;

	uint32 GetNumMatrixArrays() const { return GetNumberOf(Item::MATRIX_ARRAY); }
	String GetMatrixArrayName(uint32 idx) const { return GetNameOf(Item::MATRIX_ARRAY, idx); }
	uint32 GetMatrixArraySize(uint32 idx) const { return GetElementsInArray(Item::MATRIX_ARRAY, idx); }
	void SetMatrixArray(uint32 idx, BYTE *buffer, uint32 count, const XMFLOAT4X4 *m) const;

private:
	List<Item> _items;
	List<uint32> _itemsByType[6];
	UINT64 _sizeInBytes;

};

typedef uint32 BufferLayoutID;
#define InvalidBufferLayoutID (BufferLayoutID)0

class BufferLayoutManager
{
public:
	BufferLayoutID RegisterLayout(String config, String *err = nullptr);
	BufferLayoutID RegisterLayout(const BufferLayout &bl, String *err = nullptr) { return RegisterLayout(bl.GetConfig(), err); }
	const BufferLayout *GetLayout(BufferLayoutID id);

	bool IsSubsetOf(BufferLayoutID isThis, BufferLayoutID subsetOfThis, String &msg);

	static BufferLayoutManager &GetInstance();

private:
	BufferLayoutManager();
	~BufferLayoutManager();

	Map<String, BufferLayoutID> _layoutMap;
	List<BufferLayout> _layouts;
	struct SC
	{
		bool b;
		String msg;
	};
	Map<UINT64, SC> _subsetCache;
};


}