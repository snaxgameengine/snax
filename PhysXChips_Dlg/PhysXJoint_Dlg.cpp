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
#include "PhysXJoint_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(PhysXJoint_Dlg, PHYSXJOINT_GUID);


void PhysXJoint_Dlg::Init()
{
	AddPushButton(MTEXT("Destroy Joint"), [this](Id id, RVariant v) { GetChip()->DestroyJoint(); });
	AddCheckBox(MTEXT("Global Frame"), GetChip()->IsGlobalFrame() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) { GetChip()->SetGlobalFrame(v.ToUInt() == RCheckState::Checked); });
	AddCheckBox(MTEXT("Collision"), GetChip()->IsCollisionEnabled() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) { GetChip()->SetCollisionEnabled(v.ToUInt() == RCheckState::Checked); });
	AddCheckBox(MTEXT("Projection"), GetChip()->IsProjectionEnabled() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) { GetChip()->SetVisualization(v.ToUInt() == RCheckState::Checked); });

	float32 f, t;
	GetChip()->GetBreakForce(f, t);
	AddDoubleSpinBox(1, MTEXT("Break Force"), f, 0, std::numeric_limits<float32>::max(), 1, [this](Id id, RVariant v) { GetChip()->SetBreakForce(v.ToFloat(), GetValueFromWidget(2).ToFloat()); });
	AddDoubleSpinBox(2, MTEXT("Break Torque"), t, 0, std::numeric_limits<float32>::max(), 1, [this](Id id, RVariant v) { GetChip()->SetBreakForce(GetValueFromWidget(1).ToFloat(), v.ToFloat()); });
}
