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
#include "VectorChip.h"


namespace m3d
{


static const Guid VECTOROPERATOR_GUID = { 0x7454b3c5, 0xe34c, 0x4400, { 0xad, 0xfa, 0x30, 0x83, 0x41, 0x55, 0x63, 0xbd } };

class STDCHIPS_API VectorOperator : public VectorChip
{
	CHIPDESC_DECL;
public:
	enum class OperatorType {
		NONE, 
		ACCUMULATE,
		ADD,
		SUBTRACT,
		MULTIPLY,
		DIVIDE,

		ROTATE_YPR = 50, 
		ROTATE_QUAT, 

		TRANSFORM,
		TRANSFORM_COORD,
		TRANSFORM_NORMAL,

		NORMALIZE2 = 100,
		NORMALIZE3,
		NORMALIZE4,
		ORTHOGONAL2,
		ORTHOGONAL3,
		ORTHOGONAL4,
		CROSS2,
		CROSS3,
//		CROSS4,
		
		QUAT_YPR,
		QUAT_AXIS_ANGLE,
		QUAT_INVERSE,
		QUAT_SLERP,
		QUAT_NORMALIZE,
		QUAT_MATRIX,
		QUAT_CONJUGATE,
		QUAT_MULTIPLY,
		QUAT_SQUAD,
		QUAT_TO_AXIS_ANGLE,
		QUAT_TO_YPR,


//		REFLECT,
//		REFRACT,
//		LERP,
//		TO_AXIS_ANGLE,

		MATRIX_X_AXIS = 300,
		MATRIX_Y_AXIS,
		MATRIX_Z_AXIS,
		MATRIX_W_AXIS,
		MATRIX_TRANSLATION,
		MATRIX_ROTATION,
		MATRIX_SCALING

	};

	VectorOperator();
	virtual ~VectorOperator();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual const XMFLOAT4 &GetVector() override;

	virtual OperatorType GetOperatorType() const { return _ot; }
	virtual void SetOperatorType(OperatorType ot);

protected:
	OperatorType _ot;
};



}