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
#include "MsgLogWidget.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngine/FunctionStack.h"
#include "M3DEngineExt/ClassExt.h"
#include "M3DEngine/Application.h"
#include <ctime>
#include <qmessagebox>
#include <qheaderview>
#include <qapplication>
#include <qclipboard.h>
#include <qscrollbar.h>
#include <QMouseEvent>
#include <qinputdialog.h>
#include "AppSignals.h"

using namespace m3d;


MsgLogWidget::MsgLogWidget(QWidget *parent) : QPlainTextEdit(parent)
{
	setReadOnly(true);
	setUndoRedoEnabled(false);
	//setStyleSheet("font: 10pt \"Consolas\";");
	setFont(QFont("Consolas", 10));
	setWordWrapMode(QTextOption::NoWrap);
	setLineWrapMode(QPlainTextEdit::NoWrap);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	_msgBase = 0;
	_bufferSize = -1;
	_dumpStackTrace = true;
	_scroll = true;
	_filter = 4; // INFO level
	_defaultDialogPos = QPoint(INT_MIN, INT_MIN);
	_defaultDialogSize = QSize(0, 0);

	connect(&AppSignals::instance(), &AppSignals::messageAdded, this, &MsgLogWidget::addMessage);

	connect(verticalScrollBar(), &QScrollBar::actionTriggered, this, &MsgLogWidget::onVSliderMoved);

	static const QIcon icn[6] = 
	{
		QIcon(":/EditorApp/Resources/bug.png"),
		QIcon(":/EditorApp/Resources/info.png"),
		QIcon(":/EditorApp/Resources/notice.png"),
		QIcon(":/EditorApp/Resources/warning.png"),
		QIcon(":/EditorApp/Resources/error.png"),
		QIcon(":/EditorApp/Resources/critical.png")
	};

	_filterMenu = new QMenu("Filter", this);
	_actions[0] = _filterMenu->addAction(icn[5], "Block All");
	_actions[0]->setCheckable(true);
	_actions[1] = _filterMenu->addAction(icn[4], "Error");
	_actions[1]->setCheckable(true);
	_actions[2] = _filterMenu->addAction(icn[3], "Warning");
	_actions[2]->setCheckable(true);
	_actions[3] = _filterMenu->addAction(icn[2], "Notice");
	_actions[3]->setCheckable(true);
	_actions[4] = _filterMenu->addAction(icn[1], "Info");
	_actions[4]->setCheckable(true);
	_actions[5] = _filterMenu->addAction(icn[0], "Debug");
	_actions[5]->setCheckable(true);
	_ag = new QActionGroup(this);
	_ag->addAction(_actions[0]);
	_ag->addAction(_actions[1]);
	_ag->addAction(_actions[2]);
	_ag->addAction(_actions[3]);
	_ag->addAction(_actions[4]);
	_ag->addAction(_actions[5]);
	_actions[6] = new QAction("Clear All", this);
	_actions[7] = new QAction("Focus on Chip", this);
	_actions[8] = new QAction("Open Chip Dialog", this);
	_actions[9] = new QAction("Show Call Stack", this);
	_actions[10] = new QAction("Dump Call Stack", this);
	_actions[10]->setCheckable(true);
	_actions[11] = new QAction("Set Buffer Size...", this);

	connect(this, &QPlainTextEdit::copyAvailable, this, [=](bool) { ActionManager::instance().updateAction(ActionID::COPY); } );
}

MsgLogWidget::~MsgLogWidget()
{
}

void MsgLogWidget::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
		_scroll = false;
	QPlainTextEdit::mousePressEvent(event);
}

void MsgLogWidget::mouseDoubleClickEvent(QMouseEvent  *event)
{
	if (event->button() == Qt::LeftButton) {
		Chip *ch = nullptr;
		MessageMap::const_iterator itr = _messageAtPos(event->pos());
		if (itr != _messages.end())
			ch = engine->GetClassManager()->FindChip(itr->chip.chipID, itr->chip.clazzID);
		if (!ch)
			return QApplication::beep();
		_focusOnChip(ch);
	}
}

