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
#include "PhysXScene_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(PhysXScene_Dlg, PHYSXSCENE_GUID);


void PhysXScene_Dlg::Init()
{
	ComboBoxInitList rate, simLimit, solverType, bpType, frictionType;
	rate.push_back(std::make_pair(String(MTEXT("30 Hz")), 30.0));
	rate.push_back(std::make_pair(String(MTEXT("50 Hz")), 50.0));
	rate.push_back(std::make_pair(String(MTEXT("60 Hz")), 60.0));
	rate.push_back(std::make_pair(String(MTEXT("75 Hz")), 75.0));
	rate.push_back(std::make_pair(String(MTEXT("90 Hz")), 90.0));
	rate.push_back(std::make_pair(String(MTEXT("100 Hz")), 100.0));
	rate.push_back(std::make_pair(String(MTEXT("120 Hz")), 120.0));
	rate.push_back(std::make_pair(String(MTEXT("140 Hz")), 140.0));
	rate.push_back(std::make_pair(String(MTEXT("180 Hz")), 180.0));
	rate.push_back(std::make_pair(String(MTEXT("200 Hz")), 200.0));
	rate.push_back(std::make_pair(String(MTEXT("250 Hz")), 250.0));
	rate.push_back(std::make_pair(String(MTEXT("500 Hz")), 500.0));

	simLimit.push_back(std::make_pair(String(MTEXT("10 ms")), 10.0));
	simLimit.push_back(std::make_pair(String(MTEXT("20 ms")), 20.0));
	simLimit.push_back(std::make_pair(String(MTEXT("50 ms")), 50.0));
	simLimit.push_back(std::make_pair(String(MTEXT("100 ms")), 100.0));
	simLimit.push_back(std::make_pair(String(MTEXT("200 ms")), 200.0));

	bpType.push_back(std::make_pair(String(MTEXT("SAP (3-axes sweep-and-prune)")), (uint32)PxBroadPhaseType::eSAP));
	bpType.push_back(std::make_pair(String(MTEXT("MBP (Multi box pruning)")), (uint32)PxBroadPhaseType::eMBP));
	bpType.push_back(std::make_pair(String(MTEXT("ABP (Automatic box pruning)")), (uint32)PxBroadPhaseType::eABP));
	bpType.push_back(std::make_pair(String(MTEXT("GPU (GPU accelerated)")), (uint32)PxBroadPhaseType::eGPU));

	solverType.push_back(std::make_pair(String(MTEXT("PGS (Projected Gauss-Seidel iterative solver)")), (uint32)PxSolverType::ePGS));
	solverType.push_back(std::make_pair(String(MTEXT("TGS (Temporal Gauss-Seidel solver)")), (uint32)PxSolverType::eTGS));

	frictionType.push_back(std::make_pair(String(MTEXT("Patch Friction Model")), (uint32)PxFrictionType::ePATCH));
	frictionType.push_back(std::make_pair(String(MTEXT("One Directional Per-Contact Friction Model")), (uint32)PxFrictionType::eONE_DIRECTIONAL));
	frictionType.push_back(std::make_pair(String(MTEXT("Two Directional Per-Contact Friction Model")), (uint32)PxFrictionType::eTWO_DIRECTIONAL));

	AddComboBox(MTEXT("Simulation Rate:"), rate, GetChip()->GetSimulationRate(), [this](Id id, RVariant v) { SetDirty(); GetChip()->SetSimulationRate(v.ToDouble()); });
	AddComboBox(MTEXT("Maximum Simulation Time:"), simLimit, GetChip()->GetMaxSimulationTime(), [this](Id id, RVariant v) { SetDirty(); GetChip()->SetMaxSimulationTime(v.ToDouble()); });
	AddComboBox(MTEXT("Solver Type:"), solverType, (uint32)GetChip()->GetSolverType(), [this](Id id, RVariant v) { SetDirty(); GetChip()->SetSolverType((physx::PxSolverType::Enum)v.ToUInt()); });
	AddComboBox(MTEXT("Broad Phase Type:"), bpType, (uint32)GetChip()->GetBroadPhaseAlgorithm(), [this](Id id, RVariant v) { SetDirty(); GetChip()->SetBroadPhaseAlgorithm((physx::PxBroadPhaseType::Enum)v.ToUInt()); });
	AddComboBox(MTEXT("Friction Type:"), frictionType, (uint32)GetChip()->GetFrictionType(), [this](Id id, RVariant v) { SetDirty(); GetChip()->SetFrictionType((physx::PxFrictionType::Enum)v.ToUInt()); });
	
	auto F = [this](Id id, RVariant v)
	{
		SetDirty();
		PxSceneFlags flags;
		if (GetValueFromWidget(1).ToUInt() == RCheckState::Checked) flags |= PxSceneFlag::eENABLE_CCD;
		if (GetValueFromWidget(2).ToUInt() == RCheckState::Unchecked) flags |= PxSceneFlag::eDISABLE_CCD_RESWEEP;
		if (GetValueFromWidget(3).ToUInt() == RCheckState::Checked)  flags |= PxSceneFlag::eENABLE_ENHANCED_DETERMINISM;
		if (GetValueFromWidget(4).ToUInt() == RCheckState::Checked) flags |= PxSceneFlag::eENABLE_FRICTION_EVERY_ITERATION;
		if (GetValueFromWidget(5).ToUInt() == RCheckState::Checked) flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
		if (GetValueFromWidget(6).ToUInt() == RCheckState::Checked) flags |= PxSceneFlag::eENABLE_PCM;
		if (GetValueFromWidget(7).ToUInt() == RCheckState::Checked) flags |= PxSceneFlag::eENABLE_STABILIZATION;
		if (GetValueFromWidget(8).ToUInt() == RCheckState::Checked) flags |= PxSceneFlag::eADAPTIVE_FORCE;
		if (GetValueFromWidget(9).ToUInt() == RCheckState::Checked) flags |= PxSceneFlag::eENABLE_AVERAGE_POINT;
		GetChip()->SetSceneFlags(flags);
	};

	uint32 flags = GetChip()->GetSceneFlags();
	AddCheckBox(1, MTEXT("Enable Continuous Collision Detection (CCD)"), (flags & PxSceneFlag::eENABLE_CCD) != 0 ? RCheckState::Checked : RCheckState::Unchecked, F);
	AddCheckBox(2, MTEXT("Enable CCD Resweep"), (flags & PxSceneFlag::eDISABLE_CCD_RESWEEP) == 0 ? RCheckState::Checked : RCheckState::Unchecked, F);
	AddCheckBox(3, MTEXT("Enable Enhanched Determinism"), (flags & PxSceneFlag::eENABLE_ENHANCED_DETERMINISM) != 0 ? RCheckState::Checked : RCheckState::Unchecked, F);
	AddCheckBox(4, MTEXT("Enable Friction Every Iteration"), (flags & PxSceneFlag::eENABLE_FRICTION_EVERY_ITERATION) != 0 ? RCheckState::Checked : RCheckState::Unchecked, F);
	AddCheckBox(5, MTEXT("Enable GPU Dynamics"), (flags & PxSceneFlag::eENABLE_GPU_DYNAMICS) != 0 ? RCheckState::Checked : RCheckState::Unchecked, F);
	AddCheckBox(6, MTEXT("Enable PCM"), (flags & PxSceneFlag::eENABLE_PCM) != 0 ? RCheckState::Checked : RCheckState::Unchecked, F);
	AddCheckBox(7, MTEXT("Enable Stabilization"), (flags & PxSceneFlag::eENABLE_STABILIZATION) != 0 ? RCheckState::Checked : RCheckState::Unchecked, F);
	AddCheckBox(8, MTEXT("Enable Adaptive Forces"), (flags & PxSceneFlag::eADAPTIVE_FORCE) != 0 ? RCheckState::Checked : RCheckState::Unchecked, F);
	AddCheckBox(9, MTEXT("Enable Average Points"), (flags & PxSceneFlag::eENABLE_AVERAGE_POINT) != 0 ? RCheckState::Checked : RCheckState::Unchecked, F);
//	AddCheckBox(10, MTEXT("Enable Debug Visualization"), RCheckState::Checked, [this](Id id, RVariant v) {});

	AddLine();
	AddPushButton(MTEXT("Start"), [this](Id id, RVariant v) { GetChip()->GetScene(); GetChip()->StartSimulation(); });
	AddPushButton(MTEXT("Stop"), [this](Id id, RVariant v) { GetChip()->StopSimulation(); });
	AddPushButton(MTEXT("Destroy"), [this](Id id, RVariant v) { GetChip()->DestroyScene(); });
}

