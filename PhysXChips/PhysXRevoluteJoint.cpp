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
#include "PhysXRevoluteJoint.h"
#include "PhysXRigidActor.h"
#include "StdChips/MatrixChip.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXRevoluteJoint, MTEXT("PhysX Revolute Joint"), PHYSXREVOLUTEJOINT_GUID, PHYSXJOINT_GUID);


PhysXRevoluteJoint::PhysXRevoluteJoint()
{
	_limitEnable = false;
	_limitLower = -XM_PIDIV2;
	_limitUpper = XM_PIDIV2;
	_limitRestitution = 0.0f;
	_limitStiffness = 0.0f;
	_limitDamping = 0.0f; 
	_limitContactDistance = 0.05f;
	_driveEnable = false;
	_driveFreespin = true;
	_driveVelocity = 0.0f;
	_driveForceLimit = std::numeric_limits<float32>::max();
	_driveGearRatio = 1.0f;
	_projectionLinearTolerance = 1.0e10f;
	_projectionAngularTolerance = XM_PI;
}

PhysXRevoluteJoint::~PhysXRevoluteJoint()
{
}

bool PhysXRevoluteJoint::CopyChip(Chip *chip)
{
	PhysXRevoluteJoint *c = dynamic_cast<PhysXRevoluteJoint*>(chip);
	B_RETURN(PhysXJoint::CopyChip(c));
	_limitEnable = c->_limitEnable;
	_limitLower = c->_limitLower;
	_limitUpper = c->_limitUpper;
	_limitRestitution = c->_limitRestitution;
	_limitStiffness = c->_limitStiffness;
	_limitDamping = c->_limitDamping;
	_limitContactDistance = c->_limitContactDistance;
	_driveEnable = c->_driveEnable;
	_driveFreespin = c->_driveFreespin;
	_driveVelocity = c->_driveVelocity;
	_driveForceLimit = c->_driveForceLimit;
	_driveGearRatio = c->_driveGearRatio;
	_projectionLinearTolerance = c->_projectionLinearTolerance;
	_projectionAngularTolerance = c->_projectionAngularTolerance;
	return true;
}

bool PhysXRevoluteJoint::LoadChip(DocumentLoader &loader)
{
	B_RETURN(PhysXJoint::LoadChip(loader));
	LOADDEF("limitEnable", _limitEnable, false);
	LOADDEF("limitLower", _limitLower, -XM_PIDIV2);
	LOADDEF("limitUpper", _limitUpper, XM_PIDIV2);
	LOADDEF("limitRestitution", _limitRestitution, 0.0f);
	LOADDEF("limitStiffness", _limitStiffness, 0.0f);
	LOADDEF("limitDamping", _limitDamping, 0.0f);
	LOADDEF("limitContactDistance", _limitContactDistance, 0.05f);
	LOADDEF("driveEnable", _driveEnable, false);
	LOADDEF("driveFreespin", _driveFreespin, true);
	LOADDEF("driveVelocity", _driveVelocity, 0.0f);
	LOADDEF("driveForceLimit", _driveForceLimit, std::numeric_limits<float32>::max());
	LOADDEF("driveGearRatio", _driveGearRatio, 1.0f);
	LOADDEF("projectionLinearTolerance", _projectionLinearTolerance, 1.0e10f);
	LOADDEF("projectionAngularTolerance", _projectionAngularTolerance, XM_PI);
	return true;
}

bool PhysXRevoluteJoint::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(PhysXJoint::SaveChip(saver));
	SAVEDEF("limitEnable", _limitEnable, false);
	SAVEDEF("limitLower", _limitLower, -XM_PIDIV2);
	SAVEDEF("limitUpper", _limitUpper, XM_PIDIV2);
	SAVEDEF("limitRestitution", _limitRestitution, 0.0f);
	SAVEDEF("limitStiffness", _limitStiffness, 0.0f);
	SAVEDEF("limitDamping", _limitDamping, 0.0f);
	SAVEDEF("limitContactDistance", _limitContactDistance, 0.05f);
	SAVEDEF("driveEnable", _driveEnable, false);
	SAVEDEF("driveFreespin", _driveFreespin, true);
	SAVEDEF("driveVelocity", _driveVelocity, 0.05);
	SAVEDEF("driveForceLimit", _driveForceLimit, std::numeric_limits<float32>::max());
	SAVEDEF("driveGearRatio", _driveGearRatio, 1.0f);
	SAVEDEF("projectionLinearTolerance", _projectionLinearTolerance, 1.0e10f);
	SAVEDEF("projectionAngularTolerance", _projectionAngularTolerance, XM_PI);
	return true;
}

PxJoint *PhysXRevoluteJoint::CreateJoint(PxPhysics& physics, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1)
{
	PxRevoluteJoint *j = PxRevoluteJointCreate(physics, actor0, localFrame0, actor1, localFrame1);
	j->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, _limitEnable);
	j->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, _driveEnable);
	j->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_FREESPIN, _driveFreespin);
	PxJointAngularLimitPair l(_limitLower, _limitUpper, _limitContactDistance);
	l.damping = _limitDamping;
	l.restitution = _limitRestitution;
	l.stiffness = _limitStiffness;
	j->setLimit(l);
	j->setDriveVelocity(_driveVelocity);
	j->setDriveForceLimit(_driveForceLimit);
	j->setDriveGearRatio(_driveGearRatio);
	j->setProjectionLinearTolerance(_projectionLinearTolerance);
	j->setProjectionAngularTolerance(_projectionAngularTolerance);
	return j;
}

