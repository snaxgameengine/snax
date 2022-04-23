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
#include "SetValue.h"
#include "Value.h"

using namespace m3d;


CHIPDESCV1_DEF(SetValue, MTEXT("Set Value"), SETVALUE_GUID, VALUE_GUID);


SetValue::SetValue()
{
	CREATE_CHILD(0, VALUE_GUID, false, UP, MTEXT("Get Value"));
	CREATE_CHILD(1, VALUE_GUID, true, DOWN, MTEXT("Set Value(s)"));
}

SetValue::~SetValue()
{
}

void SetValue::CallChip()
{
	RefreshT refresh(Refresh);
	if (refresh) {
		ChildPtr<Value> ch = GetChild(0);
		if (ch)
			_value = ch->GetValue();
		for (uint32 i = 0, j = GetSubConnectionCount(1); i < j; i++) {
			ChildPtr<Value> ch = GetChild(1, i);
			if (ch)
				ch->SetValue(_value);
		}
	}
}

