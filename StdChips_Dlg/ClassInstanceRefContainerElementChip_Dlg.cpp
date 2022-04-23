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
#include "ClassInstanceRefContainerElementChip_Dlg.h"



using namespace m3d;


DIALOGDESC_DEF_ADV(ClassInstanceRefContainerElementChip_Dlg, CLASSINSTANCEREFCONTAINERELEMENTCHIP_GUID, CHIP_GUID);



void ClassInstanceRefContainerElementChip_Dlg::Init()
{
	AddItem(MTEXT("Array-Element by Index"), (uint32)ClassInstanceRefContainerElementChip::OperatorType::ARRAY);
	AddItem(MTEXT("ValueMap-Element by Key"), (uint32)ClassInstanceRefContainerElementChip::OperatorType::VALUEMAP_BY_KEY);
	AddItem(MTEXT("ValueMap-element by Index"), (uint32)ClassInstanceRefContainerElementChip::OperatorType::VALUEMAP_BY_INDEX);
	AddItem(MTEXT("TextMap-element by Key"), (uint32)ClassInstanceRefContainerElementChip::OperatorType::TEXTMAP_BY_KEY);
	AddItem(MTEXT("TextMao-element by Key"), (uint32)ClassInstanceRefContainerElementChip::OperatorType::TEXTMAP_BY_INDEX);

	SetSelectionChangedCallback([this](RData data) -> bool {
		ClassInstanceRefContainerElementChip::OperatorType cmd = (ClassInstanceRefContainerElementChip::OperatorType)data;
		if (cmd == GetChip()->GetOperatorType())
			return false;
		GetChip()->SetOperatorType(cmd);
		return true;
		});

	SetInit((uint32)GetChip()->GetOperatorType(), (uint32)ClassInstanceRefContainerElementChip::OperatorType::NONE);
}
