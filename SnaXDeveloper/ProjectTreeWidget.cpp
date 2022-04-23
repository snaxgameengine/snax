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

#include "StdAfx.h"
#include "ProjectTreeWidget.h"
#include "M3DEngine/Engine.h"
#include "M3DEngineExt/ClassExt.h"
#include "M3DEngine/Function.h"
#include "M3DEngine/Chip.h"
#include "M3DEngine/ChipManager.h"
#include "DXEditorwidget.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngine/DocumentFileTypes.h"
#include "FileIconProvider.h"
#include "M3DEngine/DocumentManager.h"
#include "M3DEngine/Document.h"
#include "M3DEngine/Environment.h"
#include <qapplication.h>
#include <QTreeWidgetItemIterator>
#include <QStringList>
#include <QHeaderView>
#include <qmouseEvent>
#include "AppSignals.h"


using namespace m3d;

#define ITEM_DATA_PATH 0, Qt::UserRole


QWidget* ProjectTreeWidget::ProjectTreeWidgetEditDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
	if (index.column() == 1)
		return nullptr;
	return QStyledItemDelegate::createEditor(parent, option, index);
}

void ProjectTreeWidget::ProjectTreeWidgetEditDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
	ProjectTreeWidget* w = dynamic_cast<ProjectTreeWidget*>(parent());
	if (w)
		w->setEditorData(editor, index);
}

void ProjectTreeWidget::ProjectTreeWidgetEditDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
	ProjectTreeWidget* w = dynamic_cast<ProjectTreeWidget*>(parent());
	if (w)
		w->setModelData(editor, model, index);
}



bool ProjectTreeWidgetItem::operator<(const QTreeWidgetItem &other) const 
{
	int i = treeWidget()->sortColumn();
	Qt::SortOrder sortOrder = treeWidget()->header()->sortIndicatorOrder();
	const ProjectTreeWidgetItem &item = (ProjectTreeWidgetItem&)other;
	if (wt != item.wt)
		return sortOrder == Qt::DescendingOrder ? (wt > item.wt) : (wt < item.wt); 
	return i == 0 ? (text(0) < other.text(0)) : (text(1) == other.text(1) ? (text(0) < other.text(0)) : (text(1) < other.text(1)));
}




ProjectTreeWidget::ProjectTreeWidget(QWidget * parent) : CustomDraggableTreeWidget(parent)
{
	header()->reset();
	headerItem()->setText(0, "Name");
	headerItem()->setText(1, "Type");
	//	header()->resizeSection(0, 200);
	header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	header()->setStretchLastSection(true);
	sortByColumn(0, Qt::AscendingOrder);
	setHeaderHidden(true);
	setSelectionBehavior(SelectionBehavior::SelectRows);

	connect(&AppSignals::instance(), &AppSignals::classAdded, this, &ProjectTreeWidget::onClassAdded);
	connect(&AppSignals::instance(), &AppSignals::classDirtyFlagChanged, this, &ProjectTreeWidget::onClassDirtyFlagChanged);
	connect(&AppSignals::instance(), &AppSignals::classDescriptionChanged, this, &ProjectTreeWidget::onClassDescriptionChanged);
	connect(&AppSignals::instance(), &AppSignals::classRenamed, this, &ProjectTreeWidget::onClassNameChanged);
	connect(&AppSignals::instance(), &AppSignals::classRemoved, this, &ProjectTreeWidget::onClassRemoved);
	connect(&AppSignals::instance(), &AppSignals::classMoved, this, &ProjectTreeWidget::onClassMoved);
	connect(&AppSignals::instance(), &AppSignals::documentFileChanged, this, &ProjectTreeWidget::onDocumentFileChanged);
	connect(&AppSignals::instance(), &AppSignals::documentAdded, this, &ProjectTreeWidget::onDocumentAdded);
	connect(&AppSignals::instance(), &AppSignals::documentRemoved, this, &ProjectTreeWidget::onDocumentRemoved);
	connect(&AppSignals::instance(), &AppSignals::documentDirtyFlagChanged, this, &ProjectTreeWidget::onDocumentDirtyFlagChanged);
	connect(&AppSignals::instance(), &AppSignals::functionAdded, this, &ProjectTreeWidget::onFunctionAdded);
	connect(&AppSignals::instance(), &AppSignals::functionChanged, this, &ProjectTreeWidget::onFunctionChanged);
	connect(&AppSignals::instance(), &AppSignals::functionRemoved, this, &ProjectTreeWidget::onFunctionRemoved);
	connect(&AppSignals::instance(), &AppSignals::startClassSet, this, &ProjectTreeWidget::rebuild);
	connect(&AppSignals::instance(), &AppSignals::updateSettings, this, &ProjectTreeWidget::updateSettings);
	connect(this, &QTreeWidget::itemDoubleClicked, this, &ProjectTreeWidget::itemDoubleClickedHandler);
	connect((QApplication*)QApplication::instance(), &QApplication::focusChanged, this, &ProjectTreeWidget::_focusChanged);
	connect(this, &QTreeWidget::currentItemChanged, this, &ProjectTreeWidget::_onCurrentItemChanged);

	_notSavedItem = new ProjectTreeWidgetItem(this, QStringList("Not Saved"), ProjectTreeWidgetItem::WT_NOTSAVED);
	_notSavedItem->setIcon(0, QIcon(":/EditorApp/Resources/folder_red.png"));
	_notSavedItem->setHidden(true);
	_notSavedItem->setExpanded(true);

	_notInProjectFolderItem = new ProjectTreeWidgetItem(this, QStringList("External"), ProjectTreeWidgetItem::WT_NOTINPROJECTFOLDER);
	_notInProjectFolderItem->setIcon(0, QIcon(":/EditorApp/Resources/folder_red.png"));
	_notInProjectFolderItem->setHidden(true); 
	_notInProjectFolderItem->setExpanded(true);

	_libraryItem = new ProjectTreeWidgetItem(this, QStringList("Library"), ProjectTreeWidgetItem::WT_LIBRARY);
	_libraryItem->setIcon(0, QIcon(":/EditorApp/Resources/open.png"));
	_libraryItem->setHidden(true);
	_libraryItem->setExpanded(true);

	// Note: icon is got from file system.
	_projectItem = new ProjectTreeWidgetItem(this, QStringList("Project"), ProjectTreeWidgetItem::WT_PROJECT);
	_projectItem->setHidden(true);
	_projectItem->setExpanded(true);

	_menu = new QMenu(this);
	_menu->addAction(ActionManager::instance().getAction(ActionID::ADD_CLASS));
	_menu->addSeparator();
	_menu->addAction(ActionManager::instance().getAction(ActionID::SAVE));
	_menu->addAction(ActionManager::instance().getAction(ActionID::SAVE_AS));
	_menu->addSeparator();
	_menu->addAction(ActionManager::instance().getAction(ActionID::DELITE));
	_renameAction = _menu->addAction("Rename...");
	_menu->addSeparator();
	_expandAction = _menu->addAction("Expand All");
	_collapseAction = _menu->addAction("Collapse All");
	_hideDocsAction = _menu->addAction("Hide Documents");
	_hideDocsAction->setCheckable(true);
	
	_hideLibsAction = _menu->addAction("Hide Libraries");
	_hideLibsAction->setCheckable(true);

	//_onCurrentItemChanged();

	_autoscrolling = true;

	setItemDelegate(new ProjectTreeWidgetEditDelegate(this));
	setEditTriggers(EditTrigger::EditKeyPressed | EditTrigger::SelectedClicked);
}

