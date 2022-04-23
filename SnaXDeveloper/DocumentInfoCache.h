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
#include "M3DCore/Containers.h"
#include "M3DCore/Path.h"

#define TEMPLATES_PATH "$app\\Templates\\"
#define LIBRARIES_PATH "$app\\Libraries\\"
#define EXAMPLES_PATH "$app\\Examples\\"


#define DATA_ITEM_TYPE 0, Qt::UserRole + 0
#define DATA_ITEM_PATH 0, Qt::UserRole + 1
#define DATA_ITEM_NAME 0, Qt::UserRole + 2
#define DATA_ITEM_TEMPPATH 0, Qt::UserRole + 3
#define DATA_ITEM_DESC 0, Qt::UserRole + 4
#define DATA_ITEM_PTR 0, Qt::UserRole + 5

namespace m3d
{

class DocumentInfoCacheTreeWidgetItem : public QTreeWidgetItem
{
public:
	DocumentInfoCacheTreeWidgetItem(QTreeWidgetItem* pitem, const QStringList& strings) : QTreeWidgetItem(pitem, strings) {}
	~DocumentInfoCacheTreeWidgetItem() {}

	bool operator<(const QTreeWidgetItem& other) const override
	{
		int i = treeWidget()->sortColumn();
		Qt::SortOrder sortOrder = treeWidget()->header()->sortIndicatorOrder();
		if (i == 0) {
			int a = data(DATA_ITEM_TYPE).toInt();
			int b = other.data(DATA_ITEM_TYPE).toInt();
			if (a != b)
				return sortOrder == Qt::AscendingOrder ? (a < b) : (b < a);
		}
		return QTreeWidgetItem::operator<(other);
	}
};


class DocumentInfoCache
{
public:
	DocumentInfoCache(QString dirKey, QString dirDefaultValue, QString cacheKey, QString cacheDefaultFile, bool includeFunctions);

	void UpdateFromSettings();
	bool UpdateAndBuildTree(QTreeWidgetItem* root, bool addOnlyClassesWithStartChip, bool hideDocs, bool useFilePathTooltips) const;

	enum ItemType { IT_FOLDER = 1, IT_DOCUMENT, IT_CLASS, IT_FUNCTION };

private:
	void _searchDir(QTreeWidgetItem* treeItem, QDir dir, QString rPath, QMap<QString, QVariant>& templateCache, bool addOnlyClassesWithStartChip, bool hideDocs, bool useFilePathTooltips, int& count) const;
	QString _dirKey;
	QString _dirDefaultValue;
	QString _cacheKey;
	QString _cacheDefaultFile;
	bool _includeFunctions;

	List<Path> _templatesPaths;
	QString _cacheFileName;
};

}