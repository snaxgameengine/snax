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
#include "PhysXRigidDynamic_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(PhysXRigidDynamic_Dlg, PHYSXRIGIDDYNAMIC_GUID);



void PhysXRigidDynamic_Dlg::Init()
{
	AddCheckBox(MTEXT("Kinematic"), GetChip()->IsKinematic() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetKinematic(v.ToUInt() == RCheckState::Checked); });
	AddDoubleSpinBox(MTEXT("Linear Damping"), GetChip()->GetLinearDamping(), 0, std::numeric_limits<float32>::max(), 0.1, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetLinearDamping(v.ToFloat()); });
	AddDoubleSpinBox(MTEXT("Angular Damping"), GetChip()->GetAngularDamping(), 0, std::numeric_limits<float32>::max(), 0.1, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetAngularDamping(v.ToFloat()); });
	AddDoubleSpinBox(MTEXT("Max Angular Velocity"), GetChip()->GetMaxAngularVelocity(), 0.0001, std::numeric_limits<float32>::max(), 0.1, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetMaxAngularVelocity(v.ToFloat()); });
	AddDoubleSpinBox(MTEXT("Sleep Threshold"), GetChip()->GetSleepThreshold(), 0.0001, std::numeric_limits<float32>::max(), 0.1, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetSleepThreshold(v.ToFloat()); });

	uint32 p, v;
	GetChip()->GetMinSolverIterations(p, v);
	AddSpinBox(1, MTEXT("Minimum Position Iterations"), p, 1, 255, 1, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetMinSolverIterations(v.ToInt(), GetValueFromWidget(2).ToInt()); });
	AddSpinBox(2, MTEXT("Minimum Velocity Iterations"), v, 1, 255, 1, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetMinSolverIterations(GetValueFromWidget(1).ToInt(), v.ToInt()); });
	AddDoubleSpinBox(MTEXT("Contact Report Threshold"), GetChip()->GetContactReportThreshold(), 0.0001, std::numeric_limits<float32>::max(), 0.1, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetContactReportThreshold(v.ToFloat()); });
}
