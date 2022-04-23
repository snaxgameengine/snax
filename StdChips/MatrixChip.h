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
#include "Numeric.h"


namespace m3d
{


static const Guid MATRIXCHIP_GUID = { 0x01466aab, 0x80e4, 0x447c, { 0x96, 0x62, 0x08, 0x9b, 0x79, 0x48, 0x52, 0x31 } };

class STDCHIPS_API MatrixChip : public Numeric
{
	CHIPDESC_DECL;
public:
	static const XMFLOAT4X4 IDENTITY;

	MatrixChip();
	virtual ~MatrixChip();

	virtual bool InitChip() override;
	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual void CallChip() override;

	MatrixChip *AsMatrix() override { return this; }

	virtual const XMFLOAT4X4 &GetMatrix();
	virtual void SetMatrix(const XMFLOAT4X4 &m);

	// These are utility functions that by default uses matrix from GetMatrix().
	// Some special Matrix-derivatives can overload these to do the calculation in a wiser manner. maybe... :/
	virtual bool GetInverseMatrix(XMFLOAT4X4 &invertedMatrix);
	virtual bool GetDecomposedMatrix(XMFLOAT3 &translation, XMFLOAT4 &rotation, XMFLOAT3 &scaling);

	virtual const XMFLOAT4X4 &GetChipMatrix() const { return _matrix; }

protected:
	XMFLOAT4X4 _matrix;
};



}