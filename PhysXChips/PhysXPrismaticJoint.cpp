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
#include "PhysXPrismaticJoint.h"
#include "PhysXRigidActor.h"
#include "StdChips/MatrixChip.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXPrismaticJoint, MTEXT("PhysX Prismatic Joint"), PHYSXPRISMATICJOINT_GUID, PHYSXJOINT_GUID);


PhysXPrismaticJoint::PhysXPrismaticJoint()
{
	_limitEnable = false;
	_limitLower = -std::numeric_limits<float32>::max();
	_limitUpper = std::numeric_limits<float32>::max();
	_limitRestitution = 0.0f;
	_limitStiffness = 0.0f;
	_limitDamping = 0.0f;
	_limitContactDistance = 0.01f;
	_projectionLinearTolerance = 1.0e10f;
	_projectionAngularTolerance = XM_PI;
}

PhysXPrismaticJoint::~PhysXPrismaticJoint()
{
}

bool PhysXPrismaticJoint::CopyChip(Chip *chip)
{
	PhysXPrismaticJoint *c = dynamic_cast<PhysXPrismaticJoint*>(chip);
	B_RETURN(PhysXJoint::CopyChip(c));
	_limitEnable = c->_limitEnable;
	_limitLower = c->_limitLower;
	_limitUpper = c->_limitUpper;
	_limitRestitution = c->_limitRestitution;
	_limitStiffness = c->_limitStiffness;
	_limitDamping = c->_limitDamping;
	_limitContactDistance = c->_limitContactDistance;
	_projectionLinearTolerance = c->_projectionLinearTolerance;
	_projectionAngularTolerance = c->_projectionAngularTolerance;
	return true;
}

bool PhysXPrismaticJoint::LoadChip(DocumentLoader &loader)
{
	B_RETURN(PhysXJoint::LoadChip(loader));
	LOADDEF("limitEnable", _limitEnable, false);
	LOADDEF("limitLower", _limitLower, -std::numeric_limits<float32>::max());
	LOADDEF("limitUpper", _limitUpper, std::numeric_limits<float32>::max());
	LOADDEF("limitRestitution", _limitRestitution, 0.0f);
	LOADDEF("limitStiffness", _limitStiffness, 0.0f);
	LOADDEF("limitDamping", _limitDamping, 0.0f);
	LOADDEF("limitContactDistance", _limitContactDistance, 0.01f);
	LOADDEF("projectionLinearTolerance", _projectionLinearTolerance, 1.0e10f);
	LOADDEF("projectionAngularTolerance", _projectionAngularTolerance, XM_PI);
	return true;
}

bool PhysXPrismaticJoint::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(PhysXJoint::SaveChip(saver));
	SAVEDEF("limitEnable", _limitEnable, false);
	SAVEDEF("limitLower", _limitLower, -std::numeric_limits<float32>::max());
	SAVEDEF("limitUpper", _limitUpper, std::numeric_limits<float32>::max());
	SAVEDEF("limitRestitution", _limitRestitution, 0.0f);
	SAVEDEF("limitStiffness", _limitStiffness, 0.0f);
	SAVEDEF("limitDamping", _limitDamping, 0.0f);
	SAVEDEF("limitContactDistance", _limitContactDistance, 0.01f);
	SAVEDEF("projectionLinearTolerance", _projectionLinearTolerance, 1.0e10f);
	SAVEDEF("projectionAngularTolerance", _projectionAngularTolerance, XM_PI);
	return true;
}

PxJoint *PhysXPrismaticJoint::CreateJoint(PxPhysics& physics, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1)
{
	PxPrismaticJoint *j = PxPrismaticJointCreate(physics, actor0, localFrame0, actor1, localFrame1);
	j->setPrismaticJointFlag(PxPrismaticJointFlag::eLIMIT_ENABLED, _limitEnable);
	PxJointLinearLimitPair l(PxTolerancesScale(), _limitLower, _limitUpper, _limitContactDistance); // TODO: PxTolerancesScale
	l.damping = _limitDamping;
	l.restitution = _limitRestitution;
	l.stiffness = _limitStiffness;
	j->setLimit(l);
	j->setProjectionLinearTolerance(_projectionLinearTolerance);
	j->setProjectionAngularTolerance(_projectionAngularTolerance);
	return j;
}

