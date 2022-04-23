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
#include "UserInput_dlg.h"


using namespace m3d;


DIALOGDESC_DEF(UserInput_Dlg, USERINPUT_GUID);


UserInput_Dlg::UserInput_Dlg()
{
	ui.setupUi(this);

	ui.comboBox_mouseType->addItem("Mouse Delta X", (uint32)UserInput::MouseType::DMOUSE_X);
	ui.comboBox_mouseType->addItem("Mouse Delta Y", (uint32)UserInput::MouseType::DMOUSE_Y);
	ui.comboBox_mouseType->addItem("Left Button State", (uint32)UserInput::MouseType::LMOUSE);
//	ui.comboBox_mouseType->addItem("Left Button Down", UserInput::MouseType::LMOUSE_STATE);
//	ui.comboBox_mouseType->addItem("Left Button Released", UserInput::MouseType::LMOUSE_UP);
	ui.comboBox_mouseType->addItem("Left Button Double Click", (uint32)UserInput::MouseType::LMOUSE_DBLCLICK);
	ui.comboBox_mouseType->addItem("Middle Button State", (uint32)UserInput::MouseType::MMOUSE);
//	ui.comboBox_mouseType->addItem("Middle Button Down", UserInput::MouseType::MMOUSE_STATE);
//	ui.comboBox_mouseType->addItem("Middle Button Released", UserInput::MouseType::MMOUSE_UP);
	ui.comboBox_mouseType->addItem("Middle Button Double Click", (uint32)UserInput::MouseType::MMOUSE_DBLCLICK);
	ui.comboBox_mouseType->addItem("Right Button State", (uint32)UserInput::MouseType::RMOUSE);
//	ui.comboBox_mouseType->addItem("Right Button Down", UserInput::MouseType::RMOUSE_STATE);
//	ui.comboBox_mouseType->addItem("Right Button Released", UserInput::MouseType::RMOUSE_UP);
	ui.comboBox_mouseType->addItem("Right Button Double Click", (uint32)UserInput::MouseType::RMOUSE_DBLCLICK);
	ui.comboBox_mouseType->addItem("Mouse Wheel", (uint32)UserInput::MouseType::MOUSE_WHEEL);
	ui.comboBox_mouseType->addItem("Cursor X", (uint32)UserInput::MouseType::CURSOR_X);
	ui.comboBox_mouseType->addItem("Cursor Y", (uint32)UserInput::MouseType::CURSOR_Y);
	ui.comboBox_mouseType->addItem("Cursor X Relative", (uint32)UserInput::MouseType::CURSOR_REL_X);
	ui.comboBox_mouseType->addItem("Cursor Y Relative", (uint32)UserInput::MouseType::CURSOR_REL_Y);

	for (uint32 i = 0; i < 4; i++) // !!
		ui.cbJoystickNr->addItem("Joystick nr " + QString::number(i + 1));

	ui.cbJoystickCmd->addItem("Axis 1", (uint32)UserInput::JoystickType::AXIS_1);
	ui.cbJoystickCmd->addItem("Axis 2", (uint32)UserInput::JoystickType::AXIS_2);
	ui.cbJoystickCmd->addItem("Axis 3", (uint32)UserInput::JoystickType::AXIS_3);
	ui.cbJoystickCmd->addItem("Axis 4", (uint32)UserInput::JoystickType::AXIS_4);
	ui.cbJoystickCmd->addItem("Axis 5", (uint32)UserInput::JoystickType::AXIS_5);
	ui.cbJoystickCmd->addItem("Hat 1", (uint32)UserInput::JoystickType::HAT_1);
	ui.cbJoystickCmd->addItem("Hat 2", (uint32)UserInput::JoystickType::HAT_2);
	ui.cbJoystickCmd->addItem("Hat 3", (uint32)UserInput::JoystickType::HAT_3);
	ui.cbJoystickCmd->addItem("Hat 4", (uint32)UserInput::JoystickType::HAT_4);
	for (uint32 i = 0; i < 32; i++)
		ui.cbJoystickCmd->addItem("Button " + QString::number(i + 1), (uint32)UserInput::JoystickType::BUTTON1 + i);

	ui.comboBox_touchType->addItem("Touch Point Count", (uint32)UserInput::TouchType::COUNT);
	ui.comboBox_touchType->addItem("Touch Point 1 State", (uint32)UserInput::TouchType::TOUCH0_STATE);
	ui.comboBox_touchType->addItem("Touch Point 1 ID", (uint32)UserInput::TouchType::TOUCH0_ID);
	ui.comboBox_touchType->addItem("Touch Point 1 X", (uint32)UserInput::TouchType::TOUCH0_X);
	ui.comboBox_touchType->addItem("Touch Point 1 Y", (uint32)UserInput::TouchType::TOUCH0_Y);
	ui.comboBox_touchType->addItem("Touch Point 1 DX", (uint32)UserInput::TouchType::TOUCH0_DX);
	ui.comboBox_touchType->addItem("Touch Point 1 DY", (uint32)UserInput::TouchType::TOUCH0_DY);
	ui.comboBox_touchType->addItem("Touch Point 2 State", (uint32)UserInput::TouchType::TOUCH1_STATE);
	ui.comboBox_touchType->addItem("Touch Point 2 ID", (uint32)UserInput::TouchType::TOUCH1_ID);
	ui.comboBox_touchType->addItem("Touch Point 2 X",  (uint32)UserInput::TouchType::TOUCH1_X);
	ui.comboBox_touchType->addItem("Touch Point 2 Y",  (uint32)UserInput::TouchType::TOUCH1_Y);
	ui.comboBox_touchType->addItem("Touch Point 2 DX", (uint32)UserInput::TouchType::TOUCH1_DX);
	ui.comboBox_touchType->addItem("Touch Point 2 DY", (uint32)UserInput::TouchType::TOUCH1_DY);
	ui.comboBox_touchType->addItem("Touch Point 3 State", (uint32)UserInput::TouchType::TOUCH2_STATE);
	ui.comboBox_touchType->addItem("Touch Point 3 ID", (uint32)UserInput::TouchType::TOUCH2_ID);
	ui.comboBox_touchType->addItem("Touch Point 3 X",  (uint32)UserInput::TouchType::TOUCH2_X);
	ui.comboBox_touchType->addItem("Touch Point 3 Y",  (uint32)UserInput::TouchType::TOUCH2_Y);
	ui.comboBox_touchType->addItem("Touch Point 3 DX", (uint32)UserInput::TouchType::TOUCH2_DX);
	ui.comboBox_touchType->addItem("Touch Point 3 DY", (uint32)UserInput::TouchType::TOUCH2_DY);
	ui.comboBox_touchType->addItem("Touch Point 4 State", (uint32)UserInput::TouchType::TOUCH3_STATE);
	ui.comboBox_touchType->addItem("Touch Point 4 ID", (uint32)UserInput::TouchType::TOUCH3_ID);
	ui.comboBox_touchType->addItem("Touch Point 4 X",  (uint32)UserInput::TouchType::TOUCH3_X);
	ui.comboBox_touchType->addItem("Touch Point 4 Y",  (uint32)UserInput::TouchType::TOUCH3_Y);
	ui.comboBox_touchType->addItem("Touch Point 4 DX", (uint32)UserInput::TouchType::TOUCH3_DX);
	ui.comboBox_touchType->addItem("Touch Point 4 DY", (uint32)UserInput::TouchType::TOUCH3_DY);

	_initDT = (UserInput::DeviceType)0;
	_initMT = (UserInput::MouseType)0;
	_initKeyCode = 0;
	_initKT = (UserInput::KeyboardType)0;
	_initBM = (UserInput::ButtonMode)0;
	_initJT = (UserInput::JoystickType)0;
	_initJoystickNr = 0;

	_initTT = (UserInput::TouchType)0;
}

