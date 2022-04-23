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

#include "Common.h"
#include "M3DEngineExt/D3DInclude.h"
#include "M3DCore/Containers.h"

namespace m3d
{

class EditorD3DDeviceListener
{
public:
	virtual void OnDestroyDevice() = 0;
};

class EditorD3DDevice
{
public:
	static EditorD3DDevice *GetInstance();

	HRESULT CreateDevice(bool debugDevice = false, bool forceD3D9Level = false);
	void DestroyDevice();
	HRESULT RecreateDevice();

	ID3D11Device *GetDevice() { return _device; }

	void AddListener(EditorD3DDeviceListener *l) { _listeners.insert(l); }
	void RemoveListener(EditorD3DDeviceListener *l) { _listeners.erase(l); }

private:
	EditorD3DDevice();
	~EditorD3DDevice();

	SID3D11Device _device;
	Set<EditorD3DDeviceListener*> _listeners;

	bool _debugDevice = false;
	bool _forceD3D9Level = false;
};

}