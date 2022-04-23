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
#include "MultiProxy.h"
#include "Value.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"

using namespace m3d;


CHIPDESCV1_DEF(MultiProxy, MTEXT("Multi Proxy"), MULTIPROXY_GUID, PROXYCHIP_GUID);

MultiProxy::MultiProxy()
{
	_mode = Mode::INDEXED;
	SetMode(Mode::GROWING);
}

MultiProxy::~MultiProxy()
{
}

bool MultiProxy::CopyChip(Chip* chip)
{
	MultiProxy* c = dynamic_cast<MultiProxy*>(chip);
	B_RETURN(ProxyChip::CopyChip(c));
	SetMode(c->_mode);
	return true;
}

bool MultiProxy::LoadChip(DocumentLoader& loader)
{
	B_RETURN(ProxyChip::LoadChip(loader));
	Mode mode;
	LOAD(MTEXT("value"), mode);
	SetMode(mode);
	return true;
}

bool MultiProxy::SaveChip(DocumentSaver& saver) const
{
	B_RETURN(ProxyChip::SaveChip(saver));
	SAVE(MTEXT("value"), _mode);
	return true;
}

ChipChildPtr MultiProxy::GetChip()
{
	return GetChild(2);
}
/*
bool MultiProxy::SetChipType(const Guid& type)
{
	B_RETURN(ProxyChip::SetChipType(type));
	ClearConnections();

	if (GetClass())
		GetClass()->OnParameterTypeSet(this);

	return true;
}
*/
uint32 MultiProxy::GetMultiConnectionChildCount() const 
{
	switch (_mode)
	{
	case Mode::GROWING:
		return GetSubConnectionCount(0);
	case Mode::INDEXED:
	{
		ChildPtr<Value> ch0 = GetChild(0);
		if (!ch0)
			return 0;
		uint32 n = (uint32)std::min(std::max(ch0->GetValue(), 0.0), 100000.0);
		return n;
	}
	case Mode::COMBO:
	{
		ChildPtr<Value> ch1 = GetChild(1);
		if (!ch1)
			return 1;
		uint32 n = (uint32)std::min(std::max(ch1->GetValue(), 0.0), 100000.0);
		return GetSubConnectionCount(0) + n;
	}
	}
	return 0;
}

ChipChildPtr MultiProxy::GetMultiConnectionChip(uint32 subIndex) 
{
	switch (_mode)
	{
	case Mode::GROWING:
		return GetChild(0, subIndex);
	case Mode::INDEXED:
	{
		ChildPtr<Value> ch1 = GetChild(1);
		if (ch1)
			ch1->SetValue(subIndex);
		ChipChildPtr ch2 = GetChild(2);
		return ch2->GetChip();
	}
	case Mode::COMBO:
	{
		uint32 n = GetSubConnectionCount(0);
		if (subIndex < n)
			return GetChild(0, subIndex);
		ChildPtr<Value> ch2 = GetChild(2);
		if (ch2)
			ch2->SetValue(subIndex - n);
		ChipChildPtr ch3 = GetChild(3);
		if (ch3)
			return ch3->GetChip();
	}
	}

	return ChipChildPtr();
}

void MultiProxy::SetMode(Mode mode)
{
	if (mode == _mode)
		return;
	_mode = mode;

	ClearConnections();

	switch (_mode)
	{
	case Mode::GROWING:
		CREATE_CHILD(0, CHIP_GUID, true, BOTH, MTEXT("Replacing Chip"));
		break;
	case Mode::INDEXED:
		CREATE_CHILD(0, VALUE_GUID, false, UP, MTEXT("Number of Chips"));
		CREATE_CHILD(1, VALUE_GUID, false, DOWN, MTEXT("Current Chip"));
		CREATE_CHILD(2, CHIP_GUID, false, BOTH, MTEXT("Replacing Chip"));
		break;
	case Mode::COMBO:
		CREATE_CHILD(0, CHIP_GUID, true, BOTH, MTEXT("Replacing Chip"));
		CREATE_CHILD(1, VALUE_GUID, false, UP, MTEXT("Number of Chips"));
		CREATE_CHILD(2, VALUE_GUID, false, DOWN, MTEXT("Current Chip"));
		CREATE_CHILD(3, CHIP_GUID, false, BOTH, MTEXT("Replacing Chip"));
		break;
	}
}