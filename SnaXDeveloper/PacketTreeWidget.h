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
#include <qwidget>
#include "CustomDraggableTreeWidget.h"
#include "M3DEngine/GlobalDef.h"
#include "M3DCore/MString.h"
#include <QActionGroup>

namespace m3d
{

class PacketTreeWidget : public CustomDraggableTreeWidget
{
	Q_OBJECT
public:
	PacketTreeWidget(QWidget * parent = 0);
	~PacketTreeWidget();

	void updateSettings(QSettings& s, bool load);

protected:
//	virtual void  mouseReleaseEvent ( QMouseEvent * event );
	virtual void  contextMenuEvent ( QContextMenuEvent * event );

	virtual bool startDragging(QTreeWidgetItem *item);
	virtual void endDragging(QTreeWidgetItem *item, QWidget *dropAt, const QPoint &pos, Qt::KeyboardModifiers modifiers);
	virtual QCursor dragCursor();

	QMenu *_menu = nullptr;
	QActionGroup *_ag = nullptr;
	QAction *_a1 = nullptr;
	QAction *_a2 = nullptr;
	QAction *_a3 = nullptr;
	QAction *_current = nullptr;
	bool _isInit = false;

public:
	void initiate();
	void onPacketLoaded(QString);
};

}