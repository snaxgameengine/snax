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
#include "CustomDraggableTreeWidget.h"
#include <QApplication>
#include <QMouseEvent>
#include <qscrollbar.h>

using namespace m3d;


CustomDraggableTreeWidget::CustomDraggableTreeWidget(QWidget * parent) : QTreeWidget(parent), _dragging(false), _dragFailed(false), _pressedItem(0), _draggedItem(0), _autoscrolling(false)
{
}

CustomDraggableTreeWidget::~CustomDraggableTreeWidget() 
{
}

void CustomDraggableTreeWidget::mouseReleaseEvent(QMouseEvent *event)
{
	QTreeWidget::mouseReleaseEvent(event);
	if (_dragging) {
		_dragging = false;
		QApplication::restoreOverrideCursor();
		if (_autoscrolling)
			stopAutoScroll();
		QPoint pos = QCursor::pos(); //event->globalPos(); // NOTE: We should use globalPos, but it does not work with scaling/on multiple monitors....
		endDragging(_draggedItem, QApplication::widgetAt(pos), pos, event->modifiers());
	}
	_pressedItem = 0;
	_draggedItem = 0;
	_dragFailed = false;
}

void CustomDraggableTreeWidget::mousePressEvent( QMouseEvent * event )
{
	QTreeWidget::mousePressEvent(event);
	if (event->button() == Qt::LeftButton ) {
		_pressedPos = event->pos();
		_pressedItem = itemAt(_pressedPos);
		_dragFailed = false;
	}
}

void CustomDraggableTreeWidget::mouseMoveEvent ( QMouseEvent * event )
{
	if (_dragFailed)
		return;
	if (_dragging) {
		//QMouseEvent tmp(event->type(), _pressedPos, event->globalPos(), event->button(), event->buttons(), event->modifiers()); // Do this to prevent the selection from change!
		//QTreeWidget::mouseMoveEvent(&tmp);
		
		QPoint p = viewport()->mapFromGlobal(event->globalPos());
		QSize s = viewport()->size();

		if (_autoscrolling) {
			if (p.x() > -16 && p.x() < s.width() + 16 && p.y() > -16 && p.y() < s.height() + 16)
				startAutoScroll();
			else
				stopAutoScroll();
		}

		QPoint pos = QCursor::pos(); //event->globalPos(); // NOTE: We should use globalPos, but it does not work with scaling/on multiple monitors....
		onDragging(_draggedItem, pos, event->modifiers());

		QWidget *w = QApplication::widgetAt(QCursor::pos());
		if (w != _currentHoverWidget) {
			_currentHoverWidget = w;
			QApplication::changeOverrideCursor(dragCursor());
		}

		return;
	}
	QTreeWidget::mouseMoveEvent(event);
	if (_dragging)
		return;
	if (!(event->buttons() & Qt::LeftButton))
		return;
	if ((event->pos() - _pressedPos).manhattanLength() < 5)
		return;
	if (!_pressedItem)
		return;

	if (!startDragging(_pressedItem)) {
		_dragFailed = true;
		return;
	}

	_draggedItem = _pressedItem;

	_dragging = true;
//	QIcon f(":/EditorApp/Resources/chip.png");QCursor(f.pixmap(32))
//	_cursor = new QCursor(f.pixmap(64));
	// Use QPixmap::grabwidget for cooler cursor?

	_currentHoverWidget = QApplication::widgetAt(QCursor::pos());

//	QWidget *d = itemWidget(item, 0);
//	QPixmap pm = QPixmap::grabWidget(this, QRect(childrenRect().x() + visualItemRect(item).x(), childrenRect().y() + visualItemRect(item).y(), visualItemRect(item).width(), visualItemRect(item).height()) );
//	this->headerItem()->vi	
//	QPixmap pm =  QPixmap::grabWidget(this, QRect(visualItemRect(item).x(), visualItemRect(item).y() + visualItemRect(this->headerItem()).height(), visualItemRect(item).width(), visualItemRect(item).height()));
//	QPixmap pm(visualItemRect(item).width(), visualItemRect(item).height());
//	render(&pm, QPoint(), QRegion(visualItemRect(item)));
	QApplication::setOverrideCursor(dragCursor());//QCursor(f.pixmap(32)));  // QCursor(pm));
}