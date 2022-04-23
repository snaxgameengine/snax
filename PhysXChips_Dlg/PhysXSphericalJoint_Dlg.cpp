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
#include "PhysXSphericalJoint_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(PhysXSphericalJoint_Dlg, PHYSXSPHERICALJOINT_GUID);


void PhysXSphericalJoint_Dlg::Init()
{
	AddCheckBox(1, MTEXT("Enable Limits"), GetChip()->IsLimitEnabled() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetLimitEnable(v.ToUInt() == RCheckState::Checked); _enableButtons(); });

	float32 y, z, r, s, d, c;
	GetChip()->GetLimitParams(y, z, r, s, d, c);

	auto F = [this](Id id, RVariant v)
	{
		SetDirty();
		float32 y, z, r, s, d, c;
		y = GetValueFromWidget(2).ToFloat();
		z = GetValueFromWidget(3).ToFloat();
		r = GetValueFromWidget(4).ToFloat();
		s = GetValueFromWidget(5).ToFloat();
		d = GetValueFromWidget(6).ToFloat();
		c = GetValueFromWidget(7).ToFloat();
		GetChip()->SetLimitParams(y, z, r, s, d, c);

	};

	AddDoubleSpinBox(2, MTEXT("y-Limit Angle"), y, 0.01f, XM_PI - 0.01f, 0.1f, F);
	AddDoubleSpinBox(3, MTEXT("z-Limit Angle"), z, 0.01f, XM_PI - 0.01f, 0.1f, F);
	AddDoubleSpinBox(4, MTEXT("Limit Restitution"), r, 0.0f, 1.0f, 0.1f, F);
	AddDoubleSpinBox(5, MTEXT("Limit Spring"), s, 0.0f, std::numeric_limits<float32>::max(), 0.1f, F);
	AddDoubleSpinBox(6, MTEXT("Limit Damping"), d, 0.0f, std::numeric_limits<float32>::max(), 0.1f, F);
	AddDoubleSpinBox(7, MTEXT("Limit Contact Distance"), c, 0.0f, std::numeric_limits<float32>::max(), 0.1f, F);
	AddDoubleSpinBox(8, MTEXT("Projection Linear Tolerance"), GetChip()->GetProjectionLinearTolerance(), 0.0f, std::numeric_limits<float32>::max(), 0.1f, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetProjectionLinearTolerance(v.ToFloat()); });
	_enableButtons();
}

void PhysXSphericalJoint_Dlg::_enableButtons()
{
	bool b = GetValueFromWidget(1).ToUInt() == RCheckState::Checked;
	SetWidgetEnabled(2, b);
	SetWidgetEnabled(3, b);
	SetWidgetEnabled(4, b);
	SetWidgetEnabled(5, b);
	SetWidgetEnabled(6, b);
	SetWidgetEnabled(7, b);
}