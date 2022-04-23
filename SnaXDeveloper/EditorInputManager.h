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

#include "Common.h"
#include "M3DEngine/InputManager.h"
#include <qevent.h>


namespace m3d
{


class EditorInputManager : public InputManager
{
public:
	EditorInputManager();
	~EditorInputManager();

	virtual void Init() {}
	virtual void Update();

	void SetDevicePixelRatio(float r);

	virtual TriState GetKeyboardScanCode(unsigned scanCode) const override { return _keyboardScanCodes[scanCode]; }
	virtual TriState GetVirtualKeyCode(unsigned vkCode) const override { return _keyboardVirtualKeys[vkCode]; }
	virtual bool GetVirtualKeyCodeState(unsigned vkCode) const override { return _keyboardVirtualKeyStates[vkCode]; }

	virtual TriState GetMouseButton(unsigned index) const override { return _mouseButtons[index]; }
	virtual TriState GetMouseButtonDblClick(unsigned index) const override { return _mouseButtonsDblClick[index]; }
	virtual float GetMouseWheel() const override { return _mouseWheel; }
	virtual XMFLOAT2 GetMousePos() const override { return _mouse; }
	virtual XMFLOAT2 GetMouseDeltaPos() const override { return _dMouse; }

	static const Touch NullTouch;
	virtual const Touch &GetTouchPoint(unsigned index) const override { return NullTouch; }
	virtual const unsigned GetTouchCount() const override { return 0; }

	virtual Joysticks* GetJoysticks() override;

	// Qt event callbacks will call these functions.
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void wheelEvent(QWheelEvent *event);
	void keyPressEvent(QKeyEvent *event);
	void keyReleaseEvent(QKeyEvent *event);

protected:
	// Keyboard stuff
	uint8_t _keyboardScanCodesRaw[256];
	uint8_t _keyboardVirtualKeysRaw[256];
	TriState _keyboardScanCodes[256];
	TriState _keyboardVirtualKeys[256];
	bool _keyboardVirtualKeyStates[256];

	// Mouse stuff
	uint8_t _mouseButtonsRaw[3];
	uint8_t _mouseButtonsDblClickRaw[3];
	TriState _mouseButtons[3];
	TriState _mouseButtonsDblClick[3];
	XMFLOAT2 _mouse;
	XMFLOAT2 _dMouse;
	float _mouseWheel;
	float _mouseWheelTmp;
	XMFLOAT2 _mouseActionPos;
	bool _mouseAction;
	float _devicePixelRatio;

	// Joysticks!
	class Joysticks* _joysticks;
};



}