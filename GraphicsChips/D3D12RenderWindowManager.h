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
#include "M3DEngine/RenderWindowManager.h"
#include "D3D12RenderWindow.h"

namespace m3d
{

class DescriptorHeapManager;

class GRAPHICSCHIPS_API D3D12RenderWindowManager : public RenderWindowManager
{
public:
	D3D12RenderWindowManager(DescriptorHeapManager *rtvHeapManager);
	~D3D12RenderWindowManager();

	RenderWindow *CreateRenderWindow(Window *wnd) override;
	bool SetRenderWindow(RenderWindow *rw, uint32 index) override;
	RenderWindow *GetRenderWindow(uint32 index) override;
	Window *GetWindow(uint32 index) override;
	void GoFullscreen(UINT width, UINT height, UINT refreshRateNumerator, UINT refreshRateDenominator, UINT outputIndex, UINT sampleDescCount, UINT sampleDescQuality, bool srgb, bool vSync, uint32 index) override;
	void GoWindowed(INT left, INT top, UINT width, UINT height, UINT sampleDescCount, UINT sampleDescQuality, bool srgb, bool vSync, uint32 index) override;
	DescriptorHeapManager* GetDescriptorHeapManager() const { return _rtvHeapManager; }

	void OnRenderWindowDestroyed(D3D12RenderWindow* rw);

protected:
	DescriptorHeapManager *_rtvHeapManager;
	D3D12RenderWindow *_rw[MAX_RENDER_WINDOWS];

	Set<D3D12RenderWindow*> _renderWindows;

	static bool _createSwapChain(D3D12RenderWindow *rw, ID3D12CommandQueue *cmdQueue, DXGI_FORMAT format);

};


}