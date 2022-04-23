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
#include "ProjectViewWidget.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/RenderWindow.h"
#include "M3DEngine/RenderWindowManager.h"
#include "M3DEngine/Application.h"
#include "EditorInputManager.h"
#include "GraphicsChips/Graphics.h"
#include "M3DEngineExt/EditorState.h"
#include "AppSignals.h"

using namespace m3d;

HWND DXWindow::GetWindowHandle() { return (HWND)_widget->winId(); }


ProjectViewWidget::ProjectViewWidget(QWidget * parent, Qt::WindowFlags f) : QWidget(parent, f), _window(this)
{
	connect(&AppSignals::instance(), &AppSignals::destroyDeviceObjects, this, &ProjectViewWidget::onDestroyDevice);
	connect(&AppSignals::instance(), &AppSignals::forceVSync, this, &ProjectViewWidget::SetForceVSync);
	connect(&AppSignals::instance(), &AppSignals::newFrame, this, &ProjectViewWidget::invalidateGraphics);


	setAttribute(Qt::WA_PaintOnScreen);
	setAttribute(Qt::WA_NativeWindow);
//    setAttribute(Qt::WA_NoSystemBackground);
//    setAutoFillBackground(true); // for compatibility

//	_frameTimer = new QTimer(this);
//	connect(_frameTimer, SIGNAL(timeout()), this, SLOT(invalidateGraphics()));
//	_frameTimer->start(0);

	setFocusPolicy(Qt::WheelFocus);

	_rw = nullptr;
	_testNext = false;
	_forceVSync = false;
	_resizeRequired = false;
}

ProjectViewWidget::~ProjectViewWidget()
{
	if (_rw) {
		mmdelete(_rw);
	}
}


#include <qpainter.h>
bool ProjectViewWidget::nativeEvent(const QByteArray &eventType, void *msg, qintptr*result)
{
	MSG *message = (MSG*)msg;
	if (message->message == WM_SIZE) {
		if (_rw) {
			if (EditorState::state == EditorState::BREAKPOINT || EditorState::state == EditorState::PAUSE) {
				_resizeRequired = true;
			}
			else {
				if (FAILED(_rw->OnResizeWindow())) {
					// Error msg?
				}
			}
		}
		InvalidateRect(message->hwnd, 0, true);
	}
	else if (message->message == WM_PAINT) {

		if (EditorState::state == EditorState::BREAKPOINT || EditorState::state == EditorState::PAUSE) {
			//if (engine->GetGraphics()->GetRenderWindowManager()->GetRenderWindow() == _rw) { // It's not our windows currently being rendered to?
				// Just clear the window using plain old GDI!
			/*	
			DOES NOT WORK!?!?!
			HWND hWnd = (HWND)winId();
				HDC hdc = GetDC(hWnd);
				RECT rect;
				//SelectObject(hdc, GetStockObject(GRAY_BRUSH));
				GetClientRect(hWnd, &rect);
				//Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
				//RECT rect;
				//GetClientRect(hWnd, &rect);
				HBRUSH hbrRed = CreateSolidBrush(RGB(0, 255, 0));
				FillRect(hdc, &rect, hbrRed);
				ReleaseDC(hWnd, hdc);
				*/
			//	return false;
		//	}

			//UINT r = _rw->Present(false, _forceVSync, true);

			return false;

		}



		if (!_rw) {
			_rw = engine->GetGraphics()->CreateRenderWindow(&_window);
		}
		if (!_rw) {
			// Error msg?
			return false;
		}

		if (_resizeRequired) {
			_resizeRequired = false;
			if (FAILED(_rw->OnResizeWindow())) {
				// Error msg?
			}
		}


		static bool skipPaintMessage = false; // assert() does not work without this. New paint messages are being sent while the assert dialog is shown causing trouble!

		if (skipPaintMessage)
			return false;

		skipPaintMessage = true;

		if (_testNext) {
			UINT r = _rw->Present(true);
			_testNext = r == PRESENT_OCCLUDED;
		}
		else {	
			newFrame();
		}

		skipPaintMessage = false;
	}

	return false;
}


HRESULT ProjectViewWidget::newFrame()
{
	if (!engine->GetGraphics()->GetRenderWindowManager()->SetRenderWindow(_rw)) {
		msg(WARN, MTEXT("Failed to set render window."));
		return E_FAIL;
	}

	engine->Run();

	engine->GetGraphics()->GetRenderWindowManager()->GetRenderWindow(); // Do this to make sure back buffer is created in editor!

	UINT r = _rw->Present(false, _forceVSync);
	if (r == PRESENT_DEVICE_RESET || r == PRESENT_DEVICE_REMOVED) {
		msg(WARN, MTEXT("The D3D12 graphics device was removed. We will try to recreate the device, but it could happen that not all your graphics resources preserves their current state..."));
		engine->GetGraphics()->DestroyDevice();
	}

	_testNext = r == PRESENT_OCCLUDED;

	engine->GetGraphics()->ClearState(); // Move this somewhere else!

	return S_OK;
}

void ProjectViewWidget::invalidateGraphics()
{
	InvalidateRect((HWND)winId(), 0, false); // Triggers a WM_PAINT message
}

void ProjectViewWidget::onDestroyDevice()
{
	if (_rw)
		_rw->OnDestroyDevice();
}

void ProjectViewWidget::mouseDoubleClickEvent (QMouseEvent *event)
{
	((EditorInputManager*)engine->GetApplication()->GetInputManager())->mouseDoubleClickEvent(event);
}

void ProjectViewWidget::mousePressEvent (QMouseEvent *event)
{
	((EditorInputManager*)engine->GetApplication()->GetInputManager())->mousePressEvent(event);
}

void ProjectViewWidget::mouseReleaseEvent (QMouseEvent *event)
{
	((EditorInputManager*)engine->GetApplication()->GetInputManager())->mouseReleaseEvent(event);
}

void ProjectViewWidget::wheelEvent(QWheelEvent *event)
{
	if (event->angleDelta().y() != 0)
		((EditorInputManager*)engine->GetApplication()->GetInputManager())->wheelEvent(event);
}

void ProjectViewWidget::keyPressEvent(QKeyEvent *event)
{
	((EditorInputManager*)engine->GetApplication()->GetInputManager())->keyPressEvent(event);
}

void ProjectViewWidget::keyReleaseEvent(QKeyEvent *event)
{
	((EditorInputManager*)engine->GetApplication()->GetInputManager())->keyReleaseEvent(event);
}

bool ProjectViewWidget::TakeScreenshot(const Char *filename)
{
	if (_rw)
		return _rw->RequestScreenshot(filename);
	return false;
}

