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
#include <qwidget.h>
#include <qtimer.h>
#include "GraphicsChips/D3D12Include.h"
#include "M3DEngine/Window.h"

namespace m3d
{

class D3D12RenderWindow;
class D3DWidget;

class GRAPHICSCHIPS_DLG_EXPORT D3DWindow : public Window
{
	D3DWidget *_widget;
public:
	D3DWindow(D3DWidget *widget) : _widget(widget) {}
	~D3DWindow() {}

	HWND GetWindowHandle() override;

	bool CanGoFullscreen() const override { return false; }
	bool CanMove() const override { return false; }
	bool CanResize() const override { return false; }
};

class GRAPHICSCHIPS_DLG_EXPORT D3DWidget : public QWidget
{
	Q_OBJECT
protected:
//	QTimer *_frameTimer;
	D3DWindow _wnd;
	D3D12RenderWindow *_rw;

	bool _isD3DInit;

	int _x, _y;

	DXGI_FORMAT _fmt;
	DXGI_SAMPLE_DESC _sampleDesc;


public:
	D3DWidget(QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
	~D3DWidget();
	
	QPaintEngine *paintEngine() const override { return 0; }

	virtual D3D12RenderWindow *GetRenderWindow() { return _rw; }

	virtual void SetFormat(DXGI_FORMAT fmt) { if (fmt != _fmt) {_fmt = fmt; DestroyDeviceObject(); } } 

protected:
	bool nativeEvent(const QByteArray &eventType, void *msg, qintptr *result) override;
	//virtual bool winEvent(MSG *message, long *result);

	void wheelEvent(QWheelEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;

	virtual HRESULT initD3D();
	virtual HRESULT newFrame();
	virtual HRESULT resizeDX();


public:
	virtual void DestroyDeviceObject();

//private:
	void invalidateGraphics();

signals:
	void resize();
	void render();
	void zoom(int,int,int);
	void move(int,int);
};

}