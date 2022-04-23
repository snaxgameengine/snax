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

#include "StdAfx.h"
#include "D3DWidget.h"
#include "M3DEngine/Renderwindow.h"
#include "M3DEngine/Engine.h"
#include "GraphicsChips/Graphics.h"
#include "GraphicsChips/D3D12RenderWindow.h"
#include "GraphicsChips/ResourceStateTracker.h"
#include "GraphicsChips/RenderSettings.h"


using namespace m3d;


HWND D3DWindow::GetWindowHandle() { return (HWND)_widget->winId(); }

D3DWidget::D3DWidget(QWidget * parent, Qt::WindowFlags f) : QWidget(parent, f), _wnd(this)
{
	winId();
	setAttribute(Qt::WA_PaintOnScreen); // Qt::WA_NoSystemBackground

	//_frameTimer = new QTimer(this);
	//connect(_frameTimer, SIGNAL(timeout()), this, SLOT(invalidateGraphics()));
	//_frameTimer->start(1);

	_isD3DInit = false;

	_rw = nullptr;

	_fmt = DXGI_FORMAT_R8G8B8A8_UNORM;
	_sampleDesc.Count = 1;
	_sampleDesc.Quality = 0;
}

D3DWidget::~D3DWidget()
{
	if (_rw)
		mmdelete(_rw);
}

//bool D3DWidget::winEvent(MSG * message, long * result)
bool D3DWidget::nativeEvent(const QByteArray &eventType, void *msg, qintptr*result)
{
	MSG *message = (MSG*)msg;
	if (message->message == WM_SIZE) {
	//	if (_isD3DInit)
			resizeDX();
		InvalidateRect(message->hwnd, 0, true);
		//return true;
	}
	else if (message->message == WM_PAINT) {
		if (!_isD3DInit) {
			_isD3DInit = true;
			initD3D();
		}
		newFrame();
	}

	return false;
}

void D3DWidget::invalidateGraphics()
{
	InvalidateRect((HWND)winId(), 0, false); // Triggers a WM_PAINT message
}


HRESULT D3DWidget::initD3D()
{
	HRESULT hr;
	
	// Ensure device created!
	((Graphics*)engine->GetGraphics())->GetDevice();

	_rw = (D3D12RenderWindow*)engine->GetGraphics()->CreateRenderWindow(&_wnd);
	_rw->SetVSync(FALSE);
	//V_RETURN(_rw->Init(((D3DGraphics*)engine->GetGraphics())->GetDevice(), (HWND)winId(), true, false, sd));
//	_rw->SetSwapChainCreationSettings(sd, DXGI_FORMAT_R8G8B8A8_UNORM);
	Graphics *g = (Graphics*)engine->GetGraphics();
	V_RETURN(_rw->CreateSwapChain(g->GetDXGIFactory(), g->GetCommandQueue(), _fmt, 3, FALSE));

//	_hpt.Start();

	return S_OK;
}

void D3DWidget::DestroyDeviceObject()
{
	if (_rw)
		mmdelete(_rw);
	_rw = nullptr;
	_isD3DInit = false;
}

HRESULT D3DWidget::resizeDX()
{
	if (_isD3DInit) {
		_rw->ReleaseBB();
		((Graphics*)engine->GetGraphics())->Flush();
		((Graphics*)engine->GetGraphics())->Sync();
		_rw->OnResizeWindow();
	//	auto gg = ((Graphics*)engine->GetGraphics())->GetRTVHeapManager();
		//int gfhfg = 5;
	}
	emit resize();
	return S_OK;
}

HRESULT D3DWidget::newFrame()
{
	HRESULT hr;


	if (_rw) {
		SResourceStateTracker rst;
		GetResourceStateTracker(_rw->GetBackBuffer(), &rst);
		((Graphics*)engine->GetGraphics())->rs()->EnterState(rst, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	((Graphics*)engine->GetGraphics())->rs()->CommitResourceBarriers();
	emit render();

/*	ID3D11RenderTargetView *rtv = _rw->GetRenderTargetView();
	((D3DGraphics*)engine->GetGraphics())->GetImmediateContext()->OMSetRenderTargets(1, &rtv, 0);

	D3D11_VIEWPORT vp;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (FLOAT)_rw->GetBackBufferDesc().Width;
	vp.Height = (FLOAT)_rw->GetBackBufferDesc().Height;

	((D3DGraphics*)engine->GetGraphics())->GetImmediateContext()->RSSetViewports(1, &vp);

	_hpt.Tick();
	_fps.NewFrame(_hpt.getTime_us());

	V_RETURN(((D3DGraphics*)engine->GetGraphics())->GetTextWriter()->Write(((D3DGraphics*)engine->GetGraphics())->GetImmediateContext(), (float32*)D3DXVECTOR2(5.0f, 5.0f), TextWriter::LEFT_TOP, (float32*)D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f), String::format(MTEXT("DirectX11 - FPS: %i"), (UINT)_fps.GetFPS())));
	*/

	((Graphics*)engine->GetGraphics())->rs()->OMSetRenderTargets(0, nullptr, nullptr);

	if (_rw) {
		SResourceStateTracker rst;
		GetResourceStateTracker(_rw->GetBackBuffer(), &rst);
		((Graphics*)engine->GetGraphics())->rs()->EnterState(rst, D3D12_RESOURCE_STATE_PRESENT);
	}

	((Graphics*)engine->GetGraphics())->rs()->CommitResourceBarriers();
	((Graphics*)engine->GetGraphics())->Flush();
	
	V_RETURN(_rw->Present());


//	((D3DGraphics*)engine->GetGraphics())->GetImmediateContext()->ClearState();

	return S_OK;
}

void D3DWidget::wheelEvent(QWheelEvent *event)
{
	if (event->angleDelta().y() != 0)
		emit zoom(event->angleDelta().y(), event->position().x(), event->position().y());
} 

void D3DWidget::mousePressEvent(QMouseEvent *event)
{
	_x = event->x();
	_y = event->y();
}

void D3DWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons().testFlag(Qt::MouseButton::MiddleButton)) {
		emit move(event->x() - _x, event->y() - _y);
	}
	_x = event->x();
	_y = event->y();
}
