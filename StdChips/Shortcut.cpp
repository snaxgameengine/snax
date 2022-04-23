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
#include "Shortcut.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/Engine.h"
#include <assert.h>


using namespace m3d;


CHIPDESCV1_DEF_HIDDEN(Shortcut, MTEXT("Shortcut"), SHORTCUT_GUID, CHIP_GUID);


Shortcut::Shortcut()
{
	ClearConnections();

	_original = nullptr;
}

Shortcut::~Shortcut()
{
}

bool Shortcut::CopyChip(Chip *chip)
{
	Shortcut *c = dynamic_cast<Shortcut*>(chip);
	B_RETURN(Chip::CopyChip(c));
	SetOriginal(c->_original);
	return true;
}

bool Shortcut::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	Guid g;
	LOAD(MTEXT("original"), g);
	loader.RegisterShortcut(this, g);
	return true;
}

bool Shortcut::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE(MTEXT("original"), _original ? _original->GetGlobalID() : NullGUID);
	return true;
}

ChipChildPtr Shortcut::GetChip()
{
	//Touch();
	if (_original) {
		_original->Touch();
		return _original->GetChip();
	}
	return ChipChildPtr();
}

void Shortcut::SetOriginal(Chip *c)
{
	if (_original) // Can only set once!
		return;
	Shortcut *s = dynamic_cast<Shortcut*>(c);
	if (s) // Shortcut to shortcut?
		c = s->GetOriginal(); // Link to original!
	if (!c)
		return;
	_original = c;
	_original->AddDestructionObserver(this);
	GetClass()->OnShortcutSet(this);
}

void Shortcut::OnDestruction(DestructionObservable *observable)
{
	if (_original == observable) {
		_original = nullptr;
		bool b = GetClass()->RemoveChip(this);
		assert(b);
	}
}