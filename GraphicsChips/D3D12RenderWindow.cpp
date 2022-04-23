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
#include "D3D12RenderWindow.h"
#include "M3DEngine/Window.h"
#include "TextWriter.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/Application.h"
#include "ResourceStateTracker.h"
#include "D3D12Formats.h"
#include <DirectXTK12/ScreenGrab.h>
#include <wincodec.h>
#include "M3DEngine/ClassManager.h"
#include "D3D12RenderWindowManager.h"

using namespace m3d;



D3D12RenderWindow::D3D12RenderWindow(Window *wnd, D3D12RenderWindowManager *manager) : RenderWindow(wnd), _manager(manager)
{
}

D3D12RenderWindow::~D3D12RenderWindow()
{
	if (_manager)
		_manager->OnRenderWindowDestroyed(this);

	ReleaseSwapChain();
}

HRESULT D3D12RenderWindow::CreateSwapChain(IDXGIFactory4 *dxgiFactory, ID3D12CommandQueue *cmdQueue, DXGI_FORMAT format, UINT bufferCount, BOOL allowTearing)
{
	HRESULT hr;

	SID3D12Device device;
	cmdQueue->GetDevice(IID_PPV_ARGS(&device));

	_cmdQueue = cmdQueue;
	_bufferCount = bufferCount;
	_allowTearing = allowTearing;

	// Nice stuff:
	// https://developer.nvidia.com/dx12-dos-and-donts
	// https://docs.microsoft.com/en-gb/windows/desktop/direct3ddxgi/dxgi-1-4-improvements
	// https://bell0bytes.eu/the-swap-chain/
	// https://blogs.msdn.microsoft.com/directx/2018/04/09/dxgi-flip-model/

	if (_wnd->GetCoreWindow()) {
		/*assert(_wnd->GetWindowHandle() == NULL);
		//SIDXGIFactory4 DXGIFactory4;
		//DXGIFactory->QueryInterface<IDXGIFactory4>(&DXGIFactory4);
		assert(DXGIFactory4.get() != nullptr);

		RECT clientRect = _wnd->GetClientRect();

		// Update current windowed size!
		UINT windowedWidth = clientRect.right - clientRect.left;
		UINT windowedHeight = clientRect.bottom - clientRect.top;

		DXGI_SWAP_CHAIN_DESC1 scDesc;
		scDesc.Width = windowedWidth;
		scDesc.Height = windowedHeight;
		scDesc.Format = format;
		scDesc.SampleDesc = sampleDesc;
		scDesc.SampleDesc.Count = 1;
		scDesc.SampleDesc.Quality = 0;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.BufferCount = 1; // On phone, only single buffering is supported.
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // On phone, no swap effects are supported.
		scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		scDesc.Flags = 0;
		scDesc.Scaling = DXGI_SCALING_STRETCH; // On phone, only stretch and aspect-ratio stretch scaling are allowed.
		scDesc.Stereo = FALSE;

		SIDXGISwapChain1 sc;
		V_RETURN(DXGIFactory4->CreateSwapChainForCoreWindow(DXGIDevice, _wnd->GetCoreWindow(), &scDesc, nullptr, &sc));
		_swapChain = sc;*/
	}
	else {
		assert(_wnd->GetWindowHandle() != NULL);

		UINT flags = 0;
		if (_allowTearing)
			flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		DXGI_SWAP_CHAIN_DESC1 scDesc = { };
		scDesc.Width = 0;
		scDesc.Height = 0;
		scDesc.Format = format;
		scDesc.BufferCount = bufferCount;
		scDesc.SampleDesc = { 1, 0 };
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		scDesc.Flags = flags;//DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		scDesc.Scaling = DXGI_SCALING_NONE;//DXGI_SCALING_STRETCH;
		scDesc.Stereo = FALSE;
		scDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	
		SIDXGISwapChain1 sc;
		V_RETURN(dxgiFactory->CreateSwapChainForHwnd(cmdQueue, _wnd->GetWindowHandle(), &scDesc, nullptr, nullptr, &sc));

		V_RETURN(sc->QueryInterface(IID_PPV_ARGS(&_swapChain)));
	}
	
	return _createRenderTargetView();
}

