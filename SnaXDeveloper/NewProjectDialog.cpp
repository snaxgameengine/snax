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
#include "NewProjectDialog.h"
#include "TemplateTreeWidget.h"
#include "FileIconProvider.h"
#include "DocumentInfoCache.h"
#include <qapplication.h>
#include "App.h"

using namespace m3d;




NewProjectDialog::NewProjectDialog(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f | Qt::MSWindowsFixedSizeDialogHint)
{
	ui.setupUi(this);
	QString version = QApplication::applicationVersion();
	setWindowTitle("Welcome to SnaX Game Engine - Version " + version);
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	
	QObject::connect(ui.treeWidget, &QTreeWidget::itemSelectionChanged, [&]() {
		QTreeWidgetItem *itm = ui.treeWidget->currentItem();
		QString desc;
		bool okEnabled = false;
		if (itm) {
			desc = itm->data(DATA_ITEM_DESC).toString();
			int t = itm->data(DATA_ITEM_TYPE).toInt();
			okEnabled = t == DocumentInfoCache::IT_DOCUMENT || t == DocumentInfoCache::IT_CLASS || t < -3;
		}
		ui.descriptionTextEdit->setPlainText(desc);
		ui.okButton->setEnabled( okEnabled ); 
		});

	connect(ui.treeWidget, &QTreeWidget::itemDoubleClicked, this, [this](QTreeWidgetItem* itm, int) { 
		if (this->ui.okButton->isEnabled())
			this->accept(); 
		});
}

NewProjectDialog::~NewProjectDialog()
{
}

void NewProjectDialog::init(const DocumentInfoCache& templatesCache)
{
	ui.treeWidget->clear();
	ui.treeWidget->sortByColumn(0, Qt::AscendingOrder);

	_templatesRootItem = new DocumentInfoCacheTreeWidgetItem(ui.treeWidget->invisibleRootItem(), QStringList("Templates"));
	_templatesRootItem->setData(0, Qt::DecorationRole, QIcon(":/EditorApp/Resources/new.png"));
	_templatesRootItem->setExpanded(true);
	_templatesRootItem->setData(DATA_ITEM_TYPE, -3); // For sorting!

	_examplesRootItem = new DocumentInfoCacheTreeWidgetItem(ui.treeWidget->invisibleRootItem(), QStringList("Examples"));
	_examplesRootItem->setData(0, Qt::DecorationRole, QIcon(":/EditorApp/Resources/idea.png"));
	_examplesRootItem->setExpanded(true);
	_examplesRootItem->setData(DATA_ITEM_TYPE, -2); // For sorting!

	_recentRootItem = new DocumentInfoCacheTreeWidgetItem(ui.treeWidget->invisibleRootItem(), QStringList("Recent Projects"));
	_recentRootItem->setData(0, Qt::DecorationRole, QIcon(":/EditorApp/Resources/clock.png"));
	_recentRootItem->setExpanded(true);
	_recentRootItem->setData(DATA_ITEM_TYPE, -1); // For sorting!

	templatesCache.UpdateAndBuildTree(_templatesRootItem, true, true, true);

	DocumentInfoCache examplesCache("settings/examples", EXAMPLES_PATH, "settings/examplesCache", "$applocaldata\\examples.cache", false);
	examplesCache.UpdateAndBuildTree(_examplesRootItem, true, true, true);

	if (_templatesRootItem->childCount() == 0) { // Fallback if there are no templates...
		DocumentInfoCacheTreeWidgetItem* itm = new DocumentInfoCacheTreeWidgetItem(_templatesRootItem, QStringList("Empty Project"));
		itm->setData(DATA_ITEM_TYPE, DocumentInfoCache::IT_CLASS);
		itm->setData(DATA_ITEM_NAME, QString());
		itm->setData(DATA_ITEM_PATH, QString());
		itm->setIcon(0, QIcon(":/EditorApp/Resources/graph.png"));
	}

	QSettings s = ((const App*)QApplication::instance())->GetSettings();
	int n = 0;
	for (QString p : s.value("settings/recentProjects", QStringList()).toStringList()) {
		QFileInfo nfo(p);
		if (nfo.exists() && nfo.isFile()) {
			QString e = QString("%1 (%2)").arg(nfo.fileName()).arg(nfo.absolutePath());
			QTreeWidgetItem* itm = new DocumentInfoCacheTreeWidgetItem(_recentRootItem, QStringList(e));
			itm->setData(DATA_ITEM_TYPE, -100 + n++);
			itm->setData(DATA_ITEM_PATH, nfo.absoluteFilePath());
			QIcon icn = FileIconProvider::icon(nfo);
			itm->setIcon(0, icn);
		}
	}

	QString init = s.value("settings/startupTemplate", "Main/Start:Default Startup").toString();
	for (QTreeWidgetItemIterator it(_templatesRootItem); *it; it++) {
		QString str = (*it)->data(DATA_ITEM_TEMPPATH).toString();
		if (str == init) {
			(*it)->setSelected(true);
			ui.treeWidget->setCurrentItem(*it);
			ui.treeWidget->scrollToItem(*it);
		}
	}

	emit ui.treeWidget->itemSelectionChanged();
}

void NewProjectDialog::accept()
{
	_selectedFile.clear();
	_selectedClass.clear();

	QTreeWidgetItem* itm = ui.treeWidget->currentItem();
	if (itm) {
		if (itm->data(DATA_ITEM_TYPE) == DocumentInfoCache::IT_CLASS) { // class
			for (QTreeWidgetItem* pitm = itm->parent(); pitm; pitm = pitm->parent()) {
				if (pitm == _templatesRootItem) {
					_selectedFile = itm->data(DATA_ITEM_PATH).toString();
					_selectedClass = itm->data(DATA_ITEM_NAME).toString();
					return QDialog::accept();
				}
			}
		}
		_selectedFile = itm->data(DATA_ITEM_PATH).toString();
	}
	QDialog::accept();
}