UserInput_Dlg::~UserInput_Dlg()
{
}

void UserInput_Dlg::Init()
{
	_initDT = GetChip()->GetDeviceType();
	ui.tabWidget->setCurrentIndex((uint32)_initDT);

	_initMT = GetChip()->GetMouseType();
	ui.comboBox_mouseType->setCurrentIndex(ui.comboBox_mouseType->findData((uint32)_initMT));

	_initBM = GetChip()->GetButtonMode();
	ui.buttonGroup_bm->button(-2 - (uint32)_initBM)->setChecked(true);

	_initKeyCode = GetChip()->GetKeyCode();
	_initKT = GetChip()->GetKeyboardType();
	ui.buttonGroup_kt->button(-2 - (uint32)_initKT)->setChecked(true);
//	ui.radioButton_vKey->setChecked(_initKT == UserInput::KeyboardType::VIRTUAL_KEY_CODES);
//	ui.radioButton_scanCodes->setChecked(_initKT == UserInput::KeyboardType::SCAN_CODES);
//	ui.radioButton_anyKey->setChecked(_initKT == UserInput::KeyboardType::ANY_KEY);

	_initJoystickNr = GetChip()->GetJoystickNr();
	_initJT = GetChip()->GetJoystickType();
	ui.cbJoystickNr->setCurrentIndex(_initJoystickNr);
	ui.cbJoystickCmd->setCurrentIndex(ui.cbJoystickCmd->findData((uint32)_initJT));

	_initTT = GetChip()->GetTouchType();
	ui.comboBox_touchType->setCurrentIndex(ui.comboBox_touchType->findData((uint32)_initTT));

	_updateKeyCaptureWidget();
}

