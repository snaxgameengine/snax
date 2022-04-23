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
#include "ViewerWindow.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/RenderWindowManager.h"
#include "GraphicsChips/Graphics.h"

using namespace m3d;


#define CLASS_NAME MTEXT("SnaXViewerClass")
#define WINDOW_NAME MTEXT("SnaXViewer")


ViewerWindow::ViewerWindow() : _hWnd(NULL), _rw(nullptr), _presentResult(PRESENT_OK)
{
}

ViewerWindow::~ViewerWindow()
{
}

bool ViewerWindow::Init()
{
	// Create a default application window, invisible! TODO: Application should be able to create a window with desired style, titlebar, buttons etc...
	_hWnd = _createWindow();
	if (_hWnd == NULL) {
		msg(FATAL, MTEXT("Failed in CreateWindow(...);"));
		return false;
	}

	_rw = engine->GetGraphics()->CreateRenderWindow(this);
	if (!_rw) {
		msg(FATAL, MTEXT("Failed to create render window."));
		return false;
	}

	// Create a new render window an assign it to our render window manager. No d3d-stuff are touched here.
	// Window is invisible and swap chain with desired settings are created on demand later.
	if (!engine->GetGraphics()->GetRenderWindowManager()->SetRenderWindow(_rw)) {
		msg(FATAL, MTEXT("Failed to set render window."));
		return false;
	}

//	engine->GetInputManager()->RegisterWindow(_hWnd);

	return true;
}

void ViewerWindow::Destroy()
{
	if (_rw)
		mmdelete(_rw);
	DestroyWindow(_hWnd);
}

void ViewerWindow::OnResizeWindow()
{
	if (_rw)
		_rw->OnResizeWindow();
}

void ViewerWindow::OnDestroyDevice()
{
	if (_rw)
		_rw->OnDestroyDevice();
}

bool ViewerWindow::CreateWindowClass(HINSTANCE hInstance, WNDPROC msgProc)
{
	WNDCLASSA wndClass;

	CHAR exePath[MAX_PATH];
	GetModuleFileNameA( NULL, exePath, MAX_PATH );
	HICON hIcon = ExtractIconA(hInstance, exePath, 0);

	ZeroMemory(&wndClass, sizeof(WNDCLASSA));
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hIcon = hIcon;
//	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.style = CS_DBLCLKS;
	wndClass.lpfnWndProc = msgProc;
	wndClass.hInstance = hInstance;
	wndClass.lpszClassName = CLASS_NAME;

	ATOM classAtom = RegisterClassA(&wndClass);

	return classAtom != 0;
}

HWND ViewerWindow::_createWindow()
{
	int sw = GetSystemMetrics(SM_CXFULLSCREEN);
	int sh = GetSystemMetrics(SM_CYFULLSCREEN);

	int x = sw / 3;
	int y = sh / 3;
	int w = sw / 3;
	int h = sh / 3;

	HWND hWnd = CreateWindowExA( /*WS_EX_APPWINDOW */ 0, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, x, y, w, h, NULL, 0, NULL, NULL );

	return hWnd;
}

void ViewerWindow::EnsureVisible()
{
	if (!IsWindowVisible(_hWnd)) {
		::ShowWindow(_hWnd, SW_SHOWDEFAULT);
	}
}

UINT ViewerWindow::Present(bool test)
{
	if (_rw)
		_presentResult = _rw->Present(test);
	return _presentResult;
}
