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
#include "EnvelopeGraph.h"
#include <qpainterpath.h>

using namespace m3d;

const float32 xMargin = 50.0f;
const float32 yMargin = 20.0f;
const float32 pixelsPrLine = 100.0f;
const float32 MIN_ZOOM = 0.01f;
const float32 MAX_ZOOM = 100000.0f;


EnvelopeGraph::EnvelopeGraph(QWidget *parent) : QWidget(parent)
{
	_xMin = -10.0f;
	_xMax = 10.0f;
	_yMin = -10.0f;
	_yMax = 10.0f;

	_spline = nullptr;
	_drag = false;
	_selected = -1;
	_zoomX = _zoomY = true;
	_editX = _editY = true;
	_showDerivative = true;
}

EnvelopeGraph::~EnvelopeGraph()
{
}

float32 EnvelopeGraph::_getSpace(float32 d, float32 w)
{
	float32 gg = d * pixelsPrLine / w;
	float32 gh = pow(10.0f, floor(log10(gg)));
	float32 gt = gg / gh;
	if (gt < 1.5f)
		gt = 1.0f;
	else if (gt < 3.5f)
		gt = 2.0f;
	else 
		gt = 5.0f;
	return gt * gh;
}

void EnvelopeGraph::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	QPen blackPen;
	blackPen.setColor(QColor(0,0,0));
	QPen darkGreyPen;
	darkGreyPen.setColor(QColor(150,150,150));
	QPen redPen;
	redPen.setColor(QColor(255,0,0));
	redPen.setWidth(0);
	QPen greenPen;
	greenPen.setColor(QColor(0,255,0));
	greenPen.setWidth(0);

	QFont f;
	f.setPointSize(8);
	painter.setFont(f);

	float32 w = width();
	float32 h = height();

	// main graph surface
	painter.fillRect(xMargin, 0, w - xMargin, h - yMargin, QColor(200, 200, 200));
	// margin surface
	painter.fillRect(0, 0, xMargin, h, QColor(100, 100, 100));
	painter.fillRect(xMargin,h - yMargin, w - xMargin, yMargin, QColor(100, 100, 100));
	
	// Draw vertical lines
	{
		float32 xSpace = _getSpace(_xMax - _xMin, w - xMargin);
		float32 a = ceil(_xMin / xSpace) * xSpace;
		for (; a < _xMax; a += xSpace) {
			float32 x = xMargin + (a - _xMin) * (w - xMargin) / (_xMax - _xMin);
			painter.setPen(darkGreyPen);
			painter.drawLine(QPointF(x, 0.0f), QPointF(x, h - yMargin));
			painter.setPen(blackPen);
			painter.drawText(QRect(x - 30, h - yMargin, 60, 20), Qt::AlignCenter, QString::number(a));
		}
		a = ceil(_xMin / (xSpace * 0.5f)) * xSpace * 0.5f;
		for(; a < _xMax; a += xSpace * 0.5f) {
			float32 x = xMargin + (a - _xMin) * (w - xMargin) / (_xMax - _xMin);
			painter.drawLine(QPointF(x, h - yMargin), QPointF(x, h - yMargin + 4.0f));
		}
		float32 x = xMargin - _xMin * (w - xMargin) / (_xMax - _xMin);
		if (x > xMargin && x < w)
			painter.drawLine(QPointF(x, 0.0f), QPointF(x, h - yMargin));
	}

	// Draw horizontal lines
	{
		float32 ySpace = _getSpace(_yMax - _yMin, h - yMargin);
		float32 a = ceil(_yMin / ySpace) * ySpace;
		for (; a < _yMax; a += ySpace) {
			float32 y = h - yMargin - (a - _yMin) * (h - yMargin) / (_yMax - _yMin);
			painter.setPen(darkGreyPen);
			painter.drawLine(QPointF(xMargin, y), QPointF(w, y));
			painter.setPen(blackPen);
			painter.drawText(QRect(0, y - 10, xMargin - 4, 20), Qt::AlignVCenter | Qt::AlignRight, QString::number(a));
		}
		a = ceil(_yMin / (ySpace * 0.5f)) * ySpace * 0.5f;
		for (;a < _yMax; a += ySpace * 0.5f) {
			float32 y = h - yMargin - (a - _yMin) * (h - yMargin) / (_yMax - _yMin);
			painter.drawLine(QPointF(xMargin - 4.0f, y), QPointF(xMargin, y));
		}
		float32 y = h - yMargin + _yMin * (h - yMargin) / (_yMax - _yMin);
		if (y < (h - yMargin) && y > 0.0f)
			painter.drawLine(QPointF(xMargin, y), QPointF(w, y));
	}

	// Draw frame
	painter.setPen(blackPen);
	painter.drawLine(xMargin, 0, xMargin, h - yMargin);
	painter.drawLine(xMargin, h - yMargin, w, h - yMargin);

	painter.setClipRect(xMargin + 1, 0, w - xMargin, h - yMargin);

	// Draw graph
	if (_spline)
	{
		QPainterPath p, q, r;

		QTransform t; // transform from data-space to graph-viewport
		t.translate(xMargin, h - yMargin);
		t.scale((w - xMargin) / (_xMax - _xMin), (yMargin - h) / (_yMax - _yMin));
		t.translate(-_xMin, -_yMin);

		uint32 first = 0;

		if (_spline->GetControlPoints().empty()) {
			p.moveTo(_xMin, 0.0f);
			p.lineTo(_xMax, 0.0f);
			if (_showDerivative) {
				q.moveTo(_xMin, 0.0f);
				q.moveTo(_xMax, 0.0f);
			}
		}
		else {
			uint32 i = 0;
			for (; i < _spline->GetControlPoints().size(); i++)
				if (_spline->GetControlPoints()[i].x > _xMin)
					break;
			p.moveTo(_xMin, _spline->Evaluate(_xMin));
			if (_showDerivative)
				q.moveTo(_xMin, _spline->EvaluateDerivative(_xMin));
			if (i == 0) {
				p.lineTo(_spline->GetControlPoints()[i].x, _spline->GetControlPoints()[i].y);
				if (_showDerivative)
					q.lineTo(_spline->GetControlPoints()[i].x, 0.0f);
			}
			else
				i--;
			first = i;
			for (;i < _spline->GetControlPoints().size() - 1 && _spline->GetControlPoints()[i].x < _xMax; i++) {
				const ValueTCBSpline::ControlPoint &cp0 = _spline->GetControlPoints()[i], &cp1 = _spline->GetControlPoints()[i + 1];
				if (cp0.type == ValueTCBSpline::ControlPoint::STEP) {
					p.lineTo(cp0.x, cp0.y);
					p.lineTo(cp1.x, cp0.y);
					p.lineTo(cp1.x, cp1.y);
					if (_showDerivative) {
						q.lineTo(cp0.x, 0.0f);
						q.lineTo(cp1.x, 0.0f);
					}
				}
				else if (cp0.type == ValueTCBSpline::ControlPoint::LINEAR) {
					p.lineTo(cp1.x, cp1.y);
					if (_showDerivative) {
						q.lineTo(cp0.x, _spline->EvaluateDerivative(cp0.x));
						q.lineTo(cp1.x, _spline->EvaluateDerivative(cp0.x));
					}
				}
				else {
					float32 t = 0.0f;
					for (float64 v = (_xMax - _xMin) / (w - xMargin), x = std::max((float64)_xMin, cp0.x); x < std::min((float64)_xMax, cp1.x); x += v) {
						p.lineTo(x, _spline->Evaluate(x));
						if (_showDerivative)
							q.lineTo(x, t = _spline->EvaluateDerivative(x));
					}
					if (_showDerivative)
						q.lineTo(cp1.x, t);
				}
			}
			if (_spline->GetControlPoints().back().x < _xMax) {
				p.lineTo(_spline->GetControlPoints().back().x, _spline->GetControlPoints().back().y);
				p.lineTo(_xMax, _spline->GetControlPoints().back().y);
				if (_showDerivative) {
					q.lineTo(_spline->GetControlPoints().back().x, 0.0f);
					q.lineTo(_xMax, 0.0f);
				}
			}
		}
		painter.setTransform(t); 
		if (_showDerivative) {
			painter.setPen(greenPen);
			painter.drawPath(q);
		}
		painter.setPen(redPen);
		painter.drawPath(p);
		
		// Render control points
		for (uint32 i = first; i < _spline->GetControlPoints().size() && _spline->GetControlPoints()[i].x < _xMax; i++) {
			QPointF po = t.map(QPointF(_spline->GetControlPoints()[i].x, _spline->GetControlPoints()[i].y));
			if (i != _selected) {
				r.moveTo(po - QPointF(3.0f, 0.0f));
				r.lineTo(po + QPointF(3.0f, 0.0f));
				r.moveTo(po - QPointF(0.0f, 3.0f));
				r.lineTo(po + QPointF(0.0f, 3.0f));
			}
			else {
				r.addEllipse(po, 2.0f, 2.0f);
				float32 ar = (_yMax - _yMin) / (_xMax - _xMin) * (w - xMargin) / (h - yMargin);
				const float32 TLENGTH = 20.0f;
				QPointF a(-TLENGTH, TLENGTH * _spline->GetControlPoints()[i].it / ar);
				QPointF b(TLENGTH, -TLENGTH * _spline->GetControlPoints()[i].ot / ar);
				r.moveTo(po + a * TLENGTH / sqrt(a.x() * a.x() + a.y() * a.y()));
				r.lineTo(po);
				r.lineTo(po + b * TLENGTH / sqrt(b.x() * b.x() + b.y() * b.y()));
			}
		}
		painter.setTransform(QTransform());
		painter.setPen(blackPen);
		painter.drawPath(r);
	}
}

