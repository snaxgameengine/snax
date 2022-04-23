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

#pragma once


#include "Exports.h"
#include "Value.h"


namespace m3d
{

static const Guid USERINPUT_GUID = { 0xd358f9a5, 0xd245, 0x4c59, { 0xbd, 0xe8, 0xbf, 0xa3, 0xce, 0xea, 0xe4, 0xc8 } };

struct TriState;

class STDCHIPS_API UserInput : public Value
{
	CHIPDESC_DECL;
public:
	enum class DeviceType { NONE, MOUSE, KEYBOARD, JOYSTICK, TOUCH };
	enum class MouseType { NONE, DMOUSE_X, DMOUSE_Y, 
		LMOUSE, LMOUSE_DBLCLICK,
		MMOUSE, MMOUSE_DBLCLICK,
		RMOUSE, RMOUSE_DBLCLICK,
		MOUSE_WHEEL, 
		CURSOR_GLOBAL_X, CURSOR_GLOBAL_Y, 
		CURSOR_X, CURSOR_Y, 
		CURSOR_REL_X, CURSOR_REL_Y,
		CURSORB_X, CURSORB_Y
	};
	enum class KeyboardType { VIRTUAL_KEY_CODES, SCAN_CODES, ANY_KEY }; 
	enum class ButtonMode { UP, PRESSING, DOWN, RELEASING };
	enum class JoystickType { NONE = 0, 
		AXIS_1 = 1, AXIS_2, AXIS_3, AXIS_4, AXIS_5, 
		HAT_1 = 10, HAT_2, HAT_3, HAT_4,
		BUTTON1 = 30, BUTTON2, BUTTON3, BUTTON4, BUTTON5,
		BUTTON6, BUTTON7, BUTTON8, BUTTON9, BUTTON10, BUTTON11, BUTTON12, BUTTON13, 
		BUTTON14, BUTTON15, BUTTON16, BUTTON17, BUTTON18, BUTTON19, BUTTON20, BUTTON21, 
		BUTTON22, BUTTON23, BUTTON24, BUTTON25, BUTTON26, BUTTON27, BUTTON28,
		BUTTON29, BUTTON30, BUTTON31, BUTTON32 };
	enum class TouchType { NONE, COUNT, 
		TOUCH0_STATE = 10, TOUCH0_ID, TOUCH0_X, TOUCH0_Y, TOUCH0_DX, TOUCH0_DY,
		TOUCH1_STATE = 30, TOUCH1_ID, TOUCH1_X, TOUCH1_Y, TOUCH1_DX, TOUCH1_DY,
		TOUCH2_STATE = 50, TOUCH2_ID, TOUCH2_X, TOUCH2_Y, TOUCH2_DX, TOUCH2_DY,
		TOUCH3_STATE = 70, TOUCH3_ID, TOUCH3_X, TOUCH3_Y, TOUCH3_DX, TOUCH3_DY
	};

	UserInput();
	virtual ~UserInput();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual value GetValue() override;
	virtual void SetValue(value v) override {}

	virtual DeviceType GetDeviceType() const { return _dt; }
	virtual void SetDeviceType(DeviceType dt) { _dt = dt; }

	virtual ButtonMode GetButtonMode() const { return _bm; }
	virtual void SetButtonMode(ButtonMode bm) { _bm = bm; }

	virtual MouseType GetMouseType() const { return _mt; }
	virtual void SetMouseType(MouseType mt) { _mt = mt; }

	virtual KeyboardType GetKeyboardType() const { return _kt; }
	virtual BYTE GetKeyCode() const { return _keyCode; }
	virtual void SetKeyboardType(KeyboardType kt) { _kt = kt; }
	virtual void SetKeyCode(BYTE code) { _keyCode = code; }

	virtual uint32 GetJoystickNr() const { return _joystickNr; }
	virtual void SetJoystickNr(uint32 joystickNr) { _joystickNr = joystickNr; }
	virtual JoystickType GetJoystickType() const { return _jt; }
	virtual void SetJoystickType(JoystickType jt) { _jt = jt; }

	virtual TouchType GetTouchType() const { return _tt; }
	virtual void SetTouchType(TouchType tt) { _tt = tt; }

protected:
	DeviceType _dt;
	MouseType _mt;
	JoystickType _jt;
	TouchType _tt;
	ButtonMode _bm;

	// Type of keyboard code
	KeyboardType _kt;
	// Key code to capture
	BYTE _keyCode;
	// Joystick nr
	uint32 _joystickNr;


	value _getMouseInput();
	value _getKeyboardInput();
	value _getJoystickInput();
	value _getTouchInput();

	value _parseButtonState(TriState s);
};



}