void MsgLogWidget::contextMenuEvent(QContextMenuEvent *event)
{
	Chip *ch = nullptr;
	MessageMap::const_iterator itr = _messageAtPos(event->pos());
	if (itr != _messages.end())
		ch = engine->GetClassManager()->FindChip(itr->chip.chipID, itr->chip.clazzID);

	_actions[_filter]->setChecked(true);
	_actions[6]->setEnabled(_messages.size() > 0);
	_actions[7]->setEnabled(ch != nullptr);
	_actions[8]->setEnabled(ch != nullptr);
	_actions[9]->setEnabled(itr != _messages.end() && itr->stackTrace.size() > 0);
	_actions[10]->setChecked(_dumpStackTrace);
	QMenu *editContextMenu = new QMenu(this);
	editContextMenu->addAction(ActionManager::instance().getAction(ActionID::COPY));
	editContextMenu->addSeparator();
	editContextMenu->addAction(ActionManager::instance().getAction(ActionID::SELECT_ALL));
	editContextMenu->addAction(_actions[6]);
	editContextMenu->addSeparator();
	editContextMenu->addAction(_actions[7]);
	editContextMenu->addAction(_actions[8]);
	editContextMenu->addAction(_actions[9]);
	editContextMenu->addSeparator();
	editContextMenu->addAction(_actions[10]);
	editContextMenu->addAction(_actions[11]);
	editContextMenu->addMenu(_filterMenu);
	QAction *a = editContextMenu->exec(viewport()->mapToGlobal(event->pos()));
	
	if (a == _actions[0]) _filter = 0;
	else if (a == _actions[1]) _filter = 1;
	else if (a == _actions[2]) _filter = 2;
	else if (a == _actions[3]) _filter = 3;
	else if (a == _actions[4]) _filter = 4;
	else if (a == _actions[5]) _filter = 5;
	else if (a == _actions[6]) {
		clear();
		_messages.clear();
		_msgBase = 0;
	}
	else if (a == _actions[7]) {
		_focusOnChip(ch);
	}
	else if (a == _actions[8]) {
		AppSignals::instance().openChipDialog(ch);
	}
	else if (a == _actions[9]) {
		QMap<unsigned, StackTraceDialog*>::iterator jtr = _dialogs.find(itr.key());
		if (jtr == _dialogs.end()) { // No dialog for this message?
			// Extract message
			QTextCursor c = textCursor();
			c.setPosition(itr.key() - _msgBase);
			if (itr + 1 == _messages.end())
				c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
			else
				c.setPosition((itr + 1).key(), QTextCursor::KeepAnchor);
			QString msg = c.selectedText();
			
			QWidget *p = this;
			while (p->parentWidget())
				p = p->parentWidget();
			StackTraceDialog *d = new StackTraceDialog(p);
			if (_defaultDialogPos != QPoint(INT_MIN, INT_MIN))
				d->move(_defaultDialogPos);
			if (_defaultDialogSize != QSize(0, 0))
				d->resize(_defaultDialogSize);

			connect(d, &StackTraceDialog::finished, this, &MsgLogWidget::onDialogClosed);
			_dialogs.insert(itr.key(), d);
			d->Init(itr.key(), msg, itr->stackTrace);
			d->show();
		}
		else { // flash
			(*jtr)->flash();
		}
	}
	else if (a == _actions[10]) {
		_dumpStackTrace = _actions[10]->isChecked();
	}
	else if (a == _actions[11]) {
		QStringList lst;
		lst << "Unlimited" << "50" << "100" << "500" << "1000";
		int c = _bufferSize == -1 ? 0 : lst.indexOf(QString::number(_bufferSize));
		QString s = QInputDialog::getItem(this, "Message Buffer Size", "Set Message Buffer Size:", lst, c, false);
		if (s == "Unlimited")
			_bufferSize = -1;
		else
			_bufferSize = s.toUInt();
	}

	editContextMenu->deleteLater();
}

void MsgLogWidget::resizeEvent(QResizeEvent *event)
{
	QPlainTextEdit::resizeEvent(event);

	if (_scroll) {
		verticalScrollBar()->setValue(verticalScrollBar()->maximum());
	}
}

void MsgLogWidget::_focusOnChip(Chip *ch)
{
	while (!ch->GetChipEditorData() && ch->GetOwner() != ch)
		ch = ch->GetOwner();

	if (!ch->GetChipEditorData())
		return QApplication::beep();

	AppSignals::instance().openClass((ClassExt*)ch->GetClass(), ch);
}

