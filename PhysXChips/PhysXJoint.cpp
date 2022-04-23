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
#include "PhysXJoint.h"
#include "PhysXRigidActor.h"
#include "PhysXScene.h"
#include "StdChips/MatrixChip.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXJoint, MTEXT("PhysX Joint"), PHYSXJOINT_GUID, CHIP_GUID);


PhysXJoint::PhysXJoint()
{
	_joint = nullptr;
	_globalFrame = true;
	_collisionEnabled = false;
	_projectionEnabled = false;
	_visualization = true;
	_breakForce = std::numeric_limits<float32>::max();
	_breakTorque = std::numeric_limits<float32>::max();
}

PhysXJoint::~PhysXJoint()
{
}

bool PhysXJoint::InitChip() 
{
	if (!Chip::InitChip())
		return false;
	UpdateConnections();
	return true; 
}

bool PhysXJoint::CopyChip(Chip *chip)
{
	PhysXJoint *c = dynamic_cast<PhysXJoint*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_globalFrame = c->_globalFrame;
	_collisionEnabled = c->_collisionEnabled;
	_projectionEnabled = c->_projectionEnabled;
	_visualization = c->_visualization;
	_breakForce = c->_breakForce;
	_breakTorque = c->_breakTorque;
	return true;
}

bool PhysXJoint::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOADDEF("globalFrame", _globalFrame, true);
	LOADDEF("collision", _collisionEnabled, false);
	LOADDEF("projection", _projectionEnabled, false);
	LOADDEF("visualization", _visualization, true);
	LOADDEF("breakForce", _breakForce, std::numeric_limits<float32>::max());
	LOADDEF("breakTorque", _breakTorque, std::numeric_limits<float32>::max());
	UpdateConnections();
	return true;
}

bool PhysXJoint::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVEDEF("globalFrame", _globalFrame, true);
	SAVEDEF("collision", _collisionEnabled, false);
	SAVEDEF("projection", _projectionEnabled, false);
	SAVEDEF("visualization", _visualization, true);
	SAVEDEF("breakForce", _breakForce, std::numeric_limits<float32>::max());
	SAVEDEF("breakTorque", _breakTorque, std::numeric_limits<float32>::max());
	return true;
}

bool PhysXJoint::Exist()
{
	if (_joint && !_joint->getScene()) {
		DestroyJoint();
		return false;
	}
	return _joint != nullptr;
}

void PhysXJoint::CallChip()
{
	if (Exist())
		return;

	ChildPtr<PhysXRigidActor> chActor1 = GetChild(0);
	ChildPtr<PhysXRigidActor> chActor2 = GetChild(1);
	if (!chActor1)
		return;

	PxRigidActor *a1 = chActor1->GetRigidActor();
	if (!a1)
		return;

	PxScene *scene = a1->getScene();
	if (!scene)
		return;

	PxRigidActor *a2 = nullptr;
	if (chActor2) {
		a2 = chActor2->GetRigidActor();
		if (!a2 || a1->getScene() != a2->getScene())
			return;
	}

	PxTransform f1 = PxTransform(PxIdentity);
	PxTransform f2 = PxTransform(PxIdentity);

	if (_globalFrame) {
		ChildPtr<MatrixChip> chGlobalFrame = GetChild(2);
		XMFLOAT3 t;
		XMFLOAT4 r;
		XMFLOAT3 s;
		if (chGlobalFrame && chGlobalFrame->GetDecomposedMatrix(t, r, s)) {
			f1 = a1->getGlobalPose().transformInv(PxTransform((PxVec3&)t, (PxQuat&)r));
			if (a2)
				f2 = a2->getGlobalPose().transformInv(PxTransform((PxVec3&)t, (PxQuat&)r));
			else
				f2 = PxTransform((PxVec3&)t, (PxQuat&)r);
		}
	}
	else {
		ChildPtr<MatrixChip> chFrame1 = GetChild(2);
		ChildPtr<MatrixChip> chFrame2 = GetChild(3);
		XMFLOAT3 t;
		XMFLOAT4 r;
		XMFLOAT3 s;
		if (chFrame1 && chFrame1->GetDecomposedMatrix(t, r, s))
			f1 = PxTransform((PxVec3&)t, (PxQuat&)r);
		if (chFrame2 && chFrame2->GetDecomposedMatrix(t, r, s))
			f2 = PxTransform((PxVec3&)t, (PxQuat&)r);
	}

	_joint = CreateJoint(scene->getPhysics(), a1, f1, a2, f2);
	if (!_joint)
		return;
	((PhysXScene*)scene->userData)->RegisterSceneObject(this);
	_joint->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, _collisionEnabled);
	_joint->setConstraintFlag(PxConstraintFlag::ePROJECTION, _projectionEnabled);
	_joint->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, _visualization);
	_joint->setBreakForce(_breakForce, _breakTorque);
}

