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
#include "CustomDraggableTreeWidget.h"
#include "M3DEngine/GlobalDef.h"
#include "M3DCore/Path.h"
#include "M3DEngineExt/ClassExt.h"

namespace m3d
{


class LibraryTreeWidget : public CustomDraggableTreeWidget
{
	Q_OBJECT
public:
	LibraryTreeWidget(QWidget* parent = 0);
	~LibraryTreeWidget();

	void init(const class DocumentInfoCache* librariesCache);

protected:
	void updateSettings(QSettings& s, bool load);
	void onDocumentAdded(Document*);
	void onDocumentRemoved(Document*);
	void onDocumentFileChanged(Document*, const Path&);
	void onDocumentDirtyFlagChanged(Document*);
	void onClassAdded(ClassExt*);
	void onClassRemoved(ClassExt*);
	void onClassDirtyFlagChanged(ClassExt*);
	void onClassDescriptionChanged(ClassExt*);
	void onClassNameChanged(ClassExt*);
	void onFunctionAdded(Function* function);
	void onFunctionChanged(Function* function);
	void onFunctionRemoved(Function* function);
	void onClassMoved(ClassExt*, Document*, Document*);

	void contextMenuEvent(QContextMenuEvent* event) override;
	bool startDragging(QTreeWidgetItem* item) override;
	void onDragging(QTreeWidgetItem* item, const QPoint& pos, Qt::KeyboardModifiers modifiers) override;
	void endDragging(QTreeWidgetItem* item, QWidget* dropAt, const QPoint& pos, Qt::KeyboardModifiers modifiers) override;
	QCursor dragCursor() override;

	QList<QTreeWidgetItem*> _findDocumentItem(Document* doc, QTreeWidgetItem** folderItem);
	QTreeWidgetItem* _getFolder(Path p);
	QTreeWidgetItem* _getDocumentItem(class Document* doc, const Path *p = nullptr);
	QTreeWidgetItem* _getClassItem(class Class* cl, Document* doc = nullptr);
	QTreeWidgetItem* _getFunctionItem(class Function* f);
	void _setCachedDocumentHidden(class Document* doc, bool isHidden);
	void _updateFunction(QTreeWidgetItem* itm, Function* f);
	void _setTooltip(QTreeWidgetItem* item, QString txt);

	void _importItem(QTreeWidgetItem* itm);

	const class DocumentInfoCache* _librariesCache = nullptr;

	class DXEditorWidget* _ew = nullptr;

	class DocumentLoader* _loader = nullptr;
	String _clazz;
	Vector4 _extent = Vector4(0, 0, 0, 0);

	bool _hideDocs = false;
};



}