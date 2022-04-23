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
#include "VectorOperator.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Engine.h"
#include "Value.h"
#include "MatrixChip.h"

using namespace m3d;


CHIPDESCV1_DEF(VectorOperator, MTEXT("Vector Operator"), VECTOROPERATOR_GUID, VECTORCHIP_GUID);


VectorOperator::VectorOperator()
{
	ClearConnections();
	_ot = OperatorType::NONE;
}

VectorOperator::~VectorOperator()
{
}

bool VectorOperator::CopyChip(Chip *chip)
{
	VectorOperator *c = dynamic_cast<VectorOperator*>(chip);
	B_RETURN(VectorChip::CopyChip(c));
	SetOperatorType(c->_ot);
	return true;
}

bool VectorOperator::LoadChip(DocumentLoader &loader) 
{ 
	B_RETURN(VectorChip::LoadChip(loader));
	OperatorType ot;
	LOAD(MTEXT("operatorType|ot"), ot);
	SetOperatorType(ot);
	return true;
}

bool VectorOperator::SaveChip(DocumentSaver &saver) const 
{
	B_RETURN(VectorChip::SaveChip(saver));
	SAVE(MTEXT("operatorType"), _ot);
	return true;
}

void _getScaling(const XMFLOAT4X4 &m, XMFLOAT4 &res)
{
	res.x = XMVectorGetX(XMVector3Length(XMLoadFloat4((XMFLOAT4*)&m+0)));
	res.y = XMVectorGetX(XMVector3Length(XMLoadFloat4((XMFLOAT4*)&m+1)));
	res.z = XMVectorGetX(XMVector3Length(XMLoadFloat4((XMFLOAT4*)&m+2)));
	res.w = 0.0f;
}

