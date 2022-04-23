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
#include "VectorOperator_dlg.h"

using namespace m3d;


DIALOGDESC_DEF(VectorOperator_Dlg, VECTOROPERATOR_GUID);



void VectorOperator_Dlg::Init()
{
	RPtr vectoro = AddItem(MTEXT("Vector Operations"), (uint32)VectorOperator::OperatorType::NONE);
	AddItem(vectoro, MTEXT("Rotate Using Euler Angles"), (uint32)VectorOperator::OperatorType::ROTATE_YPR);
	AddItem(vectoro, MTEXT("Rotate Using Quaternion"), (uint32)VectorOperator::OperatorType::ROTATE_QUAT);
	AddItem(vectoro, MTEXT("Get Orthogonal Vector 2"), (uint32)VectorOperator::OperatorType::ORTHOGONAL2);
	AddItem(vectoro, MTEXT("Get Orthogonal Vector 3"), (uint32)VectorOperator::OperatorType::ORTHOGONAL3);
	AddItem(vectoro, MTEXT("Get Orthogonal Vector 4"), (uint32)VectorOperator::OperatorType::ORTHOGONAL4);

	AddItem(vectoro, MTEXT("Accumulate"), (uint32)VectorOperator::OperatorType::ACCUMULATE);
	AddItem(vectoro, MTEXT("Add Vectors"), (uint32)VectorOperator::OperatorType::ADD);
	AddItem(vectoro, MTEXT("Subtract Vectors"), (uint32)VectorOperator::OperatorType::SUBTRACT);
	AddItem(vectoro, MTEXT("Multiply Vectors"), (uint32)VectorOperator::OperatorType::MULTIPLY);
	AddItem(vectoro, MTEXT("Divide Vectors"), (uint32)VectorOperator::OperatorType::DIVIDE);

	AddItem(vectoro, MTEXT("Transform"), (uint32)VectorOperator::OperatorType::TRANSFORM);
	AddItem(vectoro, MTEXT("Transform Coord"), (uint32)VectorOperator::OperatorType::TRANSFORM_COORD);
	AddItem(vectoro, MTEXT("Transform Normal"), (uint32)VectorOperator::OperatorType::TRANSFORM_NORMAL);

	AddItem(vectoro, MTEXT("Normalize Vector 2"), (uint32)VectorOperator::OperatorType::NORMALIZE2);
	AddItem(vectoro, MTEXT("Normalize Vector 3"), (uint32)VectorOperator::OperatorType::NORMALIZE3);
	AddItem(vectoro, MTEXT("Normalize Vector 4"), (uint32)VectorOperator::OperatorType::NORMALIZE4);

	AddItem(vectoro, MTEXT("Get Vector 2 Cross Product"), (uint32)VectorOperator::OperatorType::CROSS2);
	AddItem(vectoro, MTEXT("Get Vector 3 Cross Product"), (uint32)VectorOperator::OperatorType::CROSS3);

	RPtr quat = AddItem(MTEXT("Quaternion Operations"), (uint32)VectorOperator::OperatorType::NONE);
	AddItem(quat, MTEXT("Create Quaternion From Euler Angles"), (uint32)VectorOperator::OperatorType::QUAT_YPR);
	AddItem(quat, MTEXT("Create Quaternion From Axis and Angle"), (uint32)VectorOperator::OperatorType::QUAT_AXIS_ANGLE);
	AddItem(quat, MTEXT("Invert Quaternion"), (uint32)VectorOperator::OperatorType::QUAT_INVERSE);
	AddItem(quat, MTEXT("SLERP Quaternions"), (uint32)VectorOperator::OperatorType::QUAT_SLERP);
	AddItem(quat, MTEXT("Normalize Quaternion"), (uint32)VectorOperator::OperatorType::QUAT_NORMALIZE);
	AddItem(quat, MTEXT("Create Quaternion From Matrix"), (uint32)VectorOperator::OperatorType::QUAT_MATRIX);
	AddItem(quat, MTEXT("Get Conjugate of Quaternion"), (uint32)VectorOperator::OperatorType::QUAT_CONJUGATE);
	AddItem(quat, MTEXT("Multiply Quaternions"), (uint32)VectorOperator::OperatorType::QUAT_MULTIPLY);
	AddItem(quat, MTEXT("Interpolates Between 4 Unit Quaternions"), (uint32)VectorOperator::OperatorType::QUAT_SQUAD);
	AddItem(quat, MTEXT("Get Rotation Axis (xyz) and Angle (w) From Quaternion"), (uint32)VectorOperator::OperatorType::QUAT_TO_AXIS_ANGLE);
	AddItem(quat, MTEXT("Get Euler Angles From Quaternion"), (uint32)VectorOperator::OperatorType::QUAT_TO_YPR);
	
	RPtr mat = AddItem(MTEXT("Matrix Operations"), (uint32)VectorOperator::OperatorType::NONE);
	AddItem(mat, MTEXT("Get Matrix X-Axis"), (uint32)VectorOperator::OperatorType::MATRIX_X_AXIS);
	AddItem(mat, MTEXT("Get Matrix Y-Axis"), (uint32)VectorOperator::OperatorType::MATRIX_Y_AXIS);
	AddItem(mat, MTEXT("Get Matrix Z-Axis"), (uint32)VectorOperator::OperatorType::MATRIX_Z_AXIS);
	AddItem(mat, MTEXT("Get Matrix W-Axis"), (uint32)VectorOperator::OperatorType::MATRIX_W_AXIS);
	AddItem(mat, MTEXT("Get Matrix Translation"), (uint32)VectorOperator::OperatorType::MATRIX_TRANSLATION);
	AddItem(mat, MTEXT("Get Matrix Rotation (As Quaternion)"), (uint32)VectorOperator::OperatorType::MATRIX_ROTATION);
	AddItem(mat, MTEXT("Get Matrix Scaling"), (uint32)VectorOperator::OperatorType::MATRIX_SCALING);

	sort();

	SetSelectionChangedCallback([this](RData data) -> bool {
		VectorOperator::OperatorType cmd = (VectorOperator::OperatorType)data;
		if (cmd == GetChip()->GetOperatorType())
			return false;
		GetChip()->SetOperatorType(cmd);
		return true;
		});

	SetInit((uint32)GetChip()->GetOperatorType(), (uint32)VectorOperator::OperatorType::NONE);
}