bool PhysXPrismaticJoint::IsLimitEnabled() const
{
	PxPrismaticJoint *j = (PxPrismaticJoint*)GetJoint();
	if (j)
		return j->getPrismaticJointFlags() == PxPrismaticJointFlag::eLIMIT_ENABLED;
	return _limitEnable;
}

void PhysXPrismaticJoint::SetLimitEnable(bool b, bool chipSettings)
{
	PxPrismaticJoint *j = (PxPrismaticJoint*)GetJoint();
	if (j) 
		j->setPrismaticJointFlag(PxPrismaticJointFlag::eLIMIT_ENABLED, b);
	if (chipSettings)
		_limitEnable = b;
}

void PhysXPrismaticJoint::GetLimitParams(float32 &lower, float32 &upper, float32 &limitRestitution, float32 &limitStiffness, float32 &limitDamping, float32 &limitContactDistance) const
{
	PxPrismaticJoint *j = (PxPrismaticJoint*)GetJoint();
	if (j) {
		PxJointLinearLimitPair l = j->getLimit();
		lower = l.lower;
		upper = l.upper;
		limitRestitution = l.restitution;
		limitStiffness = l.stiffness;
		limitDamping = l.damping;
		limitContactDistance = l.contactDistance;
		return;
	}
	lower = _limitLower;
	upper = _limitUpper;
	limitRestitution = _limitRestitution;
	limitStiffness = _limitStiffness;
	limitDamping = _limitDamping;
	limitContactDistance = _limitContactDistance;
}

void PhysXPrismaticJoint::SetLimitParams(float32 lower, float32 upper, float32 limitRestitution, float32 limitStiffness, float32 limitDamping, float32 limitContactDistance, bool chipSettings)
{
	upper = std::max(lower, upper);
	limitRestitution = std::min(std::max(limitRestitution, 0.0f), 1.0f);
	limitStiffness = std::max(limitStiffness, 0.0f);
	limitDamping = std::max(limitDamping, 0.0f);
	limitContactDistance = std::max(limitContactDistance, 0.0f);
	PxPrismaticJoint *j = (PxPrismaticJoint*)GetJoint();
	if (j) {
		PxJointLinearLimitPair l(PxTolerancesScale(), lower, upper, limitContactDistance); // TODO: PxTolerancesScale
		l.damping = limitDamping;
		l.restitution = limitRestitution;
		l.stiffness = limitStiffness;
		j->setLimit(l);
		j->setPrismaticJointFlag(PxPrismaticJointFlag::eLIMIT_ENABLED, _limitEnable);
	}
	if (chipSettings) {
		_limitLower = lower;
		_limitUpper = upper;
		_limitRestitution = limitRestitution;
		_limitStiffness = limitStiffness;
		_limitDamping = limitDamping;
		_limitContactDistance = limitContactDistance;
	}
}

float32 PhysXPrismaticJoint::GetProjectionLinearTolerance() const
{
	PxPrismaticJoint *j = (PxPrismaticJoint*)GetJoint();
	if (j)
		return j->getProjectionLinearTolerance();
	return _projectionLinearTolerance;
}

void PhysXPrismaticJoint::SetProjectionLinearTolerance(float32 l, bool chipSettings)
{
	l = std::max(l, 0.0f);
	PxPrismaticJoint *j = (PxPrismaticJoint*)GetJoint();
	if (j) 
		j->setProjectionLinearTolerance(l);
	if (chipSettings)
		_projectionLinearTolerance = l;
}

float32 PhysXPrismaticJoint::GetProjectionAngularTolerance() const
{
	PxPrismaticJoint *j = (PxPrismaticJoint*)GetJoint();
	if (j)
		return j->getProjectionAngularTolerance();
	return _projectionAngularTolerance;
}

void PhysXPrismaticJoint::SetProjectionAngularTolerance(float32 l, bool chipSettings)	
{
	l = std::min(std::max(l, 0.0f), XM_PI);
	PxPrismaticJoint *j = (PxPrismaticJoint*)GetJoint();
	if (j) 
		j->setProjectionAngularTolerance(l);
	if (chipSettings)
		_projectionAngularTolerance = l;
}



