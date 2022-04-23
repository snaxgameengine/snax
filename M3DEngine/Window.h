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
#include "M3DCore/MString.h"
#include <unknwn.h>

namespace m3d
{


class M3DENGINE_API Window
{
public:
	virtual HWND GetWindowHandle() { return NULL; }
	virtual IUnknown *GetCoreWindow() { return nullptr; } // WP8
	
	virtual RECT GetClientRect();
	virtual void SetWindowPos(INT left, INT top, INT width = 0, INT height = 0, UINT flags = SWP_NOZORDER | SWP_NOSIZE);
	virtual void SetText(String s);
	virtual HWND GetParentWindow();
	virtual POINT ScreenToClient(POINT p);
	virtual void SetStyle(int32 style);
	virtual void SetExtendedStyle(int32 style);

	virtual void EnsureVisible();


	virtual bool CanGoFullscreen() const { return true; }
	virtual bool CanMove() const { return true; }
	virtual bool CanResize() const { return true; }
};



}