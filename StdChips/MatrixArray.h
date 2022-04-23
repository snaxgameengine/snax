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
#include "M3DCore/MMath.h"
#include "M3DEngine/Chip.h"
#include "ArrayChip.h"
#include "MatrixChip.h"

namespace m3d
{


static const Guid MATRIXARRAY_GUID = { 0xff95e39a, 0xd1c8, 0x4cfc, { 0x8a, 0x39, 0x95, 0x46, 0xe4, 0x15, 0x48, 0xee } };

class STDCHIPS_API MatrixArray : public ArrayChip
{
	CHIPDESC_DECL;
public:
	typedef List<XMFLOAT4X4> ArrayType;

	MatrixArray();
	virtual ~MatrixArray();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual uint32 GetContainerSize() override;
	virtual void SetContainerSize(uint32 size) override;
	virtual void ClearContainer() override;

	virtual const ArrayType &GetArray() const { return _array; }
	virtual ArrayType &GetArray() { return _array; }

	virtual void SetArray(const ArrayType &a);
	virtual void SetArray(ArrayType &&a);

	virtual const XMFLOAT4X4 &GetMatrix(uint32 index);
	virtual void SetMatrix(uint32 index, const XMFLOAT4X4 &m);

protected:
	ArrayType _array;

};



}