void MsgLogWidget::addMessage(const m3d::ApplicationMessage& msg)
{
	static const char* MSG[6] = { "DEBUG:&nbsp;&nbsp;&nbsp;", "INFO:&nbsp;&nbsp;&nbsp;&nbsp;", "NOTICE:&nbsp;&nbsp;", "WARNING:&nbsp;", "ERROR:&nbsp;&nbsp;&nbsp;" };
	static const char* MSG_COLOR[6] = { "black", "black", "blue", "orangered", "red" };

	if (msg.severity >= ALWAYS || msg.severity < (ALWAYS - _filter))
		return;

	if (_messages.size() == _bufferSize) {
		QMap<unsigned, MessageMeta>::iterator itr = _messages.begin() + qMin(10, _messages.size()); // Remove a batch of up to 10 messages
		QTextCursor c = textCursor();
		c.movePosition(QTextCursor::End);
		if (c.position() <= itr.key() - _msgBase) {
			clear();
			_messages.clear();
			_msgBase = 0;
		}
		else {
			c.setPosition(0);
			c.setPosition(itr.key() - _msgBase, QTextCursor::KeepAnchor);
			c.clearSelection();
			_msgBase += itr.key();
			while (_messages.begin() != itr)
				_messages.erase(_messages.begin());
		}
	}


	char buff[64];
	tm tt;
	localtime_s(&tt, &msg.timestamp);
	strftime(buff, 64, "%H:%M:%S ", &tt);

	QString message(QString(buff) + TOQSTRING(strUtils::format(MTEXT("[%5i] "), msg.threadID)) + MSG[msg.severity] + TOQSTRING(msg.message));

	QTextCursor c = textCursor();
	c.movePosition(QTextCursor::End); // Scroll to end
	unsigned pos = c.position();

	QMap<unsigned, MessageMeta>::iterator itr = _messages.insert(_msgBase + pos, MessageMeta());

	itr.value().chip = CGCID(msg.clazzID, msg.chipID);

	bool _dumpStackTrace = true;

	if (_dumpStackTrace) {
		CallStackWidget::DumpCurrentCallStack(itr.value().stackTrace, msg.clazzID, msg.chipID); // TODO: if the chip is a break point for some reason, we should probably set the third parameter true..
	}

	QString txt = QString(itr->chip.chipID != InvalidChipID ? "[C" : "[-") + QString(itr->stackTrace.empty() ? "-] " : "T] ") + message;
	if (msg.chipID != InvalidChipID)
		txt = "<i>" + txt + "</i>";
	txt = "<font color=\"" + QString(MSG_COLOR[msg.severity]) + "\">" + txt + "</font><br>";

	c.insertHtml(txt);
	if (_scroll) {
		verticalScrollBar()->setValue(verticalScrollBar()->maximum());
	}
}

void MsgLogWidget::onVSliderMoved()
{
	_scroll = verticalScrollBar()->sliderPosition() >= verticalScrollBar()->maximum();
}

MsgLogWidget::MessageMap::const_iterator MsgLogWidget::_messageAtPos(QPoint p)
{
	QTextCursor s = cursorForPosition(QPoint(0, p.y()));
	if (s.atEnd())
		return _messages.end();
	int pos = s.position();
	QMap<unsigned, MessageMeta>::iterator itr = _messages.upperBound(pos + _msgBase);
	if (itr == _messages.begin())
		return _messages.end();
	itr--;
	return itr;
}


void MsgLogWidget::onDialogClosed()
{
	StackTraceDialog *d = dynamic_cast<StackTraceDialog*>(sender());
	assert(d);
	_defaultDialogPos = d->pos();
	_defaultDialogSize = d->size();
	int n = _dialogs.remove(d->getMsgID());
	assert(n == 1);
	d->deleteLater();
}

void MsgLogWidget::onDialogFocusOnChip(ClassExt *cg, Chip *c)
{
	AppSignals::instance().openClass(cg, c);
}

void MsgLogWidget::onDialogOpenChipDialog(Chip *c)
{
	AppSignals::instance().openChipDialog(c);
}

void MsgLogWidget::onDialogOpenInstanceDialog(ClassInstance *instance)
{
	AppSignals::instance().openInstanceDialog(instance);
}

ActionMask MsgLogWidget::getActions() 
{
	return ActionMask(ActionID::COPY) | ActionMask(ActionID::SELECT_ALL);
}

bool MsgLogWidget::isActionEnabled(ActionID id) 
{
	if (id == ActionID::SELECT_ALL)
		return true;
	return textCursor().hasSelection();
}

void MsgLogWidget::triggered(ActionID id) 
{
	if (id == ActionID::COPY)
		copy();
	else if (id == ActionID::SELECT_ALL)
		selectAll();
}