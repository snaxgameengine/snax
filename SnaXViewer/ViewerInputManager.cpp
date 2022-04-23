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
#include "ViewerInputManager.h"
#include "M3DEngine/Joysticks.h"
#include "M3DCore/MemoryManager.h"

using namespace m3d;


const InputManager::Touch ViewerInputManager::NullTouch = InputManager::Touch();

ViewerInputManager::ViewerInputManager()
{
	_mouse = XMFLOAT2(0.0f, 0.0f);
	_dMouse = XMFLOAT2(0.0f, 0.0f);
	_mouseButtonsRaw = 0;
	_mouseButtonsDblClickRaw = 0;
	_mouseWheel = 0.0f;
	_mouseWheelTmp = 0.0f;
	_mouseActionPos.x = 0;
	_mouseActionPos.y = 0;
	_mouseAction = false;
	_joysticks = nullptr;

	memset(_keyboardScanCodesRaw, 0, sizeof(_keyboardScanCodesRaw));
	memset(_keyboardVirtualKeysRaw, 0, sizeof(_keyboardVirtualKeysRaw));
}

ViewerInputManager::~ViewerInputManager()
{
	if (_joysticks)
		mmdelete(_joysticks);
}

void ViewerInputManager::Update()
{
	_mouseButtons[LBUTTON] = _mouseButtonsRaw & MK_LBUTTON ? 1 : 0;
	_mouseButtons[MBUTTON] = _mouseButtonsRaw & MK_MBUTTON ? 1 : 0;
	_mouseButtons[RBUTTON] = _mouseButtonsRaw & MK_RBUTTON ? 1 : 0;
	_mouseButtonsDblClick[LBUTTON] = _mouseButtonsDblClickRaw & MK_LBUTTON ? 1 : 0;
	_mouseButtonsDblClick[MBUTTON] = _mouseButtonsDblClickRaw & MK_MBUTTON ? 1 : 0;
	_mouseButtonsDblClick[RBUTTON] = _mouseButtonsDblClickRaw & MK_RBUTTON ? 1 : 0;
	_mouseButtonsDblClickRaw = 0;
	_mouseWheel = _mouseWheelTmp;
	_mouseWheelTmp = 0.0f;

	POINT p;
	GetCursorPos(&p);

	_dMouse.x = float(p.x) - _mouse.x;
	_dMouse.y = float(p.y) - _mouse.y;
	_mouse = XMFLOAT2(float(p.x), float(p.y));

	for (unsigned i = 0; i < 256; i++) {
		_keyboardScanCodes[i] = _keyboardScanCodesRaw[i];
		_keyboardVirtualKeys[i] = _keyboardVirtualKeysRaw[i];
	}

	if (_joysticks)
		_joysticks->Update();
}

