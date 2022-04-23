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
#include "SimpleTreeWidgetDialogPage.h"
#include "M3DEngine/Chip.h"

using namespace m3d;



SimpleTreeWidgetDialogPage::SimpleTreeWidgetDialogPage(bool instantUpdateMode) : _instantUpdateMode(instantUpdateMode)
{
	_initData = (RData)0;
	_defaultData = (RData)0;
	_isInit = false;

	QWidget* w = static_cast<QWidget*>(_ptr);
	QGridLayout *gridLayout = new QGridLayout(w);
	QTreeWidget *treeWidget = new QTreeWidget(w);
	treeWidget->header()->setVisible(false);
	treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	gridLayout->addWidget(treeWidget, 0, 0, 1, 1);
	_treeWidget = treeWidget;

	setPreferredSize(400, 300);

	if (_instantUpdateMode)
		QObject::connect(treeWidget, &QTreeWidget::itemSelectionChanged, [this]() { _onSelectionChanged(); });

	_onSelectedChangedCallback = [this](RData data) -> bool { return this->onSelectionChanged(data); };
}

SimpleTreeWidgetDialogPage::~SimpleTreeWidgetDialogPage()
{
}

RPtr SimpleTreeWidgetDialogPage::AddItem(String text, RData data)
{
	QTreeWidget* treeWidget = static_cast<QTreeWidget*>(_treeWidget);
	QTreeWidgetItem* itm = new QTreeWidgetItem(treeWidget, QStringList(TOQSTRING(text)));
	itm->setData(0, Qt::UserRole, data);
	return itm;
}

RPtr SimpleTreeWidgetDialogPage::AddItem(RPtr parentItem, String text, RData data)
{
	QTreeWidgetItem* pItem = static_cast<QTreeWidgetItem*>(parentItem);
	QTreeWidgetItem* itm = new QTreeWidgetItem(pItem, QStringList(TOQSTRING(text)));
	itm->setData(0, Qt::UserRole, data);
	return itm;
}

void SimpleTreeWidgetDialogPage::SetInit(RData data, RData defaultData)
{
	_initData = data;
	_defaultData = defaultData;

	QTreeWidget *treeWidget = static_cast<QTreeWidget*>(_treeWidget);

	for (QTreeWidgetItemIterator itr(treeWidget); *itr; itr++) {
		if ((*itr)->data(0, Qt::UserRole) == _initData) {
			(*itr)->setSelected(true);
			treeWidget->setCurrentItem(*itr);
			treeWidget->scrollToItem(*itr);
			break;
		}
	}
	_isInit = true;
}

void SimpleTreeWidgetDialogPage::sort(RSortOrder order)
{
	static_cast<QTreeWidget*>(_treeWidget)->sortByColumn(0, (Qt::SortOrder)order);
}

RData SimpleTreeWidgetDialogPage::GetCurrent()
{
	QTreeWidget* treeWidget = static_cast<QTreeWidget*>(_treeWidget);
	QList<QTreeWidgetItem*> itms = treeWidget->selectedItems();
	if (itms.empty())
		return _defaultData;
	return (RData)itms[0]->data(0, Qt::UserRole).toUInt();
}

void SimpleTreeWidgetDialogPage::OnOK()
{
	RData current = GetCurrent();
	if (!_instantUpdateMode && _initData != current)
		_onSelectionChanged();
}

void SimpleTreeWidgetDialogPage::OnCancel()
{
	if (_instantUpdateMode && _initData != GetCurrent())
		if (_onSelectedChangedCallback)
			if (_onSelectedChangedCallback(_initData))
				SetDirty();
}

void SimpleTreeWidgetDialogPage::AfterApply()
{
	_initData = GetCurrent();
}

void SimpleTreeWidgetDialogPage::_onSelectionChanged()
{
	if (!_isInit)
		return;
	QTreeWidget* treeWidget = static_cast<QTreeWidget*>(_treeWidget);
	QList<QTreeWidgetItem*> itms = treeWidget->selectedItems();

	if (_onSelectedChangedCallback) {
		if (itms.empty()) {
			if (_onSelectedChangedCallback(_defaultData)) {
				GetChip()->SetName(GetChip()->GetChipDesc().name);
				SetDirty();
			}
		}
		else {
			if (_onSelectedChangedCallback((RData)itms[0]->data(0, Qt::UserRole).toUInt())) {
				GetChip()->SetName(FROMQSTRING(itms[0]->text(0)));
				SetDirty();
			}
		}
	}
}