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
#include "PhysXArticulationBase.h"
#include "PhysXScene.h"
#include "PhysXArticulationLink.h"

using namespace m3d;


CHIPDESCV1_DEF_HIDDEN(PhysXArticulationBase, MTEXT("PhysX Articulation Base"), PHYSXARTICULATIONBASE_GUID, CHIP_GUID);



PhysXArticulationBase::PhysXArticulationBase()
{
	CREATE_CHILD(0, PHYSXSCENE_GUID, false, BOTH, MTEXT("Scene"));
	CREATE_CHILD(1, PHYSXARTICULATIONLINK_GUID, false, BOTH, MTEXT("Root Link"));

	_sleepThreshold = 0.05f;
	_minPositionIters = 4;
	_minVelocityIters = 1;
}

PhysXArticulationBase::~PhysXArticulationBase()
{
}

bool PhysXArticulationBase::CopyChip(Chip *chip)
{
	PhysXArticulationBase *c = dynamic_cast<PhysXArticulationBase*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_sleepThreshold = c->_sleepThreshold;
	_minPositionIters = c->_minPositionIters;
	_minVelocityIters = c->_minVelocityIters;
	return true;
}

bool PhysXArticulationBase::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOADDEF("sleepThreshold", _sleepThreshold, 0.05f);
	LOADDEF("minPositionIters", _minPositionIters, 4);
	LOADDEF("minVelocityIters", _minVelocityIters, 1);
	return true;
}

bool PhysXArticulationBase::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVEDEF("sleepThreshold", _sleepThreshold, 0.05f);
	SAVEDEF("minPositionIters", _minPositionIters, 4);
	SAVEDEF("minVelocityIters", _minVelocityIters, 1);
	return true;
}

float32 PhysXArticulationBase::GetSleepThreshold()
{
	return _sleepThreshold;
}

void PhysXArticulationBase::GetMinSolverIterations(uint32 &pos, uint32 &vel)
{
	pos = _minPositionIters;
	vel = _minVelocityIters;
}

void PhysXArticulationBase::SetSleepThreshold(float32 f)
{
	_sleepThreshold = f;

	PxArticulationBase *a = GetArticulation();
	if (a)
		a->setSleepThreshold(f);
}

void PhysXArticulationBase::SetMinSolverIterations(uint32 pos, uint32 vel)
{
	_minPositionIters = pos;
	_minVelocityIters = vel;

	PxArticulationBase *a = GetArticulation();
	if (a)
		a->setSolverIterationCounts(pos, vel);
}