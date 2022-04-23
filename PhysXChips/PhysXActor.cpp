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
#include "PhysXRigidActor.h"

using namespace m3d;


CHIPDESCV1_DEF(PhysXActor, MTEXT("PhysX Actor"), PHYSXACTOR_GUID, CHIP_GUID);


PhysXActor::PhysXActor()
{
	_disableGravity = false;
	_sendSleepNotifies = false;
	_visualization = true;
	_dominanceGroup = 0;
}

PhysXActor::~PhysXActor()
{
}

bool PhysXActor::CopyChip(Chip *chip)
{
	PhysXActor *c = dynamic_cast<PhysXActor*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_disableGravity = c->_disableGravity;
	_sendSleepNotifies = c->_sendSleepNotifies;
	_visualization = c->_visualization;
	_dominanceGroup = c->_dominanceGroup;
	return true;
}

bool PhysXActor::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOADDEF("disableGravity", _disableGravity, false);
	LOADDEF("sendSleepNotifies", _sendSleepNotifies, false);
	LOADDEF("visualization", _visualization, true);
	LOADDEF("dominanceGroup", _dominanceGroup, 0);
	return true;
}

bool PhysXActor::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVEDEF("disableGravity", _disableGravity, false);
	SAVEDEF("sendSleepNotifies", _sendSleepNotifies, false);
	SAVEDEF("visualization", _visualization, true);
	SAVEDEF("dominanceGroup", _dominanceGroup, 0);
	return true;
}

bool PhysXActor::Exist() const
{
	PxActor *a = GetActor();
	assert(!a || a->getScene());
	return a != nullptr;
}

bool PhysXActor::IsDisableGravity() const
{
	PxActor *a = GetActor();
	if (a)
		return a->getActorFlags() == PxActorFlag::eDISABLE_GRAVITY;
	return _disableGravity;
}

bool PhysXActor::IsSendSleepNotifies() const
{
	PxActor *a = GetActor();
	if (a)
		return a->getActorFlags() == PxActorFlag::eSEND_SLEEP_NOTIFIES;
	return _sendSleepNotifies;
}

bool PhysXActor::IsVisualization() const
{
	PxActor *a = GetActor();
	if (a)
		return a->getActorFlags() == PxActorFlag::eVISUALIZATION;
	return _visualization;
}

PxDominanceGroup PhysXActor::GetDominanceGroup() const
{
	PxActor *a = GetActor();
	if (a)
		a->getDominanceGroup();
	return _dominanceGroup;
}

void PhysXActor::SetDisableGravity(bool b, bool chipSettings)
{
	PxActor *a = GetActor();
	if (a)
		a->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, b);
	if (chipSettings)
		_disableGravity = b;
}

void PhysXActor::SetSendSleepNotifies(bool b, bool chipSettings)
{
	PxActor *a = GetActor();
	if (a)
		a->setActorFlag(PxActorFlag::eSEND_SLEEP_NOTIFIES, b);
	if (chipSettings)
		_sendSleepNotifies = b;
}

void PhysXActor::SetVisualization(bool b, bool chipSettings)
{
	PxActor *a = GetActor();
	if (a)
		a->setActorFlag(PxActorFlag::eVISUALIZATION, b);
	if (chipSettings)
		_visualization = b;
}

void PhysXActor::SetDominanceGroup(PxDominanceGroup b, bool chipSettings)
{
	PxActor *a = GetActor();
	if (a)
		a->setDominanceGroup(b);
	if (chipSettings)
		_dominanceGroup = b;
}
