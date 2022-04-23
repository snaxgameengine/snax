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
#include "PhysXArticulationJointBase.h"
#include "PhysXArticulationLink.h"
#include "StdChips/MatrixChip.h"

using namespace m3d;


CHIPDESCV1_DEF_HIDDEN(PhysXArticulationJointBase, MTEXT("PhysX Articulation Joint Base"), PHYSXARTICULATIONJOINTBASE_GUID, CHIP_GUID);



PhysXArticulationJointBase::PhysXArticulationJointBase()
{
	CREATE_CHILD(0, PHYSXARTICULATIONLINK_GUID, false, BOTH, MTEXT("Link"));

	_globalFrame = false;
}

PhysXArticulationJointBase::~PhysXArticulationJointBase()
{
}

bool PhysXArticulationJointBase::InitChip()
{
	if (!Chip::InitChip())
		return false;
	UpdateConnections();
	return true;
}

bool PhysXArticulationJointBase::CopyChip(Chip *chip)
{
	PhysXArticulationJointBase *c = dynamic_cast<PhysXArticulationJointBase*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_globalFrame = c->_globalFrame;
	UpdateConnections();
	return true;
}

bool PhysXArticulationJointBase::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOADDEF("globalFrame", _globalFrame, true);
	UpdateConnections();
	return true;
}

bool PhysXArticulationJointBase::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVEDEF("globalFrame", _globalFrame, true);
	return true;
}


void PhysXArticulationJointBase::UpdateConnections()
{
	if (typeid(*this) == typeid(PhysXArticulationJointBase))
		return; // Skip child connections if not a subtype of Joint!

//	CREATE_CHILD_KEEP(0, PHYSXRIGIDACTOR_GUID, false, BOTH, MTEXT("Actor 1"));
//	CREATE_CHILD_KEEP(1, PHYSXRIGIDACTOR_GUID, false, BOTH, MTEXT("Actor 2 (Optional)"));

	if (_globalFrame) {
		CREATE_CHILD_KEEP(1, MATRIXCHIP_GUID, false, UP, MTEXT("Global Frame"));
		ClearConnections(2);
	}
	else {
		CREATE_CHILD_KEEP(1, MATRIXCHIP_GUID, false, UP, MTEXT("Parent Frame"));
		CREATE_CHILD_KEEP(2, MATRIXCHIP_GUID, false, UP, MTEXT("Child Frame"));
		ClearConnections(3);
	}
}

void PhysXArticulationJointBase::SetGlobalFrame(bool gf) 
{
	_globalFrame = gf; 
	UpdateConnections();
}