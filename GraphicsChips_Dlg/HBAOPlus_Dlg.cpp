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

#include "stdafx.h"
#include "HBAOPlus_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(HBAOPlusChip_Dlg, HBAOPLUSCHIP_GUID);



void HBAOPlusChip_Dlg::Init()
{
	auto chip = GetChip();
	const auto& p = chip->GetParameters();
	AddDoubleSpinBox(MTEXT("Radius:"), p.Radius, 0.0, 100.0, 0.1, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.Radius = v.ToFloat(); chip->SetParams(p); });
	AddDoubleSpinBox(MTEXT("Bias:"), p.Bias, 0.0, 1.0, 0.05, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.Bias = v.ToFloat(); chip->SetParams(p); });
	AddDoubleSpinBox(MTEXT("Small Scale AO:"), p.SmallScaleAO, 0.0, 2.0, 0.1, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.SmallScaleAO = v.ToFloat(); chip->SetParams(p); });
	AddDoubleSpinBox(MTEXT("Large Scale AO:"), p.LargeScaleAO, 0.0, 2.0, 0.1, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.LargeScaleAO = v.ToFloat(); chip->SetParams(p); });
	AddDoubleSpinBox(MTEXT("Power Exponent:"), p.PowerExponent, 1.0, 4.0, 0.1, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.PowerExponent = v.ToFloat(); chip->SetParams(p); });
	AddComboBox(MTEXT("Step Count:"), { std::make_pair(MTEXT("4 Steps"), false), std::make_pair(MTEXT("8 Steps"), true) }, p.Use8Steps, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.Use8Steps = v.ToBool(); chip->SetParams(p); });
	AddLine();
	AddCheckBox(MTEXT("Enable Depth Threshold"), p.DepthThresholdEnable ? RCheckState::Checked : RCheckState::Unchecked, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.DepthThresholdEnable = v.ToBool(); chip->SetParams(p); });
	AddDoubleSpinBox(MTEXT("Depth Threshold Max View Depth:"), p.DepthThresholdMaxViewDepth, 0.0, 1000.0, 0.1, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.DepthThresholdMaxViewDepth = v.ToFloat(); chip->SetParams(p); });
	AddDoubleSpinBox(MTEXT("Depth Threshold Sharpness:"), p.DepthThresholdSharpness, 0.0, 1000.0, 0.1, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.DepthThresholdSharpness = v.ToFloat(); chip->SetParams(p); });
	AddLine();
	AddCheckBox(MTEXT("Enable Blur"), p.BlurEnable ? RCheckState::Checked : RCheckState::Unchecked, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.BlurEnable = v.ToBool(); chip->SetParams(p); });
	AddComboBox(MTEXT("Blur Raduis:"), { std::make_pair(MTEXT("2 Pixels"), false), std::make_pair(MTEXT("4 Pixels"), true) }, p.BlurRadiusLarge, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.BlurRadiusLarge = v.ToBool(); chip->SetParams(p); });
	AddDoubleSpinBox(MTEXT("Blur Sharpness:"), p.BlurSharpness, 0.0, 100.0, 1.0, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.BlurSharpness = v.ToFloat(); chip->SetParams(p); });
	AddLine();
	AddCheckBox(MTEXT("Enable Blur Sharpness Profile (BSP)"), p.BlurSharpnessProfileEnable ? RCheckState::Checked : RCheckState::Unchecked, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.BlurSharpnessProfileEnable = v.ToBool(); chip->SetParams(p); });
	AddDoubleSpinBox(MTEXT("BSP Foreground Sharpness Scale:"), p.BlurSharpnessProfileForegroundSharpnessScale, 0.0, 100.0, 1.0, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.BlurSharpnessProfileForegroundSharpnessScale = v.ToFloat(); chip->SetParams(p); });
	AddDoubleSpinBox(MTEXT("BSP Foreground View Depth:"), p.BlurSharpnessProfileForegroundViewDepth, 0.0, 1.0, 0.1, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.BlurSharpnessProfileForegroundViewDepth = v.ToFloat(); chip->SetParams(p); });
	AddDoubleSpinBox(MTEXT("BSP Background View Depth:"), p.BlurSharpnessProfileBackgroundViewDepth, 0.0, 1.0, 0.1, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.BlurSharpnessProfileBackgroundViewDepth = v.ToFloat(); chip->SetParams(p); });
	AddLine();
	AddComboBox(MTEXT("Blend Mode:"), { std::make_pair(MTEXT("Overwrite RGB"), (uint32)HBAOPlusChip::OVERWRITE_RGB), std::make_pair(MTEXT("Multiply RGB"), (uint32)HBAOPlusChip::MULTIPLY_RGB) }, p.BlendMode, [this, chip](Id id, RVariant v) { SetDirty(); auto p = chip->GetParameters(); p.BlendMode = (HBAOPlusChip::EBlendMode)v.ToUInt(); chip->SetParams(p); });
}

