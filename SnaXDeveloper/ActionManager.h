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
#include <qaction.h>
#include "M3DCore/Containers.h"


namespace m3d
{


typedef unsigned ActionMask;
class ActionListener;

enum struct ActionID : ActionMask
{
	NEW_PROJECT = 0x1,
	OPEN_PROJECT = 0x2,
	SAVE = 0x4,
	SAVE_AS = 0x8,
	SAVE_DIRTY = 0x10,
	SAVE_ALL = 0x20,
	ADD_CLASS = 0x40,
	IMPORT_CLASS = 0x80,
	CUT = 0x100,
	COPY = 0x200,
	PASTE = 0x400,
	DELITE = 0x800,
	SELECT_ALL = 0x1000,
	CLEAR_SELECTION = 0x2000,
	LOAD_ALL = 0x4000,
	PUBLISH = 0x8000,
	CLASS_DIAGRAM = 0x10000,
	KONTINUE = 0x20000,
	SCREENSHOT = 0x40000,
	PAUSE = 0x80000,
	EXTERNALVIEW = 0x100000,
	EXTERNALVIEWFULLSCREEN = 0x200000,
	END = 0x400000
};

class ActionManager : public QObject
{
	Q_OBJECT
public:
	ActionManager(QObject *parent = nullptr);

	QAction *getAction(ActionID id);

	static ActionManager &instance();

	void focusChanged(QWidget *old, QWidget *now);

	void updateAction(ActionID id);
	void updateActions();

private:
	struct Action
	{
		QAction *action;
		ActionListener *receiver;

		Action() : action(nullptr), receiver(nullptr) {}
	};

	Map<ActionID, Action> _actions;

	ActionMask _updateAction(ActionListener *a, ActionMask used);
	void _actionTriggered(ActionID id);
	void _initAction(ActionID id, QIcon icon, QString txt, QString toolTip, bool checkable = false, QKeySequence shortcut = QKeySequence());
};


class ActionListener
{
public:
	ActionListener();
	virtual ~ActionListener();

	virtual ActionMask getActions() = 0;
	virtual bool isActionEnabled(ActionID id) = 0;
	virtual void triggered(ActionID id) = 0;
};


}