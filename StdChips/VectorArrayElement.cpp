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
#include "VectorArrayElement.h"
#include "VectorArray.h"
#include "Value.h"

using namespace m3d;


CHIPDESCV1_DEF(VectorArrayElement, MTEXT("Vector Array Element"), VECTORARRAYELEMENT_GUID, VECTORCHIP_GUID);


VectorArrayElement::VectorArrayElement()
{
	ClearConnections();
	CREATE_CHILD(0, VECTORARRAY_GUID, false, BOTH, MTEXT("Vector Array"));
	CREATE_CHILD(1, VALUE_GUID, false, BOTH, MTEXT("Array Index"));
}

VectorArrayElement::~VectorArrayElement()
{
}

const XMFLOAT4 &VectorArrayElement::GetVector()
{
	RefreshT refresh(Refresh);
	if (refresh) {
		ChildPtr<VectorArray> ch0 = GetChild(0);
		ChildPtr<Value> ch1 = GetChild(1);
		if (ch0 && ch1) {
			value idx = ch1->GetValue();
			uint32 index = (uint32)std::max(idx, 0.0);
			if (index < ch0->GetContainerSize()) {
				//ClearError();
				ch0->GetVector(index, _vector);
				return _vector;
			}
			AddMessage(ArrayChip::IndexOutOfBoundsException());
		}
		else
			AddMessage(MissingChildException(0));
		_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	return _vector;
}

void VectorArrayElement::SetVector(const XMFLOAT4 &v)
{
	ChildPtr<VectorArray> ch0 = GetChild(0);
	ChildPtr<Value> ch1 = GetChild(1);
	if (ch0 && ch1) {
		value idx = ch1->GetValue();
		uint32 index = (uint32)std::max(idx, 0.0);
		if (index < ch0->GetContainerSize()) {
			//ClearError();
			return ch0->SetVector(index, _vector = v);
		}
		AddMessage(ArrayChip::IndexOutOfBoundsException());
	}
	else
		AddMessage(MissingChildException(0));
}


