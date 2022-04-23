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
#include "ClassDiagramChip.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngine/Class.h"
#include "Shortcut.h"
#include <assert.h>

using namespace m3d;


CHIPDESCV1_DEF_HIDDEN(ClassDiagramChip, MTEXT("Class"), CLASSDIAGRAMCHIP_GUID, CHIP_GUID);


ClassDiagramChip::ClassDiagramChip()
{
	_clazz = nullptr;

	CREATE_CHILD(0, CLASSDIAGRAMCHIP_GUID, true, DOWN, MTEXT("Base Classes"));
}

ClassDiagramChip::~ClassDiagramChip()
{
}

String ClassDiagramChip::GetName() const
{
	if (_clazz)
		return _clazz->GetName();
	return Chip::GetName();
}

static bool __manualAddBase = false;

bool ClassDiagramChip::SetChild(Chip *child, uint32 index, uint32 subIndex)
{
	if (index > 0)
		return false; // Impossible!

	Chip *mainChild = child;
	if (child && child->AsShortcut())
		mainChild = child->AsShortcut()->GetOriginal();

	Chip *oldChild = GetRawChild(0, subIndex);
	Chip *mainOldChild = oldChild;
	if (oldChild && oldChild->AsShortcut())
		mainOldChild = oldChild->AsShortcut()->GetOriginal();

	if (mainChild == mainOldChild) {
		if (child == oldChild)
			return true; // No change!
		_clazz->SetDirty();
		return Chip::SetChild(child, index, subIndex); // Shortcut change!
	}

	ClassDiagramChip *ch = dynamic_cast<ClassDiagramChip*>(mainChild);

	if (ch) {
		if (_clazz->IsDirectBaseClass(ch->GetCG()))
			return false; // Can not derive from the same CG more than once!
		if (ch->GetCG()->IsBaseClass(_clazz))
			return false; // Cyclic!
	}

	if (mainOldChild) {
		ClassDiagramChip *oldCGGChild = dynamic_cast<ClassDiagramChip*>(mainOldChild);
		assert(oldCGGChild);
		if (_clazz->IsDirectBaseClass(oldCGGChild->GetCG())) { // SHOULD always be true!
			if (_clazz->RemoveBaseClass(oldCGGChild->GetCG())) { // RemoveBaseGraph() will call our OnBaseGraphRemoved() setting the child to NULL.
				_clazz->SetDirty();
				// TODO: Owners of all instances of _clazz and derived must be marked dirty!
			}
			else
				return false; // Faild to remove base graph for some reason?!
		}
	}

	assert(GetRawChild(index, subIndex) == nullptr);
//	assert(ch);
	if (ch) {
		__manualAddBase = true;
		if (!_clazz->AddBaseClass(ch->GetCG()))
			return (__manualAddBase = false);
		bool b = Chip::SetChild(child, index, subIndex); // <== This SHOULD always return true!
		assert(b);
		_clazz->SetDirty();
		// TODO: Owners of all instances of _clazz and derived must be marked dirty!
		__manualAddBase = false;
	}
	return true;
}

void ClassDiagramChip::OnBaseClassAdded(Chip *c, uint32 conn)
{
	if (__manualAddBase)
		return; // We're adding manually using class diagram.

	for (uint32 i = 0; i < GetSubConnectionCount(0); i++) {
		Chip *ch = GetRawChild(0, i);
		if (ch && ch->AsShortcut())
			ch = ch->AsShortcut()->GetOriginal();
		assert(c != ch);
		if (c == ch)
			return; // Already added (for some faulty reason!?)
	}
	if (conn == -1) {
		for (conn = 0; conn < GetSubConnectionCount(0); conn++)
			if (GetRawChild(0, conn) == nullptr)
				break;
	}
	bool b = Chip::SetChild(c, 0, conn);
	assert(b);
}

void ClassDiagramChip::OnBaseClassRemoved(ClassDiagramChip *c)
{
	for (uint32 i = 0; i < GetSubConnectionCount(0); i++) {
		Chip *ch = GetRawChild(0, i);
		if (ch && ch->AsShortcut())
			ch = ch->AsShortcut()->GetOriginal();
		if (ch == c)
			Chip::SetChild(nullptr, 0, i);
	}
}

void ClassDiagramChip::UnlinkChildren()
{
	for (uint32 i = 0; i < GetSubConnectionCount(0); i++)
		Chip::SetChild(nullptr, 0, i);
	RemoveEmptyConnections();
}

