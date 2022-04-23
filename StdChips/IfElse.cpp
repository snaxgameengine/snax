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
#include "IfElse.h"
#include "Value.h"


using namespace m3d;


CHIPDESCV1_DEF(IfElse, MTEXT("If/Else"), IFELSE_GUID, CHIP_GUID);


IfElse::IfElse()
{
	CREATE_CHILD(0, VALUE_GUID, false, UP, MTEXT("Value to Check"));
	CREATE_CHILD(1, CHIP_GUID, true, DOWN, MTEXT("Call on True"));
	CREATE_CHILD(2, CHIP_GUID, true, DOWN, MTEXT("Call on False"));
}

IfElse::~IfElse()
{
}

void IfElse::CallChip()
{
	if (!Refresh)
		return;
	ChildPtr<Value> ch0 = GetChild(0);
	if (!ch0)
		return;
	for (uint32 m = 0, i = ch0->GetValueAsBool() ? 1 : 2, n = GetSubConnectionCount(i); m < n; m++) {
		ChipChildPtr ch2 = GetChild(i, m);
		if (ch2)
			ch2->CallChip();
	}
}