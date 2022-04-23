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
#include "EditorCollection.h"
#include "EditorWidget.h"
#include "AppSignals.h"
#include <qsettings.h>

using namespace m3d;


static unsigned editorIDs = 0;



EditorCollection::EditorCollection() : _currentEditor(nullptr)
{
	connect(&AppSignals::instance(), &AppSignals::classDirtyFlagChanged, this, &EditorCollection::classDirtyFlagChanged); // This is to update EditorWidget's GUI.
	connect(&AppSignals::instance(), &AppSignals::classRenamed, this, &EditorCollection::classNameChanged); // This is to update EditorWidget's GUI.
	connect(&AppSignals::instance(), &AppSignals::classRemoved, this, &EditorCollection::classRemoved); // This is to update EditorWidget's GUI and current view.
	connect(&AppSignals::instance(), &AppSignals::documentFileChanged, this, &EditorCollection::documentFileChanged); // This is to update EditorWidget's GUI.
	connect(&AppSignals::instance(), &AppSignals::forceVSync, this, &EditorCollection::forceVSync); // Make all EditorWidgets stick to vsync.
	connect(&AppSignals::instance(), &AppSignals::newFrame, this, &EditorCollection::newFrame); // Refresh EditorWidgets.
	connect(&AppSignals::instance(), &AppSignals::functionAdded, this, &EditorCollection::functionAdded); // To update all EditorWidget's function list.
	connect(&AppSignals::instance(), &AppSignals::functionChanged, this, &EditorCollection::functionChanged); // To update all EditorWidget's function list.
	connect(&AppSignals::instance(), &AppSignals::functionRemoved, this, &EditorCollection::functionRemoved); // To update all EditorWidget's function list.
	connect(&AppSignals::instance(), &AppSignals::openClass, this, &EditorCollection::openClass); // Request to use to open a class is relayed to EditorCollection's current editor.
	connect(&AppSignals::instance(), &AppSignals::folderRemoved, this, &EditorCollection::folderRemoved); // Update EditorWidget's GUI.
	connect(&AppSignals::instance(), &AppSignals::updateSettings, this, &EditorCollection::updateSettings);

	/*
	connect(this, &EditorCollection::addClass, &AppSignals::instance(), &AppSignals::addClass); // Relayed from current editor. openClass returned to current editor.
	connect(this, &EditorCollection::importClass, &AppSignals::instance(), &AppSignals::importClass); // Relayed from current editor. openClass returned to current editor.
	connect(this, &EditorCollection::saveDocument, &AppSignals::instance(), &AppSignals::saveDocument); // Relayed from current editor, either from its save command or as a reply to our save command.
	connect(this, &EditorCollection::saveDocumentAs, &AppSignals::instance(), &AppSignals::saveDocumentAs); // Relayed from current editor as a reply from our save command.
	connect(this, &EditorCollection::showChipDialog, &AppSignals::instance(), &AppSignals::openChipDialog); // EditorWidget asked us to open a ChipDialog.
	connect(this, &EditorCollection::showClassDescriptionDialog, &AppSignals::instance(), &AppSignals::openClassDescriptionDialog); // EditorWidget asked us to open a ChipDialog.
	*/
}

EditorCollection::~EditorCollection()
{
}

EditorCollection *EditorCollection::GetInstance()
{
	static EditorCollection singleton;
	return &singleton;
}

void EditorCollection::AddEditorWidget(EditorWidget *ew)
{
	connect(this, &EditorCollection::classDirtyFlagChanged, ew, &EditorWidget::onClassNameOrDirtyFlagChanged);
	connect(this, &EditorCollection::classNameChanged, ew, &EditorWidget::onClassNameOrDirtyFlagChanged);
	connect(this, &EditorCollection::classRemoved, ew, &EditorWidget::onClassRemoved);
	connect(this, &EditorCollection::functionAdded, ew, &EditorWidget::onFunctionAdded);
	connect(this, &EditorCollection::functionChanged, ew, &EditorWidget::onFunctionChanged);
	connect(this, &EditorCollection::functionRemoved, ew, &EditorWidget::onFunctionRemoved);
	connect(this, &EditorCollection::documentFileChanged, ew, &EditorWidget::onDocumentFileChange);
	connect(this, &EditorCollection::folderRemoved, ew, &EditorWidget::onFolderRemoved);
	connect(this, &EditorCollection::forceVSync, ew, &EditorWidget::forceVSync);
	connect(this, &EditorCollection::newFrame, ew, &EditorWidget::onNewFrame);
	connect(ew, &EditorWidget::saveDocument, &AppSignals::instance(), &AppSignals::saveDocument);
	connect(ew, &EditorWidget::saveDocumentAs, &AppSignals::instance(), &AppSignals::saveDocumentAs);
	connect(ew, &EditorWidget::addClass, &AppSignals::instance(), &AppSignals::addClass);
	connect(ew, &EditorWidget::importClass, &AppSignals::instance(), &AppSignals::importClass);
	connect(ew, &EditorWidget::showChipDialog, &AppSignals::instance(), &AppSignals::openChipDialog);
	connect(ew, &EditorWidget::showClassDescriptionDialog, &AppSignals::instance(), &AppSignals::openClassDescriptionDialog);
	connect(ew, &EditorWidget::setAsCurrent, this, &EditorCollection::setSenderAsCurrentEditorWidget);
	
	// Assign a unique id to the widget for later identification (This is for embedding chip dialogs).
	unsigned id = ++editorIDs;
	_editors.insert(id, ew);
	ew->setID(id);
	ew->SetFPSProps(_showEditorFPS, _limitEditorFPS);

	if (!_currentEditor)
		SetCurrentEditorWidget(ew);
}

