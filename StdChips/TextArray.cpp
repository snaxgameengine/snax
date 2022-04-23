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
#include "TextArray.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"

using namespace m3d;


CHIPDESCV1_DEF(TextArray, MTEXT("Text Array"), TEXTARRAY_GUID, ARRAYCHIP_GUID);


TextArray::TextArray()
{
}

TextArray::~TextArray()
{
}

bool TextArray::CopyChip(Chip* chip)
{
	TextArray* c = dynamic_cast<TextArray*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_array = c->_array;
	return true;
}

bool TextArray::LoadChip(DocumentLoader& loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOADDEF(MTEXT("array"), _array, ArrayType());
	return true;
}

bool TextArray::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVEDEF(MTEXT("array"), _array, ArrayType());
	return true;
}

uint32 TextArray::GetContainerSize()
{
	return (uint32)_array.size();
}

void TextArray::SetContainerSize(uint32 size)
{
	_array.resize(size, String());
}

void TextArray::ClearContainer()
{
	_array.clear();
}

const TextArray::ArrayType& TextArray::GetArray()
{
	return _array;
}

void TextArray::SetArray(const ArrayType& a)
{
	_array = a;
}

void TextArray::SetArray(ArrayType&& a)
{
	_array = std::move(a);
}

void TextArray::GetText(uint32 index, String& v)
{
	v = _array[index];
}

void TextArray::SetText(uint32 index, const String& v)
{
	_array[index] = v;
}