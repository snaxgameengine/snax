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
#include "M3DEngine/Window.h"
#include <qwidget.h>


namespace m3d
{

class RenderWindow;
class ProjectViewWidget;

class DXWindow : public Window
{
	ProjectViewWidget *_widget;
public:
	DXWindow(ProjectViewWidget *widget) : _widget(widget) {}
	virtual ~DXWindow() {}

	virtual HWND GetWindowHandle() override;

	virtual bool CanGoFullscreen() const override { return false; }
	virtual bool CanMove() const override { return false; }
	virtual bool CanResize() const override { return false; }
};


class ProjectViewWidget : public QWidget
{
	Q_OBJECT
public:
	ProjectViewWidget(QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
	~ProjectViewWidget();

	virtual QPaintEngine *paintEngine() const override { return nullptr; }

	virtual bool TakeScreenshot(const Char *filename);

protected:
	virtual bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

	virtual void mouseDoubleClickEvent(QMouseEvent *event) override;
	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void wheelEvent(QWheelEvent *event) override;
	virtual void keyPressEvent(QKeyEvent *event) override;
	virtual void keyReleaseEvent(QKeyEvent *event) override;


	HRESULT newFrame();

public:
	void onDestroyDevice();
	void SetForceVSync(bool b) { _forceVSync = b; }
	void invalidateGraphics();

private:
//	QTimer *_frameTimer;
	DXWindow _window;
	RenderWindow *_rw;

	bool _testNext;
	bool _forceVSync;
	bool _resizeRequired;

};

}