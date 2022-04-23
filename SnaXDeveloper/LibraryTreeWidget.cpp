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
#include "LibraryTreeWidget.h"
#include "DXEditorWidget.h"
#include <QMouseEvent>
#include <qheaderview>
#include <qapplication>
#include <QMenu>
#include <QAction>
#include <QActionGroup>
#include "M3DEngine/ChipManager.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/DocumentLoader.h"
#include "M3DEngine/DocumentSaver.h"
#include "M3DEngine/DocumentFileTypes.h"
#include "M3DEngine/Document.h"
#include "M3DEngine/Environment.h"
#include <qsettings.h>
#include "M3DEngineExt/DocumentExtLoader.h"
#include "DocumentInfoCache.h"
#include "AppSignals.h"
#include "FileIconProvider.h"
#include "M3DEngine/Chip.h"
#include "M3DEngineExt/ClassExt.h"
#include "M3DEngine/DocumentManager.h"


using namespace m3d;


struct ItemTemp
{
	int type;
	QString path;
	QString name;
	qulonglong ptr;
};

inline bool operator==(const ItemTemp& e1, const ItemTemp& e2)
{
	return e1.type == e2.type && e1.path == e2.path && e1.name == e2.name && e1.ptr == e2.ptr;
}

inline uint qHash(const ItemTemp& key, uint seed)
{
	return qHash(key.ptr, qHash(key.name, qHash(key.path, qHash(key.type, seed))));
}


LibraryTreeWidget::LibraryTreeWidget(QWidget* parent) : CustomDraggableTreeWidget(parent)
{
	header()->reset();
	headerItem()->setText(0, "Name");
	headerItem()->setText(1, "Type");
//	header()->resizeSection(0, 200);
	header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	header()->setStretchLastSection(true);
	sortByColumn(0, Qt::AscendingOrder);
	setSortingEnabled(true);
	setHeaderHidden(true);

	connect(&AppSignals::instance(), &AppSignals::classAdded, this, &LibraryTreeWidget::onClassAdded);
	connect(&AppSignals::instance(), &AppSignals::classDirtyFlagChanged, this, &LibraryTreeWidget::onClassDirtyFlagChanged);
	connect(&AppSignals::instance(), &AppSignals::classDescriptionChanged, this, &LibraryTreeWidget::onClassDescriptionChanged);
	connect(&AppSignals::instance(), &AppSignals::classRenamed, this, &LibraryTreeWidget::onClassNameChanged);
	connect(&AppSignals::instance(), &AppSignals::classRemoved, this, &LibraryTreeWidget::onClassRemoved);
	connect(&AppSignals::instance(), &AppSignals::classMoved, this, &LibraryTreeWidget::onClassMoved);
	connect(&AppSignals::instance(), &AppSignals::documentFileChanged, this, &LibraryTreeWidget::onDocumentFileChanged);
	connect(&AppSignals::instance(), &AppSignals::documentAdded, this, &LibraryTreeWidget::onDocumentAdded);
	connect(&AppSignals::instance(), &AppSignals::documentRemoved, this, &LibraryTreeWidget::onDocumentRemoved);
	connect(&AppSignals::instance(), &AppSignals::documentDirtyFlagChanged, this, &LibraryTreeWidget::onDocumentDirtyFlagChanged);
	connect(&AppSignals::instance(), &AppSignals::functionAdded, this, &LibraryTreeWidget::onFunctionAdded);
	connect(&AppSignals::instance(), &AppSignals::functionChanged, this, &LibraryTreeWidget::onFunctionChanged);
	connect(&AppSignals::instance(), &AppSignals::functionRemoved, this, &LibraryTreeWidget::onFunctionRemoved);
	connect(&AppSignals::instance(), &AppSignals::updateSettings, this, &LibraryTreeWidget::updateSettings);
	connect(&AppSignals::instance(), &AppSignals::initLibraries, this, &LibraryTreeWidget::init);
}

LibraryTreeWidget::~LibraryTreeWidget()
{
	assert(_loader == nullptr);
}

void LibraryTreeWidget::updateSettings(QSettings& s, bool load) 
{
	if (load) {
		_hideDocs = s.value("mainWindow/" + objectName() + "/hideDocs", false).toBool();
		init(_librariesCache);
	}
	else {
		s.setValue("mainWindow/" + objectName() + "/hideDocs", _hideDocs);
	}
}

