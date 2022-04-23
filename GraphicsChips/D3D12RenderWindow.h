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
#include "D3D12Include.h"
#include "M3DEngine/RenderWindow.h"
#include "M3DCore/HighPrecisionTimer.h"
#include "DescriptorHeapManager.h"


namespace m3d
{

class TextWriter;
class D3D12RenderWindowManager;

class GRAPHICSCHIPS_API D3D12RenderWindow : public RenderWindow
{
public:
	D3D12RenderWindow(Window *wnd, D3D12RenderWindowManager *manager);
	~D3D12RenderWindow();

	// Creates the swap chain and back buffer.
	virtual HRESULT CreateSwapChain(IDXGIFactory4 *dxgiFactory, ID3D12CommandQueue *cmdQueue, DXGI_FORMAT format, UINT bufferCount, BOOL allowTearing);
	// Releases the swap chain and back buffer. Callback will be notified!
	virtual void ReleaseSwapChain();
	// Sets the window to fullscreen.
	virtual HRESULT SetFullscreen(DXGI_MODE_DESC mode, IDXGIOutput *output = nullptr);
	// Sets the window to windowed. Has only effect if currently in fullscreen!
	virtual HRESULT SetWindowed(bool restoreWindowedSize);
	// Will resize the window/change format using the supplied mode.
	virtual HRESULT Resize(DXGI_MODE_DESC mode);
	// Presents.
	virtual UINT Present(bool testOnly = false, bool forceVSync = false, bool skipOverlay = false) override;
	// Called by window system on WM_SIZE. Callback will be notified if releasing back buffer!
	virtual HRESULT OnResizeWindow() override;
	virtual void ReleaseBB();
	// Called when graphics device is destroyed.
	virtual void OnDestroyDevice() override { return ReleaseSwapChain(); }
	// Gets the swap chain.
	virtual IDXGISwapChain *GetSwapChain() { return _swapChain; }
	// Gets the current back buffer.
	virtual ID3D12Resource *GetBackBuffer() const;
	// Returns current rtv descriptor.
	virtual const Descriptor &GetDescriptor(bool srgb = false) const;

	UINT GetCurrentBackBufferIndex() const;

	// Gets the back buffer descriptor.
	/*virtual const D3D11_TEXTURE2D_DESC &GetBackBufferDesc() const { return _bbDesc; }
	// Gets the render target view of the back buffer.
	virtual ID3D11RenderTargetView *GetRenderTargetView() { return _rtv; }
	// Returns the last present HRESULT. If DXGI_STATUS_OCCLUDED, you can skip rendering. Call Present(true) until it returns S_OK to resume rendering.*/
	virtual HRESULT GetLastPresentResult() const { return _presentResult; }
	// Gets the size of the windowed back buffer.
	virtual void GetWindowedSize(UINT &width, UINT &height);
	
	bool RequestScreenshot(const Char *Filepath) override;

	void OnManagerDestroyed() { _manager = nullptr; }
	void SetCurrentRenderWindowIndex(UINT idx);

protected:
	// This is the command queue for the swap chain.
	SID3D12CommandQueue _cmdQueue;
	// Swap chain.
	SIDXGISwapChain4 _swapChain;
	// Last swap chain present result.
	HRESULT _presentResult = S_OK;
	// RTV heap.
	D3D12RenderWindowManager* _manager = nullptr;
	SDescriptorTable _renderTargets;
	SDescriptorTable _srgbRenderTargets;
	// Number of buffers.
	UINT _bufferCount = 0;
	// true if waiting for vSync at present.
	BOOL _vSync = FALSE;
	// Variables set when going fullscreen.
	UINT _windowedWidth = 640;
	UINT _windowedHeight = 480;

	BOOL _allowTearing = FALSE;

	UINT _currentRenderWindowIndex = -1;
	
	HRESULT _createRenderTargetView();
};

}