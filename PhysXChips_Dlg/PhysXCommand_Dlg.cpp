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
#include "PhysXCommand_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(PhysXCommand_Dlg, PHYSXCOMMAND_GUID);


void PhysXCommand_Dlg::Init()
{
	RPtr sysop = AddItem(MTEXT("Scene Commands"), PhysXCommand::CMD_NONE);
	AddItem(sysop, MTEXT("Simulate"), PhysXCommand::CMD_SCENE_SIMULATE);
	AddItem(sysop, MTEXT("Fetch Results"), PhysXCommand::CMD_SCENE_FETCH_RESULTS);
	AddItem(sysop, MTEXT("Render Debug Geometry"), PhysXCommand::CMD_SCENE_DEBUG_RENDER);
	AddItem(sysop, MTEXT("Start Simulation"), PhysXCommand::CMD_SCENE_START);
	AddItem(sysop, MTEXT("Stop Simulation"), PhysXCommand::CMD_SCENE_STOP);
	AddItem(sysop, MTEXT("Destroy Scene"), PhysXCommand::CMD_SCENE_DESTROY);

	RPtr actorop = AddItem(MTEXT("Actor Commands"), PhysXCommand::CMD_NONE);
	AddItem(actorop, MTEXT("Destroy Actor"), PhysXCommand::CMD_ACTOR_DESTROY);

	RPtr dynamicop = AddItem(MTEXT("Rigid Dynamic Commands"), PhysXCommand::CMD_NONE);
	AddItem(dynamicop, MTEXT("Add Global Force at Global Position"), PhysXCommand::CMD_RIGIDDYNAMIC_ADD_GLOBAL_FORCE_AT_GLOBAL_POINT);
	AddItem(dynamicop, MTEXT("Add Global Force at Local Position"), PhysXCommand::CMD_RIGIDDYNAMIC_ADD_GLOBAL_FORCE_AT_LOCAL_POINT);
	AddItem(dynamicop, MTEXT("Add Local Force at Global Position"), PhysXCommand::CMD_RIGIDDYNAMIC_ADD_LOCAL_FORCE_AT_GLOBAL_POINT);
	AddItem(dynamicop, MTEXT("Add Local Force at Local Position"), PhysXCommand::CMD_RIGIDDYNAMIC_ADD_LOCAL_FORCE_AT_LOCAL_POINT);
	AddItem(dynamicop, MTEXT("Add Global Torque"), PhysXCommand::CMD_RIGIDDYNAMIC_ADD_GLOBAL_TORQUE);
	AddItem(dynamicop, MTEXT("Add Local Torque"), PhysXCommand::CMD_RIGIDDYNAMIC_ADD_LOCAL_TORQUE);
	AddItem(dynamicop, MTEXT("Clear Forces and Torques"), PhysXCommand::CMD_RIGIDDYNAMIC_CLEAR_FORCES_AND_TORQUES);

	RPtr jointop = AddItem(MTEXT("Joint Commands"), PhysXCommand::CMD_NONE);
	AddItem(jointop, MTEXT("Destroy"), PhysXCommand::CMD_JOINT_DESTROY);

	sort();

	SetSelectionChangedCallback([this](RData data) -> bool {
		PhysXCommand::Command cmd = (PhysXCommand::Command)data;
		if (cmd == GetChip()->GetCommand())
			return false;
		GetChip()->SetCommand(cmd);
		return true;
		});

	SetInit(GetChip()->GetCommand(), PhysXCommand::CMD_NONE);
}
