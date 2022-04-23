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
#include "ChipMessagesWidget.h"
#include <qheaderview.h>
#include <qmenu.h>
#include <qevent.h>
#include <ctime>
#include "M3DEngine/Chip.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/Engine.h"
#include <qapplication.h>
#include "AppSignals.h"

using namespace m3d;


class ChipMessagesWidgetItem : public QTreeWidgetItem
{
public:
	Chip *ch;
	ChipMessage msg;


	ChipMessagesWidgetItem(QTreeWidget *view, const QStringList &strings, int type = Type) : QTreeWidgetItem(view, strings, type) {}
	~ChipMessagesWidgetItem() {}

/*	virtual bool operator<(const QTreeWidgetItem &other) const 
	{
		int i = treeWidget()->sortColumn();
		Qt::SortOrder sortOrder = treeWidget()->header()->sortIndicatorOrder();
		if (i == 0) {
			unsigned a = data(0, Qt::UserRole).toUInt();
			unsigned b = other.data(0, Qt::UserRole).toUInt();
			return a == b ? (sortOrder == Qt::DescendingOrder ? data(0, Qt::UserRole + 1).toUInt() < other.data(0, Qt::UserRole + 1).toUInt() : data(0, Qt::UserRole + 1).toUInt() > other.data(0, Qt::UserRole + 1).toUInt()) : a < b;
		}
		if (i == 1)
			return data(0, Qt::UserRole + 1).toUInt() < other.data(0, Qt::UserRole + 1).toUInt();
		return QTreeWidgetItem::operator<(other);
	}*/
};



ChipMessagesWidget::ChipMessagesWidget(QWidget * parent) : QTreeWidget(parent)
{
	header()->reset();
	headerItem()->setText(0, "Type");
	headerItem()->setText(1, "Time");
	headerItem()->setText(2, "Count");
	headerItem()->setText(3, "Chip Name");
	headerItem()->setText(4, "Class");
	headerItem()->setText(5, "Chip Type");
	headerItem()->setText(6, "Message");
	setIndentation(0);
	sortByColumn(1, Qt::DescendingOrder);
	header()->setStretchLastSection(true);
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setSelectionBehavior(QAbstractItemView::SelectRows);

	connect(&AppSignals::instance(), &AppSignals::chipMessageAdded, this, &ChipMessagesWidget::chipMessageAdded, Qt::DirectConnection);
	connect(&AppSignals::instance(), &AppSignals::chipMessageRemoved, this, &ChipMessagesWidget::chipMessageRemoved, Qt::DirectConnection);
	connect(&AppSignals::instance(), &AppSignals::newFrame, this, &ChipMessagesWidget::newFrame);

	connect(this, &ChipMessagesWidget::itemDoubleClicked, this, &ChipMessagesWidget::onItemDoubleClicked);

	_menu = new QMenu(this);
	_actions[0] = _menu->addAction("Focus on Chip");
	_actions[1] = _menu->addAction("Open Chip Dialog");
	_actions[2] = _menu->addAction("Acknowledge Selected Messages");
	_actions[3] = _menu->addAction("Acknowledge All Messages");

	_lastRefresh = 0;
}

ChipMessagesWidget::~ChipMessagesWidget()
{
	assert(topLevelItemCount() == 0);
}

void ChipMessagesWidget::contextMenuEvent(QContextMenuEvent *event)
{
	ChipMessagesWidgetItem *item = (ChipMessagesWidgetItem*)itemAt(event->pos());
	
	QList<QTreeWidgetItem*> si = selectedItems();
	_actions[0]->setEnabled(item != nullptr);
	_actions[1]->setEnabled(item != nullptr);
	_actions[2]->setEnabled(si.size() > 0);
	_actions[3]->setEnabled(topLevelItemCount() > 0);
	QAction *a = _menu->exec(viewport()->mapToGlobal(event->pos()));
	if (!a)
		return;
	if (a == _actions[0]) {
		_focusOnChip(item->ch);
	}
	else if (a == _actions[1]) {
		AppSignals::instance().openChipDialog(item->ch);
	}
	else if (a == _actions[2]) {
		for (int i = 0; i < si.size(); i++) {
			ChipMessagesWidgetItem *item = (ChipMessagesWidgetItem*)si[i];
			item->ch->RemoveMessage(item->msg);
		}
	}
	else if (a == _actions[3]) {
		while (topLevelItemCount() > 0) {
			ChipMessagesWidgetItem *item = (ChipMessagesWidgetItem*)topLevelItem(0);
			item->ch->RemoveMessage(item->msg);
		}
	}
}