void UserInput_Dlg::OnOK()
{
}

void UserInput_Dlg::OnCancel()
{
	GetChip()->SetDeviceType(_initDT);
	GetChip()->SetMouseType(_initMT);
	GetChip()->SetKeyboardType(_initKT);
	GetChip()->SetKeyCode(_initKeyCode);
	GetChip()->SetButtonMode(_initBM);
	GetChip()->SetJoystickNr(_initJoystickNr);
	GetChip()->SetJoystickType(_initJT);
	GetChip()->SetTouchType(_initTT);
}

void UserInput_Dlg::AfterApply()
{
	_initDT = GetChip()->GetDeviceType();
	_initMT = GetChip()->GetMouseType();
	_initKeyCode = GetChip()->GetKeyCode();
	_initKT = GetChip()->GetKeyboardType();
	_initBM = GetChip()->GetButtonMode();
	_initJoystickNr = GetChip()->GetJoystickNr();
	_initJT = GetChip()->GetJoystickType();
	_initTT = GetChip()->GetTouchType();
}

void UserInput_Dlg::deviceTypeChanged()
{
	if (!GetChipDialog())
		return; // before construction complete...
	UserInput::DeviceType dt = (UserInput::DeviceType)ui.tabWidget->currentIndex();
	if (GetChip()->GetDeviceType() == dt)
		return;
	GetChip()->SetDeviceType(dt);
	_setChipName();
	SetDirty();
}

void UserInput_Dlg::mouseTypeChanged()
{
	UserInput::MouseType mt = (UserInput::MouseType)ui.comboBox_mouseType->itemData(ui.comboBox_mouseType->currentIndex()).toUInt();
	if (mt == GetChip()->GetMouseType())
		return;
	GetChip()->SetMouseType(mt);
	SetDirty();
	_setChipName();
}

void UserInput_Dlg::keyListenerPressed(QKeyEvent *event)
{
	if (ui.radioButton_anyKey->isChecked())
		return;

	event->accept();

	bool useScanCode = ui.radioButton_scanCodes->isChecked();
	BYTE code = 0;
	if (useScanCode) { // keyboard scan codes
		quint32 scanCode = event->nativeScanCode();
		// QT returns scancode as bit 16-24 of lParam in messages like WM_KEYDOWN. We make the scan code of bit 16-22 + bit 24 as the high order bit.
		if (scanCode == 0 || (scanCode & 0xFF) > 0x7F) {
			QApplication::beep();
			return; // scan code larger than 127
		}
		code = (BYTE) ((scanCode & 0x7F) | ((scanCode & 0x100) >> 1));
	}
	else { // virtual key codes
		quint32 vKey = event->nativeVirtualKey();
		if (vKey == 0 || vKey > 254) {
			QApplication::beep();
			return; // Invalid virtual key
		}
		code = (BYTE)vKey;
	}
	if (code == GetChip()->GetKeyCode())
		return; // no change
	GetChip()->SetKeyCode(code);
	_updateKeyCaptureWidget();
	_setChipName();
	SetDirty();
}

void UserInput_Dlg::keyCodeChanged()
{
	UserInput::KeyboardType kt = (UserInput::KeyboardType)(-ui.buttonGroup_kt->checkedId() - 2);
	if (GetChip()->GetKeyboardType() == kt)
		return; // no change
	GetChip()->SetKeyboardType(kt);
	GetChip()->SetKeyCode(0);
	_setChipName();
	_updateKeyCaptureWidget();
	SetDirty();
}

