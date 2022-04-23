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
#include "PhysXArticulationJointRC_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(PhysXArticulationJointRC_Dlg, PHYSXARTICULATIONJOINTRC_GUID);



void PhysXArticulationJointRC_Dlg::Init()
{
	ComboBoxInitList type, axis, motion;
	type.push_back(std::make_pair(String(MTEXT("Prismatic")), PxArticulationJointType::ePRISMATIC));
	type.push_back(std::make_pair(String(MTEXT("Revolute")), PxArticulationJointType::eREVOLUTE));
	type.push_back(std::make_pair(String(MTEXT("Spherical")), PxArticulationJointType::eSPHERICAL));
	type.push_back(std::make_pair(String(MTEXT("Fixed")), PxArticulationJointType::eFIX));
	axis.push_back(std::make_pair(String(MTEXT("Twist")), PxArticulationAxis::eTWIST));
	axis.push_back(std::make_pair(String(MTEXT("Swing 1")), PxArticulationAxis::eSWING1));
	axis.push_back(std::make_pair(String(MTEXT("Swing 2")), PxArticulationAxis::eSWING2));
	axis.push_back(std::make_pair(String(MTEXT("X")), PxArticulationAxis::eX));
	axis.push_back(std::make_pair(String(MTEXT("Y")), PxArticulationAxis::eY));
	axis.push_back(std::make_pair(String(MTEXT("Z")), PxArticulationAxis::eZ));
	motion.push_back(std::make_pair(String(MTEXT("Locked")), PxArticulationMotion::eLOCKED));
	motion.push_back(std::make_pair(String(MTEXT("Limited")), PxArticulationMotion::eLIMITED));
	motion.push_back(std::make_pair(String(MTEXT("Free")), PxArticulationMotion::eFREE));

	auto *c = GetChip();

	AddCheckBox(1, MTEXT("Global Frame"), c->IsGlobalFrame() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetGlobalFrame(v.ToUInt() == RCheckState::Checked); } );
	AddComboBox(2, MTEXT("Type"), type, (uint32)c->GetType(), [this](Id id, RVariant v) { SetDirty(); GetChip()->SetType((PxArticulationJointType::Enum)v.ToUInt()); } );
	AddDoubleSpinBox(3, MTEXT("Friction Coefficient"), c->GetFrictionCoefficient(), 0.0f, std::numeric_limits<float32>::max(), 1.0f, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetFrictionCoefficient((PxReal)v.ToDouble()); } );
	AddDoubleSpinBox(4, MTEXT("Max Joint Velocity"), c->GetMaxJointVelocity(), 0.0f, std::numeric_limits<float32>::max(), 1.0f, [this](Id id, RVariant v) { SetDirty(); GetChip()->SetMaxJointVelocity((PxReal)v.ToDouble()); } );
	AddLine();

	AddComboBox(5, MTEXT("Axis"), axis, PxArticulationAxis::eTWIST, [this](Id id, RVariant v) {
		auto axis = (PxArticulationAxis::Enum)v.ToUInt();
		Activate(10 + axis); // Motion
		Activate(20 + axis); // Low Limit
		Activate(30 + axis); // High Limit
		Activate(40 + axis); // Stiffness
		Activate(50 + axis); // Damping
		Activate(60 + axis); // Max Force
		Activate(70 + axis); // Is Acceleration Drive
		Activate(80 + axis); // Drive Target
		Activate(90 + axis); // Drive Target Vel.
	} );

	const PhysXArticulationJointRC::AxisParams ap[PxArticulationAxis::eCOUNT] = { 
		c->GetAxisParams(PxArticulationAxis::eTWIST), c->GetAxisParams(PxArticulationAxis::eSWING1), c->GetAxisParams(PxArticulationAxis::eSWING2), 
		c->GetAxisParams(PxArticulationAxis::eX), c->GetAxisParams(PxArticulationAxis::eY), c->GetAxisParams(PxArticulationAxis::eZ) };

	// Motion
	{
		auto cb = [this](Id id, RVariant v) {
			auto axis = (PxArticulationAxis::Enum)(id - 10);
			GetChip()->SetMotion(axis, (PxArticulationMotion::Enum)v.ToUInt());
			SetDirty();
		};
		auto *w = AddComboBox(10, MTEXT("Motion"), motion, ap[PxArticulationAxis::eTWIST].motion, cb);
		for (int32 i = 1; i < PxArticulationAxis::eCOUNT; i++)
			OverloadComboBox(10 + i, w, ap[PxArticulationAxis::eTWIST + i].motion, cb);
	}

	// Low Limit
	{
		auto cb = [this](Id id, RVariant v) {
			auto axis = (PxArticulationAxis::Enum)(id - 20);
			GetChip()->SetLowLimit(axis, (PxReal)v.ToDouble());
			SetDirty();
		};
		auto *w = AddDoubleSpinBox(20, MTEXT("Low Limit"), ap[PxArticulationAxis::eTWIST].lowLimit, -std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max(), 1.0f, cb);
		for (int32 i = 1; i < PxArticulationAxis::eCOUNT; i++)
			OverloadDoubleSpinBox(20 + i, w, ap[PxArticulationAxis::eTWIST + i].lowLimit, cb);
	}

	// High Limit
	{
		auto cb = [this](Id id, RVariant v) {
			auto axis = (PxArticulationAxis::Enum)(id - 30);
			GetChip()->SetHighLimit(axis, (PxReal)v.ToDouble());
			SetDirty();
		};
		auto *w = AddDoubleSpinBox(30, MTEXT("High Limit"), ap[PxArticulationAxis::eTWIST].highLimit, -std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max(), 1.0f, cb);
		for (int32 i = 1; i < PxArticulationAxis::eCOUNT; i++)
			OverloadDoubleSpinBox(30 + i, w, ap[PxArticulationAxis::eTWIST + i].highLimit, cb);
	}

	// Stiffness
	{
		auto cb = [this](Id id, RVariant v) {
			auto axis = (PxArticulationAxis::Enum)(id - 40);
			GetChip()->SetStiffness(axis, (PxReal)v.ToDouble());
			SetDirty();
		};
		auto *w = AddDoubleSpinBox(40, MTEXT("Stiffness"), ap[PxArticulationAxis::eTWIST].stiffness, -std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max(), 1.0f, cb);
		for (int32 i = 1; i < PxArticulationAxis::eCOUNT; i++)
			OverloadDoubleSpinBox(40 + i, w, ap[PxArticulationAxis::eTWIST + i].stiffness, cb);
	}

	// Damping
	{
		auto cb = [this](Id id, RVariant v) {
			auto axis = (PxArticulationAxis::Enum)(id - 50);
			GetChip()->SetDamping(axis, (PxReal)v.ToDouble());
			SetDirty();
		};
		auto *w = AddDoubleSpinBox(50, MTEXT("Damping"), ap[PxArticulationAxis::eTWIST].damping, -std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max(), 1.0f, cb);
		for (int32 i = 1; i < PxArticulationAxis::eCOUNT; i++)
			OverloadDoubleSpinBox(50 + i, w, ap[PxArticulationAxis::eTWIST + i].damping, cb);
	}

	// Max Force
	{
		auto cb = [this](Id id, RVariant v) {
			auto axis = (PxArticulationAxis::Enum)(id - 60);
			GetChip()->SetMaxForce(axis, (PxReal)v.ToDouble());
			SetDirty();
		};
		auto *w = AddDoubleSpinBox(60, MTEXT("Max Force"), ap[PxArticulationAxis::eTWIST].maxForce, 0, std::numeric_limits<float32>::max(), 1.0f, cb);
		for (int32 i = 1; i < PxArticulationAxis::eCOUNT; i++)
			OverloadDoubleSpinBox(60 + i, w, ap[PxArticulationAxis::eTWIST + i].maxForce, cb);
	}

	// Is acceleration Drive
	{
		auto cb = [this](Id id, RVariant v) {
			auto axis = (PxArticulationAxis::Enum)(id - 70);
			GetChip()->SetIsAccelerationDrive(axis, v.ToUInt() == RCheckState::Checked);
			SetDirty();
		};
		auto *w = AddCheckBox(70, MTEXT("Is Acceleration Drive"), ap[PxArticulationAxis::eTWIST].isAccelerationDrive ? RCheckState::Checked : RCheckState::Unchecked, cb);
		for (int32 i = 1; i < PxArticulationAxis::eCOUNT; i++)
			OverloadCheckBox(70 + i, w, ap[PxArticulationAxis::eTWIST + i].isAccelerationDrive ? RCheckState::Checked : RCheckState::Unchecked, cb);
	}

	// Drive Target
	{
		auto cb = [this](Id id, RVariant v) {
			auto axis = (PxArticulationAxis::Enum)(id - 80);
			GetChip()->SetDriveTarget(axis, (PxReal)v.ToDouble());
			SetDirty();
		};
		auto *w = AddDoubleSpinBox(80, MTEXT("Drive Target"), ap[PxArticulationAxis::eTWIST].driveTarget, -std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max(), 1.0f, cb);
		for (int32 i = 1; i < PxArticulationAxis::eCOUNT; i++)
			OverloadDoubleSpinBox(80 + i, w, ap[PxArticulationAxis::eTWIST + i].driveTarget, cb);
	}

	// Drive Target Vel
	{
		auto cb = [this](Id id, RVariant v) {
			auto axis = (PxArticulationAxis::Enum)(id - 90);
			GetChip()->SetMaxForce(axis, (PxReal)v.ToDouble());
			SetDirty();
		};
		auto *w = AddDoubleSpinBox(90, MTEXT("Drive Target Velocity"), ap[PxArticulationAxis::eTWIST].driveTargetVel, 0, std::numeric_limits<float32>::max(), 1.0f, cb);
		for (int32 i = 1; i < PxArticulationAxis::eCOUNT; i++)
			OverloadDoubleSpinBox(90 + i, w, ap[PxArticulationAxis::eTWIST + i].driveTargetVel, cb);
	}
}

void PhysXArticulationJointRC_Dlg::_enableButtons()
{

}