void EnvelopeGraph::wheelEvent(QWheelEvent *event)
{
	if (event->angleDelta().y() == 0)
		return;
	float32 px = _xMin + (_xMax - _xMin) * float32(event->position().x() - xMargin) / float32(width() - xMargin);
	float32 py = _yMin + (_yMax - _yMin) * float32(height() - event->position().y() - yMargin) / float32(height() - yMargin);
	float32 d = pow(2.0f, float32(-event->angleDelta().y()) / 1000.0f);
	if (_zoomX)
		d = std::max(std::min(d, MAX_ZOOM / (_xMax - _xMin)), MIN_ZOOM / (_xMax - _xMin));
	if (_zoomY)
		d = std::max(std::min(d, MAX_ZOOM / (_yMax - _yMin)), MIN_ZOOM / (_yMax - _yMin));
	if (_zoomX) {
		_xMin = px - (px - _xMin) * d;
		_xMax = px - (px - _xMax) * d;
	}
	if (_zoomY) {
		_yMin = py - (py - _yMin) * d;
		_yMax = py - (py - _yMax) * d;
	}
	event->accept();
	repaint();
}

void EnvelopeGraph::mouseMoveEvent(QMouseEvent *event)
{
	float32 dx = (_xMin - _xMax) / (width() - xMargin) * (event->x() - _mouseX);
	float32 dy = (_yMax - _yMin) / (height() - yMargin) * (event->y() - _mouseY);
	if (event->buttons() & Qt::LeftButton) {
		if (_editX && dx != 0.0f || _editY && dy != 0.0f)
			emit moveControlPoint(_editX ? -dx : 0.0f, _editY ? -dy : 0.0f);
	}
	if (event->buttons() & Qt::MiddleButton) { // move camera
		_xMin += dx;
		_xMax += dx;
		_yMin += dy;
		_yMax += dy;
		repaint();
	}
	_mouseX = event->x();
	_mouseY = event->y();
	event->accept();
}