bool ViewerInputManager::MsgProc(MSG *msg)
{
	switch (msg->message) {
		case WM_LBUTTONDOWN:
			if (!_mouseButtonsRaw)
				SetCapture(msg->hwnd);
			_mouseButtonsRaw |= MK_LBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
			_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
			_mouseAction = true;
			break;
		case WM_LBUTTONUP:
			if (_mouseButtonsRaw & MK_LBUTTON) {
				_mouseButtonsRaw &= ~MK_LBUTTON;
				_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
				_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
				_mouseAction = true;
				if (!_mouseButtonsRaw)
					ReleaseCapture();
			}
			break;
		case WM_MBUTTONDOWN:
			if (!_mouseButtonsRaw)
				SetCapture(msg->hwnd);
			_mouseButtonsRaw |= MK_MBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
			_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
			_mouseAction = true;
			break;
		case WM_MBUTTONUP:
			if (_mouseButtonsRaw & MK_MBUTTON) {
				_mouseButtonsRaw &= ~MK_MBUTTON;
				_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
				_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
				_mouseAction = true;
				if (!_mouseButtonsRaw)
					ReleaseCapture();
			}
			break;
		case WM_RBUTTONDOWN:
			if (!_mouseButtonsRaw)
				SetCapture(msg->hwnd);
			_mouseButtonsRaw |= MK_RBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
			_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
			_mouseAction = true;
			break;
		case WM_RBUTTONUP:
			if (_mouseButtonsRaw & MK_RBUTTON) {
				_mouseButtonsRaw &= ~MK_RBUTTON;
				_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
				_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
				_mouseAction = true;
				if (!_mouseButtonsRaw)
					ReleaseCapture();
			}
			break;
		case WM_MOUSEWHEEL:
			_mouseWheelTmp += (float)GET_WHEEL_DELTA_WPARAM(msg->wParam) / (float)WHEEL_DELTA;
			break;
		case WM_LBUTTONDBLCLK:
//			_mouseStateTmp |= MK_LBUTTON;
			_mouseButtonsDblClickRaw |= MK_LBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
			_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
			_mouseAction = true;
			break;
		case WM_MBUTTONDBLCLK:
//			_mouseStateTmp |= MK_MBUTTON;
			_mouseButtonsDblClickRaw |= MK_MBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
			_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
			_mouseAction = true;
			break;
		case WM_RBUTTONDBLCLK:
//			_mouseStateTmp |= MK_RBUTTON;
			_mouseButtonsDblClickRaw |= MK_RBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
			_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
			_mouseAction = true;
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			_keyboardScanCodesRaw[(msg->lParam&0xFF0000) >> 16] = 1;
			_keyboardVirtualKeysRaw[msg->wParam] = 1;
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			_keyboardScanCodesRaw[(msg->lParam&0xFF0000) >> 16] = 0;
			_keyboardVirtualKeysRaw[msg->wParam] = 0;
			break;
		case WM_KILLFOCUS://WM_CAPTURECHANGED:
			if (msg->wParam == 0) {
				_mouseButtonsRaw = 0;
				memset(_keyboardScanCodesRaw, 0, sizeof(_keyboardScanCodesRaw));
				memset(_keyboardVirtualKeysRaw, 0, sizeof(_keyboardVirtualKeysRaw));			
			}
			/*if (_mouseStateTmp) {
				_mouseStateTmp = 0;
				_mouseAction = true;
				//_mouseActionPos
				handled = true;
			}*/
			{
			int gg = 0;
			}
			break;

		case WM_INPUT: // Raw input
			break;
			{
				// RAW INPUT:
				// I should consider using raw input for mouse to take advantage of high def mouse movement: http://msdn.microsoft.com/en-us/library/windows/desktop/ee418864(v=vs.85).aspx
				// Also, raw input gives the possibility of capturing input even when the app is not in focus.
				// Registering double clicks are a problem with raw input. I should also think about mouse ballistics not being there.

				UINT dwSize;

				GetRawInputData((HRAWINPUT)msg->lParam, RID_INPUT, nullptr, &dwSize, sizeof(RAWINPUTHEADER));
				BYTE lpb[1024];
				
				RAWINPUT* raw = (RAWINPUT*)lpb;

				if (GetRawInputData((HRAWINPUT)msg->lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize)
					return false;

				/*if (raw->header.dwType == RIM_TYPEMOUSE) {
					if (raw->data.mouse.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN) {
						if (inFocus) {
							_mouseStateTmp |= MK_LBUTTON;
							_mouseActionPos =  msg->pt;
							_mouseAction = true;
							handled = true;
						}
					}
					if (raw->data.mouse.ulButtons & RI_MOUSE_LEFT_BUTTON_UP) {
						if (inFocus || (_mouseStateTmp & MK_LBUTTON)) {
							_mouseStateTmp &= ~MK_LBUTTON;
							_mouseActionPos =  msg->pt;
							_mouseAction = true;
							handled = true;
						}
					}
					if (raw->data.mouse.ulButtons & RI_MOUSE_MIDDLE_BUTTON_DOWN) {
						if (inFocus) {
							_mouseStateTmp |= MK_MBUTTON;
							_mouseActionPos =  msg->pt;
							_mouseAction = true;
							handled = true;
						}
					}
					if (raw->data.mouse.ulButtons & RI_MOUSE_MIDDLE_BUTTON_UP) {
						if (inFocus || (_mouseStateTmp & MK_MBUTTON)) {
							_mouseStateTmp &= ~MK_MBUTTON;
							_mouseActionPos =  msg->pt;
							_mouseAction = true;
							handled = true;
						}
					}
					if (raw->data.mouse.ulButtons & RI_MOUSE_RIGHT_BUTTON_DOWN) {
						if (inFocus) {
							_mouseStateTmp |= MK_RBUTTON;
							_mouseActionPos =  msg->pt;
							_mouseAction = true;
							handled = true;
						}
					}
					if (raw->data.mouse.ulButtons & RI_MOUSE_RIGHT_BUTTON_UP) {
						if (inFocus || (_mouseStateTmp & MK_RBUTTON)) {
							_mouseStateTmp &= ~MK_RBUTTON;
							_mouseActionPos =  msg->pt;
							_mouseAction = true;
							handled = true;
						}
					}
					if (raw->data.mouse.ulButtons & RI_MOUSE_WHEEL) {
						if (inFocus) {
							_mouseWheelTmp += (float)(SHORT)raw->data.mouse.usButtonData / (float)WHEEL_DELTA;
							handled = true;
						}
					}
					if (!(raw->data.mouse.usFlags & (MOUSE_MOVE_ABSOLUTE | MOUSE_VIRTUAL_DESKTOP))) {
						_dMouseTmp.x += raw->data.mouse.lLastX;
						_dMouseTmp.y += raw->data.mouse.lLastY;
					}
				}
				else */
				if (raw->header.dwType == RIM_TYPEKEYBOARD) {
					if (raw->data.keyboard.VKey > 254)
						break; // ignore messages without virtual key code.
					USHORT makeCode = raw->data.keyboard.MakeCode;
					USHORT extBit = 0;
					if ((raw->data.keyboard.Flags & RI_KEY_E1) && raw->data.keyboard.MakeCode == 0x1D)
						makeCode = 0x45; // pause key
					if ((raw->data.keyboard.Flags & RI_KEY_E0) || raw->data.keyboard.MakeCode == 0x45) // 0xE0 flag or NumLock
						extBit = 1;
					BYTE scanCode = (BYTE)((makeCode & 0x7F) | extBit << 7);
					switch (raw->data.keyboard.Message)
					{
					case WM_KEYDOWN:
					case WM_SYSKEYDOWN:
						/*if (inFocus) {
							_keyboardScanCodesTmp[scanCode] = true;
							_keyboardVirtualKeysTmp[raw->data.keyboard.VKey] = true;
							handled = true;
						}
						break;
					case WM_KEYUP:
					case WM_SYSKEYUP:
						if (inFocus || _keyboardScanCodesTmp[scanCode]) {
							_keyboardScanCodesTmp[scanCode] = false;
							handled = true;
						}
						if (inFocus || _keyboardVirtualKeysTmp[raw->data.keyboard.VKey]) {
							_keyboardVirtualKeysTmp[raw->data.keyboard.VKey] = false;
							handled = true;
						}*/
						break;
					default:
						break;
					}
				}
			}
			break;
		default:
			break;
	}

	return false;


}

Joysticks* ViewerInputManager::GetJoysticks()
{
	if (!_joysticks) {
		_joysticks = mmnew Joysticks();
	}

	return _joysticks;
}