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
#include "ProxyChip.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ChipManager.h"


using namespace m3d;


CHIPDESCV1_DEF(ProxyChip, MTEXT("Proxy Chip"), PROXYCHIP_GUID, CHIP_GUID);

ProxyChip::ProxyChip() : _type(PROXYCHIP_GUID), _isTypeSet(false)
{
	// NOTE: There are several cases where it is a bit annoying that we set a connection here...
	CREATE_CHILD(0, CHIP_GUID, false, BOTH, MTEXT("Replacing Chip"));
}

ProxyChip::~ProxyChip()
{
}

bool ProxyChip::CopyChip(Chip *chip)
{
	ProxyChip *c = dynamic_cast<ProxyChip*>(chip);
	B_RETURN(Chip::CopyChip(c));
	SetChipType(c->_type);
	return true;
}

bool ProxyChip::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	Guid g;
	LOAD(MTEXT("type"), g);
	SetChipType(g);
	return true;
}

bool ProxyChip::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE(MTEXT("type"), _type);
	return true;
}

ChipChildPtr ProxyChip::GetChip()
{
	//Touch();

	if (_isTypeSet) {
		// This stuff performs a cast similar to c++ dynamic_cast<>().
		ChipChildPtr child = GetChild(0);
		if (!child || engine->GetChipManager()->IsChipTypeSupported(GetChipTypeIndex(), child->GetChipTypeIndex())) {
			//ClearError();
			return child;
		}
		AddMessage(CastFailedException());
	}
	else
		AddMessage(ChipTypeNotSetException());
	return ChipChildPtr();
}

bool ProxyChip::SetChipType(const Guid &type)
{
	if (type == _type)
		return true; // No change!
	if (_type != PROXYCHIP_GUID)
		return false; // Only allowed to set this once!
	_type = type;
	ResetTypeIndex();

//	ClearConnections();

//	CREATE_CHILD(0, _type, false, BOTH, MTEXT("Replacing Chip"));

	_isTypeSet = true;

	RemoveMessage(ChipTypeNotSetException());

	return true;
}

