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
#include "D3D12RenderWindowManager.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/Window.h"
#include "M3DEngine/ClassManager.h"
#include "Graphics.h"

using namespace m3d;



D3D12RenderWindowManager::D3D12RenderWindowManager(DescriptorHeapManager *rtvHeapManager) : _rtvHeapManager(rtvHeapManager)
{
	memset(_rw, 0, sizeof(_rw));
}

D3D12RenderWindowManager::~D3D12RenderWindowManager()
{
	for (const auto &n : _renderWindows)
		n->OnManagerDestroyed();
}

RenderWindow *D3D12RenderWindowManager::CreateRenderWindow(Window *wnd) 
{
	D3D12RenderWindow *rw = mmnew D3D12RenderWindow(wnd, this);
	_renderWindows.insert(rw);
	return rw;
}

void D3D12RenderWindowManager::OnRenderWindowDestroyed(D3D12RenderWindow* rw)
{
	for (uint32 i = 0; i < MAX_RENDER_WINDOWS; i++) {
		if (_rw[i] == rw) {
			_rw[i] = nullptr; // Note: Back buffer and resources is release by render window.
		}
	}
	_renderWindows.erase(rw);
}

bool D3D12RenderWindowManager::SetRenderWindow(RenderWindow *renderwindow, uint32 index)
{
	if (_rw[index] == renderwindow)
		return true; // No change!

	D3D12RenderWindow *rw = dynamic_cast<D3D12RenderWindow*>(renderwindow);

	if (!rw)
		return false; // Window type not supported
	
	if (rw) {
		for (uint32 i = 0; i < MAX_RENDER_WINDOWS; i++) {
			if (_rw[i] == rw) {
				msg(WARN, MTEXT("Can't set same render window more than once."));
				return false; // Can't assign same render window to more than one index.
			}
		}
		if (_rw[index]) { // Old window? (eg switching in the editor)
			if (rw && _rw[index]->GetBackBuffer()) { // Ensure same settings!
				D3D12_RESOURCE_DESC oldDesc = _rw[index]->GetBackBuffer()->GetDesc();
				D3D12_RESOURCE_DESC newDesc = rw->GetSwapChain() && rw->GetBackBuffer() ? rw->GetBackBuffer()->GetDesc() : oldDesc;
				if (!rw->GetSwapChain() || newDesc.Format != oldDesc.Format) {
					if (rw->GetSwapChain())
						rw->ReleaseSwapChain();
					_createSwapChain(rw, ((Graphics*)engine->GetGraphics())->GetCommandQueue(), oldDesc.Format); // If this fails, we may lose settings.
				}
			}

			_rw[index]->SetCurrentRenderWindowIndex(-1);
		}
	}

	_rw[index] = rw;
	if (_rw[index])
		_rw[index]->SetCurrentRenderWindowIndex(index);

	return true;
}

RenderWindow *D3D12RenderWindowManager::GetRenderWindow(uint32 index)
{
	D3D12RenderWindow *rw = _rw[index];

	if (rw == nullptr) {
		msg(WARN, strUtils::ConstructString(MTEXT("No render window at index %1.")).arg(index));
		return nullptr; // No window set!
	}

	try {
		ID3D12Device *device = ((Graphics*)engine->GetGraphics())->GetDevice();
	}
	catch (const GraphicsException &e) {
		msg(e.severity, e.msg, e.chip);
		return nullptr;
	}

	if (!rw->GetSwapChain()) { // No back buffer yet? Create a default one!
		ID3D12CommandQueue *cmdQueue = ((Graphics*)engine->GetGraphics())->GetCommandQueue();

		msg(INFO, strUtils::ConstructString(MTEXT("Creating a default back buffer DXGI_FORMAT_R8G8B8A8_UNORM (1, 0). for render window %1.")).arg(index));

		if (!_createSwapChain(rw, cmdQueue, DXGI_FORMAT_R8G8B8A8_UNORM)) { // Create default swap chain!
			msg(INFO, strUtils::ConstructString(MTEXT("Ouups! Failed to create default back buffer for render window %1.")).arg(index));
			return nullptr; // Failed to create swap chain!
		}
	}

	rw->GetWindow()->EnsureVisible();
	
	return rw;
}

