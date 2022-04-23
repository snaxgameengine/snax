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
#include "PhysXValue.h"
#include "PhysXScene.h"
#include "PhysXActor.h"
#include "PhysXJoint.h"
#include "PhysXRigidDynamic.h"


using namespace m3d;


CHIPDESCV1_DEF(PhysXValue, MTEXT("PhysX Value"), PHYSXVALUE_GUID, VALUE_GUID);


PhysXValue::PhysXValue()
{
	ClearConnections();
	_cmd = CMD_NONE;
}

PhysXValue::~PhysXValue()
{
}

bool PhysXValue::CopyChip(Chip *chip)
{
	PhysXValue *c = dynamic_cast<PhysXValue*>(chip);
	B_RETURN(Value::CopyChip(c));
	SetCommand(c->_cmd);
	return true;
}

bool PhysXValue::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Value::LoadChip(loader));
	Command cmd;
	LOAD("cmd", (uint32&)cmd);
	SetCommand(cmd);
	return true;
}

bool PhysXValue::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Value::SaveChip(saver));
	SAVE("cmd", (uint32)_cmd);
	return true;
}

value PhysXValue::GetValue()
{
	_value = 0.0f;
	switch (_cmd)
	{
	case CMD_SCENE_EXIST:
		{
			ChildPtr<PhysXScene> ch0 = GetChild(0);
			if (ch0)
				_value = ch0->Exist() ? 1.0f : 0.0f;
		}
		break;
	case CMD_SCENE_IS_RUNNING:
		{
			ChildPtr<PhysXScene> ch0 = GetChild(0);
			if (ch0)
				_value = ch0->IsRunning() ? 1.0f : 0.0f;
		}
		break;
	case CMD_SCENE_IS_SIMULATING:
		{
			ChildPtr<PhysXScene> ch0 = GetChild(0);
			if (ch0)
				_value = ch0->IsSimulating() ? 1.0f : 0.0f;
		}
		break;
	case CMD_SCENE_REAL_TIME_INDEX:
	{
		ChildPtr<PhysXScene> ch0 = GetChild(0);
		if (ch0)
			_value = ch0->GetRealTimeIndex();
		break;
	}
	case CMD_SCENE_SIM_INDEX:
	{
		ChildPtr<PhysXScene> ch0 = GetChild(0);
		if (ch0)
			_value = ch0->GetSimulationIndex();
		break;
	}
	case CMD_SCENE_SIM_WAIT:
	{
		ChildPtr<PhysXScene> ch0 = GetChild(0);
		if (ch0)
			_value = ch0->GetSimulationWaitingTime();
		break;
	}
	case CMD_ACTOR_EXIST:
		{
			ChildPtr<PhysXActor> ch0 = GetChild(0);
			if (ch0)
				_value = ch0->Exist() ? 1.0f : 0.0f;
		}
		break;
	case CMD_RIGID_BODY_XET_MASS:
		{
			ChildPtr<PhysXRigidBody> ch0 = GetChild(0);
			if (ch0) {
				PxRigidBody *a = ch0->GetRigidBody();
				if (a)
					_value = (value)a->getMass();
			}
		}
		break;
	case CMD_RIGID_DYNAMIC_XET_LINEAR_DAMPING:
		{
			ChildPtr<PhysXRigidDynamic> ch0 = GetChild(0);
			if (ch0) {
				PxRigidDynamic *a = ch0->GetRigidDynamic();
				if (a)
					_value = (value)a->getLinearDamping();
			}
		}
		break;
	case CMD_RIGID_DYNAMIC_XET_ANGULAR_DAMPING:
		{
			ChildPtr<PhysXRigidDynamic> ch0 = GetChild(0);
			if (ch0) {
				PxRigidDynamic *a = ch0->GetRigidDynamic();
				if (a)
					_value = (value)a->getAngularDamping();
			}
		}
		break;
	case CMD_RIGID_DYNAMIC_XET_MAX_ANGULAR_VELOCITY:
		{
			ChildPtr<PhysXRigidDynamic> ch0 = GetChild(0);
			if (ch0) {
				PxRigidDynamic *a = ch0->GetRigidDynamic();
				if (a)
					_value = (value)a->getMaxAngularVelocity();
			}
		}
		break;
	case CMD_RIGID_DYNAMIC_XET_SLEEP_THRESHOLD:
		{
			ChildPtr<PhysXRigidDynamic> ch0 = GetChild(0);
			if (ch0) {
				PxRigidDynamic *a = ch0->GetRigidDynamic();
				if (a)
					_value = (value)a->getSleepThreshold();
			}
		}
		break;
	case CMD_RIGID_DYNAMIC_IS_SLEEPING:
		{
			ChildPtr<PhysXRigidDynamic> ch0 = GetChild(0);
			if (ch0) {
				PxRigidDynamic *a = ch0->GetRigidDynamic();
				if (a)
					_value = a->isSleeping() ? 1.0f : 0.0f;
			}
		}
		break;
	case CMD_JOINT_EXIST:
		{
			ChildPtr<PhysXJoint> ch0 = GetChild(0);
			if (ch0)
				_value = ch0->Exist() ? 1.0f : 0.0f;
		}
		break;
	default:
		break;
	}
	return _value;
}

