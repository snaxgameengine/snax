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

#include "M3DCore/TriState.h"


namespace m3d
{


class InputManager3
{
public:
	static const unsigned LBUTTON = 1;
	static const unsigned RBUTTON = 2;
	static const unsigned MBUTTON = 4;
private:
	// Keyboard stuff
//	bool _keyboardScanCodesTmp[256];
	bool _keyboardVirtualKeysTmp[256];
//	TriState _keyboardScanCodes[256];
	TriState _keyboardVirtualKeys[256];

	// Mouse stuff
	TriState _mouseButtons;
	TriState _mouseButtonsDblClick;
	XMFLOAT2 _mouse;
	XMFLOAT2 _dMouse;
	float32 _mouseWheel;
	unsigned _mouseStateTmp;
	unsigned _mouseDoubleclickTmp;
	float32 _mouseWheelTmp;
	XMFLOAT2 _mouseButtonPos;

public:
	InputManager3()
	{
//		memset(_keyboardScanCodesTmp, 0, sizeof(_keyboardScanCodesTmp));
		memset(_keyboardVirtualKeysTmp, 0, sizeof(_keyboardVirtualKeysTmp));
		_mouseStateTmp =0;
		_mouseDoubleclickTmp = 0;
		_mouseWheelTmp = 0;
		_mouseButtonPos = XMFLOAT2(0.0f, 0.0f);
		_mouse = XMFLOAT2(0.0f, 0.0f);
		_dMouse = XMFLOAT2(0.0f, 0.0f);
		_mouseWheel = 0.0f;
	}

	void GotFocus(XMFLOAT2 pos)
	{
		_mouse = pos;
	}

	void MousePressed(unsigned button, XMFLOAT2 pos)
	{
		_mouseStateTmp |= button;
		_mouseButtonPos = pos;
	}

	void MouseReleased(unsigned button, XMFLOAT2 pos)
	{
		_mouseStateTmp &= ~button;
		_mouseButtonPos = pos;
	}

	void MouseDoubleClicked(unsigned button, XMFLOAT2 pos)
	{
		_mouseDoubleclickTmp |= button;
		_mouseButtonPos = pos;
	}

	void MouseWheeled(int32 delta)
	{
		_mouseWheelTmp = float32(delta) / WHEEL_DELTA;
	}

	void KeyPressed(unsigned vKey)
	{
		_keyboardVirtualKeysTmp[vKey] = true;
	}

	void KeyReleased(unsigned vKey)
	{
		_keyboardVirtualKeysTmp[vKey] = false;
	}

//	const TriState &GetKeyboardScanCode(unsigned index) const { return _keyboardScanCodes[index]; }
	const TriState &GetVirtualKeyCode(unsigned index) const { return _keyboardVirtualKeys[index]; }

	const TriState &GetMouseButtons() const { return _mouseButtons; }
	const TriState &GetMouseButtonsDblClick() const { return _mouseButtonsDblClick; }
	float32 GetMouseWheel() const { return _mouseWheel; }
	XMFLOAT2 GetMousePos() const { return _mouse; }
	XMFLOAT2 GetMouseDeltaPos() const { return _dMouse; }

	void Update(XMFLOAT2 mousePos)
	{
		_mouseButtons = _mouseStateTmp;
		_mouseButtonsDblClick = _mouseDoubleclickTmp;
		_mouseWheel = _mouseWheelTmp;
		_mouseDoubleclickTmp = 0;
		_mouseWheelTmp = 0.0f;
		if (_mouseButtons.onActivate | _mouseButtons.onDeactivate)
			mousePos = _mouseButtonPos;
		_dMouse = XMFLOAT2(mousePos.x - _mouse.x, mousePos.y - _mouse.y);
		_mouse = mousePos;

		for (unsigned i = 0; i < 256; i++) {
//			_keyboardScanCodes[i] = _keyboardScanCodesTmp[i];
			_keyboardVirtualKeys[i] = _keyboardVirtualKeysTmp[i];
		}
	}
};

}