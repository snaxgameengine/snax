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
#include "EditorInputManager.h"
#include "M3DEngine/Joysticks.h"
#include "M3DCore/MemoryManager.h"
#include <qcursor.h>

using namespace m3d;


const InputManager::Touch EditorInputManager::NullTouch = InputManager::Touch();

EditorInputManager::EditorInputManager()
{
	_mouse = XMFLOAT2(0.0f, 0.0f);
	_dMouse = XMFLOAT2(0.0f, 0.0f);
	_mouseWheel = 0.0f;
	_mouseWheelTmp = 0.0f;
	_mouseActionPos = XMFLOAT2(0.0f, 0.0f);
	_mouseAction = false;
	_joysticks = nullptr;
	_devicePixelRatio = 1.0f;

	memset(_mouseButtonsRaw, 0, sizeof(_mouseButtonsRaw));
	memset(_mouseButtonsDblClickRaw, 0, sizeof(_mouseButtonsDblClickRaw));
	memset(_keyboardScanCodesRaw, 0, sizeof(_keyboardScanCodesRaw));
	memset(_keyboardVirtualKeysRaw, 0, sizeof(_keyboardVirtualKeysRaw));
	memset(_keyboardVirtualKeyStates, 0, sizeof(_keyboardVirtualKeyStates));

}

EditorInputManager::~EditorInputManager()
{
	if (_joysticks)
		mmdelete(_joysticks);
}

void EditorInputManager::SetDevicePixelRatio(float r)
{
	_devicePixelRatio = r;
}

void EditorInputManager::Update()
{
	_mouseButtons[LBUTTON] = _mouseButtonsRaw[LBUTTON];
	_mouseButtons[MBUTTON] = _mouseButtonsRaw[MBUTTON];
	_mouseButtons[RBUTTON] = _mouseButtonsRaw[RBUTTON];
	_mouseButtonsDblClick[LBUTTON] = _mouseButtonsDblClickRaw[LBUTTON];
	_mouseButtonsDblClick[MBUTTON] = _mouseButtonsDblClickRaw[MBUTTON];
	_mouseButtonsDblClick[RBUTTON] = _mouseButtonsDblClickRaw[RBUTTON];
	_mouseButtonsDblClickRaw[LBUTTON] = 0;
	_mouseButtonsDblClickRaw[MBUTTON] = 0;
	_mouseButtonsDblClickRaw[RBUTTON] = 0;
	_mouseWheel = _mouseWheelTmp;
	_mouseWheelTmp = 0.0f;

	QPoint p = QCursor::pos();
	//_devicePixelRatio = 1;
	_dMouse.x = _devicePixelRatio * float(p.x()) - _mouse.x;
	_dMouse.y = _devicePixelRatio * float(p.y()) - _mouse.y;
	_mouse = _devicePixelRatio * XMFLOAT2(p.x(), p.y());

	for (unsigned i = 0; i < 256; i++) {
		_keyboardScanCodes[i] = _keyboardScanCodesRaw[i];
		_keyboardVirtualKeys[i] = _keyboardVirtualKeysRaw[i];
	}

	BYTE keys[256];
	::GetKeyboardState(keys);
	for (unsigned i = 0; i < 256; i++) {
		_keyboardVirtualKeyStates[i] = (keys[i] & 0x80) != 0;
	}

	if (_joysticks)
		_joysticks->Update();
}

void EditorInputManager::mouseDoubleClickEvent(QMouseEvent *event)
{
	switch(event->button())
	{
	case Qt::LeftButton:
		_mouseButtonsDblClickRaw[LBUTTON] = 1;
		break;
	case Qt::MiddleButton:
		_mouseButtonsDblClickRaw[MBUTTON] = 1;
		break;
	case Qt::RightButton:
		_mouseButtonsDblClickRaw[RBUTTON] = 1;
		break;
	default:
		return;
	}
	_mouseActionPos.x = _devicePixelRatio * event->pos().x();
	_mouseActionPos.y = _devicePixelRatio * event->pos().y();
}

void EditorInputManager::mousePressEvent(QMouseEvent *event)
{
	switch(event->button())
	{
	case Qt::LeftButton:
		_mouseButtonsRaw[LBUTTON] = 1;
		break;
	case Qt::MiddleButton:
		_mouseButtonsRaw[MBUTTON] = 1;
		break;
	case Qt::RightButton:
		_mouseButtonsRaw[RBUTTON] = 1;
		break;
	default:
		return;
	}
	_mouseActionPos.x = _devicePixelRatio * event->pos().x();
	_mouseActionPos.y = _devicePixelRatio * event->pos().y();
}

void EditorInputManager::mouseReleaseEvent(QMouseEvent *event)
{
	switch(event->button())
	{
	case Qt::LeftButton:
		_mouseButtonsRaw[LBUTTON] = 0;
		break;
	case Qt::MiddleButton:
		_mouseButtonsRaw[MBUTTON] = 0;
		break;
	case Qt::RightButton:
		_mouseButtonsRaw[RBUTTON] = 0;
		break;
	default:
		return;
	}
	_mouseActionPos.x = _devicePixelRatio * event->pos().x();
	_mouseActionPos.y = _devicePixelRatio * event->pos().y();
}

void EditorInputManager::wheelEvent(QWheelEvent *event)
{
	if (event->angleDelta().y() != 0)
		_mouseWheelTmp += (float)event->angleDelta().y() / (float)WHEEL_DELTA;
}

void EditorInputManager::keyPressEvent(QKeyEvent *event)
{
	if (event->nativeScanCode() < 256)
		_keyboardScanCodesRaw[event->nativeScanCode()] = 1;
	if ( event->nativeVirtualKey() < 256)
		_keyboardVirtualKeysRaw[ event->nativeVirtualKey()] = 1;
}

void EditorInputManager::keyReleaseEvent(QKeyEvent *event)
{
	if (event->nativeScanCode() < 256)
		_keyboardScanCodesRaw[event->nativeScanCode()] = 0;
	if (event->nativeVirtualKey() < 256)
		_keyboardVirtualKeysRaw[event->nativeVirtualKey()] = 0;
}

Joysticks* EditorInputManager::GetJoysticks()
{
	if (!_joysticks) {
		_joysticks = mmnew Joysticks();
	}

	return _joysticks;
}