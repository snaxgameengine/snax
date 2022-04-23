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
#include "InputManager2.h"
#include "Joysticks.h"


using namespace m3d;



InputManager2::InputManager2()
{
	memset(_hWnd, 0, sizeof(_hWnd));
	_wCount = 0;
	_mouse.x = 0;
	_mouse.y = 0;
	_dMouse.x = 0;
	_dMouse.y = 0;
	_dMouseTmp.x = 0;
	_dMouseTmp.y = 0;
	_mouseStateTmp = 0;
	_mouseDoubleclickTmp = 0;
	_mouseWheel = 0.0f;
	_mouseWheelTmp = 0.0f;
	_mouseActionPos.x = 0;
	_mouseActionPos.y = 0;
	_mouseAction = false;
	_joysticks = nullptr;

	memset(_keyboardScanCodesTmp, 0, sizeof(_keyboardScanCodesTmp));
	memset(_keyboardVirtualKeysTmp, 0, sizeof(_keyboardVirtualKeysTmp));
}

InputManager2::~InputManager2()
{
	if (_joysticks)
		mmdelete(_joysticks);
}

bool InputManager2::MsgProc(MSG *msg)
{
	bool handled = false;

#ifdef WINDESKTOP

	bool inFocus = false;
	for (uint32 i = 0; i < _wCount; i++) {
		if (_hWnd[i] == msg->hwnd) {
			inFocus = true;
			break;
		}
	}
	inFocus = true;

	switch (msg->message) {
		case WM_LBUTTONDOWN:
			if (inFocus) {
				if (!_mouseStateTmp)
					SetCapture(msg->hwnd);
				_mouseStateTmp |= MK_LBUTTON;
				_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
				_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
				_mouseAction = true;
				handled = true;
			}
			break;
		case WM_LBUTTONUP:
			if (inFocus || (_mouseStateTmp & MK_LBUTTON)) {
				_mouseStateTmp &= ~MK_LBUTTON;
				_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
				_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
				_mouseAction = true;
				handled = true;
				if (!_mouseStateTmp)
					ReleaseCapture();
			}
			break;
		case WM_MBUTTONDOWN:
			if (inFocus) {
				if (!_mouseStateTmp)
					SetCapture(msg->hwnd);
				_mouseStateTmp |= MK_MBUTTON;
				_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
				_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
				_mouseAction = true;
				handled = true;
			}
			break;
		case WM_MBUTTONUP:
			if (inFocus || (_mouseStateTmp & MK_MBUTTON)) {
				_mouseStateTmp &= ~MK_MBUTTON;
				_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
				_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
				_mouseAction = true;
				handled = true;
				if (!_mouseStateTmp)
					ReleaseCapture();
			}
			break;
		case WM_RBUTTONDOWN:
			if (inFocus) {
				if (!_mouseStateTmp)
					SetCapture(msg->hwnd);
				_mouseStateTmp |= MK_RBUTTON;
				_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
				_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
				_mouseAction = true;
				handled = true;
			}
			break;
		case WM_RBUTTONUP:
			if (inFocus || (_mouseStateTmp & MK_RBUTTON)) {
				_mouseStateTmp &= ~MK_RBUTTON;
				_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
				_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
				_mouseAction = true;
				handled = true;
				if (!_mouseStateTmp)
					ReleaseCapture();
			}
			break;
		case WM_MOUSEWHEEL:
			if (inFocus) {
				_mouseWheelTmp += (float32)GET_WHEEL_DELTA_WPARAM(msg->wParam) / (float32)WHEEL_DELTA;
				handled = true;
			}
			break;
		case WM_LBUTTONDBLCLK:
			if (inFocus) {
				_mouseStateTmp |= MK_LBUTTON;
				_mouseDoubleclickTmp |= MK_LBUTTON;
				_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
				_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
				_mouseAction = true;
				handled = true;
			}
			break;
		case WM_MBUTTONDBLCLK:
			if (inFocus) {
				_mouseStateTmp |= MK_MBUTTON;
				_mouseDoubleclickTmp |= MK_MBUTTON;
				_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
				_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
				_mouseAction = true;
				handled = true;
			}
			break;
		case WM_RBUTTONDBLCLK:
			if (inFocus) {
				_mouseStateTmp |= MK_RBUTTON;
				_mouseDoubleclickTmp |= MK_RBUTTON;
				_mouseActionPos.x = GET_X_LPARAM(msg->lParam);
				_mouseActionPos.y = GET_Y_LPARAM(msg->lParam);
				_mouseAction = true;
				handled = true;
			}
			break;
		case WM_CAPTURECHANGED:
			/*if (_mouseStateTmp) {
				_mouseStateTmp = 0;
				_mouseAction = true;
				//_mouseActionPos
				handled = true;
			}*/
			break;
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			if (inFocus) {
				_keyboardScanCodesTmp[(msg->lParam&0xFF0000) >> 16] = true;
				_keyboardVirtualKeysTmp[msg->wParam] = true;
				handled = true;
			}
			break;
		case WM_KEYUP:
		case WM_SYSKEYUP:
			if (inFocus || _keyboardScanCodesTmp[(msg->lParam&0xFF0000) >> 16]) {
				_keyboardScanCodesTmp[(msg->lParam&0xFF0000) >> 16] = false;
				handled = true;
			}
			if (inFocus || _keyboardVirtualKeysTmp[msg->wParam]) {
				_keyboardVirtualKeysTmp[msg->wParam] = false;
				handled = true;
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
							_mouseWheelTmp += (float32)(SHORT)raw->data.mouse.usButtonData / (float32)WHEEL_DELTA;
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
						if (inFocus) {
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
						}
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

#endif

	return handled;
}

bool InputManager2::Init(HWND w, bool useJoysticks)
{
#ifdef WINDESKTOP

	RAWINPUTDEVICE Rid[3];

	// Mouse
	Rid[0].usUsagePage = 1;
	Rid[0].usUsage = 2;
	Rid[0].dwFlags = 0;//RIDEV_INPUTSINK;
	Rid[0].hwndTarget=w;

	// Keyboard
	Rid[1].usUsagePage = 1;
	Rid[1].usUsage = 6;
	Rid[1].dwFlags = 0;//RIDEV_INPUTSINK; // Note: You can specify RIDEV_INPUTSINK to process events even when app is not in foreground (hwndTarget must be set)!
	Rid[1].hwndTarget=w;
/*	
	// Joystick
	Rid[2].usUsagePage = 1;
	Rid[2].usUsage = 4;
	Rid[2].dwFlags = 0;//RIDEV_INPUTSINK;
	Rid[2].hwndTarget=0;//w;
*/	
//	BOOL b = RegisterRawInputDevices(Rid , 2, sizeof(RAWINPUTDEVICE));
//	assert(b);
/*
	if (useJoysticks) {
		_joysticks = mmnew Joysticks();
		_joysticks->Init(w);
	}
	*/
#endif

	return true;
}

void InputManager2::Update()
{

	_mouseButtons = _mouseStateTmp;
	_mouseButtonsDblClick = _mouseDoubleclickTmp;
	_mouseWheel = _mouseWheelTmp;
	_mouseDoubleclickTmp = 0;
	_mouseWheelTmp = 0.0f;

	for (uint32 i = 0; i < 256; i++) {
		_keyboardScanCodes[i] = _keyboardScanCodesTmp[i];
		_keyboardVirtualKeys[i] = _keyboardVirtualKeysTmp[i];
	}

#ifdef WINDESKTOP
	POINT p;
	GetCursorPos(&p);

	_dMouse.x = p.x - _mouse.x;
	_dMouse.y = p.y - _mouse.y;
	_mouse = p;
//	_dMouse = _dMouseTmp;
//	_dMouseTmp.x = 0;
//	_dMouseTmp.y = 0;
#endif

	_mouseAction = false;

	if (_joysticks)
		_joysticks->Update();
}