Window *D3D12RenderWindowManager::GetWindow(uint32 index)
{
	D3D12RenderWindow *rw = _rw[index];

	if (rw == nullptr) {
		msg(WARN, strUtils::ConstructString(MTEXT("No window at index: %1.")).arg(index));
		return nullptr; // No window set!
	}

	return rw->GetWindow();
}

void D3D12RenderWindowManager::GoFullscreen(UINT width, UINT height, UINT refreshRateNumerator, UINT refreshRateDenominator, UINT outputIndex, UINT sampleDescCount, UINT sampleDescQuality, bool srgb, bool vSync, uint32 index)
{
	HRESULT hr;

	assert(sampleDescCount == 1 && sampleDescQuality == 0 && srgb == false);

	if (_rw[index] == nullptr) {
		msg(WARN, strUtils::format(MTEXT("Render window %u does not exist."), index));
		return; // No such render window.
	}

	if (!_rw[index]->GetWindow()->CanGoFullscreen()) {
		msg(WARN, strUtils::format(MTEXT("Can't go fullscreen with render window %u."), index));
		return GoWindowed(INT_MIN, INT_MIN, width, height, sampleDescCount, sampleDescQuality, srgb, vSync, index); // Not a top level window.
	}

	const OutputList &ol = ((Graphics*)engine->GetGraphics())->GetAdaptersAndOutputs()[((Graphics*)engine->GetGraphics())->GetAdapterIndex()].second;
	if (ol.size() == 0) {
		msg(WARN, MTEXT("Full screen mode is not available on this adapter."));
		return;
	}
	if (outputIndex >= ol.size()) {// Ensure valid output index!
		msg(WARN, strUtils::format(MTEXT("Invalid output index (%u) given for render window %u. Using primary (0)."), outputIndex, index));
		outputIndex = 0;
	}

	IDXGIOutput *output = ol[outputIndex]; // This represents the screen

	if (width == 0 || height == 0) { // Width and height not set? Use screen size (current resolution!)
		DXGI_OUTPUT_DESC oDesc;
		hr = output->GetDesc(&oDesc);
		width = oDesc.DesktopCoordinates.right - oDesc.DesktopCoordinates.left;
		height = oDesc.DesktopCoordinates.bottom - oDesc.DesktopCoordinates.top;
	}

	if (refreshRateDenominator == 0 && refreshRateNumerator == 0) { // Refresh rate not set? Try a default setting!
		refreshRateNumerator = 60;
		refreshRateDenominator = 1;
	}

	DXGI_MODE_DESC mDesc;

	mDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	mDesc.Width = width;
	mDesc.Height = height;
	mDesc.RefreshRate.Numerator = refreshRateNumerator;
	mDesc.RefreshRate.Denominator = refreshRateDenominator;
	mDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	mDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	ID3D12Device *device = ((Graphics*)engine->GetGraphics())->GetDevice();

	if (_rw[index]->GetSwapChain()) { // Do we already have a swap chain (with different settings)?
		D3D12_RESOURCE_DESC bbDesc = _rw[index]->GetBackBuffer()->GetDesc();
		if (bbDesc.Format != mDesc.Format)
			_rw[index]->ReleaseSwapChain(); // Release old swap chain!
	}

	if (!_rw[index]->GetSwapChain() && !_createSwapChain(_rw[index], ((Graphics*)engine->GetGraphics())->GetCommandQueue(), mDesc.Format))
		return; // (msg already set)

	hr = _rw[index]->SetFullscreen(mDesc, output);
	if (FAILED(hr))
		msg(FATAL, String(MTEXT("Failed to go fullscreen.")));
}

