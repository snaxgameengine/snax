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

#include "GlobalDef.h"
#include "M3DCore/MString.h"

namespace m3d
{


class Window;

#define PRESENT_OK 0
#define PRESENT_NO_SWAP_CHAIN 1
#define PRESENT_OCCLUDED 2
#define PRESENT_DEVICE_RESET 3
#define PRESENT_DEVICE_REMOVED 4
#define PRESENT_ERROR 5

class M3DENGINE_API RenderWindow
{
protected:
	// The window used for rendering
	Window *_wnd = nullptr;
	// true if waiting for vSync at present.
	BOOL _vSync = FALSE;
	
public:
	RenderWindow(Window *wnd);
	virtual ~RenderWindow();

	// Gets the window handle set in constructor.
	Window *GetWindow() const { return _wnd; }
	// Is vertical sync?
	BOOL IsVSync() const { return _vSync; }
	// Set vertical sync
	void SetVSync(BOOL vsync) { _vSync = vsync; }
	// Presents.
	virtual UINT Present(bool testOnly = false, bool forceVSync = false, bool skipOverlay = false) = 0;
	// Called by window system on WM_SIZE. Callback will be notified if releasing back buffer!
	virtual HRESULT OnResizeWindow() = 0;
	// Called when graphics device is destroyed.
	virtual void OnDestroyDevice() = 0;

	virtual bool RequestScreenshot(const Char *Filepath) { return false; }

};


}


