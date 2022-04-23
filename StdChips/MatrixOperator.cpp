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
#include "MatrixOperator.h"
#include "VectorChip.h"
#include "Value.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"


using namespace m3d;


CHIPDESCV1_DEF(MatrixOperator, MTEXT("Matrix Operator"), MATRIXOPERATOR_GUID, MATRIXCHIP_GUID);


MatrixOperator::MatrixOperator()
{
	_ot = OperatorType::NONE;
}

MatrixOperator::~MatrixOperator()
{
}

bool MatrixOperator::CopyChip(Chip *chip)
{
	MatrixOperator *c = dynamic_cast<MatrixOperator*>(chip);
	B_RETURN(MatrixChip::CopyChip(c));
	SetOperatorType(c->_ot);
	return true;
}

bool MatrixOperator::LoadChip(DocumentLoader &loader)
{
	B_RETURN(MatrixChip::LoadChip(loader));
	OperatorType ot;
	LOAD(MTEXT("operatorType|ot"), ot);
	SetOperatorType(ot);
	return true;
}

bool MatrixOperator::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(MatrixChip::SaveChip(saver));
	SAVE(MTEXT("operatorType"), _ot);
	return true;
}

const XMFLOAT4X4 &MatrixOperator::GetMatrix()
{
	RefreshT refresh(Refresh);
	if (!refresh)
		return _matrix;

	switch (_ot) 
	{
	case OperatorType::ORTHO:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			ChildPtr<Value> ch3 = GetChild(3);
			float32 ViewWidth = ch0 ? (float32)ch0->GetValue() : -1.0f;
			float32 ViewHeight = ch1 ? (float32)ch1->GetValue() : 1.0f;
			float32 NearZ = ch2 ? (float32)ch2->GetValue() : -1.0f;
			float32 FarZ = ch3 ? (float32)ch3->GetValue() : 1.0f;

			if (!XMScalarNearEqual(ViewWidth, 0.0f, 0.00001f) && 
				!XMScalarNearEqual(ViewHeight, 0.0f, 0.00001f) &&
				!XMScalarNearEqual(FarZ, NearZ, 0.00001f))
				XMStoreFloat4x4(&_matrix, XMMatrixOrthographicLH(ViewWidth, ViewHeight, NearZ, FarZ));
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::ORTHO_OFF_CENTER:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			ChildPtr<Value> ch3 = GetChild(3);
			ChildPtr<Value> ch4 = GetChild(4);
			ChildPtr<Value> ch5 = GetChild(5);
			float32 ViewLeft = ch0 ? (float32)ch0->GetValue() : -1.0f;
			float32 ViewRight = ch1 ? (float32)ch1->GetValue() : 1.0f;
			float32 ViewBottom = ch2 ? (float32)ch2->GetValue() : -1.0f;
			float32 ViewTop = ch3 ? (float32)ch3->GetValue() : 1.0f;
			float32 NearZ = ch4 ? (float32)ch4->GetValue() : -1.0f;
			float32 FarZ = ch5 ? (float32)ch5->GetValue() : 1.0f;

			if (!XMScalarNearEqual(ViewRight, ViewLeft, 0.00001f) &&
				!XMScalarNearEqual(ViewTop, ViewBottom, 0.00001f) && 
				!XMScalarNearEqual(FarZ, NearZ, 0.00001f))
				XMStoreFloat4x4(&_matrix, XMMatrixOrthographicOffCenterLH(ViewLeft, ViewRight, ViewBottom, ViewTop, NearZ, FarZ));
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::PERSPECTIVE:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			ChildPtr<Value> ch3 = GetChild(3);
			float32 ViewWidth = ch0 ? (float32)ch0->GetValue() : -1.0f;
			float32 ViewHeight = ch1 ? (float32)ch1->GetValue() : 1.0f;
			float32 NearZ = ch2 ? (float32)ch2->GetValue() : 1.0f;
			float32 FarZ = ch3 ? (float32)ch3->GetValue() : 10.0f;

			if ((NearZ > 0.f && FarZ > 0.f) &&
				!XMScalarNearEqual(ViewWidth, 0.0f, 0.00001f) && 
				!XMScalarNearEqual(ViewHeight, 0.0f, 0.00001f) &&
				!XMScalarNearEqual(FarZ, NearZ, 0.00001f))
				XMStoreFloat4x4(&_matrix, XMMatrixPerspectiveLH(ViewWidth, ViewHeight, NearZ, FarZ));
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::PERSPECTIVE_FOVY:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			ChildPtr<Value> ch3 = GetChild(3);
			float32 FovAngleY = ch0 ? (float32)ch0->GetValue() : 1.0f;
			float32 AspectRatio = ch1 ? (float32)ch1->GetValue() : 1.0f;
			float32 NearZ = ch2 ? (float32)ch2->GetValue() : 1.0f;
			float32 FarZ = ch3 ? (float32)ch3->GetValue() : 10.0f;

			if ((NearZ > 0.f && FarZ > 0.f) && 
				!XMScalarNearEqual(FovAngleY, 0.0f, 0.00001f * 2.0f) && 
				!XMScalarNearEqual(AspectRatio, 0.0f, 0.00001f) && 
				!XMScalarNearEqual(FarZ, NearZ, 0.00001f))
				XMStoreFloat4x4(&_matrix, XMMatrixPerspectiveFovLH(FovAngleY, AspectRatio, NearZ, FarZ));
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::PERSPECTIVE_OFF_CENTER:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			ChildPtr<Value> ch3 = GetChild(3);
			ChildPtr<Value> ch4 = GetChild(4);
			ChildPtr<Value> ch5 = GetChild(5);
			float32 ViewLeft = ch0 ? (float32)ch0->GetValue() : -1.0f;
			float32 ViewRight = ch1 ? (float32)ch1->GetValue() : 1.0f;
			float32 ViewBottom = ch2 ? (float32)ch2->GetValue() : -1.0f;
			float32 ViewTop = ch3 ? (float32)ch3->GetValue() : 1.0f;
			float32 NearZ = ch4 ? (float32)ch4->GetValue() : 1.0f;
			float32 FarZ = ch5 ? (float32)ch5->GetValue() : 10.0f;

			if ((NearZ > 0.f && FarZ > 0.f) &&
				!XMScalarNearEqual(ViewRight, ViewLeft, 0.00001f) &&
				!XMScalarNearEqual(ViewTop, ViewBottom, 0.00001f) &&
				!XMScalarNearEqual(FarZ, NearZ, 0.00001f))
				XMStoreFloat4x4(&_matrix, XMMatrixPerspectiveOffCenterLH(ViewLeft, ViewRight, ViewBottom, ViewTop, NearZ, FarZ));
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::LOOK_AT:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<VectorChip> ch1 = GetChild(1);
			ChildPtr<VectorChip> ch2 = GetChild(2);
			XMVECTOR EyePosition = ch0 ? XMLoadFloat4(&ch0->GetVector()) : XMVectorZero();
			XMVECTOR FocusPosition = ch1 ? XMLoadFloat4(&ch1->GetVector()) : XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
			XMVECTOR UpDirection = ch2 ? XMLoadFloat4(&ch2->GetVector()) : XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			XMVECTOR EyeDirection = XMVectorSubtract(FocusPosition, EyePosition);

			if (!XMVector3Equal(EyeDirection, XMVectorZero()) && 
				!XMVector3IsInfinite(EyeDirection) &&
				!XMVector3Equal(UpDirection, XMVectorZero()) &&
				!XMVector3IsInfinite(UpDirection))
				XMStoreFloat4x4(&_matrix, XMMatrixLookToLH(EyePosition, EyeDirection, UpDirection));
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::TRANSLATION:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &f = ch0->GetVector();
				XMStoreFloat4x4(&_matrix, XMMatrixTranslation(f.x, f.y, f.z));
			}
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::ROTATION_PYR:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &f = ch0->GetVector();
				XMStoreFloat4x4(&_matrix, XMMatrixRotationRollPitchYaw(f.x, f.y, f.z));
			}
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::ROTATION_QUAT:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				XMStoreFloat4x4(&_matrix, XMMatrixRotationQuaternion(XMLoadFloat4(&p0)));
			}
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::SCALING:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &f = ch0->GetVector();
				XMStoreFloat4x4(&_matrix, XMMatrixScaling(f.x, f.y, f.z));
			}
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::INVERSE:
		{
			ChildPtr<MatrixChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4X4 &m = ch0->GetMatrix();
				XMVECTOR v;
				XMStoreFloat4x4(&_matrix, XMMatrixInverse(&v, XMLoadFloat4x4(&m)));
			}
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::ROTATION_AXIS:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				const XMFLOAT4 &m = ch0->GetVector();
				float32 v = (float32)ch1->GetValue();
				XMStoreFloat4x4(&_matrix, XMMatrixRotationAxis(XMLoadFloat4(&m), v));
			}
			else
				_matrix = IDENTITY;

		}
	case OperatorType::REFLECT:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &v = ch0->GetVector();
				XMStoreFloat4x4(&_matrix, XMMatrixReflect(XMLoadFloat4(&v)));
			}
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::SHADOW:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<VectorChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				const XMFLOAT4 &v0 = ch0->GetVector();
				const XMFLOAT4 &v1 = ch1->GetVector();
				XMStoreFloat4x4(&_matrix, XMMatrixShadow(XMLoadFloat4(&v0), XMLoadFloat4(&v1)));
			}
			else
				_matrix = IDENTITY;
		}
	case OperatorType::TRANSPOSE:
		{
			ChildPtr<MatrixChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4X4 &m = ch0->GetMatrix();
				XMStoreFloat4x4(&_matrix, XMMatrixTranspose(XMLoadFloat4x4(&m)));
			}
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::MULTIPLY:
		{
			ChildPtr<MatrixChip> ch0 = GetChild(0);
			ChildPtr<MatrixChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				const XMFLOAT4X4 &m0 = ch0->GetMatrix();
				const XMFLOAT4X4 &m1 = ch1->GetMatrix();
				XMStoreFloat4x4(&_matrix, XMMatrixMultiply(XMLoadFloat4x4(&m0), XMLoadFloat4x4(&m1)));
			}
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::MULTIPLY_TRANSPOSE:
		{
			ChildPtr<MatrixChip> ch0 = GetChild(0);
			ChildPtr<MatrixChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				const XMFLOAT4X4 &m0 = ch0->GetMatrix();
				const XMFLOAT4X4 &m1 = ch1->GetMatrix();
				XMStoreFloat4x4(&_matrix, XMMatrixMultiplyTranspose(XMLoadFloat4x4(&m0), XMLoadFloat4x4(&m1)));
			}
			else
				_matrix = IDENTITY;
		}
		break;
	case OperatorType::FROM_VECTORS:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<VectorChip> ch1 = GetChild(1);
			ChildPtr<VectorChip> ch2 = GetChild(2);
			ChildPtr<VectorChip> ch3 = GetChild(3);
			XMFLOAT4 x = ch0 ? ch0->GetVector() : XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
			XMFLOAT4 y = ch1 ? ch1->GetVector() : XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
			XMFLOAT4 z = ch2 ? ch2->GetVector() : XMFLOAT4(0.0f, 0.0f, 1.0f, 0.0f);
			XMFLOAT4 w = ch3 ? ch3->GetVector() : XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			_matrix = XMFLOAT4X4(x.x, x.y, x.z, x.w, y.x, y.y, y.z, y.w, z.x, z.y, z.z, z.w, w.x, w.y, w.z, w.w);
		}
		break;
	default:
		AddMessage(UninitializedException());
		break;
	};
	return _matrix;
}

