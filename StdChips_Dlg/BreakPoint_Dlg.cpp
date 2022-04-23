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
#include "BreakPoint_Dlg.h"

using namespace m3d;

DIALOGDESC_DEF(BreakPoint_Dlg, BREAKPOINT_GUID);

BreakPoint_Dlg::BreakPoint_Dlg()
{
	ui.setupUi(this);
}

BreakPoint_Dlg::~BreakPoint_Dlg()
{
}

void BreakPoint_Dlg::Init()
{
	ui.checkBox_enabled->setChecked(GetChip()->IsEnabled());
	ui.checkBox_hitCount->setChecked(GetChip()->GetHitCountCondition() != BreakPoint::HitCountCondition::HCC_DISABLED);
	ui.checkBox_resetHitCountPrFrame->setChecked(GetChip()->IsResetHitCountPrFrame());
	ui.checkBox_childValue->setChecked(GetChip()->GetChildValueCondition() != BreakPoint::ChildValueCondition::CVC_DISABLED);
	ui.checkBox_instanceID->setChecked(GetChip()->GetClassInstanceID() != InvalidClassInstanceID);
	ui.comboBox_hitCount->setCurrentIndex((uint32)std::max(GetChip()->GetHitCountCondition(), (BreakPoint::HitCountCondition)1) - 1);
	ui.comboBox_childValue->setCurrentIndex((uint32)std::max(GetChip()->GetChildValueCondition(), (BreakPoint::ChildValueCondition)1) - 1);
	ui.lineEdit_hitCount->setText(QString::number(GetChip()->GetHitCount()));
	ui.lineEdit_currentHitCount->setText(QString::number(GetChip()->GetCurrentHitCount()));
	ui.lineEdit_childValue->setText(QString::number(GetChip()->GetChildValue()));
	ui.lineEdit_instanceID->setText(GetChip()->GetClassInstanceID() != InvalidClassInstanceID ? QString::number(GetChip()->GetClassInstanceID()) : "");
}

void BreakPoint_Dlg::OnOK()
{
	bool enabled = ui.checkBox_enabled->isChecked();
	BreakPoint::HitCountCondition hitCountCondition = BreakPoint::HitCountCondition::HCC_DISABLED;
	if (ui.checkBox_hitCount->isChecked())
		hitCountCondition = (BreakPoint::HitCountCondition)(ui.comboBox_hitCount->currentIndex() + 1);
	uint32 hitCount = std::max(ui.lineEdit_hitCount->text().toUInt(), 1u);
	uint32 currentHitCount = ui.lineEdit_currentHitCount->text().toUInt();
	bool resetHitCountPrFrame = ui.checkBox_resetHitCountPrFrame->isChecked();
	BreakPoint::ChildValueCondition childValueCondition = BreakPoint::ChildValueCondition::CVC_DISABLED;
	if (ui.checkBox_childValue->isChecked())
		childValueCondition = (BreakPoint::ChildValueCondition)(ui.comboBox_childValue->currentIndex() + 1);
	value childValue = (value)ui.lineEdit_childValue->text().toDouble();
	ClassInstanceID instanceID = InvalidClassInstanceID;
	if (ui.checkBox_instanceID->isChecked())
		instanceID = ui.lineEdit_instanceID->text().toUInt();

	if (enabled != GetChip()->IsEnabled()) {
		GetChip()->SetEnabled(enabled);
		SetDirty();
	}
	if (hitCountCondition != GetChip()->GetHitCountCondition()) {
		GetChip()->SetHitCountCondition(hitCountCondition);
		SetDirty();
	}
	if (hitCount != GetChip()->GetHitCount()) {
		GetChip()->SetHitCount(hitCount);
		SetDirty();
	}
	if (currentHitCount != GetChip()->GetCurrentHitCount()) {
		GetChip()->SetCurrentHitCount(currentHitCount);
		SetDirty();
	}
	if (resetHitCountPrFrame != GetChip()->IsResetHitCountPrFrame()) {
		GetChip()->SetResetHitCountPrFrame(resetHitCountPrFrame);
		SetDirty();
	}
	if (childValueCondition != GetChip()->GetChildValueCondition()) {
		GetChip()->SetChildValueCondition(childValueCondition);
		SetDirty();
	}
	if (childValue != GetChip()->GetChildValue()) {
		GetChip()->SetChildValue(childValue);
		SetDirty();
	}
	if (instanceID != GetChip()->GetClassInstanceID()) {
		GetChip()->SetClassInstanceID(instanceID);
		SetDirty();
	}
}
