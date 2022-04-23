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
#include "M3DEngine/Engine.h"
#include "M3DEngine/GlobalDef.h"
#include "M3DEngine/Chip.h"
#include "M3DEngine/ClassInstance.h"
#include "M3DEngine/Class.h"
#include "StackTraceDialog.h"
#include "ActionManager.h"
#include <qplaintextedit.h>
#include <qmap.h>
#include <qmenu.h>
#include <qactiongroup.h>


namespace m3d
{

class ClassExt;
struct ApplicationMessage;


class MsgLogWidget  : public QPlainTextEdit, public ActionListener
{
	Q_OBJECT
public:
	MsgLogWidget(QWidget *parent = nullptr);
	virtual ~MsgLogWidget();

	virtual void mousePressEvent(QMouseEvent *event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent  *event) override;
	virtual void contextMenuEvent(QContextMenuEvent *event) override;
	virtual void resizeEvent(QResizeEvent *event) override;

public:
	void addMessage(const m3d::ApplicationMessage &msg);
	void onVSliderMoved();
	void onDialogClosed();
	void onDialogFocusOnChip(ClassExt*,Chip*);
	void onDialogOpenChipDialog(Chip*);
	void onDialogOpenInstanceDialog(ClassInstance*);

private:
	struct MessageMeta
	{
		CGCID chip;
		CallStack stackTrace;
		MessageMeta(CGCID chip = CGCID()) : chip(chip) {}
	};

	typedef QMap<unsigned, MessageMeta> MessageMap;
	MessageMap _messages;
	unsigned _msgBase;
	unsigned _bufferSize;

	bool _dumpStackTrace;
	bool _scroll; // Auto scroll to bottom for new messages

	int _filter;

	QMap<unsigned, StackTraceDialog*> _dialogs;
	QPoint _defaultDialogPos;
	QSize _defaultDialogSize;

	QMenu *_filterMenu;
	QActionGroup *_ag;
	QAction *_actions[12];

	MessageMap::const_iterator _messageAtPos(QPoint p);
	void _focusOnChip(Chip *ch);

	ActionMask getActions() override;
	bool isActionEnabled(ActionID id) override;
	void triggered(ActionID id) override;
};
}