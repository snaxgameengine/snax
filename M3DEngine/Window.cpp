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
#include "Window.h"
#include "M3DCore/PlatformDef.h"

using namespace m3d;


RECT Window::GetClientRect()
{
	HWND hWnd = GetWindowHandle();
	RECT r = {0, 0, 0, 0};
#ifdef WINDESKTOP
	BOOL b = ::GetClientRect(hWnd, &r);
#endif
	return r;
}

void Window::SetWindowPos(INT left, INT top, INT width, INT height, UINT flags)
{
	HWND hWnd = GetWindowHandle();
#ifdef WINDESKTOP
	::SetWindowPos(hWnd, 0, left, top, width, height, flags);
#endif
}

void Window::SetText(String s) 
{
	HWND hWnd = GetWindowHandle();
#ifdef WINDESKTOP
	::SetWindowTextA(hWnd, s.c_str());
#endif
}

HWND Window::GetParentWindow()
{
	HWND hWnd = GetWindowHandle();
#ifdef WINDESKTOP
	return ::GetParent(hWnd);
#else
	return 0;
#endif
}

POINT Window::ScreenToClient(POINT p)
{
	HWND hWnd = GetWindowHandle();
#ifdef WINDESKTOP
	::ScreenToClient(hWnd, &p);
#endif
	return p;
}

void Window::EnsureVisible()
{


}

void Window::SetStyle(int32 style)
{
	HWND hWnd = GetWindowHandle();
#ifdef WINDESKTOP
	::SetWindowLong(hWnd, GWL_STYLE, style);
#endif
}

void Window::SetExtendedStyle(int32 style)
{
	HWND hWnd = GetWindowHandle();
#ifdef WINDESKTOP
	::SetWindowLong(hWnd, GWL_EXSTYLE, style);
#endif
}