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
#include "MatrixArrayElement.h"
#include "MatrixArray.h"
#include "Value.h"

using namespace m3d;


CHIPDESCV1_DEF(MatrixArrayElement, MTEXT("Matrix Array Element"), MATRIXARRAYELEMENT_GUID, MATRIXCHIP_GUID);


MatrixArrayElement::MatrixArrayElement()
{
	CREATE_CHILD(0, MATRIXARRAY_GUID, false, BOTH, MTEXT("Matrix Array"));
	CREATE_CHILD(1, VALUE_GUID, false, BOTH, MTEXT("Array Index"));
}

MatrixArrayElement::~MatrixArrayElement()
{
}

const XMFLOAT4X4 &MatrixArrayElement::GetMatrix()
{
	RefreshT refresh(Refresh);
	if (refresh) {
		ChildPtr<MatrixArray> ch0 = GetChild(0);
		ChildPtr<Value> ch1 = GetChild(1);
		if (ch0 && ch1) {
			value idx = ch1->GetValue();
			uint32 index = (uint32)std::max(idx, 0.0);
			if (index < ch0->GetContainerSize()) {
				//ClearError();
				return _matrix = ch0->GetMatrix(index);
			}
			AddMessage(ArrayChip::IndexOutOfBoundsException());
		}
		else
			AddMessage(MissingChildException(0));
		_matrix = IDENTITY;
	}
	return _matrix;
}

void MatrixArrayElement::SetMatrix(const XMFLOAT4X4 &m)
{
	ChildPtr<MatrixArray> ch0 = GetChild(0);
	ChildPtr<Value> ch1 = GetChild(1);
	if (ch0 && ch1) {
		value idx = ch1->GetValue();
		uint32 index = (uint32)std::max(idx, 0.0);
		if (index < ch0->GetContainerSize()) {
			//ClearError();
			return ch0->SetMatrix(index, _matrix = m);
		}
		AddMessage(ArrayChip::IndexOutOfBoundsException());
	}
	else
		AddMessage(MissingChildException(0));
}