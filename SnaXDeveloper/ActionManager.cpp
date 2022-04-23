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
#include "ActionManager.h"
#include "ProjectTreeWidget.h"

using namespace m3d;


ActionManager &ActionManager::instance()
{
	static ActionManager am;
	return am;
}

ActionManager::ActionManager(QObject *parent) : QObject(parent)
{
	connect((QApplication*)QApplication::instance(), &QApplication::focusChanged, this, &ActionManager::focusChanged);

	_initAction(ActionID::NEW_PROJECT, QIcon(":/EditorApp/Resources/new.png"), "&New Project", "New Project", false, QKeySequence::New);
	_initAction(ActionID::OPEN_PROJECT, QIcon(":/EditorApp/Resources/open.png"), "&Open Project", "Open Project", false, QKeySequence::Open);
	_initAction(ActionID::SAVE, QIcon(":/EditorApp/Resources/save.png"), "Save", "&Save", false, QKeySequence::Save);
	_initAction(ActionID::SAVE_AS, QIcon(":/EditorApp/Resources/save-as.png"), "Save &As", "Save As", false, QKeySequence::SaveAs);
	_initAction(ActionID::SAVE_DIRTY, QIcon(":/EditorApp/Resources/save-dirty.png"), "Save &Dirty", "Save Dirty", false, QKeySequence(Qt::CTRL, Qt::SHIFT, Qt::Key_S));
	_initAction(ActionID::SAVE_ALL, QIcon(":/EditorApp/Resources/save-all.png"), "Save A&ll", "Save All", false);
	_initAction(ActionID::ADD_CLASS, QIcon(":/EditorApp/Resources/add.png"), "Add Class", "Add Class", false, QKeySequence(Qt::CTRL, Qt::SHIFT, Qt::Key_N));
	_initAction(ActionID::IMPORT_CLASS, QIcon(":/EditorApp/Resources/import.png"), "Import Class", "Import Class", false, QKeySequence(Qt::CTRL + Qt::Key_I));
	_initAction(ActionID::CUT, QIcon(":/EditorApp/Resources/cut.png"), "Cut", "Cut", false, QKeySequence::Cut);
	_initAction(ActionID::COPY, QIcon(":/EditorApp/Resources/copy.png"), "Copy", "Copy", false, QKeySequence::Copy);
	_initAction(ActionID::PASTE, QIcon(":/EditorApp/Resources/paste.png"), "Paste", "Paste", false, QKeySequence::Paste);
	_initAction(ActionID::DELITE, QIcon(":/EditorApp/Resources/delete.png"), "Delete", "Delete", false, QKeySequence::Delete);
	_initAction(ActionID::SELECT_ALL, QIcon(), "Select All", "Select All", false, QKeySequence::SelectAll);
	_initAction(ActionID::CLEAR_SELECTION, QIcon(), "Clear Selection", "Clear Selection", false);
	_initAction(ActionID::LOAD_ALL, QIcon(":/EditorApp/Resources/refresh.png"), "Load All Project Files", "Load All Project Files", false);
	_initAction(ActionID::PUBLISH, QIcon(), "Publish...", "Publish...", false, QKeySequence(Qt::CTRL, Qt::SHIFT, Qt::Key_P));
	_initAction(ActionID::CLASS_DIAGRAM, QIcon(":/EditorApp/Resources/class-diagram.png"), "Class Diagram", "Class Diagram", false, QKeySequence(Qt::CTRL, Qt::SHIFT, Qt::Key_D));
	_initAction(ActionID::KONTINUE, QIcon(":/EditorApp/Resources/run.png"), "Continue", "Continue", true, QKeySequence(Qt::Key_F5));
	_initAction(ActionID::SCREENSHOT, QIcon(":/EditorApp/Resources/camera.png"), "Take Screenshot", "Take a screenshot of the current view", false, QKeySequence(Qt::Key_F8));
	_initAction(ActionID::PAUSE, QIcon(":/EditorApp/Resources/pause.png"), "Pause", "Pause", true, QKeySequence(Qt::Key_Pause));
	_initAction(ActionID::EXTERNALVIEW, QIcon(":/EditorApp/Resources/windows-couple.png"), "Enable External View", "Enables external project view.", true, QKeySequence(Qt::Key_F6));
	_initAction(ActionID::EXTERNALVIEWFULLSCREEN, QIcon(":/EditorApp/Resources/full-screen.png"), "Fullscreen", "Toggle fullscreen mode for external project view.", true, QKeySequence(Qt::Key_F7));
}

QAction *ActionManager::getAction(ActionID id)
{
	return _actions[id].action;
}

void ActionManager::updateAction(ActionID id)
{
	const auto &a = _actions[id];
	if (a.receiver && a.action)
		a.action->setEnabled(a.receiver->isActionEnabled(id));
}

void ActionManager::updateActions()
{
	focusChanged(nullptr, QApplication::focusWidget());
}

void ActionManager::focusChanged(QWidget* /*old*/, QWidget *now)
{
	ActionMask used = 0;
	for (QObject *obj = now; obj; obj = obj->parent()) {
		ActionListener *ar = dynamic_cast<ActionListener*>(obj);
		if (ar) {
			used = _updateAction(ar, used);
		}
	}
	for (ActionMask am = 1; am != ActionMask(ActionID::END); am <<= 1) {
		if ((am & used) == 0) {
			auto &a = _actions[ActionID(am)];
			a.receiver = nullptr;
			if (a.action)
				a.action->setEnabled(false);
		}
	}
}

void ActionManager::_actionTriggered(ActionID id)
{
	ActionListener *ar = _actions[id].receiver;
	if (ar)
		ar->triggered(id);
}

void ActionManager::_initAction(ActionID id, QIcon icon, QString txt, QString toolTip, bool checkable, QKeySequence shortcut)
{
	auto &a = _actions[id];
	a.action = new QAction(icon, txt, this);
	a.action->setToolTip(toolTip);
	a.action->setShortcut(shortcut);
	a.action->setShortcutVisibleInContextMenu(true);
	a.action->setCheckable(checkable);
	connect(a.action, &QAction::triggered, this, [=](bool s) { _actionTriggered(id); });
}


ActionMask ActionManager::_updateAction(ActionListener *ar, ActionMask used)
{
	ActionMask m = ar->getActions();
	
	for (ActionMask am = 1; am != ActionMask(ActionID::END); am <<= 1) {
		if ((used & am) == 0 && (m & am) != 0) {
			used |= am;
			auto &a = _actions[ActionID(am)];
			a.receiver = ar;
			if (a.action)
				a.action->setEnabled(ar->isActionEnabled(ActionID(am)));
		}
	}

	return used;
}

ActionListener::ActionListener()
{
}

ActionListener::~ActionListener()
{
}
