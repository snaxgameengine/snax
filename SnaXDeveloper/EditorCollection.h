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
#include <qobject.h>
#include <qmap.h>

namespace m3d
{

class EditorWidget;
class Document;
class Chip;
class ClassExt;
class Function;


class EditorCollection : public QObject
{
	Q_OBJECT
protected:
	EditorCollection();
	~EditorCollection();

	EditorWidget *_currentEditor;
	QMap<int, EditorWidget*> _editors;
	bool _showEditorFPS = false;
	bool _limitEditorFPS = true;

	void updateSettings(QSettings& s, bool load);

public:
	static EditorCollection *GetInstance();

	void AddEditorWidget(EditorWidget *ew);
	void RemoveEditorWidget(EditorWidget *ew);
	void SetCurrentEditorWidget(EditorWidget *ew);

	EditorWidget *GetCurrentEditor() { return _currentEditor; }

	EditorWidget *GetEditorByID(unsigned id) const;

public:
	void setSenderAsCurrentEditorWidget();
	void saveCurrent();
	void saveCurrentAs();


signals:
	// Relayed to all registered editors.
	void classDirtyFlagChanged(ClassExt*);
	void classNameChanged(ClassExt*);
	void classRemoved(ClassExt*);
	void functionAdded(Function*);
	void functionChanged(Function*);
	void functionRemoved(Function*);
	void documentFileChanged(Document*);
	void forceVSync(bool);
	void folderRemoved(ClassExt*, unsigned);
	void newFrame();
	// Relayed to current editor only.
	void openClass(ClassExt*, Chip*);
	void openClassOnFolder(ClassExt*, unsigned, bool);

};



}
