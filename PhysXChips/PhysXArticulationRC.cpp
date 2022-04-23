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
#include "PhysXArticulationRC.h"
#include "PhysXScene.h"
#include "PhysXArticulationLink.h"
#include "PhysXArticulationJointBase.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXArticulationRC, MTEXT("PhysX Articulation RC"), PHYSXARTICULATIONRC_GUID, PHYSXARTICULATIONBASE_GUID);



PhysXArticulationRC::PhysXArticulationRC()
{
	_articulation = nullptr;
}

PhysXArticulationRC::~PhysXArticulationRC()
{
}

bool PhysXArticulationRC::CopyChip(Chip *chip)
{
	PhysXArticulationRC *c = dynamic_cast<PhysXArticulationRC*>(chip);
	B_RETURN(PhysXArticulationBase::CopyChip(c));
	_flags = c->_flags;
	return true;
}

bool PhysXArticulationRC::LoadChip(DocumentLoader &loader)
{
	B_RETURN(PhysXArticulationBase::LoadChip(loader));
	LOADDEF("flags", (PxArticulationFlags::InternalType&)_flags, 0);
	return true;
}

bool PhysXArticulationRC::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(PhysXArticulationBase::SaveChip(saver));
	SAVEDEF("flags", (const PxArticulationFlags::InternalType&)_flags, 0);
	return true;
}

void PhysXArticulationRC::CallChip()
{
	if (Exist())
		return;

	_articulation = (PxArticulationReducedCoordinate*)CreateArticulation();

	if (_articulation)
		((PhysXScene*)_articulation->getScene()->userData)->RegisterSceneObject(this);
}

PxArticulationBase *PhysXArticulationRC::CreateArticulation()
{
	ChildPtr<PhysXScene> chScene = GetChild(0);
	if (!chScene)
		return nullptr;

	PxScene *scene = chScene->GetScene();
	if (!scene)
		return nullptr;

	ChildPtr<PhysXArticulationLink> chLink = GetChild(1);
	if (!chLink)
		return nullptr;

	PxArticulationReducedCoordinate *arti = scene->getPhysics().createArticulationReducedCoordinate();
	if (!arti)
		return nullptr;

	arti->setArticulationFlags(_flags);
	arti->setSolverIterationCounts(_minPositionIters, _minVelocityIters);
	arti->setSleepThreshold(_sleepThreshold);

	PxArticulationLink *rootLink = chLink->CreateLink(arti, nullptr);
	if (!rootLink) {
		arti->release();
		return nullptr;
	}

	scene->addArticulation(*arti);

	return arti;
}

void PhysXArticulationRC::DestroyArticulation() 
{
	if (!_articulation)
		return;

	if (_articulation->getScene())
		((PhysXScene*)_articulation->getScene()->userData)->UnregisterSceneObject(this);
	List<PxArticulationLink*> links(_articulation->getNbLinks(), (PxArticulationLink*)nullptr);
	_articulation->getLinks(&links.front(), _articulation->getNbLinks());
	for (size_t i = 0; i < links.size(); i++) {
		PxArticulationLink *l = links[i];
		const PhysXActorData *d =(const PhysXActorData*)l->userData;
		if (d) {
			PhysXArticulationLink *chip = dynamic_cast<PhysXArticulationLink*>(d->chip);
			if (chip)
				chip->ReleaseLink();
			PhysXArticulationJointBase *linkChip = dynamic_cast<PhysXArticulationJointBase*>(d->linkChip);
			if (linkChip)
				linkChip->ReleaseJoint();
			mmdelete(d);
		}
	}
	_articulation->release();
	_articulation = nullptr;
}

PxArticulationFlags PhysXArticulationRC::GetFlags() const
{
	return _flags;
}

void PhysXArticulationRC::SetFlags(PxArticulationFlags f)
{
	_flags = f;

	if (_articulation)
		_articulation->setArticulationFlags(f);
}