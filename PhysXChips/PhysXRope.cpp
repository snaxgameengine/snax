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
#include "PhysXRope.h"
#include "PhysXRigidDynamic.h"
#include "PhysXJoint.h"
#include "StdChips/VectorChip.h"
#include "StdChips/Value.h"
#include "PhysXScene.h"


using namespace m3d;


CHIPDESCV1_DEF(PhysXRope, MTEXT("PhysX Rope"), PHYSXROPE_GUID, CHIP_GUID);


PhysXRope::PhysXRope()
{
	CREATE_CHILD(0, PHYSXRIGIDDYNAMIC_GUID, false, UP, MTEXT("Actor Template"));
	CREATE_CHILD(1, PHYSXJOINT_GUID, false, UP, MTEXT("Joint Template"));
	CREATE_CHILD(2, VECTORCHIP_GUID, false, UP, MTEXT("Start Position"));
	CREATE_CHILD(3, VECTORCHIP_GUID, false, UP, MTEXT("End Position"));
	CREATE_CHILD(4, VALUE_GUID, false, UP, MTEXT("Number of Elements"));
}

PhysXRope::~PhysXRope()
{
}

void PhysXRope::CallChip()
{
	if (_actors.size())
		return;
	assert(_joints.empty());

	ChildPtr<PhysXRigidDynamic> chActor = GetChild(0);
	ChildPtr<PhysXJoint> chJoint = GetChild(1);
	ChildPtr<VectorChip> chP1 = GetChild(2);
	ChildPtr<VectorChip> chP2 = GetChild(3);
	ChildPtr<Value> chNoE = GetChild(4);

	if (!(chActor && chJoint && chP1 && chP2 && chNoE))
		return;

	PxVec3 p1 = (const PxVec3&)chP1->GetVector();
	PxVec3 p2 = (const PxVec3&)chP2->GetVector();
	uint32 n = (uint32)chNoE->GetValue();

	PxVec3 dir = (p2 - p1).getNormalized();
	PxVec3 left = dir.cross(PxVec3(-dir.y, fabs(fabs(dir.z) > 0.577f ? dir.z : dir.x), 0.0f)).getNormalized();
	PxVec3 up = left.cross(dir);
	PxMat33 orientation(dir, up, left);

	float32 length = (p2 - p1).magnitude();
	float32 eLength = length / n;

	PxTransform jTransform0(PxVec3(eLength, 0.0f, 0.0f));
	PxTransform jTransform1(PxVec3(0.0f, 0.0f, 0.0f));

	// Note: to make ropes for stable I could try to scale the inertia tensor or increase the mass of the upper (crane wire) elements.

	for (uint32 i = 0; i < n; i++) {
		PxTransform t(PxMat44(orientation, p1 + dir * eLength * (float32)i));
		PxRigidDynamic *actor = chActor->CreateActor(t);
		_actors.push_back(actor);
		//actor->setMassSpaceInertiaTensor(actor->getMassSpaceInertiaTensor() * 20);
		if (i > 0) {
			PxJoint *joint = chJoint->CreateJoint(actor->getScene()->getPhysics(), _actors[i - 1], jTransform0, _actors[i], jTransform1);
			_joints.push_back(joint);
		}
		else
			((PhysXScene*)actor->getScene()->userData)->RegisterSceneObject(this);
	}
}

void PhysXRope::DestroyRope()
{
	if (_actors.size()) {
		if (_actors.front()->getScene())
			((PhysXScene*)_actors.front()->getScene()->userData)->UnregisterSceneObject(this);
	}
	for (size_t i = 0; i < _actors.size(); i++) {
		mmdelete((PhysXActorData*)_actors[i]->userData);
		_actors[i]->release();
	}
	_actors.clear();
	for (size_t i = 0; i < _joints.size(); i++) {
		_joints[i]->release();
	}
	_joints.clear();
}