void D3D12RenderWindowManager::GoWindowed(INT left, INT top, UINT width, UINT height, UINT sampleDescCount, UINT sampleDescQuality, bool srgb, bool vSync, uint32 index)
{
	HRESULT hr;

	assert(sampleDescCount == 1 && sampleDescQuality == 0 && srgb == false);

	if (_rw[index] == nullptr) {
		msg(WARN, strUtils::format(MTEXT("Render window %u does not exist."), index));
		return; // No such render window!
	}

	if (!_rw[index]->GetWindow()->CanResize()) {
		if (width != 0 && height != 0) {
			msg(WARN, strUtils::format(MTEXT("Can't resize render window %u."), index));
			width = height = 0;
		}
	}
	if (!_rw[index]->GetWindow()->CanMove()) {
		if (left != INT_MIN && top != INT_MIN) {
			msg(WARN, strUtils::format(MTEXT("Can't move render window %u."), index));
			left = top = INT_MIN;
		}
	}

	if (width == 0 || height == 0) // Size not set?
		_rw[index]->GetWindowedSize(width, height); // If fullscreen, this is the size of the window before going fullscreen.

	DXGI_MODE_DESC mDesc;


	mDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	mDesc.Width = width;
	mDesc.Height = height;
	mDesc.RefreshRate.Numerator = 0;
	mDesc.RefreshRate.Denominator = 0;
	mDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	mDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	ID3D12Device *device = ((Graphics*)engine->GetGraphics())->GetDevice();

	if (_rw[index]->GetSwapChain()) { // Do we already have a swap chain (with different settings)?
		D3D12_RESOURCE_DESC bbDesc = _rw[index]->GetBackBuffer()->GetDesc();
		if (bbDesc.Format != mDesc.Format)
			_rw[index]->ReleaseSwapChain(); // Release old swap chain!
	}

	if (_rw[index]->GetSwapChain()) { // Do we still have a swap chain?
		BOOL isFullscreen = FALSE;
		hr = _rw[index]->GetSwapChain()->GetFullscreenState(&isFullscreen, nullptr);
		if (isFullscreen) { // Is swap chain in fullscreen mode?
			hr = _rw[index]->SetWindowed(false);
			if (FAILED(hr)) {
				msg(WARN, strUtils::format(MTEXT("Failed to leave full screen for render window %u."), index));
			}
		}
	}

	if (!_rw[index]->GetSwapChain() && !_createSwapChain(_rw[index], ((Graphics*)engine->GetGraphics())->GetCommandQueue(), mDesc.Format)) // Create swap chain
		return; // (msg already set)

	// Does current swap chain match new settings?
	D3D12_RESOURCE_DESC bbDesc = _rw[index]->GetBackBuffer()->GetDesc();
	if (bbDesc.Format != mDesc.Format || bbDesc.Width != mDesc.Width || bbDesc.Height != mDesc.Height) {
		hr = _rw[index]->Resize(mDesc); // Change swap chain settings!
		if (FAILED(hr)) {
			msg(WARN, strUtils::format(MTEXT("Failed to set new size or format for render window %u."), index));
		}
	}

	if (left != INT_MIN && top != INT_MIN) // Should window be moved?
		_rw[index]->GetWindow()->SetWindowPos(left, top);
}

bool D3D12RenderWindowManager::_createSwapChain(D3D12RenderWindow *rw, ID3D12CommandQueue *cmdQueue, DXGI_FORMAT format)
{
	Graphics *g = (Graphics*)engine->GetGraphics();
	UINT bufferCount = 3;
	HRESULT hr = rw->CreateSwapChain(g->GetDXGIFactory(), cmdQueue, format, bufferCount, g->GetAllowTearing());
	if (FAILED(hr)) {
		msg(FATAL, String(MTEXT("Failed to create Direct3D swap chain.")));
		return false;
	}
	return true;
}
