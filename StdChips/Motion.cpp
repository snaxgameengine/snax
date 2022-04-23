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
#include "Motion.h"
#include "VectorChip.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"


using namespace m3d;


CHIPDESCV1_DEF(Motion, MTEXT("Motion"), MOTION_GUID, MATRIXCHIP_GUID);


Motion::Motion()
{
	CREATE_CHILD(0, VECTORCHIP_GUID, false, UP, MTEXT("Translation"));
	CREATE_CHILD(1, VECTORCHIP_GUID, false, UP, MTEXT("Rotation"));
	CREATE_CHILD(2, VECTORCHIP_GUID, false, UP, MTEXT("Scaling"));
	CREATE_CHILD(3, MATRIXCHIP_GUID, false, UP, MTEXT("Parent"));

	_useQuat = false;
}

Motion::~Motion()
{
}

bool Motion::CopyChip(Chip *chip)
{
	Motion *c = dynamic_cast<Motion*>(chip);
	B_RETURN(MatrixChip::CopyChip(c));
	_useQuat = c->_useQuat;
	return true;
}

bool Motion::LoadChip(DocumentLoader &loader)
{
	B_RETURN(MatrixChip::LoadChip(loader));
	LOAD(MTEXT("useQuaternion|useQuat"), _useQuat);
	return true;
}

bool Motion::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(MatrixChip::SaveChip(saver));
	SAVE(MTEXT("useQuaternion"), _useQuat);
	return true;
}

const XMFLOAT4X4 &Motion::GetMatrix()
{
	RefreshT refresh(Refresh);
	if (refresh) {
		ChildPtr<VectorChip> ch0 = GetChild(0);
		XMVECTOR t = ch0 ? XMLoadFloat4(&ch0->GetVector()) : XMVectorZero();

		ChildPtr<VectorChip> ch1 = GetChild(1);
		XMFLOAT4 rot = ch1 ? ch1->GetVector() : XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR r;
		if (_useQuat || !ch1)
			r = XMQuaternionNormalize(XMLoadFloat4(&rot));
		else
			r = XMQuaternionRotationRollPitchYaw(rot.x, rot.y, rot.z);

		ChildPtr<VectorChip> ch2 = GetChild(2);
		XMVECTOR s = ch2 ? XMLoadFloat4(&ch2->GetVector()) : XMVectorSplatOne();

		XMMATRIX m = XMMatrixAffineTransformation(s, XMVectorZero(), r, t);

		ChildPtr<MatrixChip> chParent = GetChild(3);
		if (chParent) {
			XMMATRIX parent = XMLoadFloat4x4(&chParent->GetMatrix());
			m = XMMatrixMultiply(m, parent);
		}

		XMStoreFloat4x4(&_matrix, m);
	}

	return _matrix;
}
