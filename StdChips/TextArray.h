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


#include "ArrayChip.h"


namespace m3d
{


static const Guid TEXTARRAY_GUID = { 0xdad36f05, 0x3de1, 0x4d76, { 0xaa, 0x36, 0x65, 0x5c, 0x26, 0x3d, 0xa6, 0xba } };


class STDCHIPS_API TextArray : public ArrayChip
{
	CHIPDESC_DECL;
public:
	typedef List<String> ArrayType;

	TextArray();
	~TextArray();
	
	bool CopyChip(Chip* chip) override;
	bool LoadChip(DocumentLoader& loader) override;
	bool SaveChip(DocumentSaver& saver) const override;

	// Inherited from ArrayChip
	uint32 GetContainerSize() override;
	void SetContainerSize(uint32 size) override;
	void ClearContainer() override;

	virtual const ArrayType& GetArray();
	virtual void SetArray(const ArrayType& a);
	virtual void SetArray(ArrayType&& a);

	virtual void GetText(uint32 index, String& v);
	virtual void SetText(uint32 index, const String& v);

protected:
	ArrayType _array;
};



}
