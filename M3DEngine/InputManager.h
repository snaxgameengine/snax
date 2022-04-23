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
#include "M3DCore/TriState.h"
#include "M3DCore/MMath.h"


namespace m3d
{

class Joysticks;

class M3DENGINE_API InputManager
{
public:
	static const uint32 LBUTTON = 0;
	static const uint32 MBUTTON = 1;
	static const uint32 RBUTTON = 2;

	struct Touch
	{
		uint32 uniqueID;
		XMFLOAT2 pos;
		XMFLOAT2 dpos;
		TriState state;
		Touch() : uniqueID(0), pos(0.0f, 0.0f), dpos(0.0f, 0.0f) {}
	};

	virtual void Init() = 0;
	virtual void Update() = 0;

	virtual TriState GetKeyboardScanCode(uint32 scanCode) const = 0;
	virtual TriState GetVirtualKeyCode(uint32 vkCode) const = 0;
	virtual bool GetVirtualKeyCodeState(uint32 vkCode) const = 0;

	virtual TriState GetMouseButton(uint32 index) const = 0;
	virtual TriState GetMouseButtonDblClick(uint32 index) const = 0;
	virtual float32 GetMouseWheel() const = 0;
	virtual XMFLOAT2 GetMousePos() const = 0;
	virtual XMFLOAT2 GetMouseDeltaPos() const = 0;

	virtual const Touch &GetTouchPoint(uint32 index) const = 0;
	virtual const uint32 GetTouchCount() const = 0;

	virtual Joysticks *GetJoysticks() = 0;

};




}