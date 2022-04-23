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
#include "PhysXFixedJoint.h"
#include "PhysXRigidActor.h"
#include "StdChips/MatrixChip.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXFixedJoint, MTEXT("PhysX Fixed Joint"), PHYSXFIXEDJOINT_GUID, PHYSXJOINT_GUID);


PhysXFixedJoint::PhysXFixedJoint()
{
	_projectionLinearTolerance = 1.0e10f;
	_projectionAngularTolerance = XM_PI;
}

PhysXFixedJoint::~PhysXFixedJoint()
{
}

bool PhysXFixedJoint::CopyChip(Chip *chip)
{
	PhysXFixedJoint *c = dynamic_cast<PhysXFixedJoint*>(chip);
	B_RETURN(PhysXJoint::CopyChip(c));
	_projectionLinearTolerance = c->_projectionLinearTolerance;
	_projectionAngularTolerance = c->_projectionAngularTolerance;
	return true;
}

bool PhysXFixedJoint::LoadChip(DocumentLoader &loader)
{
	B_RETURN(PhysXJoint::LoadChip(loader));
	LOAD("projectionLinearTolerance", _projectionLinearTolerance);
	LOAD("projectionAngularTolerance", _projectionAngularTolerance);
	return true;
}

bool PhysXFixedJoint::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(PhysXJoint::SaveChip(saver));
	SAVE("projectionLinearTolerance", _projectionLinearTolerance);
	SAVE("projectionAngularTolerance", _projectionAngularTolerance);
	return true;
}

PxJoint *PhysXFixedJoint::CreateJoint(PxPhysics& physics, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1)
{
	PxFixedJoint *j = PxFixedJointCreate(physics, actor0, localFrame0, actor1, localFrame1);
	j->setProjectionLinearTolerance(_projectionLinearTolerance);
	j->setProjectionAngularTolerance(_projectionAngularTolerance);
	return j;
}

float32 PhysXFixedJoint::GetProjectionLinearTolerance() const
{
	PxFixedJoint *j = (PxFixedJoint*)GetJoint();
	if (j)
		return j->getProjectionLinearTolerance();
	return _projectionLinearTolerance;
}

float32 PhysXFixedJoint::GetProjectionAngularTolerance() const
{
	PxFixedJoint *j = (PxFixedJoint*)GetJoint();
	if (j)
		return j->getProjectionAngularTolerance();
	return _projectionAngularTolerance;
}

void PhysXFixedJoint::SetProjectionLinearTolerance(float32 t, bool chipSettings)
{
	t = std::max(t, 0.0f);
	PxFixedJoint *j = (PxFixedJoint*)GetJoint();
	if (j) 
		j->setProjectionLinearTolerance(t);
	if (chipSettings)
		_projectionLinearTolerance = t;
}

void PhysXFixedJoint::SetProjectionAngularTolerance(float32 t, bool chipSettings)
{
	t = std::min(std::max(t, 0.0f), XM_PI);
	PxFixedJoint *j = (PxFixedJoint*)GetJoint();
	if (j) 
		j->setProjectionAngularTolerance(t);
	if (chipSettings)
		_projectionAngularTolerance = t;
}