ProjectTreeWidget::~ProjectTreeWidget()
{
}

void ProjectTreeWidget::updateSettings(QSettings& s, bool load)
{
	s.beginGroup("mainWindow/" + objectName());
	if (load) {
		_hideDocs = s.value("hideDocs", false).toBool();
		_hideLibs = s.value("hideLibs", true).toBool();
		_rebuild();
	}
	else {
		s.setValue("hideDocs", _hideDocs);
		s.setValue("hideLibs", _hideLibs);
	}
	s.endGroup();
}

ProjectTreeWidgetItem *ProjectTreeWidget::_findItem(void *ptr)
{
	for (QTreeWidgetItemIterator itr(this); *itr; itr++) {
		ProjectTreeWidgetItem *item = (ProjectTreeWidgetItem*)*itr;
		if (item->ptr == ptr)
			return item;
	}
	return nullptr;
}

ProjectTreeWidgetItem *ProjectTreeWidget::_getFolder(ProjectTreeWidgetItem* rootItem, QString startFolder, QStringList folderList)
{
	ProjectTreeWidgetItem *item = rootItem;

	for (int i = 1; i < folderList.size() - 1; i++) {
		QString name = folderList[i];

		startFolder += name + "/";

		QFileInfo nfo(startFolder);
		
		QString absPath = nfo.absoluteFilePath();

		ProjectTreeWidgetItem *folder = nullptr;

		for (int i = 0; i < item->childCount(); i++, folder = nullptr) {
			folder = (ProjectTreeWidgetItem*)item->child(i);
			if (folder->wt == ProjectTreeWidgetItem::WT_FOLDER && folder->data(ITEM_DATA_PATH).toString().toLower() == absPath.toLower()) // Check absPath in case there are libraries with equal folders...
				break;
		}

		if (!folder) {
			folder = new ProjectTreeWidgetItem(item, QStringList(name));
			folder->setIcon(0, FileIconProvider::icon(nfo));
			folder->setToolTip(0, QString(absPath).replace('/', '\\'));
			folder->setData(ITEM_DATA_PATH, absPath);
		}
		item = folder;
	}

	return item;
}

void ProjectTreeWidget::_removeFolderIfEmpty(ProjectTreeWidgetItem *item)
{
	for (ProjectTreeWidgetItem *p = item, *q = 0; p->childCount() == 0; p = q) {
		if (p->wt < ProjectTreeWidgetItem::WT_FOLDER) { // Any root item?
			p->setHidden(true);
			break;
		}
		q = (ProjectTreeWidgetItem*)p->parent();
		q->removeChild(p);
		delete p;
	}
}

void ProjectTreeWidget::_addClasses(Document *doc)
{
	const ClassPtrList &classes = doc->GetClasss();
	for (size_t i = 0; i < classes.size(); i++) {
		onClassAdded((ClassExt*)classes[i]);
	}
}

void ProjectTreeWidget::_addFunctions(ClassExt *cg)
{
	const FunctionPtrSet &functions = cg->GetFunctions();
	for (const auto &n : functions) {
		onFunctionAdded(n);
	}
}

