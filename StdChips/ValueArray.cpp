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
#include "ValueArray.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include <algorithm>

using namespace m3d;


CHIPDESCV1_DEF(ValueArray, MTEXT("Value Array"), VALUEARRAY_GUID, ARRAYCHIP_GUID);


ValueArray::ValueArray()
{
}

ValueArray::~ValueArray()
{
}

bool ValueArray::CopyChip(Chip *chip)
{
	ValueArray *c = dynamic_cast<ValueArray*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_array = c->_array;
	return true;
}

bool ValueArray::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	// TODO: Remove array32
	List<float32> arr;
	if (loader.LoadData(MTEXT("array32"), arr)) { // float32 (TMP)
		_array.clear();
		_array.resize(arr.size());
		for (size_t i = 0; i < arr.size(); i++)
			_array[i] = arr[i];
	}
	else {
		LOADDEF(MTEXT("array"), _array, ArrayType()); // float64
	}
	return true;
}

bool ValueArray::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVEDEF(MTEXT("array"), _array, ArrayType());
	return true;
}

uint32 ValueArray::GetContainerSize() 
{
	return (uint32)_array.size(); 
}

void ValueArray::SetContainerSize(uint32 size) 
{ 
	_array.resize(size, 0.0); 
}

void ValueArray::ClearContainer() 
{ 
	_array.clear(); 
}

const ValueArray::ArrayType &ValueArray::GetArray() const 
{ 
	return _array; 
}

void ValueArray::SetArray(const ValueArray::ArrayType &a) 
{ 
	_array = a; 
}

void ValueArray::SetArray(ValueArray::ArrayType &&a) 
{ 
	_array = std::move(a); 
}

value ValueArray::GetValue(uint32 index) const 
{ 
	return (value)_array[index]; 
}

void ValueArray::SetValue(uint32 index, value v) 
{ 
	_array[index] = (float32)v; 
}

void ValueArray::Sort(int dir, uint32 start, uint32 count)
{
	if (_array.empty() || start >= _array.size() - 1 || count == 0 || dir == 0)
		return;
	count = std::min(count, (uint32)_array.size() - start);

	if (dir > 0)
		std::sort(&_array[start], &_array[start + count - 1] + 1, std::less<value>());
	else
		std::sort(&_array[start], &_array[start + count - 1] + 1, std::greater<value>());
}