void ChipMessagesWidget::chipMessageAdded(m3d::Chip*ch, const m3d::ChipMessage&msg)
{
	if (QThread::currentThread() == thread())
	{
		static const char* MSG[6] = { "Debug", "Info", "Notice", "Warning", "Error" };

		QIcon icn[6] = {
			QIcon(":/EditorApp/Resources/bug.png"),
			QIcon(":/EditorApp/Resources/info.png"),
			QIcon(":/EditorApp/Resources/notice.png"),
			QIcon(":/EditorApp/Resources/warning.png"),
			QIcon(":/EditorApp/Resources/error.png"),
			QIcon(":/EditorApp/Resources/critical.png")
		};

		time_t t = time(0);
		char buff[64];
		tm tt;
		localtime_s(&tt, &t);
		strftime(buff, 64, "%H:%M:%S", &tt);

		QString message(TOQSTRING(msg.msg));

		QStringList l;
		l << MSG[msg.severity] << buff << QString::number(msg.hitCount) << TOQSTRING(ch->GetName()) << (ch->GetClass() ? TOQSTRING(ch->GetClass()->GetName()) : "-") << QString(ch->GetChipDesc().name) << message;
		ChipMessagesWidgetItem* item = new ChipMessagesWidgetItem(this, l);
		item->setData(0, Qt::DecorationRole, icn[msg.severity]);
		item->ch = ch;
		item->msg = msg;
	}
	else
	{
		QMutexLocker l(&_m);
		_msgToAdd.push_back({ ch, msg });
		_isMsgFromOtherThread = 1;
	}
}

void ChipMessagesWidget::chipMessageRemoved(m3d::Chip*ch, const m3d::ChipMessage&msg)
{
	if (QThread::currentThread() == thread())
	{
		for (int i = 0; i < topLevelItemCount(); i++) {
			ChipMessagesWidgetItem* itm = (ChipMessagesWidgetItem*)topLevelItem(i);
			if (itm->ch == ch && itm->msg.msg == msg.msg) {
				delete itm;
				return;
			}
		}
	}
	else
	{
		QMutexLocker l(&_m);
		_msgToRemove.push_back({ ch, msg });
		_isMsgFromOtherThread = 1;
	}
}

#define REFRESH_FREQ 500

void ChipMessagesWidget::newFrame()
{
	if (_isMsgFromOtherThread == 1)
	{
		QMutexLocker l(&_m);
		_isMsgFromOtherThread = 0;
		for (const PendingMessage& msg : _msgToRemove)
		{
			chipMessageRemoved(msg.chip, msg.msg);
			for (int i = 0; i < _msgToAdd.size(); i++)
			{
				if (_msgToAdd[i].chip == msg.chip && _msgToAdd[i].msg.msg == msg.msg.msg)
				{
					_msgToAdd.removeAt(i); 
					i--;
				}
			}
		}
		_msgToRemove.clear();
		for (const PendingMessage& msg : _msgToAdd)
		{
			chipMessageAdded(msg.chip, msg.msg);
		}
		_msgToAdd.clear();
	}
	long ct = engine->GetClockTime();
	if (abs(ct - _lastRefresh) / (float(CLOCKS_PER_SEC) / 1000.0f) < REFRESH_FREQ)
		return;
	_lastRefresh = ct;

	time_t t = time(0);
	char buff[64];
	tm tt;
	localtime_s(&tt, &t);
	strftime(buff, 64, "%H:%M:%S", &tt);

	for (int i = 0; i < topLevelItemCount(); i++) {
		ChipMessagesWidgetItem *itm = (ChipMessagesWidgetItem*)topLevelItem(i);
		unsigned n = itm->ch->GetMessageHitCount(itm->msg);
		if (n > itm->msg.hitCount) {
			itm->msg.hitCount = n;
			itm->setText(1, buff);
			itm->setText(2, QString::number(n));
		}
	}
}

void ChipMessagesWidget::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
	ChipMessagesWidgetItem *i = (ChipMessagesWidgetItem*)item;

	_focusOnChip(i->ch);
}

void ChipMessagesWidget::_focusOnChip(Chip *ch)
{
	while (!ch->GetChipEditorData() && ch->GetOwner() != ch)
		ch = ch->GetOwner();

	if (!ch->GetChipEditorData())
		return QApplication::beep();

	AppSignals::instance().openClass((ClassExt*)ch->GetClass(), ch);
}