void D3D12RenderWindow::ReleaseSwapChain()
{
	HRESULT hr;
	BOOL isFullscreen = false;

	if (!_swapChain)
		return;

	V(_swapChain->GetFullscreenState(&isFullscreen, nullptr));
	if (isFullscreen) {
		V(_swapChain->SetFullscreenState(FALSE, nullptr)); // Got to leave fullscreen before releasing swap chain!
	}

	if (_currentRenderWindowIndex != -1)
		if (ClassManager* cm = engine->GetClassManager())
			cm->OnReleasingBackBuffer(this, _currentRenderWindowIndex);

	_cmdQueue = nullptr;
	_swapChain = nullptr;
	_renderTargets = nullptr;
	_srgbRenderTargets = nullptr;

	if (Graphics* graphics = (Graphics*)engine->GetGraphics()) {
		graphics->Flush();
		graphics->Sync();
	}
}

HRESULT D3D12RenderWindow::SetFullscreen(DXGI_MODE_DESC mode, IDXGIOutput *output)
{
	HRESULT hr;
	BOOL isFullscreen;
	SIDXGIOutput currentOutput;

	V_RETURN(_swapChain->GetFullscreenState(&isFullscreen, &currentOutput));

	if (isFullscreen) {
		if (output == nullptr || output == currentOutput)
			return Resize(mode);
	}
	else {
		RECT wRect = _wnd->GetClientRect();
		//		if (GetClientRect(_hWnd, &wRect)) {
		_windowedWidth = wRect.right - wRect.left;
		_windowedHeight = wRect.bottom - wRect.top;
		//		}
	}

	SIDXGIOutput o = output;

	if (output == nullptr) {
		V_RETURN(_swapChain->GetContainingOutput(&o));
	}

	SID3D12Device device;
	_cmdQueue->GetDevice(IID_PPV_ARGS(&device));

	DXGI_MODE_DESC mdesc;
	V_RETURN(output->FindClosestMatchingMode(&mode, &mdesc, device)); // Do this do ensure valid settings!

	DXGI_SWAP_CHAIN_DESC desc;
	V_RETURN(_swapChain->GetDesc(&desc));

	if (std::memcmp(&mdesc, &desc.BufferDesc, sizeof(DXGI_SWAP_CHAIN_DESC)) != 0) {
		V_RETURN(_swapChain->ResizeTarget(&mdesc)); // Call resize target only if settings changed!
	}
	V_RETURN(_swapChain->SetFullscreenState(TRUE, o));
	//	mdesc.RefreshRate.Numerator = 0;
	//	mdesc.RefreshRate.Denominator = 0;
	//	V_RETURN(_swapChain->ResizeTarget(&mdesc)); // Do this because dx-doc recommend it...
	
	return S_OK;
}
/*
HRESULT RenderWindow::SetWindowed(int32 left, int32 top, uint32 width, uint32 height)
{
HRESULT hr;
BOOL isFullscreen;
SIDXGIOutput currentOutput;

V_RETURN(_swapChain->GetFullscreenState(&isFullscreen, &currentOutput));

if (isFullscreen) {
V_RETURN(_swapChain->SetFullscreenState(FALSE, nullptr));
}

//	RECT wRect, cRect;
//	GetWindowRect(_hWnd, &wRect);
//	GetWindowRect(_hWnd, &cRect);

SetWindowPos(_hWnd, HWND_TOPMOST, left, top, width, height, SWP_NOZORDER);

return S_OK;
}*/

