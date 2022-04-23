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
#include "PhysXFixedJoint_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(PhysXFixedJoint_Dlg, PHYSXFIXEDJOINT_GUID);


void PhysXFixedJoint_Dlg::Init()
{
	AddDoubleSpinBox(MTEXT("Projection Linear Tolerance"), GetChip()->GetProjectionLinearTolerance(), 0.0f, std::numeric_limits<float32>::max(), 0.1f, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetProjectionLinearTolerance(v.ToFloat()); });
	AddDoubleSpinBox(MTEXT("Projection Angular Tolerance"), GetChip()->GetProjectionAngularTolerance(), 0.0f, XM_PI, 0.1f, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetProjectionAngularTolerance(v.ToFloat()); });
}
