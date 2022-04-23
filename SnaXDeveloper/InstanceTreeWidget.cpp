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
#include "InstanceTreeWidget.h"
#include "M3DEngine/ClassInstance.h"
#include "M3DEngineExt/ClassExt.h"
#include <QMouseEvent>
#include "AppSignals.h"

using namespace m3d;



InstanceTreeWidget::InstanceTreeWidget(QWidget *parent) : QTreeWidget(parent)
{
	connect(&AppSignals::instance(), &AppSignals::classRenamed, this, &InstanceTreeWidget::onClassNameChanged);
	connect(&AppSignals::instance(), &AppSignals::inheritanceChanged, this, &InstanceTreeWidget::updateFilter);
	connect(&AppSignals::instance(), &AppSignals::instanceRegistered, this, &InstanceTreeWidget::onInstanceRegistered);
	connect(&AppSignals::instance(), &AppSignals::instanceUnregistered, this, &InstanceTreeWidget::onInstanceUnregistered);
	connect(&AppSignals::instance(), &AppSignals::instanceNameChanged, this, &InstanceTreeWidget::onInstanceNameChanged);


//	connect(this, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(onCurrentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
	connect(this, &InstanceTreeWidget::itemDoubleClicked, this, &InstanceTreeWidget::onItemDoubleClicked);
	_filter = nullptr;

	sortByColumn(0, Qt::AscendingOrder);

	_menu = new QMenu(this);
	_a1 = _menu->addAction("Enable Filter");
	_a1->setCheckable(true);
	_a1->setChecked(true);
}

InstanceTreeWidget::~InstanceTreeWidget()
{
}

void InstanceTreeWidget::onInstanceRegistered(ClassInstance *instance)
{
	Class *cg = instance->GetClass();
	assert(cg);
	auto n = _clazzItems.find(cg);
	if (n == _clazzItems.end()) {
		QTreeWidgetItem *i = new QTreeWidgetItem(this, QStringList(TOQSTRING(cg->GetName())), 1);
		i->setData(0, Qt::UserRole, (qulonglong)cg);
		i->setHidden(!cg->IsBaseClass(_filter));
		n = _clazzItems.insert(std::make_pair(cg, i)).first;
	}
	QString name = TOQSTRING(instance->GetName());
	if (name.size() == 0)
		name = QString("Unnamed (%1)").arg(instance->GetRuntimeID());
	QTreeWidgetItem *i = new QTreeWidgetItem(n->second, QStringList(name), 2);
	i->setData(0, Qt::UserRole, (qulonglong)instance);
	_instanceItems.insert(std::make_pair(instance, i));
}

void InstanceTreeWidget::onInstanceUnregistered(ClassInstance *instance)
{
	auto n = _instanceItems.find(instance);
	if (n == _instanceItems.end()) {
		delete n->second;
		_instanceItems.erase(n);
		auto m = _clazzItems.find(instance->GetClass());
		if (m->second->childCount() == 0) {
			delete m->second;
			_clazzItems.erase(m);
		}
	}
}

void InstanceTreeWidget::onInstanceNameChanged(ClassInstance *instance)
{
	auto n = _instanceItems.find(instance);
	if (n != _instanceItems.end()) {
		QString name = TOQSTRING(instance->GetName());
		if (name.size() == 0)
			name = QString("Unnamed (%1)").arg(instance->GetRuntimeID());
		n->second->setText(0, name);
	}
}

void InstanceTreeWidget::onClassNameChanged(ClassExt *cg)
{
	auto m = _clazzItems.find(cg);
	if (m != _clazzItems.end()) {
		m->second->setText(0, TOQSTRING(cg->GetName()));
	}
}

void InstanceTreeWidget::setFilter(ClassExt *cg)
{
	if (_filter == cg)
		return;
	_filter = cg;
	updateFilter();
}

void InstanceTreeWidget::updateFilter()
{
	bool useFilter = _a1->isChecked();
	if (_filter && useFilter) {
		for (auto n : _clazzItems)
			n.second->setHidden(!n.first->IsBaseClass(_filter));
		QTreeWidgetItem *itm = currentItem();
		if (itm && itm->isHidden())
			setCurrentItem(nullptr);
	}
	else {
		for (const auto &n : _clazzItems)
			n.second->setHidden(useFilter);
		if (useFilter)
			setCurrentItem(nullptr);
	}
}

void InstanceTreeWidget::onItemDoubleClicked(QTreeWidgetItem *item, int column)
{
	if (item->type() == 1) {
		ClassExt* cg = (ClassExt*)item->data(0, Qt::UserRole).toULongLong();
		AppSignals::instance().openClass(cg, nullptr);
	}
	else if (item->type() == 2) {
		ClassInstance *instance = (ClassInstance*)item->data(0, Qt::UserRole).toULongLong();
		AppSignals::instance().openInstanceDialog(instance);
	}
}

ClassInstance *InstanceTreeWidget::GetCurrentInstance()
{
	QTreeWidgetItem *itm = currentItem();
	return itm && itm->type() == 2 ? (ClassInstance*)itm->data(0, Qt::UserRole).toULongLong() : nullptr;
}

void InstanceTreeWidget::contextMenuEvent ( QContextMenuEvent * event )
{
	QAction *a = _menu->exec(viewport()->mapToGlobal(event->pos()));
	if (a == _a1) {
		updateFilter();
	}
}
