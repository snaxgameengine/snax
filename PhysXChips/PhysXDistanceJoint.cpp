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
#include "PhysXDistanceJoint.h"
#include "PhysXRigidActor.h"
#include "StdChips/MatrixChip.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXDistanceJoint, MTEXT("PhysX Distance Joint"), PHYSXDISTANCEJOINT_GUID, PHYSXJOINT_GUID);


PhysXDistanceJoint::PhysXDistanceJoint()
{
	_minDistanceEnable = false;
	_maxDistanceEnable = true;
	_minDistance = 0.0f;
	_maxDistance = 0.0f;
	_tolerance = 0.0f;
	_springEnable = false;
	_stiffness = 0.0f;
	_damping = 0.0f;
}

PhysXDistanceJoint::~PhysXDistanceJoint()
{
}

bool PhysXDistanceJoint::CopyChip(Chip *chip)
{
	PhysXDistanceJoint *c = dynamic_cast<PhysXDistanceJoint*>(chip);
	B_RETURN(PhysXJoint::CopyChip(c));
	_minDistanceEnable = c->_minDistanceEnable;
	_maxDistanceEnable = c->_maxDistanceEnable;
	_minDistance = c->_minDistance;
	_maxDistance = c->_maxDistance;
	_tolerance = c->_tolerance;
	_springEnable = c->_springEnable;
	_stiffness = c->_stiffness;
	_damping = c->_damping;
	return true;
}

bool PhysXDistanceJoint::LoadChip(DocumentLoader &loader)
{
	B_RETURN(PhysXJoint::LoadChip(loader));
	LOADDEF("minDistanceEnable", _minDistanceEnable, false);
	LOADDEF("maxDistanceEnable", _maxDistanceEnable, true);
	LOADDEF("minDistance", _minDistance, 0.0f);
	LOADDEF("maxDistance", _maxDistance, 0.0f);
	LOADDEF("tolerance", _tolerance, 0.0f);
	LOADDEF("springEnable", _springEnable, false);
	LOADDEF("stiffness", _stiffness, 0.0f);
	LOADDEF("damping", _damping, 0.0f);
	return true;
}

bool PhysXDistanceJoint::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(PhysXJoint::SaveChip(saver));
	SAVEDEF("minDistanceEnable", _minDistanceEnable, false);
	SAVEDEF("maxDistanceEnable", _maxDistanceEnable, true);
	SAVEDEF("minDistance", _minDistance, 0.0f);
	SAVEDEF("maxDistance", _maxDistance, 0.0f);
	SAVEDEF("tolerance", _tolerance, 0.0f);
	SAVEDEF("springEnable", _springEnable, false);
	SAVEDEF("stiffness", _stiffness, 0.0f);
	SAVEDEF("damping", _damping, 0.0f);
	return true;
}

PxJoint *PhysXDistanceJoint::CreateJoint(PxPhysics& physics, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1)
{
	PxDistanceJoint *j = PxDistanceJointCreate(physics, actor0, localFrame0, actor1, localFrame1);

	j->setDistanceJointFlag(PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, _minDistanceEnable);
	j->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, _maxDistanceEnable);
	j->setMinDistance(_minDistance);
	j->setMaxDistance(_maxDistance);
	j->setTolerance(_tolerance);
	j->setDistanceJointFlag(PxDistanceJointFlag::eSPRING_ENABLED, _springEnable);
	j->setStiffness(_stiffness);
	j->setDamping(_damping);

	return j;
}

bool PhysXDistanceJoint::IsMinDistanceEnabled() const
{
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j)
		return j->getDistanceJointFlags() == PxDistanceJointFlag::eMIN_DISTANCE_ENABLED;
	return _minDistanceEnable;
}

bool PhysXDistanceJoint::IsMaxDistanceEnabled() const
{
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j)
		return j->getDistanceJointFlags() == PxDistanceJointFlag::eMAX_DISTANCE_ENABLED;
	return _maxDistanceEnable;
}

float32 PhysXDistanceJoint::GetMinDistance() const
{
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j)
		return j->getMinDistance();
	return _minDistance;
}

float32 PhysXDistanceJoint::GetMaxDistance() const
{
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j)
		return j->getMaxDistance();
	return _maxDistance;
}

float32 PhysXDistanceJoint::GetTolerance() const
{
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j)
		return j->getTolerance();
	return _tolerance;
}

bool PhysXDistanceJoint::IsSpringEnabled() const
{
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j)
		return j->getDistanceJointFlags() == PxDistanceJointFlag::eSPRING_ENABLED;
	return _springEnable;
}

float32 PhysXDistanceJoint::GetStiffness() const
{
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j)
		return j->getStiffness();
	return _stiffness;
}

float32 PhysXDistanceJoint::GetDamping() const
{
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j)
		return j->getDamping();
	return _damping;
}

void PhysXDistanceJoint::SetMinDistanceEnabled(bool b, bool chipSettings)
{
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j) 
		j->setDistanceJointFlag(PxDistanceJointFlag::eMIN_DISTANCE_ENABLED, b);
	if (chipSettings)
		_minDistanceEnable = b;
}

void PhysXDistanceJoint::SetMaxDistanceEnabled(bool b, bool chipSettings)
{
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j) 
		j->setDistanceJointFlag(PxDistanceJointFlag::eMAX_DISTANCE_ENABLED, b);
	if (chipSettings)
		_maxDistanceEnable = b;
}

void PhysXDistanceJoint::SetDistanceRange(float32 minD, float32 maxD, bool chipSettings)
{
	minD = std::max(minD, 0.0f);
	maxD = std::max(maxD, minD);
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j) {
		j->setMinDistance(minD);
		j->setMinDistance(maxD);
	}
	if (chipSettings) {
		_minDistance = minD;
		_maxDistance = maxD;
	}
}

void PhysXDistanceJoint::SetTolerance(float32 d, bool chipSettings)
{
	d = std::max(d, 0.0f);
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j) 
		j->setTolerance(d);
	if (chipSettings)
		_tolerance = d;
}

void PhysXDistanceJoint::SetSpringEnabled(bool b, bool chipSettings)
{
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j) 
		j->setDistanceJointFlag(PxDistanceJointFlag::eSPRING_ENABLED, b);
	if (chipSettings)
		_springEnable = b;
}

void PhysXDistanceJoint::SetStiffness(float32 d, bool chipSettings)
{
	d = std::max(d, 0.0f);
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j) 
		j->setStiffness(d);
	if (chipSettings)
		_stiffness = d;
}

void PhysXDistanceJoint::SetDamping(float32 d, bool chipSettings)
{
	d = std::max(d, 0.0f);
	PxDistanceJoint *j = (PxDistanceJoint*)GetJoint();
	if (j) 
		j->setDamping(d);
	if (chipSettings)
		_damping = d;
}