bool PhysXRevoluteJoint::IsLimitEnabled() const
{
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j)
		return j->getRevoluteJointFlags() == PxRevoluteJointFlag::eLIMIT_ENABLED;
	return _limitEnable;
}

void PhysXRevoluteJoint::SetLimitEnable(bool b, bool chipSettings)
{
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j) 
		j->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, b);
	if (chipSettings)
		_limitEnable = b;
}

void PhysXRevoluteJoint::GetLimitParams(float32 &lower, float32 &upper, float32 &limitRestitution, float32 &limitStiffness, float32 &limitDamping, float32 &limitContactDistance) const
{
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j) {
		PxJointAngularLimitPair l = j->getLimit();
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

void PhysXRevoluteJoint::SetLimitParams(float32 lower, float32 upper, float32 limitRestitution, float32 limitStiffness, float32 limitDamping, float32 limitContactDistance, bool chipSettings)
{
	lower = std::min(std::max(lower, -XM_PIDIV2), XM_PIDIV2);
	upper = std::min(std::max(lower, upper), XM_PIDIV2);
	limitRestitution = std::min(std::max(limitRestitution, 0.0f), 1.0f);
	limitStiffness = std::max(limitStiffness, 0.0f);
	limitDamping = std::max(limitDamping, 0.0f);
	limitContactDistance = std::max(limitContactDistance, 0.0f);
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j) {
		PxJointAngularLimitPair l(lower, upper, limitContactDistance);
		l.damping = limitDamping;
		l.restitution = limitRestitution;
		l.stiffness = limitStiffness;
		j->setLimit(l);
		j->setRevoluteJointFlag(PxRevoluteJointFlag::eLIMIT_ENABLED, _limitEnable);
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

bool PhysXRevoluteJoint::IsDriveEnabled() const
{
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j)
		return j->getRevoluteJointFlags() == PxRevoluteJointFlag::eDRIVE_ENABLED;
	return _driveEnable;
}

void PhysXRevoluteJoint::SetDriveEnable(bool b, bool chipSettings)
{
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j) 
		j->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_ENABLED, b);
	if (chipSettings)
		_driveEnable = b;
}

bool PhysXRevoluteJoint::IsDriveFreespin() const
{
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j)
		return j->getRevoluteJointFlags() == PxRevoluteJointFlag::eDRIVE_FREESPIN;
	return _driveFreespin;
}

void PhysXRevoluteJoint::SetDriveFreespin(bool b, bool chipSettings)
{
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j) 
		j->setRevoluteJointFlag(PxRevoluteJointFlag::eDRIVE_FREESPIN, b);
	if (chipSettings)
		_driveFreespin = b;
}

float32 PhysXRevoluteJoint::GetDriveVelocity() const
{
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j)
		return j->getDriveVelocity();
	return _driveVelocity;
}

void PhysXRevoluteJoint::SetDriveVelocity(float32 l, bool chipSettings)
{
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j) 
		j->setDriveVelocity(l);
	if (chipSettings)
		_driveVelocity = l;
}

float32 PhysXRevoluteJoint::GetDriveForceLimit() const
{
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j)
		return j->getDriveForceLimit();
	return _driveForceLimit;
}

void PhysXRevoluteJoint::SetDriveForceLimit(float32 l, bool chipSettings)
{
	l = std::max(l, 0.0f);
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j) 
		j->setDriveForceLimit(l);
	if (chipSettings)
		_driveForceLimit = l;
}

float32 PhysXRevoluteJoint::GetDriveGearRatio() const
{
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j)
		return j->getDriveGearRatio();
	return _driveGearRatio;
}

void PhysXRevoluteJoint::SetDriveGearRatio(float32 l, bool chipSettings)
{
	l = std::max(l, 0.0f);
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j) 
		j->setDriveGearRatio(l);
	if (chipSettings)
		_driveGearRatio = l;
}

float32 PhysXRevoluteJoint::GetProjectionLinearTolerance() const
{
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j)
		return j->getProjectionLinearTolerance();
	return _projectionLinearTolerance;
}

void PhysXRevoluteJoint::SetProjectionLinearTolerance(float32 l, bool chipSettings)
{
	l = std::max(l, 0.0f);
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j) 
		j->setProjectionLinearTolerance(l);
	if (chipSettings)
		_projectionLinearTolerance = l;
}

float32 PhysXRevoluteJoint::GetProjectionAngularTolerance() const
{
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j)
		return j->getProjectionAngularTolerance();
	return _projectionAngularTolerance;
}

void PhysXRevoluteJoint::SetProjectionAngularTolerance(float32 l, bool chipSettings)	
{
	l = std::min(std::max(l, 0.0f), XM_PI);
	PxRevoluteJoint *j = (PxRevoluteJoint*)GetJoint();
	if (j) 
		j->setProjectionAngularTolerance(l);
	if (chipSettings)
		_projectionAngularTolerance = l;
}

