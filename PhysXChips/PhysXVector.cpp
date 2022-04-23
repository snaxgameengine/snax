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
#include "PhysXVector.h"
#include "PhysXScene.h"
#include "PhysXRigidBody.h"
#include "PhysXJoint.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXVector, MTEXT("PhysX Vector"), PHYSXVECTOR_GUID, VECTORCHIP_GUID);


PhysXVector::PhysXVector()
{
	ClearConnections();
	_cmd = CMD_NONE;
}

PhysXVector::~PhysXVector()
{
}

bool PhysXVector::CopyChip(Chip *chip)
{
	PhysXVector *c = dynamic_cast<PhysXVector*>(chip);
	B_RETURN(VectorChip::CopyChip(c));
	SetCommand(c->_cmd);
	return true;
}

bool PhysXVector::LoadChip(DocumentLoader &loader)
{
	B_RETURN(VectorChip::LoadChip(loader));
	Command cmd;
	LOAD("cmd", (uint32&)cmd);
	SetCommand(cmd);
	return true;
}

bool PhysXVector::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(VectorChip::SaveChip(saver));
	SAVE("cmd", (uint32)_cmd);
	return true;
}

const XMFLOAT4 &PhysXVector::GetVector()
{
	_vector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	switch (_cmd)
	{
	case CMD_SCENE_XET_GRAVITY:
		{
			ChildPtr<PhysXScene> ch0 = GetChild(0);
			if (ch0) {
				PxScene *s = ch0->GetScene();
				if (s) 
					_vector = (XMFLOAT4&)PxVec4(s->getGravity(), 0.0f);
			}
		}
		break;
	case CMD_RIGID_BODY_XET_LINEAR_VELOCITY:
		{
			ChildPtr<PhysXRigidBody> ch0 = GetChild(0);
			if (ch0) {
				PxRigidBody *a = ch0->GetRigidBody();
				if (a) 
					_vector = (XMFLOAT4&)PxVec4(a->getLinearVelocity(), 0.0f);
			}
		}
		break;
	case CMD_RIGID_BODY_XET_ANGULAR_VELOCITY:
		{
			ChildPtr<PhysXRigidBody> ch0 = GetChild(0);
			if (ch0) {
				PxRigidBody *a = ch0->GetRigidBody();
				if (a) 
					_vector = (XMFLOAT4&)PxVec4(a->getAngularVelocity(), 0.0f);
			}
		}
		break;
	case CMD_RIGID_BODY_XET_MASS_SPACE_INERTIA_TENSOR:
		{
			ChildPtr<PhysXRigidBody> ch0 = GetChild(0);
			if (ch0) {
				PxRigidBody *a = ch0->GetRigidBody();
				if (a) 
					_vector = (XMFLOAT4&)PxVec4(a->getMassSpaceInertiaTensor(), 0.0f);
			}
		}
		break;
	case CMD_CONSTRAINT_GET_LINEAR_FORCE:
	case CMD_CONSTRAINT_GET_ANGULAR_FORCE:
		{
			ChildPtr<PhysXJoint> ch0 = GetChild(0);
			if (ch0) {
				PxJoint *j = ch0->GetJoint();
				if (j) {
					PxConstraint *c = j->getConstraint(); // Note: This is somewhat "unofficial" as the getConstraint()-method is declared private. I changed it to public to make this work! :) 
					if (c) {
						XMFLOAT4 l(0.0f, 0.0f, 0.0f, 0.0f), a(0.0f, 0.0f, 0.0f, 0.0f);
						c->getForce((PxVec3&)l, (PxVec3&)a);
						_vector = _cmd == CMD_CONSTRAINT_GET_LINEAR_FORCE ? l : a;
					}
				}
			}
		}
		break;
	default:
		break;
	}
	return _vector;
}

void PhysXVector::SetVector(const XMFLOAT4 &v)
{
	switch (_cmd)
	{
	case CMD_SCENE_XET_GRAVITY:
		{
			ChildPtr<PhysXScene> ch0 = GetChild(0);
			if (ch0) {
				PxScene *s = ch0->GetScene();
				if (s)
					s->setGravity((PxVec3&)(_vector = v));
			}
		}
		break;
	case CMD_RIGID_BODY_XET_LINEAR_VELOCITY:
		{
			ChildPtr<PhysXRigidBody> ch0 = GetChild(0);
			if (ch0) {
				PxRigidBody *a = ch0->GetRigidBody();
				if (a)
					a->setLinearVelocity((PxVec3&)(_vector = v));
			}
		}
		break;
	case CMD_RIGID_BODY_XET_ANGULAR_VELOCITY:
		{
			ChildPtr<PhysXRigidBody> ch0 = GetChild(0);
			if (ch0) {
				PxRigidBody *a = ch0->GetRigidBody();
				if (a)
					a->setAngularVelocity((PxVec3&)(_vector = v));
			}
		}
		break;
	case CMD_RIGID_BODY_XET_MASS_SPACE_INERTIA_TENSOR:
		{
			ChildPtr<PhysXRigidBody> ch0 = GetChild(0);
			if (ch0) {
				PxRigidBody *a = ch0->GetRigidBody();
				if (a)
					a->setMassSpaceInertiaTensor((PxVec3&)(_vector = v));
			}
		}
		break;
	default:
		break;
	}
}

void PhysXVector::SetCommand(Command cmd)
{
	if (_cmd == cmd)
		return;
	
	_cmd = cmd;
	
	switch (_cmd)
	{
	case CMD_SCENE_XET_GRAVITY:
		CREATE_CHILD_KEEP(0, PHYSXSCENE_GUID, false, BOTH, MTEXT("Scene"));
		ClearConnections(1);
		break;
	case CMD_RIGID_BODY_XET_LINEAR_VELOCITY:
	case CMD_RIGID_BODY_XET_ANGULAR_VELOCITY:
	case CMD_RIGID_BODY_XET_MASS_SPACE_INERTIA_TENSOR:
		CREATE_CHILD_KEEP(0, PHYSXRIGIDBODY_GUID, false, BOTH, MTEXT("Actor"));
		ClearConnections(1);
		break;
	case CMD_CONSTRAINT_GET_LINEAR_FORCE:
	case CMD_CONSTRAINT_GET_ANGULAR_FORCE:
		CREATE_CHILD_KEEP(0, PHYSXJOINT_GUID, false, UP, MTEXT("Joint"));
		ClearConnections(1);
		break;
	default:
		ClearConnections();
		break;
	}
}