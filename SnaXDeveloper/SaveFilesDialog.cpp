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
#include "SaveFilesDialog.h"
#include "M3DEngine/DocumentManager.h"
#include "M3DEngine/Document.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/Environment.h"
#include "FileIconProvider.h"

using namespace m3d;


class SaveFilesTreeWidgetItem : public QTreeWidgetItem
{
public:
	SaveFilesTreeWidgetItem(QTreeWidget* pitem, const QStringList& strings) : QTreeWidgetItem(pitem, strings) {}
	~SaveFilesTreeWidgetItem() {}

	bool operator<(const QTreeWidgetItem& other) const override
	{
		int i = treeWidget()->sortColumn();
		Qt::SortOrder sortOrder = treeWidget()->header()->sortIndicatorOrder();
		if (i == 0) {
			int a = data(0, Qt::UserRole).toInt();
			int b = other.data(0, Qt::UserRole).toInt();
			if (a != b)
				return sortOrder == Qt::AscendingOrder ? (a < b) : (b < a);
		}
		return QTreeWidgetItem::operator<(other);
	}
};


SaveFilesDialog::SaveFilesDialog(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	ui.setupUi(this);
	ui.treeWidget->setSortingEnabled(true);
	ui.treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	ui.treeWidget->header()->setStretchLastSection(true);
	ui.treeWidget->sortByColumn(0, Qt::AscendingOrder);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	connect(ui.okButton, &QPushButton::clicked, this, &SaveFilesDialog::saveSelected);
	connect(ui.dontSaveButton, &QPushButton::clicked, this, &SaveFilesDialog::dontSave);
}

SaveFilesDialog::~SaveFilesDialog()
{

}

int SaveFilesDialog::showDialog()
{
	Path rootFolder = engine->GetEnvironment()->GetProjectRootFolder();
	_notSavedItem = new SaveFilesTreeWidgetItem(ui.treeWidget, QStringList("Not Saved"));
	_notSavedItem->setIcon(0, QIcon(":/EditorApp/Resources/folder_red.png"));
	_notSavedItem->setHidden(true);
	_notSavedItem->setExpanded(true);
	_notSavedItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate | Qt::ItemIsEnabled);
	_notSavedItem->setData(0, Qt::UserRole, 0);

	_projectItem = new SaveFilesTreeWidgetItem(ui.treeWidget, QStringList("Project"));
	_projectItem->setHidden(true);
	_projectItem->setExpanded(true);
	_projectItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate | Qt::ItemIsEnabled);
	_projectItem->setIcon(0, rootFolder.IsValid() ? FileIconProvider::icon(QFileInfo(TOQSTRING(rootFolder.AsString()))) : QIcon());
	_projectItem->setData(0, Qt::UserRole, 1);


	_notInProjectFolderItem = new SaveFilesTreeWidgetItem(ui.treeWidget, QStringList("External"));
	_notInProjectFolderItem->setIcon(0, QIcon(":/EditorApp/Resources/folder_red.png"));
	_notInProjectFolderItem->setHidden(true);
	_notInProjectFolderItem->setExpanded(true);
	_notInProjectFolderItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate | Qt::ItemIsEnabled);
	_notInProjectFolderItem->setData(0, Qt::UserRole, 2);

	_libraryItem = new SaveFilesTreeWidgetItem(ui.treeWidget, QStringList("Library"));
	_libraryItem->setIcon(0, QIcon(":/EditorApp/Resources/open.png"));
	_libraryItem->setHidden(true);
	_libraryItem->setExpanded(true);
	_libraryItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate | Qt::ItemIsEnabled);
	_libraryItem->setData(0, Qt::UserRole, 3);


	const DocumentPtrSet& m = engine->GetDocumentManager()->GetDocuments();
	for (const auto &n : m) {
		if (n->IsDirty()) {
			Path p = n->GetFileName();
			QTreeWidgetItem* itm = nullptr;
			if (p.IsFile())
			{
				itm = new QTreeWidgetItem(QStringList() << TOQSTRING(p.GetName()) << TOQSTRING(p.GetDirectory().AsString()));
				itm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
				itm->setCheckState(0, Qt::Checked);
				itm->setIcon(0, rootFolder.IsValid() ? FileIconProvider::icon(QFileInfo(TOQSTRING(p.AsString()))) : QIcon());
				if (engine->GetEnvironment()->IsPathInsideProjectRootFolder(p))
					_projectItem->addChild(itm);
				else if (engine->GetEnvironment()->IsPathInsideLibraryFolder(p)) {
					_libraryItem->addChild(itm);
					itm->setCheckState(0, Qt::Unchecked);
				}
				else
					_notInProjectFolderItem->addChild(itm);
			}
			else
			{
				itm = new QTreeWidgetItem(_notSavedItem, QStringList() << "Not Saved" << "");
				itm->setCheckState(0, Qt::Checked);
				itm->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
				itm->setIcon(0, QIcon(":/EditorApp/Resources/unknown-file.png"));
			}
			itm->setData(0, Qt::UserRole, (ULONGLONG)n);
			for (size_t i = 0; i < n->GetClasss().size(); i++) {
				Class* c = n->GetClasss()[i];
				QTreeWidgetItem *jtm = new QTreeWidgetItem(itm, QStringList() << TOQSTRING(c->GetName()) << "");
				jtm->setFlags(Qt::ItemIsEnabled);
				jtm->setForeground(0, c->IsDirty() ? QColor(255, 0, 0) : QColor(0, 0, 0));
				jtm->setIcon(0, QIcon(":/EditorApp/Resources/graph.png"));
			}
		}
	}

	_notSavedItem->setHidden(_notSavedItem->childCount() == 0);
	_projectItem->setHidden(_projectItem->childCount() == 0);
	_notInProjectFolderItem->setHidden(_notInProjectFolderItem->childCount() == 0);
	_libraryItem->setHidden(_libraryItem->childCount() == 0);

	_docsToSave.clear();
	return exec();
}


void SaveFilesDialog::saveSelected()
{
	for (int i = 0; i < _notSavedItem->childCount(); i++) 
		if (_notSavedItem->child(i)->checkState(0) != Qt::CheckState::Unchecked)
			_docsToSave.push_back((Document*)_notSavedItem->child(i)->data(0, Qt::UserRole).toULongLong());
	for (int i = 0; i < _projectItem->childCount(); i++)
		if (_projectItem->child(i)->checkState(0) != Qt::CheckState::Unchecked)
			_docsToSave.push_back((Document*)_projectItem->child(i)->data(0, Qt::UserRole).toULongLong());
	for (int i = 0; i < _notInProjectFolderItem->childCount(); i++)
		if (_notInProjectFolderItem->child(i)->checkState(0) != Qt::CheckState::Unchecked)
			_docsToSave.push_back((Document*)_notInProjectFolderItem->child(i)->data(0, Qt::UserRole).toULongLong());
	for (int i = 0; i < _libraryItem->childCount(); i++)
		if (_libraryItem->child(i)->checkState(0) != Qt::CheckState::Unchecked)
			_docsToSave.push_back((Document*)_libraryItem->child(i)->data(0, Qt::UserRole).toULongLong());

	accept();
}

void SaveFilesDialog::dontSave()
{
	accept();
}
