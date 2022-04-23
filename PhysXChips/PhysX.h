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

#pragma once

#include "Exports.h"


namespace m3d
{

class PHYSXCHIPS_API PhysXSceneObject
{
public:
	virtual void OnSceneDestroyed() = 0;
};

struct PhysXForce
{
	enum Frame { GLOBAL_AT_GLOBAL_POS, GLOBAL_AT_LOCAL_POS, LOCAL_AT_GLOBAL_POS, LOCAL_AT_LOCAL_POS, GLOBAL_TORQUE, LOCAL_TORQUE };
	Frame frame;
	PxVec3 force;
	PxVec3 pos; // Not for torque!
	PxForceMode::Enum mode;

	PhysXForce(Frame frame, const PxVec3 &force, const PxVec3 &pos = PxVec3(0.0f, 0.0f, 0.0f), PxForceMode::Enum mode = PxForceMode::eFORCE) : frame(frame), force(force), pos(pos), mode(mode) {}
};

struct PhysXActorData
{
	PxActor *actor;
	PxTransform globalPose;
	List<PhysXForce> forces;
	PxTransform kinematicTarget;
	bool moveKinematic;
	Chip *chip;
	Chip *linkChip;

	PhysXActorData(PxActor *actor) : actor(actor), moveKinematic(false), chip(nullptr), linkChip(nullptr) {}
};



}