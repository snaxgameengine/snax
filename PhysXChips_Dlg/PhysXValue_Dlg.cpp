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
#include "PhysXValue_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(PhysXValue_Dlg, PHYSXVALUE_GUID);



void PhysXValue_Dlg::Init()
{
	RPtr sceneop = AddItem(MTEXT("Scene Values"), PhysXValue::CMD_NONE);
	AddItem(sceneop, MTEXT("Exist?"), PhysXValue::CMD_SCENE_EXIST);
	AddItem(sceneop, MTEXT("Is Running?"), PhysXValue::CMD_SCENE_IS_RUNNING);
	AddItem(sceneop, MTEXT("Is Simulating?"), PhysXValue::CMD_SCENE_IS_SIMULATING);
	AddItem(sceneop, MTEXT("Real-Time Index"), PhysXValue::CMD_SCENE_REAL_TIME_INDEX);
	AddItem(sceneop, MTEXT("Simulation Index"), PhysXValue::CMD_SCENE_SIM_INDEX);
	AddItem(sceneop, MTEXT("Simulation Delay (ms)"), PhysXValue::CMD_SCENE_SIM_WAIT);

	RPtr actorop = AddItem(MTEXT("Actor Values"), PhysXValue::CMD_NONE);
	AddItem(actorop, MTEXT("Exist?"), PhysXValue::CMD_ACTOR_EXIST);

	RPtr rigidbodyop = AddItem(MTEXT("Rigid Body Values"), PhysXValue::CMD_NONE);
	AddItem(rigidbodyop, MTEXT("Get/Set Mass"), PhysXValue::CMD_RIGID_BODY_XET_MASS);

	RPtr rigiddynamicop = AddItem(MTEXT("Rigid Dynamic Values"), PhysXValue::CMD_NONE);
	AddItem(rigiddynamicop, MTEXT("Get/Set Linear Damping"), PhysXValue::CMD_RIGID_DYNAMIC_XET_LINEAR_DAMPING);
	AddItem(rigiddynamicop, MTEXT("Get/Set Angular Damping"), PhysXValue::CMD_RIGID_DYNAMIC_XET_ANGULAR_DAMPING);
	AddItem(rigiddynamicop, MTEXT("Get/Set Max Angular Velocity"), PhysXValue::CMD_RIGID_DYNAMIC_XET_MAX_ANGULAR_VELOCITY);
	AddItem(rigiddynamicop, MTEXT("Get/Set Sleep Threshold"), PhysXValue::CMD_RIGID_DYNAMIC_XET_SLEEP_THRESHOLD);
	AddItem(rigiddynamicop, MTEXT("Is Sleeping?"), PhysXValue::CMD_RIGID_DYNAMIC_IS_SLEEPING);

	RPtr jointop = AddItem(MTEXT("Joint Values"), PhysXValue::CMD_NONE);
	AddItem(jointop, MTEXT("Exist?"), PhysXValue::CMD_JOINT_EXIST);

	sort();

	SetSelectionChangedCallback([this](RData data) -> bool {
		PhysXValue::Command cmd = (PhysXValue::Command)data;
		if (cmd == GetChip()->GetCommand())
			return false;
		GetChip()->SetCommand(cmd);
		return true;
		});

	SetInit(GetChip()->GetCommand(), PhysXValue::CMD_NONE);
}
