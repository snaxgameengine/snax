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
#include "MatrixArray.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"

using namespace m3d;


CHIPDESCV1_DEF(MatrixArray, MTEXT("Matrix Array"), MATRIXARRAY_GUID, ARRAYCHIP_GUID);


MatrixArray::MatrixArray()
{
}

MatrixArray::~MatrixArray()
{
}

bool MatrixArray::CopyChip(Chip *chip)
{
	MatrixArray *c = dynamic_cast<MatrixArray*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_array = c->_array;
	return true;
}

bool MatrixArray::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOAD(MTEXT("array"), _array);
	return true;
}

bool MatrixArray::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE(MTEXT("array"), _array);
	return true;
}

uint32 MatrixArray::GetContainerSize()
{ 
	return (uint32)_array.size(); 
}

void MatrixArray::SetContainerSize(uint32 size)
{ 
	_array.resize(size, MatrixChip::IDENTITY); 
}

void MatrixArray::ClearContainer() 
{ 
	_array.clear(); 
}

void MatrixArray::SetArray(const ArrayType &a)
{ 
	_array = a; 
}

void MatrixArray::SetArray(ArrayType&&a)
{ 
	_array = std::move(a); 
}

const XMFLOAT4X4 &MatrixArray::GetMatrix(uint32 index) 
{ 
	return _array[index]; 
}

void MatrixArray::SetMatrix(uint32 index, const XMFLOAT4X4 &m) 
{ 
	_array[index] = m; 
}