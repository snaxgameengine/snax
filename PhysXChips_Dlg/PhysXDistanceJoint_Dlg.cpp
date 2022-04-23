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
#include "PhysXDistanceJoint_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(PhysXDistanceJoint_Dlg, PHYSXDISTANCEJOINT_GUID);


void PhysXDistanceJoint_Dlg::Init()
{
	AddCheckBox(1, MTEXT("Enable Min Limit"), GetChip()->IsMinDistanceEnabled() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetMinDistanceEnabled(v.ToUInt() == RCheckState::Checked); _enableButtons(); });
	AddCheckBox(2, MTEXT("Enable Max Limit"), GetChip()->IsMaxDistanceEnabled() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetMaxDistanceEnabled(v.ToUInt() == RCheckState::Checked); _enableButtons(); });
	AddDoubleSpinBox(3, MTEXT("Min Distance"), GetChip()->GetMinDistance(), 0.0f, std::numeric_limits<float32>::max(), 0.1f, [this](Id id, RVariant v) { GetChip()->SetDistanceRange(v.ToFloat(), GetChip()->GetMaxDistance()); });
	AddDoubleSpinBox(4, MTEXT("Max Distance"), GetChip()->GetMaxDistance(), 0.0f, std::numeric_limits<float32>::max(), 0.1f, [this](Id id, RVariant v) { GetChip()->SetDistanceRange(GetChip()->GetMinDistance(), v.ToFloat()); });
	AddDoubleSpinBox(5, MTEXT("Limit Restitution"), GetChip()->GetTolerance(), 0.0f, std::numeric_limits<float32>::max(), 0.1f, [this](Id id, RVariant v) { GetChip()->SetTolerance(v.ToFloat()); });
	AddCheckBox(6, MTEXT("Limit Spring"), GetChip()->IsSpringEnabled() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetSpringEnabled(v.ToUInt() == RCheckState::Checked); _enableButtons(); });
	AddDoubleSpinBox(7, MTEXT("Limit Damping"), GetChip()->GetStiffness(), 0.0f, std::numeric_limits<float32>::max(), 0.1f, [this](Id id, RVariant v) { GetChip()->SetStiffness(v.ToFloat()); });
	AddDoubleSpinBox(8, MTEXT("Limit Contact Distance"), GetChip()->GetDamping(), 0.0f, std::numeric_limits<float32>::max(), 0.1f, [this](Id id, RVariant v) { GetChip()->SetDamping(v.ToFloat()); });

	_enableButtons();
}

void PhysXDistanceJoint_Dlg::_enableButtons()
{
	bool b = GetValueFromWidget(6).ToUInt() == RCheckState::Checked;
	SetWidgetEnabled(7, b);
	SetWidgetEnabled(8, b);
	bool b1 = GetValueFromWidget(1).ToUInt() == RCheckState::Checked;
	bool b2 = GetValueFromWidget(2).ToUInt() == RCheckState::Checked;
	SetWidgetEnabled(3, b1);
	SetWidgetEnabled(4, b2);
	SetWidgetEnabled(5, b1 || b2);
}