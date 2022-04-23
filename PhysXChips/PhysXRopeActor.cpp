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
#include "PhysXRopeActor.h"
#include "PhysXRope.h"
#include "StdChips/Value.h"


using namespace m3d;


CHIPDESCV1_DEF(PhysXRopeActor, MTEXT("PhysX Rope Actor"), PHYSXROPEACTOR_GUID, PHYSXRIGIDDYNAMIC_GUID);


PhysXRopeActor::PhysXRopeActor()
{
	ClearConnections();

	_type = NONE;
}

PhysXRopeActor::~PhysXRopeActor()
{
}

bool PhysXRopeActor::CopyChip(Chip *chip)
{
	PhysXRopeActor *c = dynamic_cast<PhysXRopeActor*>(chip);
	B_RETURN(Chip::CopyChip(c)); // Skip PhysXRigidDynamic
	SetType(c->_type);
	return true;
}

bool PhysXRopeActor::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader)); // Skip PhysXRigidDynamic
	Type type;
	LOAD("type", (uint32&)type);
	SetType(type);
	return true;
}

bool PhysXRopeActor::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver)); // Skip PhysXRigidDynamic
	SAVE("type", (uint32)_type);
	return true;
}

PxRigidDynamic *PhysXRopeActor::GetRigidDynamic() const
{
	ChildPtr<PhysXRope> chRope = GetChild(0);
	if (!chRope)
		return nullptr;

	uint32 index = -1;

	switch (_type)
	{
	case FIRST:
		index = 0;
		break;
	case LAST:
		index = (uint32)chRope->GetActors().size() - 1;
		break;
	case INDEX:
		{
			ChildPtr<Value> chIndex = GetChild(1);
			if (chIndex)
				index = (uint32)chIndex->GetValue();
		}
		break;
	}
	
	const List<PxRigidDynamic*> &actors = chRope->GetActors();
	if (index < actors.size())
		return actors[index];

	return nullptr;
}

void PhysXRopeActor::SetType(Type type)
{
	if (_type == type)
		return;

	_type = type;

	CREATE_CHILD_KEEP(0, PHYSXROPE_GUID, false, UP, MTEXT("Rope"));

	switch (_type)
	{
	case FIRST:
	case LAST:
	case NONE:
		ClearConnections(1);
		break;
	case INDEX:
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Actor Index"));
	}
}