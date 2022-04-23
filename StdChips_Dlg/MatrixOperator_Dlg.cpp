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

#include "StdAfx.h"
#include "MatrixOperator_dlg.h"

using namespace m3d;


DIALOGDESC_DEF(MatrixOperator_Dlg, MATRIXOPERATOR_GUID);


void MatrixOperator_Dlg::Init()
{
	RPtr projectionMatrix = AddItem(MTEXT("Projection Matrices"), (uint32)MatrixOperator::OperatorType::NONE);
	AddItem(projectionMatrix, MTEXT("Ortho"), (uint32)MatrixOperator::OperatorType::ORTHO);
	AddItem(projectionMatrix, MTEXT("Ortho Off Center"), (uint32)MatrixOperator::OperatorType::ORTHO_OFF_CENTER);
	AddItem(projectionMatrix, MTEXT("Perspective"), (uint32)MatrixOperator::OperatorType::PERSPECTIVE);
	AddItem(projectionMatrix, MTEXT("Perspective FOVY"), (uint32)MatrixOperator::OperatorType::PERSPECTIVE_FOVY);
	AddItem(projectionMatrix, MTEXT("Perspective Off Center"), (uint32)MatrixOperator::OperatorType::PERSPECTIVE_OFF_CENTER);
	RPtr transformations = AddItem(MTEXT("Transformation Matrices"), (uint32)MatrixOperator::OperatorType::NONE);
	AddItem(transformations, MTEXT("Look At"), (uint32)MatrixOperator::OperatorType::LOOK_AT);
	AddItem(transformations, MTEXT("Translation"), (uint32)MatrixOperator::OperatorType::TRANSLATION);
	AddItem(transformations, MTEXT("Rotation (Pitch, Yaw, Roll)"), (uint32)MatrixOperator::OperatorType::ROTATION_PYR);
	AddItem(transformations, MTEXT("Rotation (Quaternion)"), (uint32)MatrixOperator::OperatorType::ROTATION_QUAT);
	AddItem(transformations, MTEXT("Rotation (Axis, Angle)"), (uint32)MatrixOperator::OperatorType::ROTATION_AXIS);
	AddItem(transformations, MTEXT("Scaling"), (uint32)MatrixOperator::OperatorType::SCALING);
	AddItem(transformations, MTEXT("Reflection"), (uint32)MatrixOperator::OperatorType::REFLECT);
	AddItem(transformations, MTEXT("Shadow"), (uint32)MatrixOperator::OperatorType::SHADOW);

	RPtr matops = AddItem(MTEXT("Matrix Operations"), (uint32)MatrixOperator::OperatorType::NONE);
	AddItem(matops, MTEXT("Invert"), (uint32)MatrixOperator::OperatorType::INVERSE);
	AddItem(matops, MTEXT("Transpose"), (uint32)MatrixOperator::OperatorType::TRANSPOSE);
	AddItem(matops, MTEXT("Multiply"), (uint32)MatrixOperator::OperatorType::MULTIPLY);
	AddItem(matops, MTEXT("Multiply Transpose"), (uint32)MatrixOperator::OperatorType::MULTIPLY_TRANSPOSE);
	AddItem(matops, MTEXT("From Vectors"), (uint32)MatrixOperator::OperatorType::FROM_VECTORS);

	sort();

	SetSelectionChangedCallback([this](RData data) -> bool {
		MatrixOperator::OperatorType cmd = (MatrixOperator::OperatorType)data;
		if (cmd == GetChip()->GetOperatorType())
			return false;
		GetChip()->SetOperatorType(cmd);
		return true;
		});

	SetInit((uint32)GetChip()->GetOperatorType(), (uint32)MatrixOperator::OperatorType::NONE);
}