void LibraryTreeWidget::init(const class DocumentInfoCache* librariesCache)
{

	QHash<ItemTemp, bool> oldItems;
	for (QTreeWidgetItemIterator itr(this); *itr; itr++) {
		if (!(*itr)->isHidden() && (*itr)->isExpanded()) {
			ItemTemp it = { (*itr)->data(DATA_ITEM_TYPE).toInt(), (*itr)->data(DATA_ITEM_PATH).toString(), (*itr)->data(DATA_ITEM_NAME).toString(), (*itr)->data(DATA_ITEM_PTR).toULongLong() };
			oldItems[it] = true;
		}
	}

	clear();
	_librariesCache = librariesCache;
	if (!librariesCache)
		return;

	_librariesCache->UpdateAndBuildTree(invisibleRootItem(), false, _hideDocs, false);

	const auto &docs = engine->GetDocumentManager()->GetDocuments();
	for (const auto &n : docs) {
		onDocumentAdded(n);
		const auto &c = n->GetClasss();
		for (size_t i = 0; i < c.size(); i++)
			onClassAdded((ClassExt*)c[i]);
	}

	for (QTreeWidgetItemIterator itr(this); *itr; itr++) {
		if (!(*itr)->isHidden()) {
			ItemTemp it = { (*itr)->data(DATA_ITEM_TYPE).toInt(), (*itr)->data(DATA_ITEM_PATH).toString(), (*itr)->data(DATA_ITEM_NAME).toString(), (*itr)->data(DATA_ITEM_PTR).toULongLong() };
			(*itr)->setExpanded(oldItems.contains(it));
		}
	}
}

void LibraryTreeWidget::contextMenuEvent(QContextMenuEvent* event)
{
	QTreeWidgetItem* item = dynamic_cast<QTreeWidgetItem*>(currentItem());

	QMenu* menu = new QMenu(this);
	QAction* a0 = menu->addAction("Import");
	menu->addSeparator();
	QAction* a3 = menu->addAction("Open");
	if (item) {
		int t = item->data(DATA_ITEM_TYPE).toInt();
		a0->setEnabled((t == DocumentInfoCache::IT_CLASS || t == DocumentInfoCache::IT_DOCUMENT || t == DocumentInfoCache::IT_FUNCTION) && item->data(DATA_ITEM_PTR).toLongLong() == (UINT64)nullptr);
		a3->setEnabled((t == DocumentInfoCache::IT_CLASS || t == DocumentInfoCache::IT_FUNCTION) && item->data(DATA_ITEM_PTR).toLongLong() != (UINT64)nullptr);
	}
	else {
		a0->setEnabled(false);
		a3->setEnabled(false);
	}
	menu->addSeparator();
	QAction* a4 = menu->addAction("Expand All");
	QAction* a5 = menu->addAction("Collapse All");
	QAction* a1 = menu->addAction("Refresh");
	QAction* a2 = menu->addAction("Hide Documents");
	a2->setCheckable(true);
	a2->setChecked(_hideDocs);

	QAction* a = menu->exec(QCursor::pos());
	if (a == a0) {
		_importItem(item);
	}
	else if (a == a1) {
		init(_librariesCache);
	}
	else if (a == a2) {
		_hideDocs = a2->isChecked();
		init(_librariesCache);
	}
	else if (a == a3) {
		int t = item->data(DATA_ITEM_TYPE).toInt();
		if (t == DocumentInfoCache::IT_CLASS) {
			Class* c = (Class*)item->data(DATA_ITEM_PTR).toULongLong();
			if (c)
				AppSignals::instance().openClass((ClassExt*)c, nullptr);
		}
		else if (t == DocumentInfoCache::IT_FUNCTION) {
			Function* f = (Function*)item->data(DATA_ITEM_PTR).toULongLong();
			if (f)
				AppSignals::instance().openClass((ClassExt*)f->GetChip()->GetClass(), f->GetChip());
		}
	}
	else if (a == a4 || a == a5) {
		QTreeWidgetItem* itm = item;
		if (!itm)
			itm = invisibleRootItem();
		for (QTreeWidgetItemIterator itr(itm); *itr; itr++) {
			if (!(*itr)->isHidden())
				(*itr)->setExpanded(a == a4);
		}
	}

	menu->deleteLater();
}