const XMFLOAT4 &VectorOperator::GetVector()
{
	RefreshT refresh(Refresh);
	if (!refresh)
		return _vector;

	switch (_ot) 
	{
	case OperatorType::ACCUMULATE:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();

				XMStoreFloat4(&_vector, XMVectorAdd(XMLoadFloat4(&_vector), XMLoadFloat4(&p0)));
			}
		}
		break;
	case OperatorType::ADD:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			XMVECTOR v = XMVectorZero();
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				v = XMLoadFloat4(&p0);
			}
			for (uint32 i = 0, j = GetSubConnectionCount(1); i < j; i++) {
				ChildPtr<VectorChip> ch1 = GetChild(1, i);
				if (ch1) {
					const XMFLOAT4 &p1 = ch1->GetVector();
					v = XMVectorAdd(v, XMLoadFloat4(&p1));
				}
			}
			XMStoreFloat4(&_vector, v);
		}
		break;
	case OperatorType::SUBTRACT:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			XMVECTOR v = XMVectorZero();
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				v = XMLoadFloat4(&p0);
			}
			for (uint32 i = 0, j = GetSubConnectionCount(1); i < j; i++) {
				ChildPtr<VectorChip> ch1 = GetChild(1, i);
				if (ch1) {
					const XMFLOAT4 &p1 = ch1->GetVector();
					v = XMVectorSubtract(v, XMLoadFloat4(&p1));
				}
			}
			XMStoreFloat4(&_vector, v);
		}
		break;
	case OperatorType::MULTIPLY:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			XMVECTOR v = XMVectorZero();
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				v = XMLoadFloat4(&p0);
			}
			for (uint32 i = 0, j = GetSubConnectionCount(1); i < j; i++) {
				ChildPtr<VectorChip> ch1 = GetChild(1, i);
				if (ch1) {
					const XMFLOAT4 &p1 = ch1->GetVector();
					v = XMVectorMultiply(v, XMLoadFloat4(&p1));
				}
			}
			XMStoreFloat4(&_vector, v);
		}
		break;
	case OperatorType::DIVIDE:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			XMVECTOR v = XMVectorZero();
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				v = XMLoadFloat4(&p0);
			}
			for (uint32 i = 0, j = GetSubConnectionCount(1); i < j; i++) {
				ChildPtr<VectorChip> ch1 = GetChild(1, i);
				if (ch1) {
					const XMFLOAT4 &p1 = ch1->GetVector();
					v = XMVectorDivide(v, XMLoadFloat4(&p1));
				}
			}
			XMStoreFloat4(&_vector, v);
		}
		break;
	case OperatorType::ROTATE_YPR: 
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<VectorChip> ch1 = GetChild(1);
			if (ch0)
				_vector = ch0->GetVector();
			if (ch1) {
				const XMFLOAT4 &f = ch1->GetVector();
				XMStoreFloat4(&_vector, XMVector3Rotate(XMLoadFloat4(&_vector), XMQuaternionRotationRollPitchYaw(f.x, f.y, f.z)));
			}
		} 
		break;
	case OperatorType::ROTATE_QUAT: 
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<VectorChip> ch1 = GetChild(1);
			if (ch0)
				_vector = ch0->GetVector();
			if (ch1) {
				const XMFLOAT4 &f = ch1->GetVector();
				XMStoreFloat4(&_vector, XMVector3Rotate(XMLoadFloat4(&_vector), XMLoadFloat4(&f)));
			}
		}
		break;
	case OperatorType::NORMALIZE2:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			if (ch0) {
				const XMFLOAT4 &f = ch0->GetVector();
				XMStoreFloat2((XMFLOAT2*)&_vector, XMVector2Normalize(XMLoadFloat2((XMFLOAT2*)&f)));
			}
		}
		break;
	case OperatorType::NORMALIZE3:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			if (ch0) {
				const XMFLOAT4 &f = ch0->GetVector();
				XMStoreFloat3((XMFLOAT3*)&_vector, XMVector3Normalize(XMLoadFloat3((XMFLOAT3*)&f)));
			}
		}
		break;
	case OperatorType::NORMALIZE4:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
			if (ch0) {
				const XMFLOAT4 &f = ch0->GetVector();
				XMStoreFloat4(&_vector, XMVector4Normalize(XMLoadFloat4(&f)));
			}
		}
		break;
	case OperatorType::ORTHOGONAL2:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				XMStoreFloat2((XMFLOAT2*)&_vector, XMVector2Orthogonal(XMLoadFloat2((XMFLOAT2*)&p0)));
			}
			else
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	case OperatorType::ORTHOGONAL3:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				XMStoreFloat3((XMFLOAT3*)&_vector, XMVector3Orthogonal(XMLoadFloat3((XMFLOAT3*)&p0)));
			}
			else
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	case OperatorType::ORTHOGONAL4:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				XMStoreFloat4(&_vector, XMVector4Orthogonal(XMLoadFloat4(&p0)));
			}
			else
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	case OperatorType::CROSS2:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<VectorChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				const XMFLOAT4 &p1 = ch1->GetVector();
				XMStoreFloat4(&_vector, XMVector2Cross(XMLoadFloat2((XMFLOAT2*)&p0), XMLoadFloat2((XMFLOAT2*)&p1)));
			}
			else
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	case OperatorType::CROSS3:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<VectorChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				const XMFLOAT4 &p1 = ch1->GetVector();
				XMStoreFloat4(&_vector, XMVector3Cross(XMLoadFloat3((XMFLOAT3*)&p0), XMLoadFloat3((XMFLOAT3*)&p1)));
			}
			else
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
//	case OperatorType::CROSS4:
//		break;
	case OperatorType::TRANSFORM:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<MatrixChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				const XMFLOAT4X4 &p1 = ch1->GetMatrix();
				XMStoreFloat4(&_vector, XMVector4Transform(XMLoadFloat4(&p0), XMLoadFloat4x4(&p1)));
			}
			else
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	case OperatorType::TRANSFORM_COORD:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<MatrixChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				const XMFLOAT4X4 &p1 = ch1->GetMatrix();
				XMStoreFloat4(&_vector, XMVector3TransformCoord(XMLoadFloat4(&p0), XMLoadFloat4x4(&p1)));
			}
			else
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	case OperatorType::TRANSFORM_NORMAL:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<MatrixChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				const XMFLOAT4X4 &p1 = ch1->GetMatrix();
				XMStoreFloat4(&_vector, XMVector3TransformNormal(XMLoadFloat4(&p0), XMLoadFloat4x4(&p1)));
			}
			else
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	case OperatorType::QUAT_YPR:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &f = ch0->GetVector();
				XMStoreFloat4(&_vector, XMQuaternionRotationRollPitchYaw(f.x, f.y, f.z));
			}
			else
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		}
		break;
	case OperatorType::QUAT_AXIS_ANGLE:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				float32 p1 = (float32)ch1->GetValue();
				XMStoreFloat4(&_vector, XMQuaternionRotationAxis(XMVector3Normalize(XMLoadFloat4(&p0)), p1));
			}
			else 
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		}
		break;
	case OperatorType::QUAT_INVERSE:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				XMStoreFloat4(&_vector, XMQuaternionInverse(XMLoadFloat4(&p0)));
			}
			else
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		}
		break;
	case OperatorType::QUAT_SLERP:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<VectorChip> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			if (ch0 && ch1 && ch2) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				const XMFLOAT4 &p1 = ch1->GetVector();
				float32 p2 = (float32)ch2->GetValue();
				XMStoreFloat4(&_vector, XMQuaternionSlerp(XMLoadFloat4(&p0), XMLoadFloat4(&p1), p2));
			}
			else 
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		}
		break;
	case OperatorType::QUAT_NORMALIZE:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			if (ch0) {
				const XMFLOAT4& f = ch0->GetVector();
				XMStoreFloat4(&_vector, XMQuaternionNormalize(XMLoadFloat4(&f)));
			}
		}
		break;
	case OperatorType::QUAT_MATRIX:
		{
			ChildPtr<MatrixChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4X4 &p0 = ch0->GetMatrix();
				_getScaling(p0, _vector);
				XMStoreFloat4(&_vector, XMQuaternionRotationMatrix(XMMatrixScaling(1.0f / _vector.x, 1.0f / _vector.y, 1.0f / _vector.z) * XMLoadFloat4x4(&p0)));
			}
			else 
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		}
		break;
	case OperatorType::QUAT_CONJUGATE:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				XMStoreFloat4(&_vector, XMQuaternionConjugate(XMLoadFloat4(&p0)));
			}
			else 
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		}
		break;
	case OperatorType::QUAT_MULTIPLY:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<VectorChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				const XMFLOAT4 &p1 = ch1->GetVector();
				XMStoreFloat4(&_vector, XMQuaternionMultiply(XMLoadFloat4(&p0), XMLoadFloat4(&p1)));
			}
			else 
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		}
		break;
	case OperatorType::QUAT_SQUAD:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<VectorChip> ch1 = GetChild(1);
			ChildPtr<VectorChip> ch2 = GetChild(2);
			ChildPtr<VectorChip> ch3 = GetChild(3);
			ChildPtr<Value> ch4 = GetChild(4);
			if (ch0 && ch1 && ch2 && ch3 && ch4) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				const XMFLOAT4 &p1 = ch1->GetVector();
				const XMFLOAT4 &p2 = ch2->GetVector();
				const XMFLOAT4 &p3 = ch3->GetVector();
				float32 t = (float32)ch4->GetValue();
				XMStoreFloat4(&_vector, XMQuaternionSquad(XMLoadFloat4(&p0), XMLoadFloat4(&p1), XMLoadFloat4(&p2), XMLoadFloat4(&p3), t));
			}
			else 
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		}
		break;
	case OperatorType::QUAT_TO_AXIS_ANGLE:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				XMVECTOR axis;
				XMQuaternionToAxisAngle(&axis, &_vector.w, XMLoadFloat4(&p0));
				XMStoreFloat3((XMFLOAT3*)&_vector, axis);
			}
			else 
				_vector = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	case OperatorType::QUAT_TO_YPR:
		{
		ChildPtr<VectorChip> ch0 = GetChild(0);
		if (ch0) {
			const XMFLOAT4& q = ch0->GetVector();

			// This stuff is from http://www.euclideanspace.com/maths/geometry/rotations/conversions/quaternionToEuler/

			// Convert from LH (x: right, y: up, z: forward) to RH (x: forward, y: up, z: right) to match notation from site.
			XMFLOAT4 q1 = XMFLOAT4(-q.z, -q.y, -q.x, q.w);

			float32 sqw = q1.w * q1.w;
			float32 sqx = q1.x * q1.x;
			float32 sqy = q1.y * q1.y;
			float32 sqz = q1.z * q1.z;
			float32 unit = sqx + sqy + sqz + sqw; // if normalised is one, otherwise is correction factor
			float32 test = q1.x * q1.y + q1.z * q1.w;
			float32 heading, attitude, bank;
			if (test > 0.499f * unit) { // singularity at north pole
				heading = 2.0f * atan2(q1.x, q1.w);
				attitude = XM_PI *0.5f;
				bank = 0.0f;
			}
			else if (test < -0.499f * unit) { // singularity at south pole
				heading = -2.0f * atan2(q1.x, q1.w);
				attitude = -XM_PI * 0.5f;
				bank = 0.0f;
			}
			else {
				heading = atan2(2.0f * q1.y * q1.w - 2.0f * q1.x * q1.z, sqx - sqy - sqz + sqw);
				attitude = asin(2.0f * test / unit);
				bank = atan2(2.0f * q1.x * q1.w - 2.0f * q1.y * q1.z, -sqx + sqy - sqz + sqw);
			}
			_vector = XMFLOAT4(-attitude, -heading, -bank, 0.0f);
		}
		else
			_vector = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);

		}
		break;
	case OperatorType::MATRIX_X_AXIS:
		{
			ChildPtr<MatrixChip> ch0 = GetChild(0);
			if (ch0)
				_vector = XMFLOAT4((float32*)&ch0->GetMatrix() + 0);
			else 
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	case OperatorType::MATRIX_Y_AXIS:
		{
			ChildPtr<MatrixChip> ch0 = GetChild(0);
			if (ch0)
				_vector = XMFLOAT4((float32*)&ch0->GetMatrix() + 4);
			else 
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	case OperatorType::MATRIX_Z_AXIS:
		{
			ChildPtr<MatrixChip> ch0 = GetChild(0);
			if (ch0)
				_vector = XMFLOAT4((float32*)&ch0->GetMatrix() + 8);
			else 
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	case OperatorType::MATRIX_W_AXIS: // fall through
	case OperatorType::MATRIX_TRANSLATION:
		{
			ChildPtr<MatrixChip> ch0 = GetChild(0);
			if (ch0)
				_vector = XMFLOAT4((float32*)&ch0->GetMatrix() + 12);
			else 
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	case OperatorType::MATRIX_ROTATION:
		{
			ChildPtr<MatrixChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4X4 &p0 = ch0->GetMatrix();
				_getScaling(p0, _vector);
				XMStoreFloat4(&_vector, XMQuaternionRotationMatrix(XMMatrixScaling(1.0f / _vector.x, 1.0f / _vector.y, 1.0f / _vector.z) * XMLoadFloat4x4(&p0)));
			}
			else 
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);			
		}
		break;
	case OperatorType::MATRIX_SCALING:
		{
			ChildPtr<MatrixChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4X4 &p0 = ch0->GetMatrix();
				_getScaling(p0, _vector);
			}
			else 
				_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		break;
	default:
		AddMessage(UninitializedException());
		break;
	}

	return _vector;
}