void EnvelopeGraph::mousePressEvent(QMouseEvent *event)
{
	_mouseX = event->x();
	_mouseY = event->y();
	_drag = false;
	if (event->button() == Qt::LeftButton && _spline) {
		float32 w = width(), h = height();
		float32 x = _xMin + (_xMax - _xMin) * (_mouseX - xMargin) / (w - xMargin);
		float32 y = _yMin + (_yMax - _yMin) * (h - _mouseY - yMargin) / (h - yMargin);
		float32 m = std::numeric_limits<float32>::max();
		uint32 j = -1;
		for (uint32 i = 0; i < _spline->GetControlPoints().size(); i++) {
			const ValueTCBSpline::ControlPoint &cp = _spline->GetControlPoints()[i];
			if (abs(cp.x - x) < 4.0f) {
				float32 tmp = (cp.x - x) * (cp.x - x) + (cp.y - y) * (cp.y - y);
				if (tmp < m && tmp < 4.0f * 4.0f) {
					m = tmp;
					j = i;
				}
			}
			else if (cp.x - x > 4.0f)
				break;
		}
		if (j != -1) {
			_drag = true;
			if (j != _selected) {
				_selected = j;
				emit selectionChanged(j);
				repaint();
			}
		}
	}
	event->accept();
}

void EnvelopeGraph::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::RightButton && _spline) {
		float32 w = width(), h = height();
		float32 x = _xMin + (_xMax - _xMin) * (_mouseX - xMargin) / (w - xMargin);
		float32 y = _yMin + (_yMax - _yMin) * (h - _mouseY - yMargin) / (h - yMargin);
		if (x < _xMin || y < _yMin)
			return; // at margin

		QMenu m;
		QAction *a0 = m.addAction("Add Control Point");
		m.addSeparator();
		QAction *a1 = m.addAction("View All");
		QAction *a2 = m.addAction("Show Derivative");
		a2->setCheckable(true);
		a2->setChecked(_showDerivative);
		QAction *a = m.exec(event->globalPos());
		if (a == a0)
			emit addControlPoint(x, y);
		else if (a == a1)
			viewAll();
		else if (a == a2) {
			_showDerivative = !_showDerivative;
			repaint();
		}
	}
	event->accept();
}

