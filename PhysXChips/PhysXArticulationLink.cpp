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
#include "PhysXArticulationLink.h"
#include "PhysXArticulationJointRC.h"
#include "PhysXScene.h"
#include "PhysXShape.h"
#include "StdChips/MatrixChip.h"
#include "StdChips/Value.h"
#include "StdChips/VectorChip.h"


using namespace m3d;

CHIPDESCV1_DEF(PhysXArticulationLink, MTEXT("PhysX Articulation Link"), PHYSXARTICULATIONLINK_GUID, PHYSXRIGIDBODY_GUID);


PhysXArticulationLink::PhysXArticulationLink() : _link(nullptr)
{
	CREATE_CHILD(0, MATRIXCHIP_GUID, false, UP, MTEXT("Transform"));
	CREATE_CHILD(1, VALUE_GUID, false, UP, MTEXT("Mass"));
	CREATE_CHILD(2, VECTORCHIP_GUID, false, UP, MTEXT("Center of Mass"));
	CREATE_CHILD(3, PHYSXSHAPE_GUID, true, UP, MTEXT("Shapes"));
	CREATE_CHILD(4, PHYSXARTICULATIONJOINTBASE_GUID, true, BOTH, MTEXT("Child Links"));
}

PhysXArticulationLink::~PhysXArticulationLink()
{
	if (_link) {
		PhysXActorData *d = (PhysXActorData*)_link->userData;
		if (d && d->chip == this)
			d->chip = nullptr;
	}
}

PxArticulationLink *PhysXArticulationLink::CreateLink(PxArticulationReducedCoordinate *a, PxArticulationLink *p)
{
	if (_link)
		return nullptr;

	PxTransform transform = PxTransform(PxIdentity);

	ChildPtr<MatrixChip> chTransform = GetChild(0);
	if (chTransform) {
		transform = PxTransform(PxMat44((float32*)&chTransform->GetMatrix()));
	}

	PxArticulationLink* link = a->createLink(p, transform);
	if (!link)
		return nullptr;

	// Note: a-links can not be kinematic, no damping or contact report thresholds!
	//	link->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, _disableGravity);
	//	link->setActorFlag(PxActorFlag::eSEND_SLEEP_NOTIFIES, _sendSleepNotifies);

	List<PxReal> shapeMasses;

	for (uint32 i = 0; i < GetSubConnectionCount(3); i++) {
		ChildPtr<PhysXShape> chShape = GetChild(3, i);
		if (chShape) {
			PxShape *shape = chShape->GetShape();
			if (shape) {
				link->attachShape(*shape);
				PxReal mass = chShape->GetMass();
				shapeMasses.push_back(mass);
			}
		}
	}

	float32 mass = 0.0f;
	ChildPtr<Value> chMass = GetChild(1);
	if (chMass)
		mass = (float32)chMass->GetValue();
	mass = std::max(mass, 0.0f);

	PxVec3 com(0.0f, 0.0f, 0.0f);
	ChildPtr<VectorChip> chCOM = GetChild(2);
	if (chCOM) {
		XMFLOAT4 v = chCOM->GetVector();
		com = PxVec3(v.x, v.y, v.z);
	}

	if (mass == 0.0f) {
		bool b = PxRigidBodyExt::setMassAndUpdateInertia(*link, &shapeMasses.front(), (PxU32)shapeMasses.size(), &com);
	}
	else {
		bool b = PxRigidBodyExt::setMassAndUpdateInertia(*link, mass, &com);
	}

	PhysXActorData *data = mmnew PhysXActorData(link);
	data->globalPose = link->getGlobalPose();
	data->chip = this;
	link->userData = data;

	for (size_t i = 0; i < GetSubConnectionCount(4); i++) {
		ChildPtr<PhysXArticulationJointRC> ch4 = GetChild(4, i);
		if (!ch4)
			continue;
		if (ch4->CreateLink(a, link) == nullptr)
			continue;
	}

	return link;
}

void PhysXArticulationLink::ReleaseLink()
{
	_link = nullptr;
}