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
#include "PhysXVector_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(PhysXVector_Dlg, PHYSXVECTOR_GUID);


void PhysXVector_Dlg::Init()
{
	RPtr sceneop = AddItem(MTEXT("Scene Vectors"), PhysXVector::CMD_NONE);
	AddItem(sceneop, MTEXT("Get/Set Gravity"), PhysXVector::CMD_SCENE_XET_GRAVITY);

	RPtr rigidbodyop = AddItem(MTEXT("Rigid Body Vectors"), PhysXVector::CMD_NONE);
	AddItem(rigidbodyop, MTEXT("Get/Set Angular Velocity"), PhysXVector::CMD_RIGID_BODY_XET_ANGULAR_VELOCITY);
	AddItem(rigidbodyop, MTEXT("Get/Set Linear Velocity"), PhysXVector::CMD_RIGID_BODY_XET_LINEAR_VELOCITY);
	AddItem(rigidbodyop, MTEXT("Get/Set Mass Space Inertia Tensor"), PhysXVector::CMD_RIGID_BODY_XET_MASS_SPACE_INERTIA_TENSOR);

	RPtr jointop = AddItem(MTEXT("Joint Vectors"), PhysXVector::CMD_NONE);
	AddItem(jointop, MTEXT("Get Linear Force"), PhysXVector::CMD_CONSTRAINT_GET_LINEAR_FORCE);
	AddItem(jointop, MTEXT("Get Angular Force"), PhysXVector::CMD_CONSTRAINT_GET_ANGULAR_FORCE);

	sort();

	SetSelectionChangedCallback([this](RData data) -> bool {
		PhysXVector::Command cmd = (PhysXVector::Command)data;
		if (cmd == GetChip()->GetCommand())
			return false;
		GetChip()->SetCommand(cmd);
		return true;
		});

	SetInit(GetChip()->GetCommand(), PhysXVector::CMD_NONE);
}