void EnvelopeGraph::keyPressEvent(QKeyEvent *event)
{
 	if (event->key() == Qt::Key_Delete)
		emit deleteControlPoint();
	event->accept();
}

void EnvelopeGraph::setCurrentControlPoint(uint32 i)
{
	if (i >= _spline->GetControlPoints().size())
		i = -1;
	_selected = i;
	emit selectionChanged(i);
	repaint();
}

void EnvelopeGraph::viewAll()
{
	if (_spline->GetControlPoints().empty())
		return restoreView();
	_xMin = _spline->GetControlPoints().front().x;
	_xMax = _spline->GetControlPoints().back().x;
	_yMin = std::numeric_limits<float32>::max();
	_yMax = -std::numeric_limits<float32>::max();
	for (size_t i = 0; i < _spline->GetControlPoints().size(); i++) {
		_yMin = std::min(_yMin, (float32)_spline->GetControlPoints()[i].y);
		_yMax = std::max(_yMax, (float32)_spline->GetControlPoints()[i].y);
	}
	float32 dx = 0.0f, dy = 0.0f;
	if (_spline->GetControlPoints().size() == 1) {
		dx = dy = 1.0f;
	}
	else {
		dx = (_xMax - _xMin) * 0.1f;
		dy = (_yMax - _yMin) * 0.1f;
	}
	_xMin -= dx;
	_xMax += dx;
	_yMin -= dy;
	_yMax += dy;
}

void EnvelopeGraph::restoreView()
{
	_xMin = _yMin = -10.0f;
	_xMax = _yMax = 10.0f;
}

void EnvelopeGraph::prevPoint()
{
	if (_selected != -1 && _selected > 0)
		setCurrentControlPoint(_selected - 1);
}

void EnvelopeGraph::nextPoint()
{
	if (_selected != -1 && _selected < _spline->GetControlPoints().size() - 1)
		setCurrentControlPoint(_selected + 1);
}
