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
#include "PhysXSphericalJoint.h"
#include "PhysXRigidActor.h"
#include "StdChips/MatrixChip.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXSphericalJoint, MTEXT("PhysX Spherical Joint"), PHYSXSPHERICALJOINT_GUID, PHYSXJOINT_GUID);


PhysXSphericalJoint::PhysXSphericalJoint()
{
	_limitEnable = false;
	_yLimitAngle = XM_PIDIV2;
	_zLimitAngle = XM_PIDIV2;
	_limitRestitution = 0.0f;
	_limitStiffness = 0.0f;
	_limitDamping = 0.0f; 
	_limitContactDistance = 0.05f;
	_projectionLinearTolerance = 1.0e10f;
}

PhysXSphericalJoint::~PhysXSphericalJoint()
{
}

bool PhysXSphericalJoint::CopyChip(Chip *chip)
{
	PhysXSphericalJoint *c = dynamic_cast<PhysXSphericalJoint*>(chip);
	B_RETURN(PhysXJoint::CopyChip(c));
	_limitEnable = c->_limitEnable;
	_yLimitAngle = c->_yLimitAngle;
	_zLimitAngle = c->_zLimitAngle;
	_limitRestitution = c->_limitRestitution;
	_limitStiffness = c->_limitStiffness;
	_limitDamping = c->_limitDamping;
	_limitContactDistance = c->_limitContactDistance;
	_projectionLinearTolerance = c->_projectionLinearTolerance;
	return true;
}

bool PhysXSphericalJoint::LoadChip(DocumentLoader &loader)
{
	B_RETURN(PhysXJoint::LoadChip(loader));
	LOADDEF("limitEnable", _limitEnable, false);
	LOADDEF("yLimitAngle", _yLimitAngle, XM_PIDIV2);
	LOADDEF("zLimitAngle", _zLimitAngle, XM_PIDIV2);
	LOADDEF("limitRestitution", _limitRestitution, 0.0f);
	LOADDEF("limitStiffness", _limitStiffness, 0.0f);
	LOADDEF("limitDamping", _limitDamping, 0.0f);
	LOADDEF("limitContactDistance", _limitContactDistance, 0.05f);
	LOADDEF("projectionLinearTolerance", _projectionLinearTolerance, 1.0e10f);
	return true;
}

bool PhysXSphericalJoint::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(PhysXJoint::SaveChip(saver));
	SAVEDEF("limitEnable", _limitEnable, false);
	SAVEDEF("yLimitAngle", _yLimitAngle, XM_PIDIV2);
	SAVEDEF("zLimitAngle", _zLimitAngle, XM_PIDIV2);
	SAVEDEF("limitRestitution", _limitRestitution, 0.0f);
	SAVEDEF("limitStiffness", _limitStiffness, 0.0f);
	SAVEDEF("limitDamping", _limitDamping, 0.0f);
	SAVEDEF("limitContactDistance", _limitContactDistance, 0.05f);
	SAVEDEF("projectionLinearTolerance", _projectionLinearTolerance, 1.0e10f);
	return true;
}

PxJoint *PhysXSphericalJoint::CreateJoint(PxPhysics& physics, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1)
{
	PxSphericalJoint *j = PxSphericalJointCreate(physics, actor0, localFrame0, actor1, localFrame1);
	j->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED, _limitEnable);
	PxJointLimitCone l(_yLimitAngle, _zLimitAngle, _limitContactDistance);
	l.damping = _limitDamping;
	l.restitution = _limitRestitution;
	l.stiffness = _limitStiffness;
	j->setLimitCone(l);
	j->setProjectionLinearTolerance(_projectionLinearTolerance);
	return j;
}

bool PhysXSphericalJoint::IsLimitEnabled() const
{
	PxSphericalJoint *j = (PxSphericalJoint*)GetJoint();
	if (j)
		return j->getSphericalJointFlags() == PxSphericalJointFlag::eLIMIT_ENABLED;
	return _limitEnable;
}

void PhysXSphericalJoint::SetLimitEnable(bool b, bool chipSettings)
{
	PxSphericalJoint *j = (PxSphericalJoint*)GetJoint();
	if (j) 
		j->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED, b);
	if (chipSettings)
		_limitEnable = b;
}

void PhysXSphericalJoint::GetLimitParams(float32 &yLimitAngle, float32 &zLimitAngle, float32 &limitRestitution, float32 &limitStiffness, float32 &limitDamping, float32 &limitContactDistance) const
{
	PxSphericalJoint *j = (PxSphericalJoint*)GetJoint();
	if (j) {
		PxJointLimitCone l = j->getLimitCone();
		yLimitAngle = l.yAngle;
		zLimitAngle = l.zAngle;
		limitRestitution = l.restitution;
		limitStiffness = l.stiffness;
		limitDamping = l.damping;
		limitContactDistance = l.contactDistance;
		return;
	}
	yLimitAngle = _yLimitAngle;
	zLimitAngle = _zLimitAngle;
	limitRestitution = _limitRestitution;
	limitStiffness = _limitStiffness;
	limitDamping = _limitDamping;
	limitContactDistance = _limitContactDistance;
}

void PhysXSphericalJoint::SetLimitParams(float32 yLimitAngle, float32 zLimitAngle, float32 limitRestitution, float32 limitSpring, float32 limitDamping, float32 limitContactDistance, bool chipSettings)
{
	yLimitAngle = std::min(std::max(yLimitAngle, 0.01f), XM_PI - 0.01f);
	zLimitAngle = std::min(std::max(zLimitAngle, 0.01f), XM_PI - 0.01f);
	limitRestitution = std::min(std::max(limitRestitution, 0.0f), 1.0f);
	limitSpring = std::max(limitSpring, 0.0f);
	limitDamping = std::max(limitDamping, 0.0f);
	limitContactDistance = std::max(limitContactDistance, 0.0f);
	PxSphericalJoint *j = (PxSphericalJoint*)GetJoint();
	if (j) {
		PxJointLimitCone l(yLimitAngle, zLimitAngle, limitContactDistance);
		l.damping = limitDamping;
		l.restitution = limitRestitution;
		l.stiffness = limitSpring;
		j->setLimitCone(l);
		j->setSphericalJointFlag(PxSphericalJointFlag::eLIMIT_ENABLED, _limitEnable); // Need this for joint to update!
	}
	if (chipSettings) {
		_yLimitAngle = yLimitAngle;
		_zLimitAngle = zLimitAngle;
		_limitRestitution = limitRestitution;
		_limitStiffness = limitSpring;
		_limitDamping = limitDamping;
		_limitContactDistance = limitContactDistance;
	}
}

float32 PhysXSphericalJoint::GetProjectionLinearTolerance() const
{
	PxSphericalJoint *j = (PxSphericalJoint*)GetJoint();
	if (j)
		return j->getProjectionLinearTolerance();
	return _projectionLinearTolerance;
}

void PhysXSphericalJoint::SetProjectionLinearTolerance(float32 l, bool chipSettings)
{
	l = std::max(l, 0.0f);
	PxSphericalJoint *j = (PxSphericalJoint*)GetJoint();
	if (j) 
		j->setProjectionLinearTolerance(l);
	if (chipSettings)
		_projectionLinearTolerance = l;
}