void ProjectTreeWidget::_updateFunction(Function *function, ProjectTreeWidgetItem *item)
{
	static const QIcon FUNCTION_ICONS[3] = {QIcon(":/EditorApp/Resources/static-func.png"), QIcon(":/EditorApp/Resources/nonvirt-func.png"), QIcon(":/EditorApp/Resources/virt-func.png")};

	assert(item != nullptr && item->wt == ProjectTreeWidgetItem::WT_FUNCTION);

	Chip *chip = function->GetChip();
	const ChipInfo* nfo = engine->GetChipManager()->GetChipInfo(chip->GetChipType());

	ChipEditorData *edata = chip->GetChipEditorData();
	QString comment;
	if (edata)
		comment = TOQSTRING(edata->comment);

	item->setIcon(0, FUNCTION_ICONS[(uint32)function->GetType()]);
	item->setText(0, TOQSTRING(chip->GetName()));
	item->setText(1, "    " + TOQSTRING(nfo->chipDesc.name));
	_setTooltip(item, comment);
}

void ProjectTreeWidget::contextMenuEvent( QContextMenuEvent * event )
{
	ProjectTreeWidgetItem *item = dynamic_cast<ProjectTreeWidgetItem*>(currentItem());

	_renameAction->setEnabled(item && (item->wt == ProjectTreeWidgetItem::WT_DOC && item->doc && item->doc->GetFileName().IsFile() || item->wt == ProjectTreeWidgetItem::WT_CG));
	_hideDocsAction->setChecked(_hideDocs);
	_hideLibsAction->setChecked(_hideLibs);
	QAction *a = _menu->exec(QCursor::pos());

	if (a == _renameAction) {
		editItem(item, 0);
	}
	else if (a == _hideDocsAction) {
		if (_hideDocsAction->isChecked() != _hideDocs) {
			_hideDocs = !_hideDocs;
			_rebuild();
		}
	}
	else if (a == _hideLibsAction) {
		if (_hideLibsAction->isChecked() != _hideLibs) {
			_hideLibs = !_hideLibs;
			_libraryItem->setHidden(_hideLibs || _libraryItem->childCount() == 0);
		}
	}
	else if (a == _expandAction || a == _collapseAction) {
		QTreeWidgetItem* itm = item;
		if (!itm)
			itm = invisibleRootItem();
		for (QTreeWidgetItemIterator itr(itm); *itr; itr++) {
			if (!(*itr)->isHidden())
				(*itr)->setExpanded(a == _expandAction);
		}
	}
}

bool ProjectTreeWidget::startDragging(QTreeWidgetItem *item) 
{
	ProjectTreeWidgetItem *i = dynamic_cast<ProjectTreeWidgetItem*>(item);
	if (!i)
		return false;
	if ((i->wt == ProjectTreeWidgetItem::WT_DOC && i->doc->GetFileName().IsFile()) || i->wt == ProjectTreeWidgetItem::WT_CG || i->wt == ProjectTreeWidgetItem::WT_FUNCTION)
		return true;
	return false; 
}

void ProjectTreeWidget::onDragging(QTreeWidgetItem* item, const QPoint& pos, Qt::KeyboardModifiers modifiers)
{
	ProjectTreeWidgetItem* i = dynamic_cast<ProjectTreeWidgetItem*>(item);
	if (!i)
		return;

	if (QApplication::widgetAt(QCursor::pos()) == viewport()) {
		QApplication::changeOverrideCursor(dragCursor());
	}
}

void ProjectTreeWidget::endDragging(QTreeWidgetItem *item, QWidget *dropAt, const QPoint &pos, Qt::KeyboardModifiers modifiers) 
{
	ProjectTreeWidgetItem *i = dynamic_cast<ProjectTreeWidgetItem*>(item);
	if (!i)
		return;
	if (i->wt == ProjectTreeWidgetItem::WT_DOC) {
		if (dropAt == viewport()) { // Moving class from one doc to another?
			if (_hideDocs)
				return; // Not available in this case!
		}
		ProjectTreeWidgetItem* dropItem = dynamic_cast<ProjectTreeWidgetItem*>(itemAt(viewport()->mapFromGlobal(pos)));
		if (dropItem) {
			Path oldPath = i->doc->GetFileName(), newDir;
			if (dropItem->wt == ProjectTreeWidgetItem::WT_PROJECT) {
				Class* clazzStart = engine->GetClassManager()->GetStartClass();
				Document* start = clazzStart ? clazzStart->GetDocument() : nullptr;
				if (!start)
					return;
				newDir = start->GetFileName().GetDirectory();
			}
			else if (dropItem->wt == ProjectTreeWidgetItem::WT_FOLDER) {
				newDir = Path::Dir(FROMQSTRING(dropItem->data(ITEM_DATA_PATH).toString()));
			}
			else
				return;
			if (!newDir.IsDirectory())
				return;
			Path newPath = Path::File(oldPath.GetName(), newDir);
			engine->GetDocumentManager()->RenameDocument(i->doc, newPath);
		}
	}
	else if (i->wt == ProjectTreeWidgetItem::WT_CG) {
		if (dropAt == viewport()) { // Moving class from one doc to another?
			if (_hideDocs)
				return; // Not available in this case!
			ProjectTreeWidgetItem *dropItem = dynamic_cast<ProjectTreeWidgetItem*>(itemAt(viewport()->mapFromGlobal(pos)));
			if (dropItem) {
				Document *newDoc = nullptr;
				if (dropItem->wt == ProjectTreeWidgetItem::WT_DOC)
					newDoc = dropItem->doc;
				ClassExt * clazz = i->clazz;
				Document *oldDoc = clazz->GetDocument();
				if (newDoc && newDoc != oldDoc) {
					if (newDoc->MoveClass(clazz)) {
						oldDoc->SetDirty();
						newDoc->SetDirty();
						clazz->SetDirty();
						clazz->SetRelationsDirty(DR_DERIVED | DR_FUNCTION_CALLS | DR_INSTANCES | DR_CG);
						//engine->GetClassManager()->GetStartClass() == clazz
					}
				}
			}
		}
		else {
			DXEditorWidget *ew = dynamic_cast<DXEditorWidget*>(dropAt);
			if (!ew)
				return;
			ew->addClassChip(i->clazz->GetID());
		}
	}
	else if (i->wt == ProjectTreeWidgetItem::WT_FUNCTION) {
		DXEditorWidget *ew = dynamic_cast<DXEditorWidget*>(dropAt);
		if (!ew)
			return;
		ew->addFunctionCallChip(i->func->GetChip()->GetClass()->GetID(), i->func->GetChip()->GetID(), modifiers & Qt::ShiftModifier);
	}
}

