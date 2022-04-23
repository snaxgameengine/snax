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
#include "TemplateChip.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ChipManager.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"


using namespace m3d;


CHIPDESCV1_DEF_VIRTUAL(TemplateChip, MTEXT("Template Chip"), TEMPLATECHIP_GUID, PROXYCHIP_GUID);


TemplateChip::TemplateChip() : _template(nullptr)
{
}

TemplateChip::~TemplateChip()
{
	SAFE_RELEASE(_template);
}

const ChildConnectionList &TemplateChip::ProvideChildren() const
{
	if (_template)
		return _template->GetChildren();
	return ProxyChip::ProvideChildren();
}

ChipChildPtr TemplateChip::GetChip()
{
	//Touch();
	return ChipChildPtr(); // No function!
}

bool TemplateChip::CopyChip(Chip *chip)
{
	TemplateChip *c = dynamic_cast<TemplateChip*>(chip);
	B_RETURN(ProxyChip::CopyChip(c));
	return true;
}

bool TemplateChip::LoadChip(DocumentLoader &loader)
{
	B_RETURN(ProxyChip::LoadChip(loader));
	LOAD(MTEXT("template"), _template); // Load template first
	if (_template && _template->GetChipDesc().type != ProxyChip::GetChipType()) {
		SAFE_RELEASE(_template);
		return false;
	}
	return true;
}

bool TemplateChip::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(ProxyChip::SaveChip(saver));
	SAVE(MTEXT("template"), _template);
	return true;
}

Chip *TemplateChip::GetTemplate()
{
	if (_template)
		return _template;
	CreateTemplate();
	return _template; // Could be null if fail
}

void TemplateChip::SetClass(Class* clazz)
{
	Chip::SetClass(clazz);
	if (_template) {
		_template->SetClass(clazz);
	}
}

bool TemplateChip::CreateTemplate(Chip *copyTemplateFrom)
{
	if (_template)
		return true;

	if (!IsChipTypeSet()) {
		AddMessage(ChipTypeNotSetException());
		return false;
	}

	_template = engine->GetChipManager()->CreateChip(GetChipType());

	if (!_template)
		return false;

	_template->SetClass(GetClass());
	_template->SetOwner(this);

	if (copyTemplateFrom) {
		if (!_template->CopyChip(copyTemplateFrom)) {
			SAFE_RELEASE(_template);
		}
	}
	else { 
		if (!_template->InitChip()) {
			SAFE_RELEASE(_template);
		}
	}

	//ClearError();

	return _template != nullptr;
}

Chip *TemplateChip::CreateChipFromTemplate()
{
	CreateTemplate();
	if (!_template)
		return nullptr;
	Chip *chip = engine->GetChipManager()->CreateChip(_template->GetChipDesc().type);
	if (!chip)
		return nullptr;

	// Make us the owner of the new chip. Caller is free to change it later!
	chip->SetOwner(this);
	chip->SetClass(this->GetClass());

	if (!chip->CopyChip(_template)) {
		SAFE_RELEASE(chip);
		return nullptr;
	}

	chip->SetChildProvider(_template);

	return chip;
}

void TemplateChip::OnDestroyDevice() 
{
	ProxyChip::OnDestroyDevice();
	if (_template)
		_template->OnDestroyDevice();
}

void TemplateChip::OnReleasingBackBuffer(RenderWindow *rw) 
{
	ProxyChip::OnReleasingBackBuffer(rw);
	if (_template)
		_template->OnReleasingBackBuffer(rw);
}

void TemplateChip::RestoreChip() 
{
	ProxyChip::RestoreChip();
	if (_template)
		_template->RestoreChip();
}

void TemplateChip::AddDependencies(ProjectDependencies &deps) 
{
	ProxyChip::AddDependencies(deps);
	if (_template)
		_template->AddDependencies(deps);
}

Chip *TemplateChip::FindChip(ChipID chipID)
{
	Chip *c = ProxyChip::FindChip(chipID);
	if (!c && _template)
		c = _template->FindChip(chipID);
	return c;

}

void TemplateChip::SetConnection(uint32 index, const ChildConnectionDesc& connection, bool keepChildren)
{
	// This is for multilink to work correctly!
	if (_template)
		_template->SetConnection(index, connection, keepChildren);
}