void PhysXValue::SetValue(value v)
{
	switch (_cmd)
	{
	case CMD_RIGID_BODY_XET_MASS:
		{
			ChildPtr<PhysXRigidBody> ch0 = GetChild(0);
			if (ch0) {
				PxRigidBody *a = ch0->GetRigidBody();
				if (a) {
					a->setMass(PxReal(_value = std::max(_value, 1.0e-6)));
					PxVec3 v = a->getCMassLocalPose().p;
					PxRigidBodyExt::setMassAndUpdateInertia(*a, a->getMass(), &v);
				}
			}
		}
		break;
	case CMD_RIGID_DYNAMIC_XET_LINEAR_DAMPING:
		{
			ChildPtr<PhysXRigidDynamic> ch0 = GetChild(0);
			if (ch0) {
				PxRigidDynamic *a = ch0->GetRigidDynamic();
				if (a)
					a->setLinearDamping(PxReal(_value = std::max(_value, 0.0)));
			}
		}
		break;
	case CMD_RIGID_DYNAMIC_XET_ANGULAR_DAMPING:
		{
			ChildPtr<PhysXRigidDynamic> ch0 = GetChild(0);
			if (ch0) {
				PxRigidDynamic *a = ch0->GetRigidDynamic();
				if (a)
					a->setAngularDamping(PxReal(_value = std::max(_value, 0.0)));
			}
		}
		break;
	case CMD_RIGID_DYNAMIC_XET_MAX_ANGULAR_VELOCITY:
		{
			ChildPtr<PhysXRigidDynamic> ch0 = GetChild(0);
			if (ch0) {
				PxRigidDynamic *a = ch0->GetRigidDynamic();
				if (a)
					a->setAngularDamping(PxReal(_value = std::max(_value, 1.0e-6)));
			}
		}
		break;
	case CMD_RIGID_DYNAMIC_XET_SLEEP_THRESHOLD:
		{
			ChildPtr<PhysXRigidDynamic> ch0 = GetChild(0);
			if (ch0) {
				PxRigidDynamic *a = ch0->GetRigidDynamic();
				if (a)
					a->setAngularDamping(PxReal(_value = std::max(_value, 1.0e-6)));
			}
		}
		break;
	default:
		break;
	}
}

void PhysXValue::SetCommand(Command cmd)
{
	if (_cmd == cmd)
		return;
	
	_cmd = cmd;

	switch (_cmd)
	{
	case CMD_SCENE_EXIST:
	case CMD_SCENE_IS_RUNNING:
	case CMD_SCENE_IS_SIMULATING:
	case CMD_SCENE_REAL_TIME_INDEX:
	case CMD_SCENE_SIM_INDEX:
	case CMD_SCENE_SIM_WAIT:
		CREATE_CHILD_KEEP(0, PHYSXSCENE_GUID, false, UP, MTEXT("Scene"));
		ClearConnections(1);
		break;
	case CMD_ACTOR_EXIST:
		CREATE_CHILD_KEEP(0, PHYSXACTOR_GUID, false, UP, MTEXT("Actor"));
		ClearConnections(1);
		break;
	case CMD_RIGID_BODY_XET_MASS:
		CREATE_CHILD_KEEP(0, PHYSXRIGIDBODY_GUID, false, BOTH, MTEXT("Actor"));
		ClearConnections(1);
		break;
	case CMD_RIGID_DYNAMIC_XET_LINEAR_DAMPING:
	case CMD_RIGID_DYNAMIC_XET_ANGULAR_DAMPING:
	case CMD_RIGID_DYNAMIC_XET_MAX_ANGULAR_VELOCITY:
	case CMD_RIGID_DYNAMIC_XET_SLEEP_THRESHOLD:
		CREATE_CHILD_KEEP(0, PHYSXRIGIDDYNAMIC_GUID, false, BOTH, MTEXT("Actor"));
		ClearConnections(1);
		break;
	case CMD_RIGID_DYNAMIC_IS_SLEEPING:
		CREATE_CHILD_KEEP(0, PHYSXRIGIDDYNAMIC_GUID, false, UP, MTEXT("Actor"));
		ClearConnections(1);
		break;
	case CMD_JOINT_EXIST:
		CREATE_CHILD_KEEP(0, PHYSXJOINT_GUID, false, UP, MTEXT("Joint"));
		ClearConnections(1);
		break;
	default:
		ClearConnections();
		break;
	}
}