QCursor ProjectTreeWidget::dragCursor()
{
	static const QCursor stdCursor = QCursor(Qt::ArrowCursor);
	static const QCursor moveCursor = QCursor(Qt::DragMoveCursor);
	static const QCursor forbiddenCursor = QCursor(Qt::ForbiddenCursor);
	static const QCursor chipCursor(QCursor(QIcon(":/EditorApp/Resources/chip.png").pixmap(32)));
	static const QCursor classCursor(QCursor(QIcon(":/EditorApp/Resources/graph.png").pixmap(32)));
	ProjectTreeWidgetItem *draggedItem = dynamic_cast<ProjectTreeWidgetItem*>(_draggedItem);
	if (draggedItem->wt == ProjectTreeWidgetItem::WT_DOC) {
		if (_currentHoverWidget == viewport()) {
			ProjectTreeWidgetItem* hoverItem = dynamic_cast<ProjectTreeWidgetItem*>(itemAt(viewport()->mapFromGlobal(QCursor::pos())));
			if (hoverItem) {
				if (hoverItem == draggedItem)
					return moveCursor;
				else if ((hoverItem->wt == ProjectTreeWidgetItem::WT_PROJECT || hoverItem->wt == ProjectTreeWidgetItem::WT_FOLDER) && draggedItem->parent() != hoverItem)
					return moveCursor;
				else
					return forbiddenCursor;
			}
		}
		return forbiddenCursor;
	}
	if (draggedItem->wt == ProjectTreeWidgetItem::WT_CG) {
		if (_currentHoverWidget == viewport()) {
			ProjectTreeWidgetItem* hoverItem = dynamic_cast<ProjectTreeWidgetItem*>(itemAt(viewport()->mapFromGlobal(QCursor::pos())));
			if (hoverItem) {
				if (hoverItem == draggedItem)
					return classCursor;
				else if (hoverItem->wt == ProjectTreeWidgetItem::WT_DOC && draggedItem->parent() != hoverItem)
					return moveCursor;
				else
					return forbiddenCursor;
			}
		}
		return classCursor;
	}
	if (draggedItem->wt == ProjectTreeWidgetItem::WT_FUNCTION) {
		return chipCursor;
	}
	return forbiddenCursor;
}


ActionMask ProjectTreeWidget::getActions()
{
	return ActionMask(ActionID::SAVE) | ActionMask(ActionID::SAVE_AS) | ActionMask(ActionID::ADD_CLASS) | ActionMask(ActionID::DELITE);
}

bool ProjectTreeWidget::isActionEnabled(ActionID action) 
{
	if (action == ActionID::ADD_CLASS)
		return true;
	if (action == ActionID::SAVE || action == ActionID::SAVE_AS || action == ActionID::DELITE) {
		ProjectTreeWidgetItem *item = dynamic_cast<ProjectTreeWidgetItem*>(currentItem());
		return item && (item->wt == ProjectTreeWidgetItem::WT_DOC || item->wt == ProjectTreeWidgetItem::WT_CG);
	}
	return false;
}

void ProjectTreeWidget::triggered(ActionID action) 
{
	switch (action)
	{
	case ActionID::ADD_CLASS:
		return AppSignals::instance().addClass();
	case ActionID::SAVE:
		return save();
	case ActionID::SAVE_AS:
		return saveAs();
	case ActionID::DELITE:
		return delite();
	}
}


void ProjectTreeWidget::_rebuild()
{
	for (int i = 0; i < topLevelItemCount(); i++)
		qDeleteAll(topLevelItem(i)->takeChildren());

	_notSavedItem->setHidden(true);
	_notInProjectFolderItem->setHidden(true);
	_libraryItem->setHidden(true);
	_projectItem->setHidden(true);

	const DocumentPtrSet &m = engine->GetDocumentManager()->GetDocuments();
	for (const auto &n : m) {
		onDocumentAdded(n);
		const ClassPtrList & clazzList = n->GetClasss();
		for (size_t i = 0; i < clazzList.size(); i++)
			onClassAdded((ClassExt*)clazzList[i]);
	}
}

bool ProjectTreeWidget::_isItemInsideLibrary(QTreeWidgetItem* item)
{
	for (;item; item = item->parent())
		if (item == _libraryItem)
			return true;
	return false;
}

// *****
// Slots
// *****

