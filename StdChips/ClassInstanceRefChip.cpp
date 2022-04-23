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
#include "ClassInstanceRefChip.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/ClassInstance.h"
#include "M3DEngine/Engine.h"

using namespace m3d;


CHIPDESCV1_DEF(ClassInstanceRefChip, MTEXT("Instance Ref"), CLASSINSTANCEREFCHIP_GUID, CHIP_GUID);



ClassInstanceRefChip::ClassInstanceRefChip()
{
	_saveRef = true;
}

ClassInstanceRefChip::~ClassInstanceRefChip()
{
	_clear();
}

void ClassInstanceRefChip::_clear()
{
	if (_ref.IsOwner())
		_ref->Release();
	else
		_ref.Reset();
}

bool ClassInstanceRefChip::CopyChip(Chip *chip)
{
	_clear();

	ClassInstanceRefChip *c = dynamic_cast<ClassInstanceRefChip*>(chip);
	B_RETURN(Chip::CopyChip(c));

	_saveRef = c->_saveRef;
	_preload = c->_preload;

	if (_saveRef) {
		if (c->_ref.IsOwner())
			_ref = c->_ref->Clone(this);
		else
			_ref = c->_ref;
	}
	SetUpdateStamp();

	return true;
}

bool ClassInstanceRefChip::LoadChip(DocumentLoader &loader)
{
	_clear();

	B_RETURN(Chip::LoadChip(loader));

	LOAD(MTEXT("save"), _saveRef);
	if (loader.GetDocumentVersion() >= Version(1,2,1,0)) // TODO: Remove!
		LOAD(MTEXT("preload"), _preload);
	if (_saveRef)
		LOAD(MTEXT("object"), _ref);
	SetUpdateStamp();

	return true;
}

bool ClassInstanceRefChip::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));

	SAVE(MTEXT("save"), _saveRef);
	SAVE(MTEXT("preload"), _preload);
	if (_saveRef)
		SAVE(MTEXT("object"), _ref);

	return true;
}

void ClassInstanceRefChip::OnDestroyDevice()
{
	if (_ref.IsOwner())
		_ref->OnDestroyDevice();
}

void ClassInstanceRefChip::OnReleasingBackBuffer(RenderWindow *rw)
{
	if (_ref.IsOwner())
		_ref->OnReleasingBackBuffer(rw);
}

void ClassInstanceRefChip::RestoreChip()
{
	if (!_ref.Prepare(this)) {
		//msg(WARN, MTEXT("Failed to initialize instance reference."), this);
	}
	if (_ref.IsOwner())
		_ref->RestoreChip();
}

void ClassInstanceRefChip::AddDependencies(ProjectDependencies &deps)
{
	if (_ref.IsOwner())
		_ref->AddDependencies(deps);
}

Chip *ClassInstanceRefChip::FindChip(ChipID chipID)
{
	Chip *c = Chip::FindChip(chipID);
	if (!c && _ref.IsOwner())
		c = _ref->FindChip(chipID);
	return c;
}

void ClassInstanceRefChip::CreateInstance(Class *instanceOf)
{
	_clear();

	SetUpdateStamp();

	if (!instanceOf)
		return;

	_ref = ClassInstance::Create(instanceOf, this);
}

void ClassInstanceRefChip::SetInstance(ClassInstanceRef ref, AssignType at)
{
	switch (at)
	{
	case AT_REF:
		if (_ref != ref) {
			_clear();
			SetUpdateStamp();
			_ref = ClassInstanceRef(ref, false);
		}
		break;
	case AT_MOVE:
		if (_ref != ref) {
			_clear();
			SetUpdateStamp();
			_ref = ClassInstanceRef(ref, false);
		}
		if (_ref && !_ref.IsOwner()) {
			_ref.SetOwner(true);
			_ref->SetOwner(this);
		}
		break;
	case AT_COPY:
		if (ref)
			ref = ref->Clone(this);
		_clear();
		SetUpdateStamp();
		_ref = ref;
		break;
	}
}

void ClassInstanceRefChip::ClearInstance()
{
	_clear();
	SetUpdateStamp();
}

void ClassInstanceRefChip::MakeOwner() 
{
	if (!_ref.IsOwner() && _ref) {
		_ref->SetOwner(this);
		_ref.SetOwner(true);
	}
}

void ClassInstanceRefChip::SetClass(Class* clazz)
{
	Chip::SetClass(clazz);
	if (_ref && _ref.IsOwner())
		_ref->SetOwner(this);
}