HRESULT D3D12RenderWindow::SetWindowed(bool restoreWindowedSize)
{
	HRESULT hr;
	BOOL isFullscreen;

	V_RETURN(_swapChain->GetFullscreenState(&isFullscreen, nullptr));

	if (!isFullscreen)
		return S_OK;

	V_RETURN(_swapChain->SetFullscreenState(FALSE, nullptr));

	if (restoreWindowedSize) {
		DXGI_MODE_DESC mode;
		mode.Format = GetBackBuffer()->GetDesc().Format;
		mode.Width = _windowedWidth;
		mode.Height = _windowedHeight;
		mode.RefreshRate.Numerator = 0;
		mode.RefreshRate.Denominator = 0;
		mode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		mode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		V_RETURN(Resize(mode));
	}
	
	return S_OK;
}

HRESULT D3D12RenderWindow::Resize(DXGI_MODE_DESC mode)
{
	HRESULT hr;
	BOOL isFullscreen;
	SIDXGIOutput output;

	V_RETURN(_swapChain->GetFullscreenState(&isFullscreen, &output));

	DXGI_SWAP_CHAIN_DESC desc;
	V_RETURN(_swapChain->GetDesc(&desc));
	V_RETURN(_swapChain->GetContainingOutput(&output));

	SID3D12Device device;
	V_RETURN(_cmdQueue->GetDevice(IID_PPV_ARGS(&device)));

	if (isFullscreen) {
		DXGI_MODE_DESC mdesc;
		V_RETURN(output->FindClosestMatchingMode(&mode, &mdesc, device)); // Do this do ensure valid settings!
		if (std::memcmp(&mdesc, &desc.BufferDesc, sizeof(DXGI_SWAP_CHAIN_DESC)) != 0) {
			V_RETURN(_swapChain->ResizeTarget(&mdesc)); // Call resize target only if settings changed!

		}
	}
	else {
		mode.RefreshRate.Numerator = 0;
		mode.RefreshRate.Denominator = 0;
		mode.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		mode.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		if (mode.Width != desc.BufferDesc.Width || mode.Height != desc.BufferDesc.Height || mode.Format != desc.BufferDesc.Format) {
			V_RETURN(_swapChain->ResizeTarget(&mode));
		}
	}
	return S_OK;
}

void D3D12RenderWindow::ReleaseBB()
{
	_renderTargets = nullptr;
	_srgbRenderTargets = nullptr;
}

HRESULT D3D12RenderWindow::OnResizeWindow()
{
	HRESULT hr;
	
	if (!_swapChain)
		return S_OK; // nothing to do!

	if (_currentRenderWindowIndex != -1)
		if (ClassManager* cm = engine->GetClassManager())
			cm->OnReleasingBackBuffer(this, _currentRenderWindowIndex);

	// Note: We need to be sure that references to the back buffer is released! That is, released after gpu finished using them!

	_renderTargets = nullptr;
	_srgbRenderTargets = nullptr;

	if (Graphics* graphics = (Graphics*)engine->GetGraphics()) {
		graphics->Flush();
		graphics->Sync();
	}

	DXGI_SWAP_CHAIN_DESC desc;
	V_RETURN(_swapChain->GetDesc(&desc));

	List<UINT> nodeMasks((size_t)_bufferCount, (UINT)0);
	List<IUnknown*> presentQueues((size_t)_bufferCount, (IUnknown*)_cmdQueue);

	V_RETURN(_swapChain->ResizeBuffers1(0, 0, 0, DXGI_FORMAT_UNKNOWN , desc.Flags, &nodeMasks.front(), &presentQueues.front()));

	return _createRenderTargetView();
}

ID3D12Resource *D3D12RenderWindow::GetBackBuffer() const
{
	return GetDescriptor().resource;
}

const Descriptor &D3D12RenderWindow::GetDescriptor(bool srgb) const
{
	if (srgb)
		if (_srgbRenderTargets)
			return _srgbRenderTargets->GetDescriptor(GetCurrentBackBufferIndex());
	return _renderTargets->GetDescriptor(GetCurrentBackBufferIndex());
}

