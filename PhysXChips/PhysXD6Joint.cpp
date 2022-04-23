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
#include "PhysXD6Joint.h"
#include "PhysXRigidActor.h"
#include "StdChips/MatrixChip.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXD6Joint, MTEXT("PhysX D6 Joint"), PHYSXD6JOINT_GUID, PHYSXJOINT_GUID);


PhysXD6Joint::PhysXD6Joint()
{
	_motion[PxD6Axis::eX] = PxD6Motion::eLOCKED;
	_motion[PxD6Axis::eY] = PxD6Motion::eLOCKED;
	_motion[PxD6Axis::eZ] = PxD6Motion::eLOCKED;
	_motion[PxD6Axis::eTWIST] = PxD6Motion::eLOCKED;
	_motion[PxD6Axis::eSWING1] = PxD6Motion::eLOCKED;
	_motion[PxD6Axis::eSWING2] = PxD6Motion::eLOCKED;
/*	_linearLimit.contactDistance = 0.05f;
	_linearLimit.damping = 0.0f;
	_linearLimit.restitution = 0.0f;
	_linearLimit.spring = 0.0f;
	_linearLimit.value = 0.0f;
	_twistLimit.contactDistance = 0.05f;
	_twistLimit.damping = 0.0f;
	_twistLimit.restitution = 0.0f;
	_twistLimit.spring = 0.0f;
	_twistLimit.lower = -XM_PIDIV2;
	_twistLimit.upper = XM_PIDIV2;
	_swingLimit.contactDistance = 0.05f;
	_swingLimit.damping = 0.0f;
	_swingLimit.restitution = 0.0f;
	_swingLimit.spring = 0.0f;
	_swingLimit.yAngle = XM_PIDIV2;
	_swingLimit.zAngle = XM_PIDIV2;
*/
	_projectionLinearTolerance = 1.0e10f;
	_projectionAngularTolerance = XM_PI;
}

PhysXD6Joint::~PhysXD6Joint()
{
}

bool PhysXD6Joint::CopyChip(Chip *chip)
{
	PhysXD6Joint *c = dynamic_cast<PhysXD6Joint*>(chip);
	B_RETURN(PhysXJoint::CopyChip(c));

	return true;
}

bool PhysXD6Joint::LoadChip(DocumentLoader &loader)
{
	B_RETURN(PhysXJoint::LoadChip(loader));

	return true;
}

bool PhysXD6Joint::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(PhysXJoint::SaveChip(saver));

	return true;
}

PxJoint *PhysXD6Joint::CreateJoint(PxPhysics& physics, PxRigidActor* actor0, const PxTransform& localFrame0, PxRigidActor* actor1, const PxTransform& localFrame1)
{
	PxD6Joint *j = PxD6JointCreate(physics, actor0, localFrame0, actor1, localFrame1);


	return j;
}











/*
	PxD6Motion _motionY;
	PxD6Motion _motionZ; 
	PxD6Motion _motionTwist;
	PxD6Motion _motionSwing1; 
	PxD6Motion _motionSwing2; 

	float32 _linearLimitExtent;
	float32 _linearLimitRestitution;
	float32 _linearLimitSpring;
	float32 _linearLimitDamping; 
	float32 _linearLimitContactDistance;

	float32 _twistLimitLower;
	float32 _twistLimitUpper;
	float32 _twistLimitRestitution;
	float32 _twistLimitSpring;
	float32 _twistLimitDamping; 
	float32 _twistLimitContactDistance;

	float32 _swingLimitYAngle;
	float32 _swingLimitZAngle;
	float32 _swingLimitRestitution;
	float32 _swingLimitSpring;
	float32 _swingLimitDamping; 
	float32 _swingLimitContactDistance;
*/