bool LibraryTreeWidget::startDragging(QTreeWidgetItem* item)
{
	if (!item)
		return false;
	int t = item->data(DATA_ITEM_TYPE).toInt();
	return (t == DocumentInfoCache::IT_CLASS || t == DocumentInfoCache::IT_FUNCTION) && item->data(DATA_ITEM_PTR).toLongLong() != (UINT64)nullptr;
}

void LibraryTreeWidget::onDragging(QTreeWidgetItem* item, const QPoint& pos, Qt::KeyboardModifiers modifiers)
{
}

void LibraryTreeWidget::endDragging(QTreeWidgetItem* item, QWidget* dropAt, const QPoint& pos, Qt::KeyboardModifiers modifiers)
{
	if (!item)
		return;

	int t = item->data(DATA_ITEM_TYPE).toInt();

	DXEditorWidget* ew = dynamic_cast<DXEditorWidget*>(dropAt);
	if (!ew)
		return;

	if (t == DocumentInfoCache::IT_CLASS) {
		Class *c = (Class*)item->data(DATA_ITEM_PTR).toULongLong();
		if (c)
			ew->addClassChip(c->GetID());
	}
	else if (t == DocumentInfoCache::IT_FUNCTION) {
		Function* f = (Function*)item->data(DATA_ITEM_PTR).toULongLong();
		if (f)
			ew->addFunctionCallChip(f->GetChip()->GetClass()->GetID(), f->GetChip()->GetID(), modifiers & Qt::ShiftModifier);
	}
}

QCursor LibraryTreeWidget::dragCursor()
{
	static const QCursor forbiddenCursor = QCursor(Qt::ForbiddenCursor);
	static const QCursor chipCursor(QCursor(QIcon(":/EditorApp/Resources/chip.png").pixmap(32)));
	static const QCursor classCursor(QCursor(QIcon(":/EditorApp/Resources/graph.png").pixmap(32)));

	if (!_draggedItem)
		return forbiddenCursor;

	int t = _draggedItem->data(DATA_ITEM_TYPE).toInt();

	if (t == DocumentInfoCache::IT_CLASS) {
		return classCursor;
	}
	else if (t == DocumentInfoCache::IT_FUNCTION) {
		return chipCursor;
	}
	return forbiddenCursor;
}


