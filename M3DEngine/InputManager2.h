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
#include "GlobalDef.h"
#include "M3DCore/Containers.h"
#include "M3DCore/TriState.h"

namespace m3d
{

/*
The following are the Visual C++ key codes:

VK_A through VK_Z are the same as their ASCII equivalents: 'A' thru 'Z'.
VK_0 through VK_9 are the same as their ASCII equivalents: '0' thru '9'.
VK_LBUTTON
VK_RBUTTON
VK_CANCEL
VK_MBUTTON
VK_BACK
VK_TAB
VK_CLEAR
VK_RETURN
VK_SHIFT
VK_CONTROL
VK_MENU
VK_PAUSE
VK_CAPITAL
VK_ESCAPE
VK_SPACE
VK_PRIOR
VK_NEXT 
VK_END
VK_HOME 
VK_LEFT 
VK_UP
VK_RIGHT
VK_DOWN
VK_SELECT
VK_PRINT
VK_EXECUTE
VK_SNAPSHOT
VK_INSERT
VK_DELETE
VK_HELP

VK_NUMPAD0
VK_NUMPAD1
VK_NUMPAD2
VK_NUMPAD3
VK_NUMPAD4
VK_NUMPAD5
VK_NUMPAD6
VK_NUMPAD7
VK_NUMPAD8
VK_NUMPAD9
VK_MULTIPLY
VK_ADD
VK_SEPARATOR
VK_SUBTRACT
VK_DECIMAL
VK_DIVIDE
VK_F1
VK_F2
VK_F3
VK_F4
VK_F5
VK_F6
VK_F7
VK_F8
VK_F9
VK_F10
VK_F11
VK_F12
VK_F13
VK_F14
VK_F15
VK_F16
VK_F17
VK_F18
VK_F19
VK_F20
VK_F21
VK_F22
VK_F23
VK_F24
VK_NUMLOCK
VK_SCROLL
*/

namespace mbuttons
{
	static const uint32 LBUTTON = MK_LBUTTON;
	static const uint32 RBUTTON = MK_RBUTTON;
	static const uint32 MBUTTON = MK_MBUTTON;
}

namespace keystate
{
	static const BYTE DOWN = 0x80;
	static const BYTE TOGGLED = 0x01;
}


class Joysticks;


class M3DENGINE_API InputManager2
{
protected:
	HWND _hWnd[4];
	uint32 _wCount;

	// Keyboard stuff
	bool _keyboardScanCodesTmp[256];
	bool _keyboardVirtualKeysTmp[256];
	TriState _keyboardScanCodes[256];
	TriState _keyboardVirtualKeys[256];

	// Mouse stuff
	TriState _mouseButtons;
	TriState _mouseButtonsDblClick;
	POINT _mouse;
	POINT _dMouse;
	float32 _mouseWheel;
	uint32 _mouseStateTmp;
	uint32 _mouseDoubleclickTmp;
	POINT _dMouseTmp;
	float32 _mouseWheelTmp;
	POINT _mouseActionPos;
	bool _mouseAction;

	// Joysticks!
	Joysticks *_joysticks;

public:
	InputManager2();
	~InputManager2();

	void RegisterWindow(HWND hWnd) { if (_wCount < 4) _hWnd[_wCount++] = hWnd; }

	bool Init(HWND w, bool useJoysticks = true);

	bool MsgProc(MSG *msg);

	void Update();

	const TriState &GetKeyboardScanCode(BYTE index) const { return _keyboardScanCodes[index]; }
	const TriState &GetVirtualKeyCode(BYTE index) const { return _keyboardVirtualKeys[index]; }

	const TriState &GetMouseButtons() const { return _mouseButtons; }
	const TriState &GetMouseButtonsDblClick() const { return _mouseButtonsDblClick; }
	float32 GetMouseWheel() const { return _mouseWheel; }
	const POINT &GetMousePos() const { return _mouse; }
	const POINT &GetMouseDeltaPos() const { return _dMouse; }

	Joysticks *GetJoysticks() { return _joysticks; }
};




}
