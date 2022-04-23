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
#include "CustomDraggabletreeWidget.h"
#include "M3DCore/MString.h"
#include "M3DEngine/GlobalDef.h"
#include "ActionManager.h"
#include <qthread.h>
#include <qmutex.h>
#include <qstyleditemdelegate>


namespace m3d
{

class Chip;
class ClassExt;

class ProjectTreeWidgetItem : public QTreeWidgetItem
{
public:
	enum WType {WT_NOTSAVED, WT_NOTINPROJECTFOLDER, WT_LIBRARY, WT_PROJECT, WT_FOLDER, WT_DOC, WT_CG, WT_FUNCTION};

	WType wt;
	union
	{
		void *ptr;
		Document *doc;
		ClassExt *clazz;
		Function *func;
	};


	// Root constructor
	ProjectTreeWidgetItem(QTreeWidget *view, const QStringList &strings, WType wt) : QTreeWidgetItem(view, strings, Type), wt(wt), ptr(nullptr) {}

	// Folder constructor
	ProjectTreeWidgetItem(QTreeWidgetItem *view, const QStringList &strings) : QTreeWidgetItem(view, strings, Type), wt(WT_FOLDER), ptr(nullptr) {}

	// Document constructor
	ProjectTreeWidgetItem(QTreeWidgetItem *widget, const QStringList &strings, Document *doc) : QTreeWidgetItem(widget, strings, Type), wt(WT_DOC), doc(doc) {}

	// Class constructor
	ProjectTreeWidgetItem(QTreeWidgetItem *widget, const QStringList &strings, ClassExt *clazz) : QTreeWidgetItem(widget, strings, Type), wt(WT_CG), clazz(clazz) {}

	// Function constructor
	ProjectTreeWidgetItem(QTreeWidgetItem *widget, const QStringList &strings, Function *func) : QTreeWidgetItem(widget, strings, Type), wt(WT_FUNCTION), func(func) {}

	~ProjectTreeWidgetItem() {}

	virtual bool operator<(const QTreeWidgetItem &other) const;

};


class ProjectTreeWidget : public CustomDraggableTreeWidget, public ActionListener
{
	Q_OBJECT
public:
	ProjectTreeWidget(QWidget * parent = 0);
	~ProjectTreeWidget();

public:
	void updateSettings(QSettings& s, bool load);
	void onDocumentAdded(Document*);
	void onDocumentRemoved(Document*);
	void onDocumentFileChanged(Document*,const Path&);
	void onDocumentDirtyFlagChanged(Document*);
	void onClassAdded(ClassExt*);
	void onClassRemoved(ClassExt*);
	void onClassDirtyFlagChanged(ClassExt*);
	void onClassDescriptionChanged(ClassExt*);
	void onClassNameChanged(ClassExt*);
	void onFunctionAdded(Function *function);
	void onFunctionChanged(Function *function);
	void onFunctionRemoved(Function *function);
	void onClassMoved(ClassExt*,Document*,Document*);

	void itemDoubleClickedHandler(QTreeWidgetItem *item, int column);
	void rebuild();

	void save();
	void saveAs();
	void delite();


protected:
	ProjectTreeWidgetItem *_notSavedItem = nullptr;
	ProjectTreeWidgetItem *_notInProjectFolderItem = nullptr;
	ProjectTreeWidgetItem* _libraryItem = nullptr;
	ProjectTreeWidgetItem *_projectItem = nullptr;
	bool _hideDocs = false;
	bool _hideLibs = true;

	QMenu *_menu = nullptr;
	QAction *_renameAction = nullptr;
	QAction *_hideDocsAction = nullptr;
	QAction* _hideLibsAction = nullptr;
	QAction* _expandAction = nullptr;
	QAction* _collapseAction = nullptr;

	ProjectTreeWidgetItem *_findItem(void *ptr);
	ProjectTreeWidgetItem *_getFolder(ProjectTreeWidgetItem* rootItem, QString startFolder, QStringList folderList);
	void _removeFolderIfEmpty(ProjectTreeWidgetItem *item);
	void _addClasses(Document *doc);
	void _addFunctions(ClassExt *clazz);
	void _updateFunction(Function *function, ProjectTreeWidgetItem *item);
	void _rebuild();
	bool _isItemInsideLibrary(QTreeWidgetItem* item);
	void _setTooltip(QTreeWidgetItem* item, ClassExt* clazz);
	void _setTooltip(QTreeWidgetItem* item, QString txt);

	virtual void contextMenuEvent(QContextMenuEvent * event) override;
	virtual bool startDragging(QTreeWidgetItem *item) override;
	virtual void onDragging(QTreeWidgetItem* item, const QPoint& pos, Qt::KeyboardModifiers modifiers) override;
	virtual void endDragging(QTreeWidgetItem *item, QWidget *dropAt, const QPoint &pos, Qt::KeyboardModifiers modifiers) override;
	virtual QCursor dragCursor() override;

	ActionMask getActions() override;
	bool isActionEnabled(ActionID action) override;
	void triggered(ActionID action) override;

	void setEditorData(QWidget* editor, const QModelIndex& index);
	void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index);

	class ProjectTreeWidgetEditDelegate : public QStyledItemDelegate {
	public:
		ProjectTreeWidgetEditDelegate(QObject* parent = 0) : QStyledItemDelegate(parent) {}

		QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
		void setEditorData(QWidget* editor, const QModelIndex& index) const override;
		void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
	};

private:
	void _focusChanged(QWidget *old, QWidget *now);
	void _onCurrentItemChanged();
};

}