void LibraryTreeWidget::onDocumentAdded(Document*doc) 
{
	QTreeWidgetItem* itm = _getFolder(doc->GetFileName());
	if (!itm)
		return;

	_setCachedDocumentHidden(doc, true);

	if (_hideDocs)
		return;

	QFileInfo nfo(TOQSTRING(doc->GetFileName().AsString()));

	DocumentInfoCacheTreeWidgetItem* item = new DocumentInfoCacheTreeWidgetItem(itm, QStringList(nfo.baseName()));
	item->setIcon(0, FileIconProvider::icon(nfo));
	item->setToolTip(0, nfo.absoluteFilePath());
	item->setData(DATA_ITEM_TYPE, DocumentInfoCache::IT_DOCUMENT);
	item->setData(DATA_ITEM_PATH, nfo.absoluteFilePath());
	item->setData(DATA_ITEM_PTR, (UINT64)doc);
	item->setForeground(0, doc->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
	QFont f = item->font(0);
	f.setBold(true);
	item->setFont(0, f);
	item->setExpanded(true);
}

void LibraryTreeWidget::onDocumentRemoved(Document*doc) 
{
	if (_hideDocs) {
		assert(doc->GetClasss().empty());
	}
	else {
		QTreeWidgetItem* itm = _getDocumentItem(doc);
		if (!itm)
			return;

		delete itm;
	}

	_setCachedDocumentHidden(doc, false);
}

void LibraryTreeWidget::onDocumentFileChanged(Document*doc, const Path&oldFileName)
{
	Path lp = engine->GetEnvironment()->GetLibraryFolder(oldFileName);
	if (lp.IsDirectory())
	{
		if (_hideDocs) {
			QTreeWidgetItem *itm = _getFolder(oldFileName);
			if (itm) {
				QFileInfo nfo(TOQSTRING(oldFileName.AsString()));
				QList<QTreeWidgetItem*> l;
				for (int i = 0; i < itm->childCount(); i++) {
					QTreeWidgetItem* jtm = itm->child(i);
					if (QFileInfo(jtm->data(DATA_ITEM_PATH).toString()) == nfo) {
						ClassExt *ptr = (ClassExt*)jtm->data(DATA_ITEM_PTR).toULongLong();
						if (ptr != nullptr)
							l.push_back(jtm);
					}
				}
				for (QTreeWidgetItem* jtm : l)
					delete jtm;
			}
		}
		else {
			QTreeWidgetItem *itm = _getDocumentItem(doc, &oldFileName);
			if (itm)
				delete itm;
		}
	}

	onDocumentAdded(doc);
	for (size_t i = 0; i < doc->GetClasss().size(); i++)
		onClassAdded((ClassExt*)doc->GetClasss()[i]);
}

void LibraryTreeWidget::onDocumentDirtyFlagChanged(Document*doc) 
{
	QTreeWidgetItem* itm = _getDocumentItem(doc);
	if (itm) {
		itm->setForeground(0, doc->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
		for (itm; itm; itm = itm->parent())
			itm->setIcon(0, FileIconProvider::icon(QFileInfo(itm->data(DATA_ITEM_PATH).toString())));
	}
}

void LibraryTreeWidget::onClassAdded(ClassExt*c) 
{
	static const QIcon classIcon = QIcon(":/EditorApp/Resources/graph.png");

	QTreeWidgetItem* itm = _hideDocs ? _getFolder(c->GetDocument()->GetFileName()) : _getDocumentItem(c->GetDocument());
	if (!itm)
		return;

	QString name = TOQSTRING(c->GetName());
	QFileInfo nfo(TOQSTRING(c->GetDocument()->GetFileName().AsString()));

	QTreeWidgetItem* item = new DocumentInfoCacheTreeWidgetItem(itm, QStringList(name));
	item->setIcon(0, classIcon);
	item->setData(DATA_ITEM_TYPE, DocumentInfoCache::IT_CLASS);
	item->setData(DATA_ITEM_PATH, nfo.absoluteFilePath());
	item->setData(DATA_ITEM_NAME, name);
	item->setData(DATA_ITEM_PTR, (UINT64)c);
	item->setForeground(0, c->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
	_setTooltip(item, c->GetDescription().empty() ? name : TOQSTRING(c->GetDescription()));
	QFont f = item->font(0);
	f.setBold(true);
	item->setFont(0, f);
	item->setExpanded(true);

	const FunctionPtrSet& functions = c->GetFunctions();
	for (const auto &n : functions) {
		onFunctionAdded(n);
	}
}

void LibraryTreeWidget::onClassRemoved(ClassExt*c) 
{
	QTreeWidgetItem* itm = _getClassItem(c);
	if (!itm)
		return;

	delete itm;
}

void LibraryTreeWidget::onClassDirtyFlagChanged(ClassExt*c) 
{
	QTreeWidgetItem *itm = _getClassItem(c);
	if (itm) {
		itm->setForeground(0, c->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
		for (QTreeWidgetItem* jtm = itm->parent(); jtm; jtm = jtm->parent())
			jtm->setIcon(0, FileIconProvider::icon(QFileInfo(jtm->data(DATA_ITEM_PATH).toString())));
	}
}

void LibraryTreeWidget::onClassDescriptionChanged(ClassExt*c) 
{
	QTreeWidgetItem* itm = _getClassItem(c);
	if (itm) {
		_setTooltip(itm, c->GetDescription().empty() ? TOQSTRING(c->GetName()) : TOQSTRING(c->GetDescription()));
	}
}

void LibraryTreeWidget::onClassNameChanged(ClassExt*c) 
{
	QTreeWidgetItem* itm = _getClassItem(c);
	if (itm) {
		itm->setText(0, TOQSTRING(c->GetName()));
		_setTooltip(itm, c->GetDescription().empty() ? TOQSTRING(c->GetName()) : TOQSTRING(c->GetDescription()));
	}
}

void LibraryTreeWidget::onFunctionAdded(Function* function) 
{
	QTreeWidgetItem *itm = _getClassItem(function->GetChip()->GetClass());
	if (itm) {
		QTreeWidgetItem* fitm = new DocumentInfoCacheTreeWidgetItem(itm, QStringList() << "" << "");
		fitm->setData(DATA_ITEM_TYPE, DocumentInfoCache::IT_FUNCTION);
		fitm->setData(DATA_ITEM_PTR, (UINT64)function);
		QFont f = fitm->font(0);
		f.setBold(true);
		fitm->setFont(0, f);
		_updateFunction(fitm, function);
	}
}

void LibraryTreeWidget::onFunctionChanged(Function* function) 
{
	QTreeWidgetItem* itm = _getFunctionItem(function);
	if (itm) {
		_updateFunction(itm, function);
	}
}

void LibraryTreeWidget::onFunctionRemoved(Function* function) 
{
	QTreeWidgetItem* itm = _getFunctionItem(function);
	if (itm) {
		delete itm;
	}
}

void LibraryTreeWidget::onClassMoved(ClassExt*c, Document*from, Document*to) 
{
	QTreeWidgetItem *itm = _getClassItem(c, from);
	if (itm)
		delete itm;
	onClassAdded(c);
}

QTreeWidgetItem* LibraryTreeWidget::_getFolder(Path p)
{
	Path lp = engine->GetEnvironment()->GetLibraryFolder(p);
	if (!lp.IsDirectory())
		return nullptr;
	Path rp = p.GetDirectory().GetRelativePath(lp);
	QStringList folderList = TOQSTRING(rp.AsString()).split('\\', Qt::SkipEmptyParts);

	QTreeWidgetItem* item = invisibleRootItem();

	QString s = TOQSTRING(lp.AsString());

	for (int i = 1; i < folderList.size(); i++) {
		QString name = folderList[i];

		s += name + "/";

		QFileInfo nfo(s);

		QString absPath = nfo.absoluteFilePath();

		QTreeWidgetItem* folder = nullptr;

		for (int i = 0; i < item->childCount(); i++, folder = nullptr) {
			folder = item->child(i);
			if (folder->data(DATA_ITEM_TYPE).toInt() == DocumentInfoCache::IT_FOLDER && nfo == QFileInfo(folder->data(DATA_ITEM_PATH).toString()))
				break;
		}

		if (!folder) {
			folder = new DocumentInfoCacheTreeWidgetItem(item, QStringList(name));
			item->setIcon(0, FileIconProvider::icon(nfo));
			item->setData(DATA_ITEM_TYPE, DocumentInfoCache::IT_FOLDER);
			item->setData(DATA_ITEM_PATH, nfo.absoluteFilePath());
		}
		item = folder;
	}

	return item;
}

QTreeWidgetItem* LibraryTreeWidget::_getDocumentItem(Document* doc, const Path* p)
{
	if (!doc)
		return nullptr;

	Path fn = p ? *p : doc->GetFileName();

	if (_hideDocs)
		return nullptr;

	QTreeWidgetItem *itm = _getFolder(fn);
	if (itm) {
		for (int i = 0; i < itm->childCount(); i++) {
			QTreeWidgetItem* jtm = itm->child(i);
			if (jtm->data(DATA_ITEM_TYPE).toInt() == DocumentInfoCache::IT_DOCUMENT)
				if (jtm->data(DATA_ITEM_PTR).toULongLong() == (UINT64)doc)
					return jtm;
		}
	}
	return nullptr;
}

QTreeWidgetItem* LibraryTreeWidget::_getClassItem(Class* cl, Document *doc)
{
	if (!cl)
		return nullptr;
	if (!doc)
		doc = cl->GetDocument();
	if (!doc)
		return nullptr;

	QTreeWidgetItem* itm = nullptr;
	if (_hideDocs) {
		itm = _getFolder(doc->GetFileName());
	}
	else {
		itm = _getDocumentItem(doc);
	}
	if (itm) {
		for (int i = 0; i < itm->childCount(); i++) {
			QTreeWidgetItem* jtm = itm->child(i);
			if (jtm->data(DATA_ITEM_TYPE).toUInt() == DocumentInfoCache::IT_CLASS)
				if (jtm->data(DATA_ITEM_PTR).toULongLong() == (UINT64)cl)
					return jtm;
		}
	}
	return nullptr;
}

QTreeWidgetItem* LibraryTreeWidget::_getFunctionItem(class Function* f)
{
	if (!f)
		return nullptr;

	QTreeWidgetItem* itm = _getClassItem(f->GetChip()->GetClass());
	if (itm) {
		for (int i = 0; i < itm->childCount(); i++) {
			QTreeWidgetItem* jtm = itm->child(i);
			if (jtm->data(DATA_ITEM_TYPE).toUInt() == DocumentInfoCache::IT_FUNCTION)
				if (jtm->data(DATA_ITEM_PTR).toULongLong() == (UINT64)f)
					return jtm;
		}
	}
	return nullptr;
}

void LibraryTreeWidget::_setCachedDocumentHidden(class Document* doc, bool isHidden)
{
	if (!doc)
		return;

	QTreeWidgetItem* itm = _getFolder(doc->GetFileName());
	if (itm) {
		QFileInfo nfo(TOQSTRING(doc->GetFileName().AsString()));
		if (_hideDocs) {
			for (int i = 0; i < itm->childCount(); i++) {
				QTreeWidgetItem* jtm = itm->child(i);
				if (jtm->data(DATA_ITEM_TYPE).toInt() == DocumentInfoCache::IT_CLASS) {
					if (jtm->data(DATA_ITEM_PTR).toULongLong() == (UINT64)nullptr) {
						if (QFileInfo(jtm->data(DATA_ITEM_PATH).toString()) == nfo) {
							jtm->setHidden(isHidden);
							jtm->setExpanded(false);
						}
					}
				}
			}
		}
		else {
			for (int i = 0; i < itm->childCount(); i++) {
				QTreeWidgetItem* jtm = itm->child(i);
				if (jtm->data(DATA_ITEM_TYPE).toInt() == DocumentInfoCache::IT_DOCUMENT) {
					if (jtm->data(DATA_ITEM_PTR).toULongLong() == (UINT64)nullptr) {
						if (QFileInfo(jtm->data(DATA_ITEM_PATH).toString()) == nfo) {
							jtm->setHidden(isHidden);
							jtm->setExpanded(false);
							break;
						}
					}
				}
			}
		}
	}
}

void LibraryTreeWidget::_updateFunction(QTreeWidgetItem* itm, Function* function)
{
	static const QIcon FUNCTION_ICONS[3] = { QIcon(":/EditorApp/Resources/static-func.png"), QIcon(":/EditorApp/Resources/nonvirt-func.png"), QIcon(":/EditorApp/Resources/virt-func.png") };

	Chip* chip = function->GetChip();
	const ChipInfo* nfo = engine->GetChipManager()->GetChipInfo(chip->GetChipType());

	ChipEditorData* edata = chip->GetChipEditorData();
	QString comment;
	if (edata)
		comment = TOQSTRING(edata->comment);

	QString name = TOQSTRING(chip->GetName());

	itm->setIcon(0, FUNCTION_ICONS[(uint32)function->GetType()]);
	itm->setText(0, name);
	itm->setText(1, "    " + TOQSTRING(nfo->chipDesc.name));
	itm->setData(DATA_ITEM_NAME, name);
	itm->setData(DATA_ITEM_DESC, comment);
	_setTooltip(itm, comment.isEmpty() ? name : comment);
}

void LibraryTreeWidget::_setTooltip(QTreeWidgetItem* item, QString txt)
{
	if (txt.length() > 250)
		txt = txt.left(247) + "...";
	if (txt.isEmpty())
		item->setToolTip(0, QString());
	else
		item->setToolTip(0, "<font>" + txt.toHtmlEscaped() + "</font>");
}

void LibraryTreeWidget::_importItem(QTreeWidgetItem* itm)
{
	if (!itm)
		return;
	int t = itm->data(DATA_ITEM_TYPE).toInt();
	if (t == DocumentInfoCache::IT_FUNCTION)
		return _importItem(itm->parent());

	Path p;

	if (t == DocumentInfoCache::IT_DOCUMENT || t == DocumentInfoCache::IT_CLASS) {
		if (itm->data(DATA_ITEM_PTR).toULongLong() == (UINT64)nullptr)
			p = FROMQSTRING(itm->data(DATA_ITEM_PATH).toString());
	}

	if (p.IsFile())
		AppSignals::instance().importClass2(p);
}
