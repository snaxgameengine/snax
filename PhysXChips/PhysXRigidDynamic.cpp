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
#include "PhysXRigidDynamic.h"
#include "PhysXScene.h"
#include "PhysXShape.h"
#include "StdChips/MatrixChip.h"
#include "StdChips/Value.h"
#include "StdChips/VectorChip.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXRigidDynamic, MTEXT("PhysX Rigid Dynamic"), PHYSXRIGIDDYNAMIC_GUID, PHYSXRIGIDBODY_GUID);


PhysXRigidDynamic::PhysXRigidDynamic()
{
	CREATE_CHILD(0, PHYSXSCENE_GUID, false, BOTH, MTEXT("Scene"));
	CREATE_CHILD(1, MATRIXCHIP_GUID, false, UP, MTEXT("Transform"));
	CREATE_CHILD(2, VALUE_GUID, false, UP, MTEXT("Mass"));
	CREATE_CHILD(3, VECTORCHIP_GUID, false, UP, MTEXT("Center of Mass"));
	CREATE_CHILD(4, PHYSXSHAPE_GUID, true, UP, MTEXT("Shapes"));

	_actor = nullptr;

	_kinematic = false;
	_linearDamping = 0.0f;
	_angularDamping = 0.0f;
	_maxAngularVelocity = 7.0f;
	_sleepThreshold = 0.05f;
	_minPositionIters = 4;
	_minVelocityIters = 1;
	_contactReportThreshold = std::numeric_limits<float32>::max();
}

PhysXRigidDynamic::~PhysXRigidDynamic()
{
}

bool PhysXRigidDynamic::CopyChip(Chip *chip)
{
	PhysXRigidDynamic *c = dynamic_cast<PhysXRigidDynamic*>(chip);
	B_RETURN(PhysXRigidBody::CopyChip(c));
	_kinematic = c->_kinematic;
	_linearDamping = c->_linearDamping;
	_angularDamping = c->_angularDamping;
	_maxAngularVelocity = c->_maxAngularVelocity;
	_sleepThreshold = c->_sleepThreshold;
	_minPositionIters = c->_minPositionIters;
	_minVelocityIters = c->_minVelocityIters;
	_contactReportThreshold = c->_contactReportThreshold;
	return true;
}

bool PhysXRigidDynamic::LoadChip(DocumentLoader &loader)
{
	B_RETURN(PhysXRigidBody::LoadChip(loader));
	LOADDEF("kinematic", _kinematic, false);
	LOADDEF("linearDamping", _linearDamping, 0.0f);
	LOADDEF("angularDamping", _angularDamping, 0.0f);
	LOADDEF("maxAngularVelocity", _maxAngularVelocity, 7.0f);
	LOADDEF("sleepThreshold", _sleepThreshold, 0.05f);
	LOADDEF("minPositionIters", _minPositionIters, 4);
	LOADDEF("minVelocityIters", _minVelocityIters, 1);
	LOADDEF("contactReportThreshold", _contactReportThreshold, std::numeric_limits<float32>::max());
	return true;
}

bool PhysXRigidDynamic::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(PhysXRigidBody::SaveChip(saver));
	SAVEDEF("kinematic", _kinematic, false);
	SAVEDEF("linearDamping", _linearDamping, 0.0f);
	SAVEDEF("angularDamping", _angularDamping, 0.0f);
	SAVEDEF("maxAngularVelocity", _maxAngularVelocity, 7.0f);
	SAVEDEF("sleepThreshold", _sleepThreshold, 0.05f);
	SAVEDEF("minPositionIters", _minPositionIters, 4);
	SAVEDEF("minVelocityIters", _minVelocityIters, 1);
	SAVEDEF("contactReportThreshold", _contactReportThreshold, std::numeric_limits<float32>::max());
	return true;
}

void PhysXRigidDynamic::CallChip()
{
	if (Exist()) {
		// Update shapes?
		return;
	}

	PxTransform transform = PxTransform(PxIdentity);
	
	ChildPtr<MatrixChip> chTransform = GetChild(1);
	if (chTransform) {
		transform = PxTransform(PxMat44((float32*)&chTransform->GetMatrix()));
	}

	_actor = CreateActor(transform);

	if (_actor)
		((PhysXScene*)_actor->getScene()->userData)->RegisterSceneObject(this);
}

PxRigidDynamic *PhysXRigidDynamic::CreateActor(const PxTransform &pose)
{
	ChildPtr<PhysXScene> chScene = GetChild(0);
	if (!chScene)
		return nullptr;

	PxScene *scene = chScene->GetScene();
	if (!scene)
		return nullptr;

	PxRigidDynamic *actor = scene->getPhysics().createRigidDynamic(pose);
	if (!actor)
		return nullptr;

	actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, _disableGravity);
	actor->setActorFlag(PxActorFlag::eSEND_SLEEP_NOTIFIES, _sendSleepNotifies);
	actor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, _kinematic);
	actor->setAngularDamping(_angularDamping);
	actor->setLinearDamping(_linearDamping);
	actor->setMaxAngularVelocity(_maxAngularVelocity);
	actor->setSolverIterationCounts(_minPositionIters, _minVelocityIters);
	actor->setSleepThreshold(_sleepThreshold);
	actor->setContactReportThreshold(_contactReportThreshold);

	List<PxReal> shapeMasses;
	
	for (uint32 i = 0; i < GetSubConnectionCount(4); i++) {
		ChildPtr<PhysXShape> ch2 = GetChild(4, i);
		if (ch2) {
			PxShape *shape = ch2->GetShape();
			if (shape) {
				actor->attachShape(*shape);
				PxReal mass = ch2->GetMass();
				shapeMasses.push_back(mass);
			}
		}
	}

	float32 mass = 0.0f;
	ChildPtr<Value> chMass = GetChild(2);
	if (chMass)
		mass = (float32)chMass->GetValue();
	mass = std::max(mass, 0.0f);

	PxVec3 com(0.0f, 0.0f, 0.0f);
	ChildPtr<VectorChip> chCOM = GetChild(3);
	if (chCOM) {
		XMFLOAT4 v = chCOM->GetVector();
		com = PxVec3(v.x, v.y, v.z);
	}

	if (mass == 0.0f) {
		bool b = PxRigidBodyExt::setMassAndUpdateInertia(*actor, &shapeMasses.front(), (PxU32)shapeMasses.size(), &com);
	}
	else {
		bool b = PxRigidBodyExt::setMassAndUpdateInertia(*actor, mass, &com);
	}
	//	PxRigidBodyExt::updateMassAndInertia(*actor, 10);

		
	PhysXActorData *data = mmnew PhysXActorData(actor);
	data->globalPose = actor->getGlobalPose();
	actor->userData = data;

	scene->addActor(*actor);

	return actor;
}