void PhysXJoint::SetGlobalFrame(bool globalFrame)
{
	if (globalFrame == _globalFrame)
		return;
	_globalFrame = globalFrame;
	UpdateConnections();
}

void PhysXJoint::UpdateConnections()
{
	if (typeid(*this) == typeid(PhysXJoint))
		return; // Skip child connections if not a subtype of Joint!

	CREATE_CHILD_KEEP(0, PHYSXRIGIDACTOR_GUID, false, BOTH, MTEXT("Actor 1"));
	CREATE_CHILD_KEEP(1, PHYSXRIGIDACTOR_GUID, false, BOTH, MTEXT("Actor 2 (Optional)"));

	if (_globalFrame) {
		CREATE_CHILD_KEEP(2, MATRIXCHIP_GUID, false, UP, MTEXT("Global Frame"));
		ClearConnections(3);
	}
	else {
		CREATE_CHILD_KEEP(2, MATRIXCHIP_GUID, false, UP, MTEXT("Frame 1"));
		CREATE_CHILD_KEEP(3, MATRIXCHIP_GUID, false, UP, MTEXT("Frame 2"));
		ClearConnections(4);
	}
}

void PhysXJoint::DestroyJoint()
{
	if (_joint) {
		if (_joint->getScene())
			((PhysXScene*)_joint->getScene()->userData)->UnregisterSceneObject(this);
		_joint->release();
		_joint = nullptr;
	}
}

bool PhysXJoint::IsBroken() const
{
	PxJoint *j = GetJoint();
	if (j)
		j->getConstraintFlags() == PxConstraintFlag::eBROKEN;
	return false;
}

bool PhysXJoint::IsCollisionEnabled() const
{
	PxJoint *j = GetJoint();
	if (j)
		j->getConstraintFlags() == PxConstraintFlag::eCOLLISION_ENABLED;
	return _collisionEnabled;
}

bool PhysXJoint::IsProjectionEnabled() const
{
	PxJoint *j = GetJoint();
	if (j)
		j->getConstraintFlags() == PxConstraintFlag::ePROJECTION;
	return _projectionEnabled;
}

bool PhysXJoint::IsVisualization() const
{
	PxJoint *j = GetJoint();
	if (j)
		j->getConstraintFlags() == PxConstraintFlag::eVISUALIZATION;
	return _visualization;
}

void PhysXJoint::GetBreakForce(float32 &f, float32 &t) const
{
	PxJoint *j = GetJoint();
	if (j)
		return j->getBreakForce(f, t);
	f = _breakForce;
	t = _breakTorque;
}

void PhysXJoint::SetCollisionEnabled(bool b, bool chipSettings)
{
	PxJoint *j = GetJoint();
	if (j)
		j->setConstraintFlag(PxConstraintFlag::eCOLLISION_ENABLED, b);
	if (chipSettings)
		_collisionEnabled = b;
}

void PhysXJoint::SetProjectionEnabled(bool b, bool chipSettings)
{
	PxJoint *j = GetJoint();
	if (j)
		j->setConstraintFlag(PxConstraintFlag::ePROJECTION, b);
	if (chipSettings)
		_projectionEnabled = b;
}

void PhysXJoint::SetVisualization(bool b, bool chipSettings)
{
	PxJoint *j = GetJoint();
	if (j)
		j->setConstraintFlag(PxConstraintFlag::eVISUALIZATION, b);
	if (chipSettings)
		_visualization = b;
}

void PhysXJoint::SetBreakForce(float32 f, float32 t, bool chipSettings)
{
	f = std::max(f, 0.0f);
	t = std::max(f, 0.0f);
	PxJoint *j = GetJoint();
	if (j)
		j->setBreakForce(f, t);
	if (chipSettings) {
		_breakForce = f;
		_breakForce = t;
	}
}