UINT D3D12RenderWindow::GetCurrentBackBufferIndex() const
{
	return _swapChain->GetCurrentBackBufferIndex();
}

UINT D3D12RenderWindow::Present(bool testOnly, bool forceVSync, bool skipOverlay)
{
	if (!_swapChain)
		return PRESENT_NO_SWAP_CHAIN;

	if (engine->IsBreakMode())
		return S_OK; // If we are in break point, we can not swap!

	UINT syncInterval = _vSync || forceVSync ? 1 : 0;

	UINT flags = 0;
	if (testOnly)
		flags = DXGI_PRESENT_TEST;
	else {
		// Do not block if there is a queue. Just discard the frame and continue with the next!
		flags = DXGI_PRESENT_DO_NOT_WAIT;
		if (_allowTearing && syncInterval == 0)
			flags |= DXGI_PRESENT_ALLOW_TEARING;
	}

	_presentResult = _swapChain->Present(syncInterval, flags);

	UINT result = 0;

	switch (_presentResult)
	{
		// All good!
	case S_OK:
		// There is a queue. the frame is discarded, continue happily!
	case DXGI_ERROR_WAS_STILL_DRAWING: result = PRESENT_OK; break;
		// The device failed due to a badly formed command. This is a run-time issue; The application should destroy and recreate the device.
	case DXGI_ERROR_DEVICE_RESET: 
		result = PRESENT_DEVICE_RESET;
		// The video card has been physically removed from the system, or a driver upgrade for the video card has occurred. 
		// The application should destroy and recreate the device. For help debugging the problem, call ID3D11Device::GetDeviceRemovedReason.
	//case D3DDDIERR_DEVICEREMOVED:
	case DXGI_ERROR_DEVICE_REMOVED: 
	{
		SID3D12Device device;
		if (_cmdQueue->GetDevice(IID_PPV_ARGS(&device)) == S_OK) {
			HRESULT deviceRemovedReason = device->GetDeviceRemovedReason();
			String txt;
			switch (deviceRemovedReason)
			{
			case DXGI_ERROR_DEVICE_HUNG: txt = MTEXT("The D3D12-device reported a problem: Device hung."); break;
			case DXGI_ERROR_DEVICE_REMOVED: txt = MTEXT("The D3D12-device reported a problem: Device was physically removed or driver upgrade occured."); break;
			case DXGI_ERROR_DEVICE_RESET: txt = MTEXT("The D3D12-device reported a problem: Device was reset due to badly formed commands."); break;
			case DXGI_ERROR_DRIVER_INTERNAL_ERROR: txt = MTEXT("The D3D12-device reported a problem: Device internal error."); break;
			case DXGI_ERROR_INVALID_CALL: txt = MTEXT("The D3D12-device reported a problem: Device encountered an invalid call."); break;
			default: txt = MTEXT("The D3D12-device reported a problem: No reason identified."); break;
			}
			msg(WARN, txt);
		}
		if (result == 0)
			result = PRESENT_DEVICE_REMOVED; break;
	}
		// The window content is not visible. When receiving this status, an application can stop rendering and use DXGI_PRESENT_TEST to determine when to resume rendering.
	case DXGI_STATUS_OCCLUDED: result = PRESENT_OCCLUDED; break;
		//	case D3DDDIERR_DEVICEREMOVED:
	default:
		result = PRESENT_ERROR; break;
	}

	return result;
}

void D3D12RenderWindow::GetWindowedSize(UINT &width, UINT &height)
{
	if (_swapChain) {
		BOOL isFullscreen = FALSE;
		_swapChain->GetFullscreenState(&isFullscreen, nullptr);
		if (isFullscreen) {
			width = _windowedWidth;
			height = _windowedHeight;
		}
	}
	RECT wRect = _wnd->GetClientRect();
	//	GetClientRect(_hWnd, &wRect);
	width = wRect.right - wRect.left;
	height = wRect.bottom - wRect.top;
}


