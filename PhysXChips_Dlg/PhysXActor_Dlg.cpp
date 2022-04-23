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
#include "PhysXActor_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(PhysXActor_Dlg, PHYSXACTOR_GUID);



void PhysXActor_Dlg::Init()
{
	AddPushButton(MTEXT("Destroy Actor"), [this](Id id, RVariant v) { GetChip()->DestroyActor(); });
	AddCheckBox(MTEXT("Disable Gravity"), GetChip()->IsDisableGravity() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetDisableGravity(v.ToBool()); });
	AddCheckBox(MTEXT("Send Sleep Notifications"), GetChip()->IsSendSleepNotifies() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetSendSleepNotifies(v.ToBool()); });
	AddCheckBox(MTEXT("Enable Debug Visualization"), GetChip()->IsVisualization() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetVisualization(v.ToBool()); });
	AddSpinBox(MTEXT("Dominance Group"), GetChip()->GetDominanceGroup(), 0, 255, 1, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetDominanceGroup(v.ToInt()); });
}

