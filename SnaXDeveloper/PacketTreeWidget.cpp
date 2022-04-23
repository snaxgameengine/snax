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
#include "PacketTreeWidget.h"
#include <QMouseEvent>
#include "DXEditorWidget.h"
#include <qapplication>
#include <QMenu>
#include <QAction>
#include "M3DEngine/ChipManager.h"
#include "M3DEngine/Engine.h"
#include "AppSignals.h"


using namespace m3d;


PacketTreeWidget::PacketTreeWidget(QWidget * parent) : CustomDraggableTreeWidget(parent)
{
	header()->reset();
	headerItem()->setText(0, "Name");
	header()->setStretchLastSection(true);
	sortByColumn(0, Qt::AscendingOrder);
	setHeaderHidden(true);
	setSelectionBehavior(SelectionBehavior::SelectRows);

	connect(&AppSignals::instance(), &AppSignals::initChips, this, &PacketTreeWidget::initiate);
	connect(&AppSignals::instance(), &AppSignals::packetLoaded, this, &PacketTreeWidget::onPacketLoaded);
	connect(&AppSignals::instance(), &AppSignals::updateSettings, this, &PacketTreeWidget::updateSettings);

	_menu = new QMenu(this);
	_a1 = _menu->addAction("Packet List");
	_a1->setCheckable(true);
	_a2 = _menu->addAction("Chip List");
	_a2->setCheckable(true);
	_a3 = _menu->addAction("Chip Tree");
	_a3->setCheckable(true);
	_ag = new QActionGroup(this);
	_ag->addAction(_a1);
	_ag->addAction(_a2);
	_ag->addAction(_a3);
	_a2->setChecked(true);
	_current = _a2;
}

PacketTreeWidget::~PacketTreeWidget()
{
}

void PacketTreeWidget::updateSettings(QSettings& s, bool load)
{
	s.beginGroup("mainWindow/" + objectName());
	if (load) {
		int n = 0;
		n = s.value("listType", 1).toInt();
		_current = n == 0 ? _a1 : (n == 1 ? _a2 : _a3);
		_current->setChecked(true);
		if (_isInit)
			initiate();
	}
	else {
		int n = _current == _a1 ? 0 : (_current == _a2 ? 1 : 2);
		s.setValue("listType", n);
	}
	s.endGroup();
}

void PacketTreeWidget::initiate()
{
	static QIcon packetIcon = QIcon(":/EditorApp/Resources/box.png");
	static QIcon chipIcon = QIcon(":/EditorApp/Resources/chip.png");
	static QIcon chipIconOpen = QIcon(":/EditorApp/Resources/box-open.png");

	_isInit = true;

	clear();

	const PacketPtrByStringMap &pm = engine->GetChipManager()->GetPacketMap();

	if (_current == _a1) { // Packet List
		for (const auto &n : pm) {
			QTreeWidgetItem *item = new QTreeWidgetItem(this, QStringList(TOQSTRING(n.second->name)));
			item->setData(0, Qt::DecorationRole, n.second->module.load() ? chipIconOpen : packetIcon);
			item->setData(0, Qt::UserRole, -1);
			for (const auto &m : n.second->chips) {
				QTreeWidgetItem *chItem = new QTreeWidgetItem(item, QStringList(TOQSTRING(m.second->chipDesc.name)));
				chItem->setData(0, Qt::DecorationRole, chipIcon);
				if (m.second->chipDesc.usage == ChipDesc::HIDDEN) {
					chItem->setData(0, Qt::ForegroundRole, QColor(128,128,128));
					chItem->setData(0, Qt::UserRole, -1);
				}
				else {
					chItem->setData(0, Qt::UserRole, m.second->chipTypeIndex);
				}
			}
		}
		setIndentation(20);
	}
	else if (_current == _a2) { // Chip list
		for (const auto &n : pm) {
			for (const auto &m : n.second->chips) {
				if (m.second->chipDesc.usage == ChipDesc::HIDDEN)
					continue;
				QTreeWidgetItem *chItem = new QTreeWidgetItem(this, QStringList(TOQSTRING(m.second->chipDesc.name)));
				chItem->setData(0, Qt::DecorationRole, chipIcon);
				chItem->setData(0, Qt::UserRole, m.second->chipTypeIndex);
			}
		}
		setIndentation(0);
	}
	else { // Chip hierachy
		Map<Guid, QTreeWidgetItem*> tmp;
		for (const auto &n : pm) {
			for (const auto &m : n.second->chips) {
				auto pNode = tmp.insert(std::make_pair(m.second->chipDesc.basetype, (QTreeWidgetItem*)0)).first;
				if (pNode->second == 0)
					pNode->second = new QTreeWidgetItem();
				auto node = tmp.insert(std::make_pair(m.second->chipDesc.type, (QTreeWidgetItem*)0)).first;
				if (node->second == 0)
					node->second = new QTreeWidgetItem();
				node->second->setText(0, TOQSTRING(m.second->chipDesc.name));
				node->second->setData(0, Qt::DecorationRole, chipIcon);
				if (m.second->chipDesc.usage == ChipDesc::HIDDEN) {
					node->second->setData(0, Qt::ForegroundRole, QColor(128,128,128));
					node->second->setData(0, Qt::UserRole, -1);
				}
				else {
					node->second->setData(0, Qt::UserRole, m.second->chipTypeIndex);
				}
				if (pNode->second == node->second)
					addTopLevelItem(node->second);
				else
					pNode->second->addChild(node->second);
			}
		}
		setIndentation(20);
	}
}

void PacketTreeWidget::onPacketLoaded(QString packet)
{
	static QIcon chipIconOpen = QIcon(":/EditorApp/Resources/box-open.png");

	if (_current == _a1) {
		QList<QTreeWidgetItem*> items = this->findItems(packet, Qt::MatchExactly);
		if (items.size() == 1)
			items.front()->setIcon(0, chipIconOpen);
	}
}

void PacketTreeWidget::contextMenuEvent ( QContextMenuEvent * event )
{
	QAction *a = _menu->exec(viewport()->mapToGlobal(event->pos()));
	if (a && a != _current) {
		_current = a;
		initiate();
	}
}

bool PacketTreeWidget::startDragging(QTreeWidgetItem *item)
{
	ChipTypeIndex idx = (ChipTypeIndex)item->data(0, Qt::UserRole).toUInt();
	if (idx == -1)
		return false;
	return true;
}

void PacketTreeWidget::endDragging(QTreeWidgetItem *item, QWidget *dropAt, const QPoint &pos, Qt::KeyboardModifiers modifiers)
{
	ChipTypeIndex idx = (ChipTypeIndex)item->data(0, Qt::UserRole).toUInt();
	if (idx == -1)
		return;
	DXEditorWidget *ew = dynamic_cast<DXEditorWidget*>(dropAt);
	if (ew)
		ew->addChip(idx);
}

QCursor PacketTreeWidget::dragCursor()
{
	static QCursor c(QCursor(QIcon(":/EditorApp/Resources/chip.png").pixmap(32)));
	return c;
}