HRESULT D3D12RenderWindow::_createRenderTargetView()
{
	HRESULT hr;

	SID3D12Device device;
	V_RETURN(_cmdQueue->GetDevice(IID_PPV_ARGS(&device)));

	DescriptorHeapManager *rtvHeapManager = _manager->GetDescriptorHeapManager();

	{
		rtvHeapManager->GetDescriptorTable(_bufferCount, &_renderTargets);

		// Create a RTV for each frame.
		for (UINT n = 0; n < _bufferCount; n++)
		{
			Descriptor &d = _renderTargets->InitDescriptor(n, DescriptorType::RTV, FALSE);
			V_RETURN(_swapChain->GetBuffer(n, IID_PPV_ARGS(&d.resource)));
			// From d3d doc: Swap chain back buffers automatically start out in the D3D12_RESOURCE_STATE_COMMON state.
			V_RETURN(CreateResourceStateTracker(d.resource, D3D12_RESOURCE_STATE_COMMON, nullptr));
		}
	
		rtvHeapManager->CreateDescriptors(_renderTargets);
	}

	// SRGB descriptors
	{
		rtvHeapManager->GetDescriptorTable(_bufferCount, &_srgbRenderTargets);

		// Create a RTV for each frame.
		for (UINT n = 0; n < _bufferCount; n++)
		{
			Descriptor &d = _srgbRenderTargets->InitDescriptor(n, DescriptorType::RTV, TRUE);
			V_RETURN(_swapChain->GetBuffer(n, IID_PPV_ARGS(&d.resource)));
			const auto desc = d.resource->GetDesc();
			d.rtv->ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			d.rtv->Format = dxgiformat::MakeSRGB(desc.Format);
			d.rtv->Texture2D.MipSlice = 0;
			d.rtv->Texture2D.PlaneSlice = 0;
		}

		rtvHeapManager->CreateDescriptors(_srgbRenderTargets);
	}

	return S_OK;
}

void D3D12RenderWindow::SetCurrentRenderWindowIndex(UINT idx)
{
	if (_currentRenderWindowIndex == idx)
		return;
	if (_currentRenderWindowIndex != -1)
		if (ClassManager* cm = engine->GetClassManager())
			cm->OnReleasingBackBuffer(this, _currentRenderWindowIndex);
	_currentRenderWindowIndex = idx;
}

bool D3D12RenderWindow::RequestScreenshot(const Char *Filepath)
{
	String ext = Path(Filepath).GetFileExtention();
	HRESULT hr = E_FAIL;
	ID3D12Resource *res = GetBackBuffer();
	if (!res)
		return false;
	wchar_t wpath[MAX_PATH];
	if (strUtils::compareNoCase(ext, String(MTEXT("dds"))) == 0) {
		hr = SaveDDSTextureToFile(_cmdQueue, res,
			strUtils::widen(wpath, MAX_PATH, Filepath), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_PRESENT);
	}
	else {
		GUID guid = {};
		if (strUtils::compareNoCase(ext, String(MTEXT("jpg"))) == 0)
			guid = GUID_ContainerFormatJpeg;
		else if (strUtils::compareNoCase(ext, String(MTEXT("png"))) == 0)
			guid = GUID_ContainerFormatPng;
		else if (strUtils::compareNoCase(ext, String(MTEXT("bmp"))) == 0)
			guid = GUID_ContainerFormatBmp;
		else if (strUtils::compareNoCase(ext, String(MTEXT("tiff"))) == 0)
			guid = GUID_ContainerFormatTiff;
		else
			return false;
		hr = SaveWICTextureToFile(_cmdQueue, res,
			guid, strUtils::widen(wpath, MAX_PATH, Filepath),
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_PRESENT);//, &GUID_WICPixelFormat32bppBGRA);
	}
	return SUCCEEDED(hr);
}

