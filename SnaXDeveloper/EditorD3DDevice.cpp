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

#include "stdafx.h"
#include "EditorD3DDevice.h"

using namespace m3d;


EditorD3DDevice *EditorD3DDevice::GetInstance()
{
	static EditorD3DDevice d;
	return &d;
}

EditorD3DDevice::EditorD3DDevice()
{
}

EditorD3DDevice::~EditorD3DDevice()
{
	DestroyDevice();
}

HRESULT EditorD3DDevice::CreateDevice(bool debugDevice, bool forceD3D9Level)
{
	D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_9_1;

	HRESULT hr;

	_debugDevice = debugDevice;
	_forceD3D9Level = forceD3D9Level;

	SIDXGIFactory1 dxgiFactory1;
	V_RETURN(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory1)));

	SIDXGIAdapter1 adapter;
	SIZE_T maxMem = 0;

	// Enumerate adapters.
	for (UINT i = 0;; i++) {
		SIDXGIAdapter1 a;

		hr = dxgiFactory1->EnumAdapters1(i, &a);

		if (hr == DXGI_ERROR_NOT_FOUND)
			break; // No more found.

		V_RETURN(hr);

		D3D_FEATURE_LEVEL fl;

		// Can this adapter create a d3d12 device?
		if (FAILED(D3D11CreateDevice(a, D3D_DRIVER_TYPE_UNKNOWN, nullptr, 0, nullptr, 0, D3D11_SDK_VERSION, nullptr, nullptr, nullptr)))
			continue;

		DXGI_ADAPTER_DESC1 desc;
		a->GetDesc1(&desc);

		// Prefer adapter with the most video memory available.
		if (desc.DedicatedVideoMemory > maxMem) {
			adapter = a;
			maxMem = desc.DedicatedVideoMemory;
		}
	}


	while (true) {
		hr = D3D11CreateDevice(adapter, adapter ? D3D_DRIVER_TYPE_UNKNOWN : D3D_DRIVER_TYPE_HARDWARE, nullptr,
			_debugDevice ? D3D11_CREATE_DEVICE_DEBUG : 0,
			_forceD3D9Level ? &fl : nullptr, _forceD3D9Level ? 1 : 0, D3D11_SDK_VERSION, &_device, nullptr, nullptr);
		if (FAILED(hr)) {
			if (_debugDevice) {
				_debugDevice = false;
				msg(FATAL, MTEXT("Failed to create D3D Device for Studio graphics! Trying again without debug layer!"));
				continue;
			}
		}
		break;
	}	

	return hr;
}

void EditorD3DDevice::DestroyDevice()
{
	if (!_device)
		return;
	for (const auto &n : _listeners) {
		n->OnDestroyDevice();
	}
	_device = nullptr;
}

HRESULT EditorD3DDevice::RecreateDevice()
{
	DestroyDevice();
	return CreateDevice(_debugDevice, _forceD3D9Level);
}