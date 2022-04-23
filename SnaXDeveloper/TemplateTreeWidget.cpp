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
#include "TemplateTreeWidget.h"
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
#include <qsettings.h>
#include "M3DEngineExt/DocumentExtLoader.h"
#include "DocumentInfoCache.h"
#include "AppSignals.h"

using namespace m3d;




TemplateTreeWidget::TemplateTreeWidget(QWidget * parent) : CustomDraggableTreeWidget(parent)
{
	sortByColumn(0, Qt::AscendingOrder);
	setHeaderHidden(true);

	connect(&AppSignals::instance(), &AppSignals::initTemplates, this, &TemplateTreeWidget::init);
	connect(&AppSignals::instance(), &AppSignals::updateSettings, this, &TemplateTreeWidget::updateSettings);
}

TemplateTreeWidget::~TemplateTreeWidget()
{
	assert(_loader == nullptr);
}

void TemplateTreeWidget::updateSettings(QSettings& s, bool load)
{
	s.beginGroup("mainWindow/" + objectName());
	if (load) {
		_hideDocs = s.value("hideDocs", false).toBool();

		_rebuild();
	}
	else {
		s.setValue("hideDocs", _hideDocs);
	}
	s.endGroup();
}

void TemplateTreeWidget::init(class DocumentInfoCache* templatesCache)
{
	_templatesCache = templatesCache;
	_rebuild();
}

void TemplateTreeWidget::_rebuild()
{
	clear();
	if (_templatesCache) {
		_templatesCache->UpdateFromSettings();
		_templatesCache->UpdateAndBuildTree(invisibleRootItem(), false, _hideDocs, false);
	}
}

void TemplateTreeWidget::contextMenuEvent ( QContextMenuEvent * event )
{
	QMenu *menu = new QMenu(this);
	QAction *a1 = menu->addAction("Refresh");
	QAction *a2 = menu->addAction("Hide Documents");
	a2->setCheckable(true);
	a2->setChecked(_hideDocs);

	QAction *a = menu->exec(QCursor::pos());
	if (a == a1) {
		init(_templatesCache);
	}
	if (a == a2) {
		_hideDocs = a2->isChecked();
		init(_templatesCache);
	}
	menu->deleteLater();
}

bool TemplateTreeWidget::startDragging(QTreeWidgetItem *item)
{
	QString file = item->data(DATA_ITEM_PATH).toString();
	_clazz = FROMQSTRING(item->data(DATA_ITEM_NAME).toString());
	if (file.isEmpty() || _clazz.empty())
		return false;

	const DocumentFileTypes::FileTypeDesc *ft = DocumentFileTypes::GetFileType(FROMQSTRING(file));
	if (!ft) {
		msg(WARN, MTEXT("Could not find template loader."));
		return false;
	}

	_loader = (*ft->loader)();
	if (!_loader) {
		msg(WARN, MTEXT("Failed to create template loader."));
		return false;
	}
	if (!_loader->OpenFile(FROMQSTRING(file))) {
		msg(WARN, MTEXT("Failed to open template document \'") + FROMQSTRING(file) + MTEXT("\'."));
		DocumentFileTypes::Free(_loader);
		_loader = nullptr;
		return false;
	}

	_loader->SetAllowIrregularLoading(true);

	int32 iextent[4] = { 0, 0, 0, 0 };

	if (!(_loader->GetDocumentVersion() > Version(1, 2, 5, 0) ? _loader->EnterGroup(DocumentTags::Classes) : true)
		|| !_loader->EnterGroup(DocumentTags::Class, DocumentTags::name, _clazz) 
		|| !_loader->EnterGroup(DocumentTags::Editor) 
		|| !_loader->EnterGroup(DocumentTags::ClassVisual) 
		|| !(_loader->GetDocumentVersion() > Version(1, 2, 5, 0) ? _loader->GetAttribute(DocumentTags::extent, iextent, 4) : _loader->GetAttribute(DocumentTags::extent, (float*)&_extent, 4)) 
		|| !_loader->Reset()) {
		msg(WARN, MTEXT("Could not create template \'") + _clazz + MTEXT("\' from \'") + FROMQSTRING(file) + MTEXT("\'."));
		DocumentFileTypes::Free(_loader);
		_loader = nullptr;
		return false;
	}

	if (_loader->GetDocumentVersion() > Version(1, 2, 5, 0))
		_extent = Vector4(iextent[0] * 0.01f, iextent[1] * 0.01f, iextent[2] * 0.01f, iextent[3] * 0.01f);

	_ew = nullptr;

	return true;
}

void TemplateTreeWidget::onDragging(QTreeWidgetItem *item, const QPoint &pos, Qt::KeyboardModifiers modifiers) 
{
	DXEditorWidget *ew = dynamic_cast<DXEditorWidget*>(QApplication::widgetAt(pos));
	QString s = item->data(DATA_ITEM_PATH).toString();
	if (s.isEmpty())
		return;
	if (ew != _ew) {
		if (_ew)
			_ew->onTemplateDragLeave();
		_ew = nullptr;
		if (ew) {
			_ew = ew;
			_ew->onTemplateDragEnter(_extent);
		}
	}
}

void TemplateTreeWidget::endDragging(QTreeWidgetItem *item, QWidget *dropAt, const QPoint &pos, Qt::KeyboardModifiers modifiers)
{
	QString s = item->data(DATA_ITEM_PATH).toString();
	if (s.isEmpty())
		return;
	DXEditorWidget *ew = dynamic_cast<DXEditorWidget*>(dropAt);
	if (ew != _ew && _ew != nullptr)
		_ew->onTemplateDragLeave();

	if (ew)
		ew->addTemplate(*_loader, _clazz);

	DocumentFileTypes::Free(_loader);
	_loader = nullptr;

	_ew = nullptr;
}

QCursor TemplateTreeWidget::dragCursor()
{
	static QCursor c(QCursor(QIcon(":/EditorApp/Resources/graph.png").pixmap(32)));
	return c;
}