void ProjectTreeWidget::onDocumentAdded(Document *doc)
{
	if (_hideDocs)
		return; // We do not add document items in this case!

	Environment *dm = engine->GetEnvironment();

	if (doc->GetFileName().IsFile()) {
		const DocumentFileTypes::FileTypeDesc *d = DocumentFileTypes::GetFileType(doc->GetFileName());
		
		QIcon icon = FileIconProvider::icon(QFileInfo(TOQSTRING(doc->GetFileName().AsString())));

		if (dm->IsPathInsideProjectRootFolder(doc->GetFileName())) {
			QStringList folders = TOQSTRING(doc->GetFileName().GetDirectory().GetRelativePath(dm->GetProjectRootFolder()).AsString()).split("\\");

			ProjectTreeWidgetItem *p = _getFolder(_projectItem, TOQSTRING(dm->GetProjectRootFolder().AsString()), folders);
			ProjectTreeWidgetItem *i = new ProjectTreeWidgetItem(p, QStringList(TOQSTRING(doc->GetFileName().GetFileNameWithoutExtention())), doc);
			i->setIcon(0, icon);
			i->setForeground(0, doc->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
			i->setData(ITEM_DATA_PATH, TOQSTRING(doc->GetFileName().AsString()));
			i->setToolTip(0, TOQSTRING(doc->GetFileName().AsString()));
			i->setFlags(i->flags() | Qt::ItemFlag::ItemIsEditable);
			if (_projectItem->isHidden()) {
				_projectItem->setHidden(false);
				_projectItem->setIcon(0, FileIconProvider::icon(QFileInfo(TOQSTRING(dm->GetProjectRootFolder().AsString()))));
				_projectItem->setToolTip(0, TOQSTRING(dm->GetProjectRootFolder().AsString()));
			}
		}
		else if (dm->IsPathInsideLibraryFolder(doc->GetFileName())) {
			QStringList folders = TOQSTRING(dm->CreateLibraryPath(doc->GetFileName().GetDirectory())).split("\\");

			ProjectTreeWidgetItem* p = _getFolder(_libraryItem, TOQSTRING(dm->GetLibraryFolder(doc->GetFileName()).AsString()), folders);
			ProjectTreeWidgetItem* i = new ProjectTreeWidgetItem(p, QStringList(TOQSTRING(doc->GetFileName().GetFileNameWithoutExtention())), doc);
			i->setIcon(0, icon);
			i->setForeground(0, doc->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
			i->setData(ITEM_DATA_PATH, TOQSTRING(doc->GetFileName().AsString()));
			i->setToolTip(0, TOQSTRING(doc->GetFileName().AsString()));
			i->setFlags(i->flags() | Qt::ItemFlag::ItemIsEditable);
			if (_libraryItem->isHidden()) {
				_libraryItem->setHidden(_hideLibs);
			}
		}
		else {
			ProjectTreeWidgetItem *i = new ProjectTreeWidgetItem(_notInProjectFolderItem, QStringList(TOQSTRING(doc->GetFileName().GetFileNameWithoutExtention())), doc);
			i->setIcon(0, icon);
			i->setForeground(0, doc->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
			i->setData(ITEM_DATA_PATH, TOQSTRING(doc->GetFileName().AsString()));
			i->setToolTip(0, TOQSTRING(doc->GetFileName().AsString()));
			i->setFlags(i->flags() | Qt::ItemFlag::ItemIsEditable);
			if (_notInProjectFolderItem->isHidden()) {
				_notInProjectFolderItem->setHidden(false);
			}
		}
	}
	else {
		ProjectTreeWidgetItem *i = new ProjectTreeWidgetItem(_notSavedItem, QStringList("Not Saved"), doc);
		i->setIcon(0, QIcon(":/EditorApp/Resources/unknown-file.png"));
		i->setForeground(0, doc->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
		i->setToolTip(0, QString("Not Saved"));
		if (_notSavedItem->isHidden()) {
			_notSavedItem->setHidden(false);
		}
	}
}

void ProjectTreeWidget::onDocumentRemoved(Document *doc) // TODO: What if start doc?!
{
	ProjectTreeWidgetItem *item = _findItem(doc);
	if (item) {
		ProjectTreeWidgetItem *p = (ProjectTreeWidgetItem*)item->parent();
		p->takeChild(p->indexOfChild(item));
		delete item;
		_removeFolderIfEmpty(p);
	}
}

void ProjectTreeWidget::onDocumentFileChanged(Document *doc, const Path&)
{
	if (_hideDocs)
		return;

	Class *clazzStart = engine->GetClassManager()->GetStartClass();
	Document *start = clazzStart ? clazzStart->GetDocument() : nullptr;

	Environment* dm = engine->GetEnvironment();

	ProjectTreeWidgetItem *item = _findItem(doc);
	if (!item)
		return;

	ProjectTreeWidgetItem *oldParentItem = (ProjectTreeWidgetItem*)item->parent();

	if (doc->GetFileName().IsFile()) {
		if (dm->IsPathInsideProjectRootFolder(doc->GetFileName())) {
			QStringList folders = TOQSTRING(doc->GetFileName().GetDirectory().GetRelativePath(dm->GetProjectRootFolder()).AsString()).split("\\");

			ProjectTreeWidgetItem *p = _getFolder(_projectItem, TOQSTRING(dm->GetProjectRootFolder().AsString()), folders);
			if (p != oldParentItem) {
				if (doc == start)
					return _rebuild();
				oldParentItem->takeChild(oldParentItem->indexOfChild(item));
				p->addChild(item);
			}
			// else no change!
		}
		else if (dm->IsPathInsideLibraryFolder(doc->GetFileName())) {
			QStringList folders = TOQSTRING(dm->CreateLibraryPath(doc->GetFileName().GetDirectory())).split("\\");

			ProjectTreeWidgetItem* p = _getFolder(_libraryItem, TOQSTRING(dm->GetLibraryFolder(doc->GetFileName()).AsString()), folders);
			if (p != oldParentItem) {
				if (doc == start)
					return _rebuild();
				oldParentItem->takeChild(oldParentItem->indexOfChild(item));
				p->addChild(item);
				_libraryItem->setHidden(false);
			}
			// else no change!
		}
		else {
			if (oldParentItem != _notInProjectFolderItem) {
				if (doc == start)
					return _rebuild();
				oldParentItem->takeChild(oldParentItem->indexOfChild(item));
				_notInProjectFolderItem->addChild(item);
				_notInProjectFolderItem->setHidden(false);
			}
			// else no change!
		}

		item->setData(ITEM_DATA_PATH, TOQSTRING(doc->GetFileName().AsString()));
		item->setToolTip(0, TOQSTRING(doc->GetFileName().AsString()));
		item->setText(0, TOQSTRING(doc->GetFileName().GetFileNameWithoutExtention()));
		item->setIcon(0, FileIconProvider::icon(QFileInfo(TOQSTRING(doc->GetFileName().AsString()))));
		item->setFlags(item->flags() | Qt::ItemFlag::ItemIsEditable);
	}
	else {
		if (oldParentItem != _notSavedItem) {
			if (doc == start)
				return _rebuild();
			oldParentItem->takeChild(oldParentItem->indexOfChild(item));
			_notSavedItem->addChild(item);
			_notSavedItem->setHidden(false);
			item->setData(0, Qt::ToolTipRole, "Not Saved");
			item->setData(ITEM_DATA_PATH, QVariant());
			item->setText(0, "Not Saved");
			item->setData(0, Qt::DecorationRole, QIcon(":/EditorApp/Resources/unknown-file.png"));
		}
		// else no change!

		item->setFlags(item->flags() & ~Qt::ItemFlag::ItemIsEditable);
	}

	// Remove empty folders!
	_removeFolderIfEmpty(oldParentItem);
}

void ProjectTreeWidget::onDocumentDirtyFlagChanged(Document *doc)
{
	ProjectTreeWidgetItem *item = _findItem(doc);
	if (item) {
		item->setForeground(0, doc->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
		if (!doc->IsDirty() && item->parent() != _notSavedItem) { // doc not dirty: just saved! Icon overlay updated?
			for (ProjectTreeWidgetItem *itm = item; itm && itm != _notInProjectFolderItem && itm != _libraryItem; itm = (ProjectTreeWidgetItem*)itm->parent())
				itm->setIcon(0, FileIconProvider::icon(QFileInfo(itm->data(0, Qt::ToolTipRole).toString())));
		}
	}
}

void ProjectTreeWidget::onClassAdded(ClassExt *cg)
{
	static const QIcon ICON = QIcon(":/EditorApp/Resources/graph.png");

	if (_hideDocs) { // In this case, cgs are added to folders. Not documents!

		Document *doc = cg->GetDocument();
		Environment* dm = engine->GetEnvironment();

		if (doc->GetFileName().IsFile()) {
			if (dm->IsPathInsideProjectRootFolder(doc->GetFileName())) {
				QStringList folders = TOQSTRING(doc->GetFileName().GetDirectory().GetRelativePath(dm->GetProjectRootFolder()).AsString()).split("\\");

				ProjectTreeWidgetItem* p = _getFolder(_projectItem, TOQSTRING(dm->GetProjectRootFolder().AsString()), folders);
				ProjectTreeWidgetItem *i = new ProjectTreeWidgetItem(p, QStringList(TOQSTRING(cg->GetName())), cg);
				i->setIcon(0, ICON);
				i->setForeground(0, cg->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
				i->setData(ITEM_DATA_PATH, TOQSTRING(doc->GetFileName().AsString()));
				i->setFlags(i->flags() | Qt::ItemFlag::ItemIsEditable);
				_setTooltip(i, cg);
				if (_projectItem->isHidden()) {
					_projectItem->setHidden(false);
					_projectItem->setIcon(0, FileIconProvider::icon(QFileInfo(TOQSTRING(dm->GetProjectRootFolder().AsString()))));
					_projectItem->setToolTip(0, TOQSTRING(dm->GetProjectRootFolder().AsString()));
				}
			}
			else if (dm->IsPathInsideLibraryFolder(doc->GetFileName())) {
				QStringList folders = TOQSTRING(dm->CreateLibraryPath(doc->GetFileName().GetDirectory())).split("\\");

				ProjectTreeWidgetItem* p = _getFolder(_libraryItem, TOQSTRING(dm->GetLibraryFolder(doc->GetFileName()).AsString()), folders);
				ProjectTreeWidgetItem* i = new ProjectTreeWidgetItem(p, QStringList(TOQSTRING(cg->GetName())), cg);
				i->setIcon(0, ICON);
				i->setForeground(0, cg->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
				i->setData(ITEM_DATA_PATH, TOQSTRING(doc->GetFileName().AsString()));
				i->setFlags(i->flags() | Qt::ItemFlag::ItemIsEditable);
				_setTooltip(i, cg);
				if (_libraryItem->isHidden()) {
					_libraryItem->setHidden(_hideLibs);
				}
			}
			else {
				ProjectTreeWidgetItem *i = new ProjectTreeWidgetItem(_notInProjectFolderItem, QStringList(TOQSTRING(cg->GetName())), cg);
				i->setIcon(0, ICON);
				i->setForeground(0, cg->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
				i->setData(ITEM_DATA_PATH, TOQSTRING(doc->GetFileName().AsString()));
				i->setFlags(i->flags() | Qt::ItemFlag::ItemIsEditable);
				_setTooltip(i, cg);
				_notInProjectFolderItem->setHidden(false);
			}
		}
		else {
			ProjectTreeWidgetItem *i = new ProjectTreeWidgetItem(_notSavedItem,QStringList(TOQSTRING(cg->GetName())), cg);
			i->setIcon(0, ICON);
			i->setForeground(0, cg->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
			_setTooltip(i, cg);
			i->setFlags(i->flags() | Qt::ItemFlag::ItemIsEditable);
			_notSavedItem->setHidden(false);
		}
		_addFunctions(cg);
	}
	else { // Find parent document and add!
		ProjectTreeWidgetItem *docItem = _findItem(cg->GetDocument());
		if (!docItem)
			return;
		ProjectTreeWidgetItem *i = new ProjectTreeWidgetItem(docItem, QStringList(TOQSTRING(cg->GetName())), cg);
		i->setIcon(0, ICON);
		i->setForeground(0, cg->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
		// No ITEM_DATA_PATH here!
		_setTooltip(i, cg);
		i->setFlags(i->flags() | Qt::ItemFlag::ItemIsEditable);
		_addFunctions(cg);
	}
}

void ProjectTreeWidget::onClassRemoved(ClassExt *cg)
{
	ProjectTreeWidgetItem *item = _findItem(cg);
	if (item) {
		ProjectTreeWidgetItem *p = (ProjectTreeWidgetItem*)item->parent();
		p->takeChild(p->indexOfChild(item));
		delete item;
		if (_hideDocs)
			_removeFolderIfEmpty(p); // Parent is a folder in this case!
	}
}

void ProjectTreeWidget::onClassNameChanged(ClassExt *cg)
{
	ProjectTreeWidgetItem *item = _findItem(cg);
	if (item) {
		item->setText(0, TOQSTRING(cg->GetName()));
		_setTooltip(item, cg);
	}
}

void ProjectTreeWidget::onClassDirtyFlagChanged(ClassExt *cg)
{
	ProjectTreeWidgetItem *item = _findItem(cg);
	if (item) {
		item->setForeground(0, cg->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
		if (_hideDocs && !cg->IsDirty() && item->parent() != _notSavedItem) { // doc not dirty: just saved! Icon overlay updated?
			for (ProjectTreeWidgetItem *itm = (ProjectTreeWidgetItem*)item->parent(); itm && itm != _notInProjectFolderItem && itm != _libraryItem; itm = (ProjectTreeWidgetItem*)itm->parent())
				itm->setIcon(0, FileIconProvider::icon(QFileInfo(itm->data(ITEM_DATA_PATH).toString())));
		}
	}
}

void ProjectTreeWidget::onClassDescriptionChanged(ClassExt*clazz)
{
	ProjectTreeWidgetItem* item = _findItem(clazz);
	if (item) {
		_setTooltip(item, clazz);
	}
}

void ProjectTreeWidget::_setTooltip(QTreeWidgetItem* item, ClassExt* clazz)
{
	QString desc = TOQSTRING(clazz->GetDescription());
	if (desc.isEmpty())
		desc = TOQSTRING(clazz->GetName());
	_setTooltip(item, desc);
}

void ProjectTreeWidget::_setTooltip(QTreeWidgetItem* item, QString txt)
{
	if (txt.length() > 250)
		txt = txt.left(247) + "...";
	if (txt.isEmpty())
		item->setToolTip(0, QString());
	else
		item->setToolTip(0, "<font>" + txt.toHtmlEscaped() + "</font>");
}

void ProjectTreeWidget::onFunctionAdded(Function *function)
{
	Chip *chip = function->GetChip();
	Class *cg = chip->GetClass();

	ProjectTreeWidgetItem *cgItem = _findItem(cg);
	if (!cgItem)
		return;

	ProjectTreeWidgetItem *i = new ProjectTreeWidgetItem(cgItem, QStringList(TOQSTRING(chip->GetName())), function);

	_updateFunction(function, i);
}

void ProjectTreeWidget::onFunctionChanged(Function *function)
{
	ProjectTreeWidgetItem *i = _findItem(function);
	if (i) {
		_updateFunction(function, i);
	}
}

void ProjectTreeWidget::onFunctionRemoved(Function *function)
{
	ProjectTreeWidgetItem *i = _findItem(function);
	if (i) {
		delete i;
	}
}

void ProjectTreeWidget::itemDoubleClickedHandler(QTreeWidgetItem * item, int column)
{
	if (!item)
		return;

	ProjectTreeWidgetItem *i = (ProjectTreeWidgetItem*)item;

	if (i->wt == ProjectTreeWidgetItem::WT_CG) {
		ClassExt *cg = i->clazz;
		if (!cg)
			return;
		emit AppSignals::instance().openClass(cg, nullptr);
	}
	else if (i->wt == ProjectTreeWidgetItem::WT_FUNCTION) {
		Chip *ch = i->func->GetChip();
		if (!ch)
			return;
		Class *cg = ch->GetClass();
		if (ch)
			emit AppSignals::instance().openClass((ClassExt*)cg, ch);
	}
}

void ProjectTreeWidget::rebuild()
{
	_rebuild();
}

void ProjectTreeWidget::onClassMoved(ClassExt*cg,Document*oldDoc,Document*newDoc)
{
	onClassRemoved(cg);
	onClassAdded(cg);
}

void ProjectTreeWidget::save()
{
	ProjectTreeWidgetItem *item = dynamic_cast<ProjectTreeWidgetItem*>(currentItem());
	if (item) {
		if (item->wt == ProjectTreeWidgetItem::WT_DOC)
			emit AppSignals::instance().saveDocument(item->doc);
		else if (item->wt == ProjectTreeWidgetItem::WT_CG)
			emit AppSignals::instance().saveDocument(item->clazz->GetDocument());
	}
}

void ProjectTreeWidget::saveAs()
{
	ProjectTreeWidgetItem *item = dynamic_cast<ProjectTreeWidgetItem*>(currentItem());
	if (item) {
		if (item->wt == ProjectTreeWidgetItem::WT_DOC)
			emit AppSignals::instance().saveDocumentAs(item->doc);
		else if (item->wt == ProjectTreeWidgetItem::WT_CG)
			emit AppSignals::instance().saveDocumentAs(item->clazz->GetDocument());
	}
}

void ProjectTreeWidget::delite()
{
	ProjectTreeWidgetItem *item = dynamic_cast<ProjectTreeWidgetItem*>(currentItem());
	if (item) {
		if (item->wt == ProjectTreeWidgetItem::WT_DOC)
			emit AppSignals::instance().deleteDocument(item->doc);
		else if (item->wt == ProjectTreeWidgetItem::WT_CG)
			emit AppSignals::instance().deleteClass(item->clazz);
	}
}

void ProjectTreeWidget::_focusChanged(QWidget *old, QWidget *now)
{
	if (!old && now) {
		for (QTreeWidgetItemIterator itr(this); *itr; itr++) {
			ProjectTreeWidgetItem *item = (ProjectTreeWidgetItem*)*itr;
			if (item->wt == ProjectTreeWidgetItem::WT_FOLDER || item->wt == ProjectTreeWidgetItem::WT_DOC && item->parent() != _notSavedItem || item->wt == ProjectTreeWidgetItem::WT_PROJECT && !item->isHidden()) {
				item->setIcon(0, FileIconProvider::icon(QFileInfo(item->data(ITEM_DATA_PATH).toString()))); // TODO: NOT CORRECT!!!
			}
		}
	}
}

void ProjectTreeWidget::_onCurrentItemChanged()
{
	ActionManager::instance().updateAction(ActionID::SAVE);
	ActionManager::instance().updateAction(ActionID::SAVE_AS);
	ActionManager::instance().updateAction(ActionID::DELITE);
}

void ProjectTreeWidget::setEditorData(QWidget* editor, const QModelIndex& index)
{
	QLineEdit* e = dynamic_cast<QLineEdit*>(editor);
	if (!e)
		return;
	ProjectTreeWidgetItem *item = dynamic_cast<ProjectTreeWidgetItem*>(itemFromIndex(index));
	if (!item)
		return;

	if (item->wt == ProjectTreeWidgetItem::WT_FOLDER) {
		e->setText(item->text(0));
	}
	else if (item->wt == ProjectTreeWidgetItem::WT_DOC) {
		if (item->doc->GetFileName().IsFile())
			e->setText(TOQSTRING(item->doc->GetFileName().GetFileNameWithoutExtention()));
	}
	else if (item->wt == ProjectTreeWidgetItem::WT_CG) {
		e->setText(TOQSTRING(item->clazz->GetName()));
	}
}

void ProjectTreeWidget::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index)
{
	QLineEdit* e = dynamic_cast<QLineEdit*>(editor);
	if (!e)
		return;
	QString txt = e->text().trimmed();
	ProjectTreeWidgetItem* item = dynamic_cast<ProjectTreeWidgetItem*>(itemFromIndex(index));
	if (!item)
		return;

	if (item->wt == ProjectTreeWidgetItem::WT_FOLDER) {
		// TODO
	}
	else if (item->wt == ProjectTreeWidgetItem::WT_DOC) {
		closePersistentEditor(item, 0);
		static const String inv = MTEXT("|*?<>:\"\\/");
		if (FROMQSTRING(txt).find_first_of(inv) != String::npos) {
			QMessageBox::information(this, "Rename Failed", "The text contains invalid characters.");
			edit(index);
			return;
		}

		Path oldPath = item->doc->GetFileName();
		Path newPath = oldPath.ChangeBaseName(FROMQSTRING(txt));
		if (oldPath == newPath)
			return;
		if (newPath.CheckExistence()) {
			QMessageBox::information(this, "Rename Failed", "A file with the given name already exist!");
			edit(index);
		}
		else {
			if (!engine->GetDocumentManager()->RenameDocument(item->doc, newPath)) {
				QMessageBox::information(this, "Rename Failed", "Failed to rename file!");
				edit(index);
			}
		}
	}
	else if (item->wt == ProjectTreeWidgetItem::WT_CG) {
		if (engine->GetClassManager()->SetClassName(item->clazz, FROMQSTRING(txt))) {
			item->clazz->SetDirty();
			item->clazz->SetRelationsDirty(DR_FUNCTION_CALLS);
		}
		else {
			QMessageBox::information(this, "Invalid Class Name", "Class name is not valid or already taken!");
			closePersistentEditor(item, 0);
			edit(index);
		}

	}
}