void MatrixOperator::SetOperatorType(OperatorType ot)
{
	if (_ot == ot)
		return;
	_ot = ot;

	switch (_ot) 
	{
	case OperatorType::ORTHO:
	case OperatorType::PERSPECTIVE:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("Width"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Height"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("Near"));
		CREATE_CHILD_KEEP(3, VALUE_GUID, false, UP, MTEXT("Far"));
		ClearConnections(4);
		break;
	case OperatorType::ORTHO_OFF_CENTER:
	case OperatorType::PERSPECTIVE_OFF_CENTER:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("Left"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Right"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("Bottom"));
		CREATE_CHILD_KEEP(3, VALUE_GUID, false, UP, MTEXT("Top"));
		CREATE_CHILD_KEEP(4, VALUE_GUID, false, UP, MTEXT("Near"));
		CREATE_CHILD_KEEP(5, VALUE_GUID, false, UP, MTEXT("Far"));
		ClearConnections(6);
		break;
	case OperatorType::PERSPECTIVE_FOVY:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("FOVY"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Aspect Ratio"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("Near"));
		CREATE_CHILD_KEEP(3, VALUE_GUID, false, UP, MTEXT("Far"));
		ClearConnections(4);
		break;
	case OperatorType::LOOK_AT:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Eye Position"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Focus Point"));
		CREATE_CHILD_KEEP(2, VECTORCHIP_GUID, false, UP, MTEXT("Up Direction"));
		ClearConnections(3);
		break;
	case OperatorType::TRANSLATION:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Translation"));
		ClearConnections(1);
		break;
	case OperatorType::ROTATION_PYR:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Rotation (PYR)"));
		ClearConnections(1);
		break;
	case OperatorType::ROTATION_QUAT:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Rotation (Quat)"));
		ClearConnections(1);
		break;
	case OperatorType::SCALING:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Scaling"));
		ClearConnections(1);
		break;
	case OperatorType::INVERSE:
		CREATE_CHILD_KEEP(0, MATRIXCHIP_GUID, false, UP, MTEXT("Matrix"));
		ClearConnections(1);
		break;
	case OperatorType::ROTATION_AXIS:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Axis"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Angle"));
		ClearConnections(2);
		break;
	case OperatorType::REFLECT:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Reflection Plane"));
		ClearConnections(1);
		break;
	case OperatorType::SHADOW:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Shadow Plane"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Light Position"));
		ClearConnections(2);
		break;
	case OperatorType::TRANSPOSE:
		CREATE_CHILD_KEEP(0, MATRIXCHIP_GUID, false, UP, MTEXT("Matrix"));
		ClearConnections(1);
		break;
	case OperatorType::MULTIPLY:
		CREATE_CHILD_KEEP(0, MATRIXCHIP_GUID, false, UP, MTEXT("Matrix 1"));
		CREATE_CHILD_KEEP(1, MATRIXCHIP_GUID, false, UP, MTEXT("Matrix 2"));
		ClearConnections(2);
		break;
	case OperatorType::MULTIPLY_TRANSPOSE:
		CREATE_CHILD_KEEP(0, MATRIXCHIP_GUID, false, UP, MTEXT("Matrix 1"));
		CREATE_CHILD_KEEP(1, MATRIXCHIP_GUID, false, UP, MTEXT("Matrix 2"));
		ClearConnections(2);
		break;
	case OperatorType::FROM_VECTORS:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("x-Axis"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("y-Axis"));
		CREATE_CHILD_KEEP(2, VECTORCHIP_GUID, false, UP, MTEXT("z-Axis"));
		CREATE_CHILD_KEEP(3, VECTORCHIP_GUID, false, UP, MTEXT("w-Axis"));
		ClearConnections(4);
		break;
	default:
		ClearConnections();
		break;
	};
	RemoveMessage(UninitializedException());
}