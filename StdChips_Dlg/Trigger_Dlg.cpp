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

#include "StdAfx.h"
#include "Trigger_dlg.h"


using namespace m3d;


DIALOGDESC_DEF(Trigger_Dlg, TRIGGER_GUID);


Trigger_Dlg::Trigger_Dlg()
{
	ui.setupUi(this);
	_initType = Trigger::Type::VALUE_CHANGE;
	_initAt = 0.0f;
	_initTriggerAtFirstCall = false;
}

Trigger_Dlg::~Trigger_Dlg()
{
}

void Trigger_Dlg::Init()
{
	_initType = GetChip()->GetType();
	_initAt = GetChip()->GetTriggerAtValue();
	_initTriggerAtFirstCall = GetChip()->IsTriggerAtFirstCall();
	ui.radioButton->setChecked(_initType == Trigger::Type::VALUE_CHANGE);
	ui.radioButton_2->setChecked(_initType == Trigger::Type::VALUE_INC);
	ui.radioButton_3->setChecked(_initType == Trigger::Type::VALUE_DEC);
	ui.radioButton_4->setChecked(_initType == Trigger::Type::AT_VALUE_BOTH);
	ui.radioButton_5->setChecked(_initType == Trigger::Type::AT_VALUE_INC);
	ui.radioButton_6->setChecked(_initType == Trigger::Type::AT_VALUE_DEC);
	ui.radioButton_7->setChecked(_initType == Trigger::Type::DELAY_MS);
	ui.checkBox->setChecked(_initTriggerAtFirstCall);
	ui.lineEdit->setText(QString().setNum(_initAt));
}

void Trigger_Dlg::OnCancel()
{
	if (IsEdited()) {
		GetChip()->SetType(_initType);
		GetChip()->SetTriggerAtValue(_initAt);
	}
}

void Trigger_Dlg::AfterApply()
{
	_initType = GetChip()->GetType();
	_initAt = GetChip()->GetTriggerAtValue();
	_initTriggerAtFirstCall = GetChip()->IsTriggerAtFirstCall();
}

void Trigger_Dlg::inputChanged()
{
	int i = -ui.buttonGroup->checkedId() - 2;
	if (i != -1) {
		GetChip()->SetType((Trigger::Type)i);
		SetDirty();
	}
	bool ok = false;
	value f = (value)ui.lineEdit->text().toDouble(&ok);
	if (ok) {
		GetChip()->SetTriggerAtValue(f);
		SetDirty();
	}
	bool triggerAtFirstCall = ui.checkBox->isChecked();
	GetChip()->SetIsTriggerAtFirstCall(triggerAtFirstCall);
}


