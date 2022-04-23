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
#include "FileTreeWidget.h"
#include <qdir.h>
#include "FileIconProvider.h"


using namespace m3d;

class FileTreeWidget::TreeWidgetItem : public QTreeWidgetItem
{
public:
	TreeWidgetItem(QTreeWidgetItem *parent, QFileInfo i);

	void explore();
	void filterChanged();
	void getSelectedFiles(QStringList &lst, QString p ="", bool includeSelf = true);

	bool operator<(const QTreeWidgetItem &rhs) const
	{
		const TreeWidgetItem &i = (const TreeWidgetItem&)rhs;
		return (_isDir + i._isDir) % 2 ? _isDir : (QTreeWidgetItem::operator<(rhs));
	}

	TreeWidgetItem *find(QString file);

	const QFileInfo& fileInfo() const { return _nfo; }

private:
	TreeWidgetItem(TreeWidgetItem *parent, QFileInfo i, const Qt::CheckState *cs = nullptr);

	QFileInfo _nfo;
	bool _explored;
	int _isDir;
};

FileTreeWidget::TreeWidgetItem::TreeWidgetItem(QTreeWidgetItem*parent, QFileInfo i) : QTreeWidgetItem(parent), _nfo(i), _explored(false)
{
	setText(0, i.absoluteFilePath());
	setIcon(0, FileIconProvider::icon(i));
	setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	setFlags(flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
	setCheckState(0, Qt::Unchecked);
	_isDir = i.isDir() ? 1 : 0;
}

FileTreeWidget::TreeWidgetItem::TreeWidgetItem(TreeWidgetItem *parent, QFileInfo i, const Qt::CheckState* cs) : QTreeWidgetItem(), _nfo(i), _explored(false)
{
	setText(0, i.fileName());
	setIcon(0, FileIconProvider::icon(i));
	if (i.isDir())
		setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
	setFlags(flags() | Qt::ItemIsUserCheckable | Qt::ItemIsAutoTristate);
	setCheckState(0, cs ? *cs : (parent->checkState(0) == Qt::Checked ? Qt::Checked : Qt::Unchecked));
	_isDir = i.isDir() ? 1 : 0;
	parent->addChild(this);
}

void FileTreeWidget::TreeWidgetItem::explore()
{
	if (_explored)
		return;
	_explored = true;

	QString folder = _nfo.absoluteFilePath();
	QDir dir(folder);

	QFileInfoList lst = dir.entryInfoList(((FileTreeWidget*)treeWidget())->filters(), QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
	for (int i = 0; i < lst.size(); i++) {	
		TreeWidgetItem *n = new TreeWidgetItem(this, lst[i]);
	}
	if (childCount() == 0)
		setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
}

void FileTreeWidget::TreeWidgetItem::filterChanged()
{
	QDir dir(_nfo.absoluteFilePath());
	QFileInfoList lst = dir.entryInfoList(((FileTreeWidget*)treeWidget())->filters(), QDir::Files);
	QMap<QString, int> m;
	for (int i = 0; i <lst.size(); i++)
		m.insert(lst[i].fileName().toLower(), i);
	QList<TreeWidgetItem*> remove;
	for (int i = 0, j = childCount(); i < j; i++) {
		TreeWidgetItem *itm = (TreeWidgetItem*)child(i);
		if (itm->_isDir)
			continue;
		QMap<QString, int>::iterator itr = m.find(itm->_nfo.fileName().toLower());
		if (itr == m.end())
			remove.push_back(itm);
		else
			m.erase(itr);
	}
	Qt::CheckState cs = Qt::Unchecked;
	for (QMap<QString, int>::Iterator itr = m.begin(); itr != m.end(); itr++)
		new TreeWidgetItem(this, lst[itr.value()], &cs);
	for (int i = 0; i < remove.size(); i++)
		delete remove[i];
}

FileTreeWidget::TreeWidgetItem *FileTreeWidget::TreeWidgetItem::find(QString file)
{
	for (int i = 0, j = childCount(); i < j; i++)
		if (((TreeWidgetItem*)child(i))->_nfo.fileName().compare(file, Qt::CaseInsensitive) == 0)
			return (TreeWidgetItem*)child(i);
	return nullptr;
}

void __recursiveFileSearch(QString dir, QStringList& lst, QString p, QStringList filters)
{
	QFileInfoList l = QDir(dir).entryInfoList(filters, QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files);
	for (int i = 0; i < l.size(); i++) {
		QFileInfo nfo(l[i]);
		QString filename = nfo.fileName();
		if (nfo.isFile())
			lst.push_back(p + filename);
		else if (nfo.isDir())
			__recursiveFileSearch(nfo.absoluteFilePath(), lst, p + filename + "\\", filters);
	}
}

void FileTreeWidget::TreeWidgetItem::getSelectedFiles(QStringList &lst, QString p, bool includeSelf)
{
	QString fn = _nfo.fileName();
	if (_nfo.isFile()) {
		if (checkState(0) == Qt::Checked)
			lst.push_back(p + fn);
	}
	else if (_nfo.isDir()) {
		if (includeSelf)
			fn = fn + "\\";
		else
			fn = "";

		if (checkState(0) == Qt::Checked && childCount() == 0) {
			__recursiveFileSearch(_nfo.absoluteFilePath(), lst, p + fn, ((FileTreeWidget*)treeWidget())->filters());
		}
		else if (checkState(0) != Qt::Unchecked) {
			for (int i = 0, j = childCount(); i < j; i++)
				((TreeWidgetItem*)child(i))->getSelectedFiles(lst, p + fn);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////////////


FileTreeWidget::FileTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
	_rootItem = nullptr;
	sortByColumn(0, Qt::AscendingOrder);
	connect(this, &FileTreeWidget::expanded, this, &FileTreeWidget::_onExpanded);

	_projectItem = new QTreeWidgetItem(this, QStringList("Project"));
	_projectItem->setExpanded(true);
	_librariesItem = new QTreeWidgetItem(this, QStringList("Libraries"));
	_librariesItem->setExpanded(true);
}

FileTreeWidget::~FileTreeWidget()
{
}

bool FileTreeWidget::setProjectDirectory(QString root)
{
	if (_rootItem)
		delete _rootItem;
	_rootItem = nullptr;
	root = root.replace('/', '\\');
	while (root.endsWith('\\'))
		root = root.left(root.size() - 1);
	QFileInfo nfo(root);
	if (!nfo.isDir())
		return false;
	_rootItem = new TreeWidgetItem(_projectItem, nfo);
	_rootDir = root;
	return true;
}

bool FileTreeWidget::setLibraryDirectories(QStringList libs)
{
	for (int i = 0; i < _libItems.size(); i++)
		delete _libItems[i];
	_libItems.clear();
		
	bool ok = true;
	for (int i = 0; i < libs.size(); i++) {
		QString root = libs[i].replace('/', '\\');
		while (root.endsWith('\\'))
			root = root.left(root.size() - 1);
		QFileInfo nfo(root);
		if (!nfo.isDir()) {
			ok = false;
			continue;
		}
		TreeWidgetItem* itm = new TreeWidgetItem(_librariesItem, nfo);
		_libItems.push_back(itm);
	}
	return ok;
}

void FileTreeWidget::setFilters(QStringList filters)
{
	if (filters == _filters)
		return;
	_filters = filters;
	if (_rootItem)
		_rootItem->filterChanged();
}

bool FileTreeWidget::selectFile(QString file)
{
	TreeWidgetItem* itm = _rootItem;
	if (file.startsWith("$lib")) {
		bool ok = false;
		for (int i = 0; i < _libItems.size(); i++) {
			QString f = _libItems[i]->fileInfo().absoluteFilePath() + "\\" + file.mid(4);
			QFileInfo nfo(f);
			if (nfo.exists()) {
				itm = _libItems[i];
				file = file.replace("$lib", ".");
				ok = true;
				break;
			}
		}
		if (!ok)
			return false; // library file was not found!
	}

	file.replace('\\', '/');
	QStringList l = file.split('/', Qt::SkipEmptyParts);
	l.removeAll("."); // Remove dots
	for (size_t i = 0; i < l.size(); i++) {
		itm->explore();
		itm->setExpanded(true);
		itm = itm->find(l[i]);
		if (!itm)
			return false; // project file was not found!
	}
	itm->setCheckState(0, Qt::Checked);
	return true;
}

QStringList FileTreeWidget::selectedFiles()
{
	QStringList l;
	if (_rootItem) {
		_rootItem->getSelectedFiles(l, ".\\", false);
	}
	for (int i = 0; i < _libItems.size(); i++) {
		_libItems[i]->getSelectedFiles(l, "$lib\\", false);
	}
	return l;
}

void FileTreeWidget::_onExpanded(const QModelIndex &index)
{
	TreeWidgetItem *itm = dynamic_cast<TreeWidgetItem*>(itemFromIndex(index));
	if (itm)
		itm->explore();
}
