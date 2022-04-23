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
#include "MatrixChip.h"


namespace m3d
{


static const Guid MATRIXOPERATOR_GUID = { 0x8dd16693, 0x4c81, 0x481b, { 0x91, 0x2b, 0xd0, 0x3b, 0x7c, 0x66, 0x4c, 0xb9 } };

class STDCHIPS_API MatrixOperator : public MatrixChip
{
	CHIPDESC_DECL;
public:
	enum class OperatorType 
	{ 
		NONE, 
		ORTHO, 
		ORTHO_OFF_CENTER, 
		PERSPECTIVE, 
		PERSPECTIVE_FOVY, 
		PERSPECTIVE_OFF_CENTER,
		LOOK_AT, 
		TRANSLATION,
		ROTATION_PYR,
		ROTATION_QUAT,
		SCALING, 
		INVERSE,
		ROTATION_AXIS,
		REFLECT,
		SHADOW,
		TRANSPOSE,
		MULTIPLY,
		MULTIPLY_TRANSPOSE,
		FROM_VECTORS
	};

	MatrixOperator();
	virtual ~MatrixOperator();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual const XMFLOAT4X4 &GetMatrix() override;

	virtual OperatorType GetOperatorType() const { return _ot; }
	virtual void SetOperatorType(OperatorType ot);

protected:
	OperatorType _ot;
};



}