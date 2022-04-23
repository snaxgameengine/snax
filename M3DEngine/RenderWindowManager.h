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

#include "RenderWindow.h"

namespace m3d
{


#define MAX_RENDER_WINDOWS 1



class M3DENGINE_API RenderWindowManager
{
public:
	RenderWindowManager();
	virtual ~RenderWindowManager();

	// Creates a compatible render window.
	virtual RenderWindow *CreateRenderWindow(Window *wnd) = 0;
	// Set the render window at the given index. If the index was previously set by another window, a releasing-back-buffer call will be made.
	// Returns true on success, false if the window was set an another index previously.
	virtual bool SetRenderWindow(RenderWindow *rw, uint32 index = 0) = 0;
	// Gets the render window at the given index. Can be null. If the swap chain is not created (lacy init), it will be, possibly using the default parameters.
	// Window will be made visible if not.
	virtual RenderWindow *GetRenderWindow(uint32 index = 0) = 0;
	// Gets the window used for rendering. Does not make it visible. Allows for style change etc without being visible.
	virtual Window *GetWindow(uint32 index = 0) = 0;
	// Goes to fullscreen using the given parameters.
	virtual void GoFullscreen(UINT width, UINT height, UINT refreshRateNumerator, UINT refreshRateDenominator, UINT outputIndex, UINT sampleDescCount, UINT sampleDescQuality, bool srgb, bool vSync, uint32 index = 0) = 0;
	// Goes windowed using the given params.
	virtual void GoWindowed(INT left, INT top, UINT width, UINT height, UINT sampleDescCount, UINT sampleDescQuality, bool srgb, bool vSync, uint32 index = 0) = 0;
};

}