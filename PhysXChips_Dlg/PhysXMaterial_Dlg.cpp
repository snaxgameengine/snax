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
#include "PhysXMaterial_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(PhysXMaterial_Dlg, PHYSXMATERIAL_GUID);


void PhysXMaterial_Dlg::Init()
{
	AddDoubleSpinBox(MTEXT("Static Friction"), GetChip()->GetStaticFriction(), 0, std::numeric_limits<float32>::max(), 0.1f, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetStaticFriction(v.ToFloat()); });
	AddDoubleSpinBox(MTEXT("Dynamic Friction"), GetChip()->GetDynamicFriction(), 0, std::numeric_limits<float32>::max(), 0.1f, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetDynamicFriction(v.ToFloat()); });
	AddDoubleSpinBox(MTEXT("Restitution"), GetChip()->GetRestitution(), 0, 1, 0.1f, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetRestitution(v.ToFloat()); });
	AddCheckBox(1, MTEXT("Disable Friction"), GetChip()->IsDisableFriction() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetDisableFriction(v.ToUInt() == RCheckState::Checked); _enableButtons(); });
	AddCheckBox(2, MTEXT("Disable Strong Friction"), GetChip()->IsDisableStrongFriction() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetDisableStrongFriction(v.ToUInt() == RCheckState::Checked); });
	ComboBoxInitList lst;
	lst.push_back(std::make_pair(String(MTEXT("Average")), RVariant(physx::PxCombineMode::eAVERAGE)));
	lst.push_back(std::make_pair(String(MTEXT("Minimum")), RVariant(physx::PxCombineMode::eMIN)));
	lst.push_back(std::make_pair(String(MTEXT("Multiply")), RVariant(physx::PxCombineMode::eMULTIPLY)));
	lst.push_back(std::make_pair(String(MTEXT("Maximum")), RVariant(physx::PxCombineMode::eMAX)));
	AddComboBox(3, MTEXT("Friction Combine Mode"), lst, GetChip()->GetFrictionCombineMode(), [this](Id id, RVariant v) { SetDirty(); GetChip()->SetFrictionCombineMode((physx::PxCombineMode::Enum)v.ToUInt()); });
	AddComboBox(4, MTEXT("Restitution Combine Mode"), lst, GetChip()->GetRestitutionCombineMode(), [this](Id id, RVariant v) { SetDirty(); GetChip()->SetRestitutionCombineMode((physx::PxCombineMode::Enum)v.ToUInt()); });

	_enableButtons();
}

void PhysXMaterial_Dlg::_enableButtons()
{
	bool b = GetValueFromWidget(1).ToUInt() == RCheckState::Unchecked;
	SetWidgetEnabled(2, b);
	SetWidgetEnabled(3, b);
}