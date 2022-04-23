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
#include "PhysXRigidStatic.h"
#include "PhysXScene.h"
#include "PhysXShape.h"
#include "StdChips/MatrixChip.h"
#include "StdChips/Value.h"
#include "StdChips/VectorChip.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXRigidStatic, MTEXT("PhysX Rigid Static"), PHYSXRIGIDSTATIC_GUID, PHYSXRIGIDACTOR_GUID);


PhysXRigidStatic::PhysXRigidStatic()
{
	CREATE_CHILD(0, PHYSXSCENE_GUID, false, BOTH, MTEXT("Scene"));
	CREATE_CHILD(1, MATRIXCHIP_GUID, false, UP, MTEXT("Transform"));
	CREATE_CHILD(2, PHYSXSHAPE_GUID, true, UP, MTEXT("Shapes"));

	_actor = nullptr;
}

PhysXRigidStatic::~PhysXRigidStatic()
{
}

void PhysXRigidStatic::CallChip()
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
}

PxRigidStatic *PhysXRigidStatic::CreateActor(const PxTransform &pose)
{
	ChildPtr<PhysXScene> chScene = GetChild(0);
	if (!chScene)
		return nullptr;

	PxScene *scene = chScene->GetScene();
	if (!scene)
		return nullptr;

	PxRigidStatic *actor = scene->getPhysics().createRigidStatic(pose);
	if (!actor)
		return nullptr;

	chScene->RegisterSceneObject(this);

	List<PxReal> shapeMasses;
	uint32 shapeCount = 0;
	
	for (uint32 i = 0; i < GetSubConnectionCount(2); i++) {
		ChildPtr<PhysXShape> ch2 = GetChild(2, i);
		if (ch2) {
			
			PxShape *shape = ch2->GetShape();
			if (shape) {
				actor->attachShape(*shape);
				PxReal mass = ch2->GetMass();
				shapeMasses.push_back(mass);
			}
		}
	}

	PhysXActorData *data = mmnew PhysXActorData(actor);
	data->globalPose = actor->getGlobalPose();
	actor->userData = data;
	scene->addActor(*actor);

	return actor;
}

void PhysXRigidStatic::DestroyActor() 
{
	if (_actor) {
		if (_actor->getScene())
			((PhysXScene*)_actor->getScene()->userData)->UnregisterSceneObject(this);
		mmdelete((PhysXActorData*)_actor->userData);
		_actor->release();
		_actor = nullptr;
	}
}