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
#include "ForLoop.h"
#include "Value.h"


using namespace m3d;


CHIPDESCV1_DEF(ForLoop, MTEXT("For Loop"), FORLOOP_GUID, CHIP_GUID);


ForLoop::ForLoop()
{
	CREATE_CHILD(0, VALUE_GUID, false, UP, MTEXT("Loop Times"));
	CREATE_CHILD(1, VALUE_GUID, false, DOWN, MTEXT("Current Value"));
	CREATE_CHILD(2, CHIP_GUID, true, DOWN, MTEXT("Chips to Call"));
}

ForLoop::~ForLoop()
{
}

void ForLoop::CallChip()
{
	if (!Refresh)
		return;
	ChildPtr<Value> ch0 = GetChild(0);
	ChildPtr<Value> ch1 = GetChild(1);
	if (!(ch0 && ch1)) {
		AddMessage(MissingChildException(0));
		return;
	}

	//ClearError();

	for (int i = 0, j = (int)ch0->GetValue(); i < j; i = (int)ch1->GetValue() + 1) {
		ch1->SetValue((value)i);		
		for (uint32 m = 0, n = GetSubConnectionCount(2); m < n; m++) {
			ChipChildPtr ch2 = GetChild(2, m);
			if (ch2)
				ch2->CallChip();
		}
	}
}