void EditorCollection::RemoveEditorWidget(EditorWidget *ew)
{
	disconnect(this, &EditorCollection::classDirtyFlagChanged, ew, &EditorWidget::onClassNameOrDirtyFlagChanged);
	disconnect(this, &EditorCollection::classNameChanged, ew, &EditorWidget::onClassNameOrDirtyFlagChanged);
	disconnect(this, &EditorCollection::classRemoved, ew, &EditorWidget::onClassRemoved);
	disconnect(this, &EditorCollection::functionAdded, ew, &EditorWidget::onFunctionAdded);
	disconnect(this, &EditorCollection::functionChanged, ew, &EditorWidget::onFunctionChanged);
	disconnect(this, &EditorCollection::functionRemoved, ew, &EditorWidget::onFunctionRemoved);
	disconnect(this, &EditorCollection::documentFileChanged, ew, &EditorWidget::onDocumentFileChange);
	disconnect(this, &EditorCollection::folderRemoved, ew, &EditorWidget::onFolderRemoved);
	disconnect(this, &EditorCollection::forceVSync, ew, &EditorWidget::forceVSync);
	disconnect(this, &EditorCollection::newFrame, ew, &EditorWidget::onNewFrame);
	disconnect(ew, &EditorWidget::saveDocument, &AppSignals::instance(), &AppSignals::saveDocument);
	disconnect(ew, &EditorWidget::saveDocumentAs, &AppSignals::instance(), &AppSignals::saveDocumentAs);
	disconnect(ew, &EditorWidget::addClass, &AppSignals::instance(), &AppSignals::addClass);
	disconnect(ew, &EditorWidget::importClass, &AppSignals::instance(), &AppSignals::importClass);
	disconnect(ew, &EditorWidget::showChipDialog, &AppSignals::instance(), &AppSignals::openChipDialog);
	disconnect(ew, &EditorWidget::setAsCurrent, this, &EditorCollection::setSenderAsCurrentEditorWidget);

	_editors.remove(ew->getID());
	ew->setID(0);

	if (ew == _currentEditor)
		SetCurrentEditorWidget(_editors.empty() ? nullptr : _editors.first());
	else if (_currentEditor && _editors.size() == 1)
		_currentEditor->hightlight(false);
}

void EditorCollection::SetCurrentEditorWidget(EditorWidget *ew)
{
	if (ew == _currentEditor)
		return;
	if (_currentEditor) {
		disconnect(this, &EditorCollection::openClass, _currentEditor, &EditorWidget::openClass);
		disconnect(this, &EditorCollection::openClassOnFolder, _currentEditor, &EditorWidget::openClassOnFolder);
		_currentEditor->hightlight(false);
	}

	_currentEditor = ew;

	if (_currentEditor) {
		connect(this, &EditorCollection::openClass, _currentEditor, &EditorWidget::openClass);
		connect(this, &EditorCollection::openClassOnFolder, _currentEditor, &EditorWidget::openClassOnFolder);
		if (_editors.size() > 1)
			_currentEditor->hightlight(true);
	}
}

EditorWidget *EditorCollection::GetEditorByID(unsigned id) const
{
	QMap<int, EditorWidget*>::const_iterator itr = _editors.find(id);
	if (itr == _editors.end())
		return nullptr;
	return itr.value();
}



void EditorCollection::setSenderAsCurrentEditorWidget()
{
	EditorWidget *ew = dynamic_cast<EditorWidget*>(sender());
	if (ew)
		SetCurrentEditorWidget(ew);
}

void EditorCollection::saveCurrent()
{
	if (!_currentEditor)
		return;
	Document *doc = _currentEditor->getCurrentDocument();
	if (!doc)
		return;
	AppSignals::instance().saveDocument(doc);
}

void EditorCollection::saveCurrentAs()
{
	if (!_currentEditor)
		return;
	Document *doc = _currentEditor->getCurrentDocument();
	if (!doc)
		return;
	AppSignals::instance().saveDocumentAs(doc);
}

void EditorCollection::updateSettings(QSettings& s, bool load)
{
	if (load) {
		_showEditorFPS = s.value("settings/showEditorFPS", false).toBool();
		_limitEditorFPS = s.value("settings/limitEditorFPS", true).toBool();
		for (auto n : _editors)
			n->SetFPSProps(_showEditorFPS, _limitEditorFPS);
	}
	else {
		s.setValue("settings/showEditorFPS", _showEditorFPS);
		s.setValue("settings/limitEditorFPS", _limitEditorFPS);
	}
}