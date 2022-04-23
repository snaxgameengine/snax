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

#include <qwidget.h>
#include "StdChips/Envelope.h"

namespace m3d
{

class EnvelopeGraph : public QWidget
{
	Q_OBJECT
public:
	EnvelopeGraph(QWidget *parent);
	~EnvelopeGraph();

	void SetSpline(const ValueTCBSpline *spline) { _spline = spline; }
	const ValueTCBSpline *GetSpline() { return _spline; }
	uint32 GetCurrentControlPoint() const { return _selected; }

public slots:
	void enableZoomX(bool b) { _zoomX = b; }
	void enableZoomY(bool b) { _zoomY = b; }
	void enableEditX(bool b) { _editX = b; }
	void enableEditY(bool b) { _editY = b; }
	void prevPoint();
	void nextPoint();
	void setCurrentControlPoint(uint32 i);
	void viewAll();
	void restoreView();

signals:
	void selectionChanged(uint32);
	void addControlPoint(value x, value y);
	void moveControlPoint(value dx, value dy);
	void deleteControlPoint();

protected:
	void paintEvent(QPaintEvent *event);
	void wheelEvent(QWheelEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event);

	float32 _getSpace(float32 d, float32 w);

	const ValueTCBSpline *_spline;

	float32 _xMin;
	float32 _xMax;
	float32 _yMin;
	float32 _yMax;

	float32 _mouseX;
	float32 _mouseY;
	bool _drag;
	uint32 _selected;

	bool _zoomX;
	bool _zoomY;
	bool _editX;
	bool _editY;
	bool _showDerivative;

};

}