void UserInput_Dlg::_updateKeyCaptureWidget()
{
	BYTE code = GetChip()->GetKeyCode();
	ui.lineEdit_keyCapture->setEnabled(GetChip()->GetKeyboardType() != UserInput::KeyboardType::ANY_KEY);
	if (GetChip()->GetKeyboardType() == UserInput::KeyboardType::ANY_KEY) {
		ui.lineEdit_keyCapture->setText("Any key");
	}
	else if (code == 0) {
		ui.lineEdit_keyCapture->setText("Press key to capture");
	}
	else if (GetChip()->GetKeyboardType() == UserInput::KeyboardType::SCAN_CODES) {
		Char buff[256];
		// GetKeyNameText() expect code as bit 16-24 of lParam in messages like WM_KEYDOW.
		if (GetKeyNameTextA((((code & 0x80) << 1) | code & 0x7F) << 16, buff, 256))
			ui.lineEdit_keyCapture->setText(TOQSTRING(String(buff)));
		else
			ui.lineEdit_keyCapture->setText(TOQSTRING((String(MTEXT("Code: ")) + strUtils::fromNum(code))));
	}
	else {
		ui.lineEdit_keyCapture->setText(VIRTUALKEYCODENAMES[code]);
	}
}

void UserInput_Dlg::joystickChanged()
{
	uint32 jnr = ui.cbJoystickNr->currentIndex();
	UserInput::JoystickType jt = (UserInput::JoystickType)ui.cbJoystickCmd->itemData(ui.cbJoystickCmd->currentIndex()).toUInt();
	if (jt == GetChip()->GetJoystickType() && jnr == GetChip()->GetJoystickNr())
		return;
	GetChip()->SetJoystickNr(jnr);
	GetChip()->SetJoystickType(jt);
	SetDirty();
	_setChipName();
}

void UserInput_Dlg::touchChanged()
{
	UserInput::TouchType tt = (UserInput::TouchType)ui.comboBox_touchType->itemData(ui.comboBox_touchType->currentIndex()).toUInt();
	if (tt == GetChip()->GetTouchType())
		return;
	GetChip()->SetTouchType(tt);
	SetDirty();
	_setChipName();
}

void UserInput_Dlg::_setChipName()
{
	switch (GetChip()->GetDeviceType()) 
	{
	case UserInput::DeviceType::MOUSE:
		if (GetChip()->GetMouseType() == UserInput::MouseType::NONE)
			GetChip()->SetName(GetChip()->GetChipDesc().name);
		else
			GetChip()->SetName(FROMQSTRING(ui.comboBox_mouseType->currentText()));
		break;
	case UserInput::DeviceType::KEYBOARD: 
		{
			BYTE code = GetChip()->GetKeyCode();
			switch (GetChip()->GetKeyboardType()) 
			{
			case UserInput::KeyboardType::VIRTUAL_KEY_CODES:
			case UserInput::KeyboardType::SCAN_CODES:
				GetChip()->SetName(GetChip()->GetKeyCode() == 0 ? GetChip()->GetChipDesc().name : FROMQSTRING(ui.lineEdit_keyCapture->text()));
				break;
			default:
				GetChip()->SetName(MTEXT("Any Key"));
				break;
			};
		}
		break;
	case UserInput::DeviceType::JOYSTICK:
		{
			if (GetChip()->GetJoystickType() == UserInput::JoystickType::NONE)
				GetChip()->SetName(GetChip()->GetChipDesc().name);
			else
				GetChip()->SetName(MTEXT("Joystick ") + strUtils::fromNum(GetChip()->GetJoystickNr() + 1) + MTEXT(" - ") + FROMQSTRING(ui.cbJoystickCmd->currentText()));
		}
		break;
	case UserInput::DeviceType::TOUCH:
		{
		if (GetChip()->GetTouchType() == UserInput::TouchType::NONE)
			GetChip()->SetName(GetChip()->GetChipDesc().name);
		else
			GetChip()->SetName(FROMQSTRING(ui.comboBox_touchType->currentText()));
		}
		break;
	default:
		GetChip()->SetName(GetChip()->GetChipDesc().name);
		break;
	}
}

void UserInput_Dlg::buttonModeChanged()
{
	auto bm = (UserInput::ButtonMode)(-ui.buttonGroup_bm->checkedId() - 2);
	if (bm != GetChip()->GetButtonMode()) {
		SetDirty();
		GetChip()->SetButtonMode(bm);
	}
}



