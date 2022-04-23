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
#include "VectorChip.h"
#include "Value.h"
#include "M3DEngine/DocumentSaver.h"
#include "M3DEngine/DocumentLoader.h"

using namespace m3d;


CHIPDESCV1_DEF(VectorChip, MTEXT("Vector"), VECTORCHIP_GUID, NUMERIC_GUID);


VectorChip::VectorChip()
{
	CREATE_CHILD(0, VALUE_GUID, false, UP, MTEXT("X/R/Left/Pitch"));
	CREATE_CHILD(1, VALUE_GUID, false, UP, MTEXT("Y/G/Top/Yaw"));
	CREATE_CHILD(2, VALUE_GUID, false, UP, MTEXT("Z/B/Width/Roll"));
	CREATE_CHILD(3, VALUE_GUID, false, UP, MTEXT("W/A/Height"));
}

VectorChip::~VectorChip()
{
}

bool VectorChip::InitChip()
{
	B_RETURN(Numeric::InitChip());
	_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	return true;
}

bool VectorChip::CopyChip(Chip *chip)
{
	VectorChip *c = dynamic_cast<VectorChip*>(chip);
	B_RETURN(Numeric::CopyChip(c));
	_vector = c->_vector;
	return true;
}

bool VectorChip::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Numeric::LoadChip(loader));
	LOADARRAY(MTEXT("vector"), (float32*)&_vector, 4);
	return true;
}

bool VectorChip::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Numeric::SaveChip(saver));
	SAVEARRAY(MTEXT("vector"), (float32*)&_vector, 4);
	return true;
}

void VectorChip::CallChip()
{
	GetVector();
}

const XMFLOAT4 &VectorChip::GetVector()
{
	RefreshT refresh(Refresh);
	if (refresh) {
		ChildPtr<Value> chX = GetChild(0);
		ChildPtr<Value> chY = GetChild(1);
		ChildPtr<Value> chZ = GetChild(2);
		ChildPtr<Value> chW = GetChild(3);
		if (chX)
			_vector.x = (float32)chX->GetValue();
		if (chY)
			_vector.y = (float32)chY->GetValue();
		if (chZ)
			_vector.z = (float32)chZ->GetValue();
		if (chW)
			_vector.w = (float32)chW->GetValue();
	}
	return _vector;
}

void VectorChip::SetVector(const XMFLOAT4 &v)
{
	_vector = v;
}

String VectorChip::GetValueAsString() const
{
	return strUtils::format(MTEXT("(%g, %g, %g, %g)"), _vector.x, _vector.y, _vector.z, _vector.w);
}

