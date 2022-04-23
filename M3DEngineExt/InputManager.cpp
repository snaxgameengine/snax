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
#include "M3DEngine/InputManager.h"
#include <windowsx.h>



using namespace m3d;
/*


InputManager::InputManager()
{
	_hWnd = 0;
	_mouse.x = 0;
	_mouse.y = 0;
	_dMouse.x = 0;
	_dMouse.y = 0;
	_mouseStateTmp = 0;
	_mouseDoubleclick = 0;
	_mouseDoubleclickTmp = 0;
	_mouseWheel = 0.0f;
	_mouseWheelTmp = 0.0f;
	_mouseAction = false;
}

InputManager::~InputManager()
{
}

bool InputManager::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, POINT p)
{
	if (hWnd != _hWnd)
		return false;

	bool b = true;

	switch (uMsg) {
		case WM_LBUTTONDBLCLK:
			_mouseDoubleclickTmp |= MK_LBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(lParam);
			_mouseActionPos.y = GET_Y_LPARAM(lParam);
			_mouseAction = true;
	//		break;
		case WM_LBUTTONDOWN:
			_mouseStateTmp |= MK_LBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(lParam);
			_mouseActionPos.y = GET_Y_LPARAM(lParam);
			_mouseAction = true;
			{
			/*	POINT p, q;
				GetCursorPos(&q);
				GetPhysicalCursorPos(&p);
				POINT s = p;
	
			//	RECT r, wr;
			//	GetClientRect(_hWnd, &r);
			//	GetWindowRect(_hWnd, &wr);
			//	p.x = p.x - (wr.left + r.left);
			//	p.y = p.y - (wr.top + r.top);
				CURSORINFO io;
				io.hCursor = GetCursor();
				io.cbSize = sizeof(CURSORINFO);
				BOOL bb = GetCursorInfo(&io);
				//p = io.ptScreenPos;* /
				ScreenToClient(_hWnd, &p);

	
				if (_mouseActionPos.x != p.x || _mouseActionPos.y != p.y)
					int32 gg = 5;
			}
			break;
		case WM_MOUSEMOVE:
			break;
		case WM_LBUTTONUP:
			_mouseStateTmp &= ~MK_LBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(lParam);
			_mouseActionPos.y = GET_Y_LPARAM(lParam);
			_mouseAction = true;
			break;
		case WM_MBUTTONDBLCLK:
			_mouseDoubleclickTmp |= MK_MBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(lParam);
			_mouseActionPos.y = GET_Y_LPARAM(lParam);
			_mouseAction = true;
//			break;
		case WM_MBUTTONDOWN:
			_mouseStateTmp |= MK_MBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(lParam);
			_mouseActionPos.y = GET_Y_LPARAM(lParam);
			_mouseAction = true;
			break;
		case WM_MBUTTONUP:
			_mouseStateTmp &= ~MK_MBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(lParam);
			_mouseActionPos.y = GET_Y_LPARAM(lParam);
			_mouseAction = true;
			break;
		case WM_RBUTTONDBLCLK:
			_mouseDoubleclickTmp |= MK_RBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(lParam);
			_mouseActionPos.y = GET_Y_LPARAM(lParam);
			_mouseAction = true;
//			break;
		case WM_RBUTTONDOWN:
			_mouseStateTmp |= MK_RBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(lParam);
			_mouseActionPos.y = GET_Y_LPARAM(lParam);
			_mouseAction = true;
			break;
		case WM_RBUTTONUP:
			_mouseStateTmp &= ~MK_RBUTTON;
			_mouseActionPos.x = GET_X_LPARAM(lParam);
			_mouseActionPos.y = GET_Y_LPARAM(lParam);
			_mouseAction = true;
			break;
		case WM_MOUSEWHEEL:
			_mouseWheelTmp = (float32)GET_WHEEL_DELTA_WPARAM(wParam) / (float32)WHEEL_DELTA;
			break;
		case WM_KILLFOCUS:
//		case WM_ACTIVATEAPP:
//			if (!wParam) {
				_mouseStateTmp = 0;
				_mouseDoubleclickTmp = 0;
//			}
			break;
		case WM_INPUT: // Raw input
			{
				int32 Gf = 5;
//				RAWINPUT buffer;
	//			UINT bufferSize = sizeof(RAWINPUT);
		//		UINT g = GetRawInputData((HRAWINPUT)lParam, RID_INPUT, (LPVOID)&buffer, &bufferSize, sizeof (RAWINPUTHEADER));
			//	if (g > 0) {
				//	if (buffer.header.dwType == RIM_DESCMOUSE) {
					//	if (buffer.data.mouse.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)
						//	int32 gg = 5;
	//				}
			

		//		}
				
			}
			break;
		default:
			b = false;
			break;
	}

	return b;
}
*/
/*
usUsagePage

1 for generic desktop controls
2 for simulation controls
3 for vr
4 for sport
5 for game
6 for generic device
7 for keyboard
8 for leds
9 button
usUsage values when usUsagePage is 1

0 - undefined
1 - pointer
2 - mouse
3 - reserved
4 - joystick
5 - game pad
6 - keyboard
7 - keypad
8 - multi-axis controller
9 - Tablet PC controls
*/
/*
bool InputManager::Init()
{
	RAWINPUTDEVICE Rid[2];

	// Mouse
	Rid[0].usUsagePage = 1;
	Rid[0].usUsage = 2;
	Rid[0].dwFlags = 0;
	Rid[0].hwndTarget=NULL;

	// Keyboard
	Rid[1].usUsagePage = 1;
	Rid[1].usUsage = 6;
	Rid[1].dwFlags = RIDEV_INPUTSINK;
	Rid[1].hwndTarget=NULL;

//	BOOL b = RegisterRawInputDevices(Rid, 1, sizeof(RAWINPUTDEVICE));

	return true;
}

void InputManager::_readRaw()
{/* commented 18/2/11
	UINT cbSize=40;

	bool ok = GetRawInputBuffer(NULL, &cbSize, sizeof(RAWINPUTHEADER)) == 0;

	if (!ok)
		return;

	if (!cbSize)
		return;

	cbSize *= 16; // this is a wild guess

	PRAWINPUT pRawInput = (PRAWINPUT)malloc(cbSize); 

	while (true) {
		UINT cbSizeT = cbSize;
		UINT nInput = GetRawInputBuffer(pRawInput, &cbSizeT, sizeof(RAWINPUTHEADER));

		if (nInput == -1) {
			DWORD gg =  GetLastError();
			int32 gg1 = 5;
		}

		if (nInput == 0) 
			break;

		PRAWINPUT* paRawInput = (PRAWINPUT*)malloc(sizeof(PRAWINPUT) * nInput);

		PRAWINPUT pri = pRawInput;
		for (UINT i = 0; i < nInput; ++i) { 
			if (pri->header.dwType == RIM_TYPEMOUSE) {
				if (pri->data.mouse.ulButtons & RI_MOUSE_LEFT_BUTTON_DOWN)
					int32 gg = 5;
			}


			paRawInput[i] = pri;
			pri = NEXTRAWINPUTBLOCK(pri);
		}

		// to clean the buffer
		DefRawInputProc(paRawInput, nInput, sizeof(RAWINPUTHEADER)); 

		free(paRawInput);
	}
	free(pRawInput);* /
}

void InputManager::Update()
{
//	_readRaw();

	_mouseButtons = _mouseStateTmp;
	_mouseDoubleclick = _mouseDoubleclickTmp;
	_mouseWheel = _mouseWheelTmp;

	_mouseDoubleclickTmp = 0;
	_mouseWheelTmp = 0.0f;

	POINT p;
	if (_mouseAction) // _mouseAction: With low fremerates, pressing a button will be reported at a different location than the actual position, beacuse the mouse may move alot before this function is called! This stuff fix it!
		p = _mouseActionPos;
	else {
		GetCursorPos(&p);
	
		RECT r, wr;
		GetClientRect(_hWnd, &r);
		GetWindowRect(_hWnd, &wr);
		p.x = p.x - (wr.left + r.left);
		p.y = p.y - (wr.top + r.top);
	}
	_mouseAction = false;

	_dMouse.x = p.x - _mouse.x;
	_dMouse.y = p.y - _mouse.y;

	_mouse = p;

	bool inFocus = GetFocus() == _hWnd;

	if (inFocus) {
		BYTE keys[256];
		::GetKeyboardState(keys);
		for (unsigned i = 0; i < 256; i++) {
			_keyboard[i] = (unsigned)keys[i];
		}
	}
	else {
		for (unsigned i = 0; i < 256; i++)
			_keyboard[i] = 0;
	}
}
/*
const POINT InputManager::GetMousePos(HWND hWnd) const
{
	POINT p = _mouse;
	RECT r, wr;
	GetClientRect(hWnd, &r);
	GetWindowRect(hWnd, &wr);
	p.x = p.x - (wr.left + r.left);
	p.y = p.y - (wr.top + r.top);
	return p;
}*/