void VectorOperator::SetOperatorType(OperatorType ot)
{ 
	if (_ot == ot)
		return;
	_ot = ot; 

	switch (_ot) 
	{
	case OperatorType::ACCUMULATE:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Vector"));
		ClearConnections(1);
		break;
	case OperatorType::ADD:
	case OperatorType::SUBTRACT:
	case OperatorType::MULTIPLY:
	case OperatorType::DIVIDE:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Left Vector"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, true, UP, MTEXT("Right Vector(s)"));
		ClearConnections(2);
		break;
	case OperatorType::ROTATE_YPR: 
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Vector"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Euler Angles"));
		ClearConnections(2);
		break;
	case OperatorType::ROTATE_QUAT: 
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Vector"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Quaternion"));
		ClearConnections(2);
		break;
	case OperatorType::NORMALIZE2:
	case OperatorType::NORMALIZE3:
	case OperatorType::NORMALIZE4:
	case OperatorType::ORTHOGONAL2:
	case OperatorType::ORTHOGONAL3:
	case OperatorType::ORTHOGONAL4:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Vector"));
		ClearConnections(1);
		break;
	case OperatorType::CROSS2:
	case OperatorType::CROSS3:
//	case OperatorType::CROSS4:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Vector 1"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Vector 2"));
		ClearConnections(2);
		break;
	case OperatorType::TRANSFORM:
	case OperatorType::TRANSFORM_COORD:
	case OperatorType::TRANSFORM_NORMAL:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Vector"));
		CREATE_CHILD_KEEP(1, MATRIXCHIP_GUID, false, UP, MTEXT("Matrix"));
		ClearConnections(2);
		break;
	case OperatorType::QUAT_YPR:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Euler Angles"));
		ClearConnections(1);
		break;
	case OperatorType::QUAT_AXIS_ANGLE:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Axis"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Angle"));
		ClearConnections(2);
		break;
	case OperatorType::QUAT_SLERP:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Quaternion 1"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Quaternion 2"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("Interpolation Value"));
		ClearConnections(3);
		break;
	case OperatorType::QUAT_INVERSE:
	case OperatorType::QUAT_NORMALIZE:
	case OperatorType::QUAT_CONJUGATE:
	case OperatorType::QUAT_TO_AXIS_ANGLE:
	case OperatorType::QUAT_TO_YPR:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Quaternion"));
		ClearConnections(1);
		break;
	case OperatorType::QUAT_MATRIX:
		CREATE_CHILD_KEEP(0, MATRIXCHIP_GUID, false, UP, MTEXT("Rotation Matrix"));
		ClearConnections(1);
		break;
	case OperatorType::QUAT_MULTIPLY:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Quaternion 1"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Quaternion 2"));
		ClearConnections(2);
		break;
	case OperatorType::QUAT_SQUAD:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Quaternion 1"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Quaternion 2"));
		CREATE_CHILD_KEEP(2, VECTORCHIP_GUID, false, UP, MTEXT("Quaternion 3"));
		CREATE_CHILD_KEEP(3, VECTORCHIP_GUID, false, UP, MTEXT("Quaternion 4"));
		CREATE_CHILD_KEEP(4, VALUE_GUID, false, UP, MTEXT("Interpolation value"));
		ClearConnections(5);
		break;
	case OperatorType::MATRIX_X_AXIS:
	case OperatorType::MATRIX_Y_AXIS:
	case OperatorType::MATRIX_Z_AXIS:
	case OperatorType::MATRIX_W_AXIS:
	case OperatorType::MATRIX_TRANSLATION:
	case OperatorType::MATRIX_ROTATION:
	case OperatorType::MATRIX_SCALING:
		CREATE_CHILD_KEEP(0, MATRIXCHIP_GUID, false, UP, MTEXT("Matrix"));
		ClearConnections(1);
		break;
	default:
		ClearConnections();
		break;
	}
	RemoveMessage(UninitializedException());
}