void PhysXRigidDynamic::DestroyActor() 
{
	if (_actor) {
		if (_actor->getScene())
			((PhysXScene*)_actor->getScene()->userData)->UnregisterSceneObject(this);
		mmdelete((PhysXActorData*)_actor->userData);
		_actor->release();
		_actor = nullptr;
	}
}

void PhysXRigidDynamic::AddForce(const PhysXForce &force)
{
	PxRigidActor *actor = GetRigidActor();
	if (!actor)
		return;
	PhysXActorData *data = (PhysXActorData*)actor->userData;
	data->forces.push_back(force);
}

void PhysXRigidDynamic::ClearForces()
{
	PxRigidActor *actor = GetRigidActor();
	if (!actor)
		return;
	PhysXActorData *data = (PhysXActorData*)actor->userData;
	data->forces.clear();
}

bool PhysXRigidDynamic::IsKinematic()
{
	PxRigidDynamic *a = GetRigidDynamic();
	if (a)
		return a->getRigidBodyFlags() & PxRigidBodyFlag::eKINEMATIC;
	return _kinematic;
}

float32 PhysXRigidDynamic::GetLinearDamping()
{
	PxRigidDynamic *a = GetRigidDynamic();
	if (a)
		return a->getLinearDamping();
	return _linearDamping;
}

float32 PhysXRigidDynamic::GetAngularDamping()
{
	PxRigidDynamic *a = GetRigidDynamic();
	if (a)
		return a->getAngularDamping();
	return _angularDamping;
}

float32 PhysXRigidDynamic::GetMaxAngularVelocity()
{
	PxRigidDynamic *a = GetRigidDynamic();
	if (a)
		return a->getMaxAngularVelocity();
	return _maxAngularVelocity;
}

float32 PhysXRigidDynamic::GetSleepThreshold()
{
	PxRigidDynamic *a = GetRigidDynamic();
	if (a)
		return a->getSleepThreshold();
	return _sleepThreshold;
}

void PhysXRigidDynamic::GetMinSolverIterations(uint32 &pos, uint32 &vel)
{
	PxRigidDynamic *a = GetRigidDynamic();
	if (a)
		return a->getSolverIterationCounts(pos, vel);
	pos = _minPositionIters;
	vel = _minVelocityIters;
}

float32 PhysXRigidDynamic::GetContactReportThreshold()
{
	PxRigidDynamic *a = GetRigidDynamic();
	if (a)
		return a->getContactReportThreshold();
	return _contactReportThreshold;
}

void PhysXRigidDynamic::SetKinematic(bool k, bool chipSettings)
{
	PxRigidDynamic *a = GetRigidDynamic();
	if (a)
		a->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, k);
	if (chipSettings)
		_kinematic = k;
}

void PhysXRigidDynamic::SetLinearDamping(float32 f, bool chipSettings)
{
	PxRigidDynamic *a = GetRigidDynamic();
	if (a)
		a->setLinearDamping(f);
	if (chipSettings)
		_linearDamping = f;
}

void PhysXRigidDynamic::SetAngularDamping(float32 f, bool chipSettings)
{
	PxRigidDynamic *a = GetRigidDynamic();
	if (a)
		a->setAngularDamping(f);
	if (chipSettings)
		_angularDamping = f;
}

void PhysXRigidDynamic::SetMaxAngularVelocity(float32 f, bool chipSettings)
{
	PxRigidDynamic *a = GetRigidDynamic();
	if (a)
		a->setMaxAngularVelocity(f);
	if (chipSettings)
		_maxAngularVelocity = f;
}

void PhysXRigidDynamic::SetSleepThreshold(float32 f, bool chipSettings)
{
	PxRigidDynamic *a = GetRigidDynamic();
	if (a)
		a->setSleepThreshold(f);
	if (chipSettings)
		_sleepThreshold = f;
}

void PhysXRigidDynamic::SetMinSolverIterations(uint32 pos, uint32 vel, bool chipSettings)
{
	PxRigidDynamic *a = GetRigidDynamic();
	if (a)
		a->setSolverIterationCounts(pos, vel);
	if (chipSettings) {
		_minPositionIters = pos;
		_minVelocityIters = vel;
	}
}

void PhysXRigidDynamic::SetContactReportThreshold(float32 f, bool chipSettings)
{
	PxRigidDynamic *a = GetRigidDynamic();
	if (a)
		a->setContactReportThreshold(f);
	if (chipSettings)
		_contactReportThreshold = f;
}



