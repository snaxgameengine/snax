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
#include "M3DEngine/Chip.h"
#include "ArrayChip.h"
#include "ValueDef.h"

namespace m3d
{


static const Guid VALUEARRAY_GUID = { 0x18f99330, 0xdd34, 0x46f6, { 0xa2, 0x6e, 0x4a, 0x29, 0x78, 0xe9, 0xc0, 0x77 } };

class STDCHIPS_API ValueArray : public ArrayChip
{
	CHIPDESC_DECL;
public:
	typedef List<value> ArrayType;

	ValueArray();
	virtual ~ValueArray();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual uint32 GetContainerSize() override;
	virtual void SetContainerSize(uint32 size) override;
	virtual void ClearContainer() override;

	virtual const ArrayType &GetArray() const;
	virtual void SetArray(const ArrayType &a);
	virtual void SetArray(ArrayType &&a);

	virtual value GetValue(uint32 index) const;
	virtual void SetValue(uint32 index, value v);

	virtual void Sort(int dir = 1, uint32 start = 0, uint32 count = -1);

protected:
	ArrayType _array; // Using float32 array because this is the gpu-standard.
};



}