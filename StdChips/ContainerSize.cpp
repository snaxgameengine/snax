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
#include "ContainerSize.h"
#include "ContainerChip.h"

using namespace m3d;


CHIPDESCV1_DEF(ContainerSize, MTEXT("Container Size"), CONTAINERSIZE_GUID, VALUE_GUID);


ContainerSize::ContainerSize()
{
	CREATE_CHILD(0, CONTAINERCHIP_GUID, false, BOTH, MTEXT("Container"));
}

ContainerSize::~ContainerSize()
{
}

value ContainerSize::GetValue()
{
	RefreshT refresh(Refresh);
	if (refresh) {
		ChildPtr<ContainerChip> ch0 = GetChild(0);
		if (ch0)
			_value = (value)ch0->GetContainerSize();
		else
			_value = 0.0f;
	}
	return _value;
}

void ContainerSize::SetValue(value v)
{
	ChildPtr<ContainerChip> ch0 = GetChild(0);
	if (ch0)
		return ch0->SetContainerSize(uint32(_value = std::min((uint32)v, 0x00FFFFFFu))); // Limit size to somewhat reasonable 16M.
}

