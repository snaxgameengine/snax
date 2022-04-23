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
#include "ClassInstanceRefByValueMapChip_Dlg.h"
#include "M3DEngine/Class.h"
#include "StdChips/InstanceData.h"
#include "StdChips/Text.h"
#include "StdChips/Value.h"
#include "StdChips/VectorChip.h"
#include "ChipDialogs/ChipDialogManager.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"




using namespace m3d;

DIALOGDESC_DEF(ClassInstanceRefByValueMapChip_Dlg, CLASSINSTANCEREFBYVALUEMAPCHIP_GUID);

#define KEY_COLUMN 0
#define INSTANCE_COLUMN 1
#define NAME_COLUMN 2
#define DATA_START 3

#define HORIZONTAL_HEADER_UNDERSCORED Qt::UserRole + 0
#define HORIZONTAL_HEADER_BASE_COUNT Qt::UserRole + 1
#define HORIZONTAL_HEADER_REF_COUNT Qt::UserRole + 2

#define KEY_COLUMN_INSTANCE_ID Qt::UserRole + 0
#define KEY_COLUMN_KEY Qt::UserRole + 1

#define DATA_COLUMN_DATA_ID Qt::UserRole + 0
#define DATA_COLUMN_REFRESH_INFO Qt::UserRole + 1


ClassInstanceRefByValueMapChip_Dlg::ClassInstanceRefByValueMapChip_Dlg()
{
	ui.setupUi(this);
	QPalette p = ui.tableWidget->viewport()->palette();
	p.setColor(QPalette::Base, Qt::gray);
	ui.tableWidget->viewport()->setPalette(p);

	ui.tableWidget->setColumnWidth(0, 150);
	ui.tableWidget->setColumnWidth(1, 170);
	ui.tableWidget->setColumnWidth(2, 150);
	ui.tableWidget->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
	ui.tableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
	ui.tableWidget->horizontalHeaderItem(2)->setTextAlignment(Qt::AlignLeft);
}

ClassInstanceRefByValueMapChip_Dlg::~ClassInstanceRefByValueMapChip_Dlg()
{
}

// Adds a new data column
Map<Guid, int>::iterator ClassInstanceRefByValueMapChip_Dlg::_addColumn(const Guid& globalID, int baseCount, QString name, bool underscored) // OK
{
	int column = ui.tableWidget->columnCount(); // We add it to the end of the (logical) columns
	auto p = _dataTypeMap.insert(std::make_pair(globalID, column)).first;
	ui.tableWidget->insertColumn(column);
	ui.tableWidget->setColumnWidth(column, 150);
	QTableWidgetItem* us = new QTableWidgetItem(name); // Create header item
	us->setData(HORIZONTAL_HEADER_UNDERSCORED, underscored); // underscored?
	us->setData(HORIZONTAL_HEADER_BASE_COUNT, baseCount); // base count, used for sorting of columns
	us->setData(HORIZONTAL_HEADER_REF_COUNT, 1); // item count. For removing column when no more items.
	us->setTextAlignment(Qt::AlignLeft);
	ui.tableWidget->setHorizontalHeaderItem(column, us);
	_setColumnInOrder(column);
	if (ui.checkBox_hideMembers->isChecked() && underscored)
		ui.tableWidget->hideColumn(column); // It's visible by default
	return p;
}

// Set data column at correct place.
void ClassInstanceRefByValueMapChip_Dlg::_setColumnInOrder(int column) // OK
{
	QTableWidgetItem* us = ui.tableWidget->horizontalHeaderItem(column); // column is the logical index.
	int visualColumn = ui.tableWidget->horizontalHeader()->visualIndex(column); // find visual index
	int i = DATA_START;
	for (; i < ui.tableWidget->columnCount(); i++) {
		if (i == visualColumn)
			continue;
		QTableWidgetItem* other = ui.tableWidget->horizontalHeaderItem(ui.tableWidget->horizontalHeader()->logicalIndex(i));
		int baseCountCompare = us->data(HORIZONTAL_HEADER_BASE_COUNT).toInt() - other->data(HORIZONTAL_HEADER_BASE_COUNT).toInt();
		if (baseCountCompare < 0 || baseCountCompare == 0 && us->text() < other->text())
			break;
	}
	if (i > visualColumn)
		i--;
	if (visualColumn != i)
		ui.tableWidget->horizontalHeader()->moveSection(visualColumn, i); // from visual index to visual index
}

// Add rows and set header numbering
void ClassInstanceRefByValueMapChip_Dlg::_addRows(int from, int count) // OK
{
	if (from < 0 || count < 0)
		return;
	if (ui.tableWidget->rowCount() == 0) {
		ui.tableWidget->setRowCount(count);
		for (int i = 0; i < count; i++)
			ui.tableWidget->setVerticalHeaderItem(i, new QTableWidgetItem(QString::number(i)));
	}
	else {
		for (int i = from; i < from + count; i++) {
			ui.tableWidget->insertRow(i);
			ui.tableWidget->setVerticalHeaderItem(i, new QTableWidgetItem(QString::number(i)));
		}
		for (int i = from + count, rowCount = ui.tableWidget->rowCount(); i < rowCount; i++)
			ui.tableWidget->verticalHeaderItem(i)->setText(QString::number(i)); // Renumber the ones below..
	}
}

// Remove rows
void ClassInstanceRefByValueMapChip_Dlg::_removeRows(int from, int count) // OK
{
	if (from < 0 || count < 0)
		return;
	if (from == 0 && count == ui.tableWidget->rowCount()) {
		ui.tableWidget->setRowCount(0);
		// Remove ALL data columns!
		while (ui.tableWidget->columnCount() > DATA_START)
			ui.tableWidget->removeColumn(ui.tableWidget->columnCount() - 1);
		_dataTypeMap.clear();
	}
	else {
		_clearRows(from, count); // remove content from rows...
		for (int i = 0; i < count; i++)
			ui.tableWidget->removeRow(from); // remove rows.
		for (int i = from; i < ui.tableWidget->rowCount(); i++)
			ui.tableWidget->verticalHeaderItem(i)->setText(QString::number(i)); // renumber rows belows ones removed..
	}
}

void ClassInstanceRefByValueMapChip_Dlg::_clearRows(int from, int count) // OK
{
	if (from < 0 || count < 0)
		return;
	for (int i = from; i < from + count; i++) { // erase data in selected columns..
		QTableWidgetItem* itmKey = ui.tableWidget->item(i, KEY_COLUMN);
		if (!itmKey)
			continue;
		for (int j = DATA_START; j < ui.tableWidget->columnCount(); j++) { // Iterate data items...
			QTableWidgetItem* itm = ui.tableWidget->item(i, j);
			if (!itm)
				continue;
			// Decrease refCount for data header item
			QTableWidgetItem* hitm = ui.tableWidget->horizontalHeaderItem(j);
			int refCount = hitm->data(HORIZONTAL_HEADER_REF_COUNT).toInt() - 1;
			hitm->setData(HORIZONTAL_HEADER_REF_COUNT, refCount);
			ui.tableWidget->setCellWidget(i, j, nullptr);
			delete itm; // Delete data item
			if (refCount == 0) {
				ui.tableWidget->hideColumn(j); // Hide data column if no more data for it
			}
		}
		delete itmKey; // Delete item for first column
		delete ui.tableWidget->item(i, INSTANCE_COLUMN); // Delete item for second column
		delete ui.tableWidget->item(i, NAME_COLUMN); // Delete item for third column
	}
}

void ClassInstanceRefByValueMapChip_Dlg::_setInstance(int row, value key, ClassInstanceRef ref) // OK
{
	if (ui.tableWidget->item(row, KEY_COLUMN) != nullptr)
		_clearRows(row, 1); // Ensure row is clean.

	// Column 0 (map key)
	QTableWidgetItem* itm0 = new QTableWidgetItem(QString::number(key));
	itm0->setData(KEY_COLUMN_INSTANCE_ID, InvalidClassInstanceID);
	itm0->setData(KEY_COLUMN_KEY, key);
	itm0->setBackground(Qt::green);
	ui.tableWidget->setItem(row, KEY_COLUMN, itm0);

	if (!ref)
		return;

	QString typeName;
	if (ref->IsSerialized()) {
		const ClassInstanceSerialization* s = ref->GetSerialization();
		typeName = TOQSTRING(GuidToString(s->clazzid)); // Set name of the instance type to be the guid of the class.

		// Iterate data
		for (const auto &n : s->data) {
			auto p = _dataTypeMap.find(n.first); // Does column for this InstanceData exist?
			if (p == _dataTypeMap.end()) { // No?
				p = _addColumn(n.first, -1, TOQSTRING(GuidToString(n.first)), false);
			}
			else {
				QTableWidgetItem* itm = ui.tableWidget->horizontalHeaderItem(p->second);
				itm->setData(HORIZONTAL_HEADER_REF_COUNT, itm->data(HORIZONTAL_HEADER_REF_COUNT).toInt() + 1); // Indicate one more item at this column
				ui.tableWidget->showColumn(p->second); // ensure column is visible!
			}
			_setDataItem(n.second, ref->GetRuntimeID(), row, p->second);
		}
	}
	else {
		typeName = TOQSTRING(ref->GetClass()->GetName());

		// Iterate data
		for (const auto &n : ref->GetData()) {
			auto p = _dataTypeMap.find(n.first->GetGlobalID()); // Does column for this InstanceData exist?
			if (p == _dataTypeMap.end()) { // No?
				bool underscorePrefixed = n.first->GetName().size() && n.first->GetName()[0] == MCHAR('_');
				p = _addColumn(n.first->GetGlobalID(), n.first->GetClass()->GetBaseCount(), QString("%1\n(%2)").arg(TOQSTRING(n.first->GetName())).arg(TOQSTRING(n.first->GetClass()->GetName())), underscorePrefixed);
			}
			else {
				QTableWidgetItem* itm = ui.tableWidget->horizontalHeaderItem(p->second); // Item at logical index
				itm->setData(HORIZONTAL_HEADER_REF_COUNT, itm->data(HORIZONTAL_HEADER_REF_COUNT).toInt() + 1); // Indicate one more item at this column
				if (itm->data(HORIZONTAL_HEADER_BASE_COUNT).toInt() == -1) { // It can happen that this was serialized at another item... (baseCount == -1)
					itm->setData(HORIZONTAL_HEADER_BASE_COUNT, n.first->GetClass()->GetBaseCount());
					itm->setText(QString("%1 (%2)").arg(TOQSTRING(n.first->GetName())).arg(TOQSTRING(n.first->GetClass()->GetName())));
					bool underscorePrefixed = n.first->GetName().size() && n.first->GetName()[0] == MCHAR('_');
					itm->setData(HORIZONTAL_HEADER_UNDERSCORED, underscorePrefixed);
					_setColumnInOrder(p->second);
				}
				if (ui.checkBox_hideMembers->isChecked() && itm->data(HORIZONTAL_HEADER_UNDERSCORED).toBool())
					ui.tableWidget->hideColumn(p->second); // Hide underscored
				else
					ui.tableWidget->showColumn(p->second); // ensure column is visible
			}
			_setDataItem(n.second, ref->GetRuntimeID(), row, p->second);
		}
	}

	itm0->setData(KEY_COLUMN_INSTANCE_ID, (uint32)ref->GetRuntimeID());

	// Column 1
	QTableWidgetItem* itm1 = new QTableWidgetItem(typeName);
	itm1->setFlags((itm1->flags() & ~Qt::ItemIsEditable) | Qt::ItemIsUserCheckable);
	itm1->setBackground(Qt::white);
	itm1->setCheckState(ref.IsOwner() ? Qt::Checked : Qt::Unchecked);
	ui.tableWidget->setItem(row, INSTANCE_COLUMN, itm1);

	// Column 2
	QTableWidgetItem* itm2 = new QTableWidgetItem(TOQSTRING(ref->GetName()));
	itm2->setBackground(Qt::white);
	itm2->setToolTip(QString("ID: %1").arg(ref->GetRuntimeID()));
	if (ref->GetOwner() == nullptr)
		itm2->setFlags(itm1->flags() & ~Qt::ItemIsEditable);
	ui.tableWidget->setItem(row, NAME_COLUMN, itm2);
}

void ClassInstanceRefByValueMapChip_Dlg::_setDataItem(Chip* data, ClassInstanceID instanceID, int row, int column) // OK
{
	static QIcon f(":/StdChips_Dialogs/Resources/dialog.png");
	static QPixmap formPixMap = f.pixmap(16, 16);

	QTableWidgetItem* itm = new QTableWidgetItem("");
	itm->setData(DATA_COLUMN_DATA_ID, data->GetID());
	itm->setBackground(Qt::white);

	QString txt;
	int align = Qt::AlignLeft;
	QVariant extra;

	Text* t = dynamic_cast<Text*>(data);
	if (t) {
		txt = TOQSTRING(t->GetChipText());
		extra = t->GetUpdateStamp();
	}
	else {
		Value* v = dynamic_cast<Value*>(data);
		if (v) {
			value val = v->GetChipValue();
			txt = QString::number(val);
			extra = val;
			align = Qt::AlignRight;
		}
		else {
			VectorChip* vec = dynamic_cast<VectorChip*>(data);
			if (vec) {
				XMFLOAT4 v = vec->GetChipVector();
				txt = QString::number(v.x) + "; " + QString::number(v.y) + "; " + QString::number(v.z) + "; " + QString::number(v.w);
				align = Qt::AlignRight;
				extra.fromValue<XMFLOAT4>(v);
			}
			else {
				itm->setFlags(itm->flags() & ~Qt::ItemIsEditable);
				QLabel* l = new QLabel();
				l->setPixmap(formPixMap);
				l->setAlignment(Qt::AlignCenter);
				QPalette pal = l->palette();
				pal.setColor(l->backgroundRole(), Qt::white);
				l->setPalette(pal);
				ui.tableWidget->setCellWidget(row, column, l);
			}
		}
	}

	itm->setText(txt);
	itm->setTextAlignment(align | Qt::AlignVCenter);
	itm->setData(DATA_COLUMN_REFRESH_INFO, extra);
	ui.tableWidget->setItem(row, column, itm);
}

bool ClassInstanceRefByValueMapChip_Dlg::_getInstance(int row, ClassInstanceRef& ref) // OK
{
	QTableWidgetItem* itmKey = ui.tableWidget->item(row, KEY_COLUMN);
	assert(itmKey);

	value key = _keyAtRow(row);
	ClassInstanceID instanceID = _instanceIDAtRow(row);

	auto n = GetChip()->GetMap().find(key);
	if (n == GetChip()->GetMap().end())
		return false; // No such key in map

	if (!n->second) {
		if (instanceID != InvalidClassInstanceID)
			return false; // Instance in map does not match table row
		ref = ClassInstanceRef();
		return true;
	}

	if (n->second->GetRuntimeID() != instanceID)
		return false; // Instance in map does not match table row

	ref = n->second;

	return true;
}

// Returns the data-chip for the given instance and with the given id.
Chip* ClassInstanceRefByValueMapChip_Dlg::_getDataChip(ClassInstanceRef instance, ChipID dataID) // OK
{
	if (!instance)
		return nullptr;

	if (instance->IsSerialized()) {
		const ClassInstanceSerialization* s = instance->GetSerialization();

		for (const auto &n : s->data)
			if (n.second->GetID() == dataID)
				return n.second;
	}
	else {
		const ChipPtrByInstanceDataPtrMap& m = instance->GetData();
		for (const auto &n : m)
			if (n.second->GetID() == dataID)
				return n.second;
	}
	return nullptr;
}

void ClassInstanceRefByValueMapChip_Dlg::_swapRows(uint32 a, uint32 b)
{
	if (a == b || std::max(a, b) >= ui.tableWidget->rowCount())
		return;
	for (uint32 i = 0; i < ui.tableWidget->columnCount(); i++) {
		QTableWidgetItem* itmA = ui.tableWidget->takeItem(a, i);
		QTableWidgetItem* itmB = ui.tableWidget->takeItem(b, i);
		ui.tableWidget->setItem(a, i, itmB);
		ui.tableWidget->setItem(b, i, itmA);
		QWidget* wA = ui.tableWidget->cellWidget(a, i);
		QWidget* wB = ui.tableWidget->cellWidget(b, i);
		ui.tableWidget->setCellWidget(a, i, wB);
		ui.tableWidget->setCellWidget(b, i, wA);
	}
}

void ClassInstanceRefByValueMapChip_Dlg::Init() // OK
{
	_addRows(0, GetChip()->GetContainerSize());
	uint32 i = 0;
	for (const auto &n : GetChip()->GetMap()) {
		ClassInstanceRef r = n.second;
		r.Prepare(GetChip());
		_setInstance(i, n.first, r);
		i++;
	}

	ui.checkBox_saveInstance->setChecked(GetChip()->IsSaveContent());
	ui.checkBox_preload->setChecked(GetChip()->isPreload());
	_initSaveRef = GetChip()->IsSaveContent();
	_initPreload = GetChip()->isPreload();

	updateButtons();
}

void ClassInstanceRefByValueMapChip_Dlg::OnCancel() // OK
{
	GetChip()->SetSaveContent(_initSaveRef);
	GetChip()->setPreload(_initPreload);
}

void ClassInstanceRefByValueMapChip_Dlg::AfterApply() // OK
{
	_initSaveRef = GetChip()->IsSaveContent();
	_initPreload = GetChip()->isPreload();
}

// Refresh the UI. Called every 0.5 sec.
void ClassInstanceRefByValueMapChip_Dlg::Refresh() // OK
{
	int row = ui.tableWidget->rowAt(0);
	if (row == -1)
		return;
	int end = ui.tableWidget->rowAt(ui.tableWidget->viewport()->height());
	if (end == -1)
		end = ui.tableWidget->rowCount();

	bool editing = ui.tableWidget->editing();

	for (; row < end; row++) { // We do only update the visible ones. Performance!
		value key = _keyAtRow(row);
		ClassInstanceID instanceID = _instanceIDAtRow(row);
		auto n = GetChip()->GetMap().find(key);
		if (n == GetChip()->GetMap().end()) {
			_removeRows(row, 1);
			end = qMin(end + 1, ui.tableWidget->rowCount());
			continue; // No instance at this row.
		}

		ClassInstanceRef instance = n->second;

		if (!instance && instanceID == InvalidClassInstanceID)
			continue;

		if (!instance && instanceID != InvalidClassInstanceID || instance->GetRuntimeID() != instanceID) {
			_setInstance(row, key, instance);
			continue;
		}

		// Update owner flag
		QTableWidgetItem* itmInstance = ui.tableWidget->item(row, INSTANCE_COLUMN);
		if (itmInstance->checkState() == Qt::Checked != instance.IsOwner())
			itmInstance->setCheckState(instance.IsOwner() ? Qt::Checked : Qt::Unchecked);

		// Update name
		QString name = TOQSTRING(instance->GetName());
		QTableWidgetItem* itmName = ui.tableWidget->item(row, NAME_COLUMN);
		if (!editing || ui.tableWidget->currentItem() != itmName) {
			if (name != itmName->text())
				itmName->setText(name);
		}

		for (int col = DATA_START; col < ui.tableWidget->columnCount(); col++) { // Update data columns!
			QTableWidgetItem* w = ui.tableWidget->item(row, col);
			if (!w)
				continue;

			if (editing && ui.tableWidget->currentItem() == w)
				continue; // Editing this item

			ChipID dataID = w->data(DATA_COLUMN_DATA_ID).toUInt();

			Chip* c = _getDataChip(instance, dataID);
			Text* txt = dynamic_cast<Text*>(c);
			if (txt) {
				UpdateStamp extra = w->data(DATA_COLUMN_REFRESH_INFO).toUInt();
				UpdateStamp us = txt->GetUpdateStamp();
				if (extra != us) {
					w->setData(DATA_COLUMN_REFRESH_INFO, us);
					w->setText(TOQSTRING(txt->GetChipText()));
				}
				continue;
			}
			Value* v = dynamic_cast<Value*>(c);
			if (v) {
				value extra = w->data(DATA_COLUMN_REFRESH_INFO).toDouble();
				value val = v->GetChipValue();
				if (extra != val) {
					w->setData(DATA_COLUMN_REFRESH_INFO, val);
					w->setText(QString::number(val));
				}
				continue;
			}
			VectorChip* vec = dynamic_cast<VectorChip*>(c);
			if (vec) {
				XMFLOAT4 extra = w->data(DATA_COLUMN_REFRESH_INFO).value<XMFLOAT4>();
				XMFLOAT4 val = vec->GetChipVector();
				if (extra.x != val.x || extra.y != val.y || extra.z != val.z || extra.w != val.w) {
					w->setText(QString::number(val.x) + "; " + QString::number(val.y) + "; " + QString::number(val.z) + "; " + QString::number(val.w));
					w->setData(DATA_COLUMN_REFRESH_INFO, QVariant::fromValue<XMFLOAT4>(val));
				}
				continue;
			}
		}
	}
}

void ClassInstanceRefByValueMapChip_Dlg::saveInstanceClicked() // OK
{
	GetChip()->SetSaveContent(ui.checkBox_saveInstance->isChecked());
}

void ClassInstanceRefByValueMapChip_Dlg::refresh()
{
	_removeRows(0, ui.tableWidget->rowCount());

	_addRows(0, GetChip()->GetContainerSize());
	uint32 i = 0;
	for (const auto &n : GetChip()->GetMap()) {
		ClassInstanceRef r = n.second;
		r.Prepare(GetChip());
		_setInstance(i, n.first, r);
		i++;
	}
}

void ClassInstanceRefByValueMapChip_Dlg::showDataDialog(uint32 row, ClassInstanceID instanceID, ChipID dataID)
{
	/*	Chip* c = _getDataChip(_getInstance(row, instanceID), dataID);
		if (c)
			GetDialogManager()->OpenChipDialog(c);
	*/
}


void ClassInstanceRefByValueMapChip_Dlg::onTableDoubleClick(int row, int column) // OK
{
	ClassInstanceRef instance;
	if (!_getInstance(row, instance) || !instance)
		return;

	if (column < DATA_START) {
		GetDialogManager()->OpenInstanceDialog(instance.GetRawPtr());
		return;
	}
	QTableWidgetItem* w = ui.tableWidget->item(row, column);
	if (!w)
		return;
	ChipID dataID = w->data(DATA_COLUMN_DATA_ID).toUInt();
	Chip* c = _getDataChip(instance, dataID);
	if (c)
		GetDialogManager()->OpenChipDialog(c);
}

void ClassInstanceRefByValueMapChip_Dlg::onTableCellChanged(int row, int column) // OK
{
	if (row == -1 || column == -1)
		return;
	ClassInstanceRef ref;
	if (!_getInstance(row, ref))
		return;

	if (ui.tableWidget->editCalled() && column == INSTANCE_COLUMN) {
		if (!ref)
			return;
		QTableWidgetItem* itm = ui.tableWidget->item(row, INSTANCE_COLUMN);
		if (!itm)
			return;

		if (!ref.IsOwner() && itm->checkState() == Qt::Checked) {
			GetChip()->MakeOwner(ui.tableWidget->item(row, KEY_COLUMN)->data(KEY_COLUMN_KEY).toDouble());
			SetDirty(); // TODO: Mark old owner dirty?
		}

		itm->setCheckState(ref.IsOwner() ? Qt::Checked : Qt::Unchecked);

		return;
	}

	if (!ui.tableWidget->editing()) {
		return;
	}

	if (column == KEY_COLUMN) {
		value key = _keyAtRow(row);
		bool ok = false;
		value newKey = ui.tableWidget->item(row, KEY_COLUMN)->text().toDouble(&ok);
		if (!ok || key == newKey)
			return;
		if (GetChip()->MoveElement(key, newKey)) {

			auto n = GetChip()->GetMap().find(newKey);
			if (n == GetChip()->GetMap().end())
				return;

			auto m = n;
			std::advance(m, 1);

			int rowNew = 0;
			if (m != GetChip()->GetMap().end()) {
				for (; rowNew < ui.tableWidget->rowCount(); rowNew++)
					if (_keyAtRow(rowNew) == m->first)
						break;
			}
			else
				rowNew = ui.tableWidget->rowCount();

			_removeRows(row, 1);
			if (rowNew > row)
				rowNew--;
			_addRows(rowNew, 1);
			_setInstance(rowNew, newKey, n->second);
			ui.tableWidget->scrollToItem(ui.tableWidget->item(rowNew, 0));
			ui.tableWidget->item(rowNew, 0)->setSelected(true);
		}
	}
	else if (column == NAME_COLUMN) { // Name changed
		if (!ref)
			return;
		QTableWidgetItem* itmName = ui.tableWidget->item(row, NAME_COLUMN);
		if (!itmName)
			return;

		String name = FROMQSTRING(itmName->text());
		ref->SetName(name);
		SetDirty(ref->GetOwner()->GetClass()); // Make owner dirty!
	}
	else if (column >= DATA_START) { // Data updated...
		if (!ref)
			return;
		QTableWidgetItem* itmData = ui.tableWidget->item(row, column);
		if (!itmData)
			return;
		ChipID dataID = itmData->data(DATA_COLUMN_DATA_ID).toUInt();

		Chip* c = _getDataChip(ref, dataID);

		ui.tableWidget->blockSignals(true);

		Text* t = dynamic_cast<Text*>(c);
		if (t) {
			t->SetText(FROMQSTRING(itmData->text()));
			itmData->setData(DATA_COLUMN_REFRESH_INFO, t->GetUpdateStamp());
			SetDirty(ref->GetOwner()->GetClass());
		}
		else {
			Value* v = dynamic_cast<Value*>(c);
			if (v) {
				bool ok = false;
				value val = itmData->text().toDouble(&ok);
				if (ok) {
					v->SetValue(val);
					itmData->setData(DATA_COLUMN_REFRESH_INFO, val);
					SetDirty(ref->GetOwner()->GetClass());
				}
				else
					itmData->setText(QString::number(v->GetChipValue()));
			}
			else {
				VectorChip* vec = dynamic_cast<VectorChip*>(c);
				if (vec) {
					XMFLOAT4 n = vec->GetChipVector();
					QStringList lst = itmData->text().split(';');
					bool ok;
					float32 tmp;
					int c = 0;
					if (lst.size() > 0) { tmp = lst[0].toFloat(&ok); if (ok) { n.x = tmp; c++; } }
					if (lst.size() > 1) { tmp = lst[1].toFloat(&ok); if (ok) { n.y = tmp; c++; } }
					if (lst.size() > 2) { tmp = lst[2].toFloat(&ok); if (ok) { n.z = tmp; c++; } }
					if (lst.size() > 3) { tmp = lst[3].toFloat(&ok); if (ok) { n.w = tmp; c++; } }
					if (c > 0) {
						vec->SetVector(n);
						SetDirty(ref->GetOwner()->GetClass());
					}
					if (c < 4)
						itmData->setText(QString::number(n.x) + "; " + QString::number(n.y) + "; " + QString::number(n.z) + "; " + QString::number(n.w));
					itmData->setData(DATA_COLUMN_REFRESH_INFO, QVariant::fromValue<XMFLOAT4>(n));
				}
			}
		}
		ui.tableWidget->blockSignals(false);
	}
}

void ClassInstanceRefByValueMapChip_Dlg::hideMembersClicked() // OK
{
	bool b = ui.checkBox_hideMembers->isChecked();
	for (uint32 i = DATA_START, j = ui.tableWidget->columnCount(); i < j; i++) {
		if (b && ui.tableWidget->horizontalHeaderItem(i)->data(HORIZONTAL_HEADER_UNDERSCORED).toBool() || ui.tableWidget->horizontalHeaderItem(i)->data(HORIZONTAL_HEADER_REF_COUNT).toInt() == 0)
			ui.tableWidget->hideColumn(i);
		else
			ui.tableWidget->showColumn(i);
	}
}

void ClassInstanceRefByValueMapChip_Dlg::preloadClicked() // OK
{
	GetChip()->setPreload(ui.checkBox_preload->isChecked());
}

// Get range of selected items.
QPair<int, int> ClassInstanceRefByValueMapChip_Dlg::_getSelection() // OK
{
	QList<QTableWidgetSelectionRange> lst = ui.tableWidget->selectedRanges();
	if (lst.size() != 1 || lst.front().rowCount() == 0)
		return qMakePair(-1, -1);
	return qMakePair(lst.front().topRow(), lst.front().bottomRow());
}

// Shows a dialogbox where user can enter number and type of new instances!
bool ClassInstanceRefByValueMapChip_Dlg::_getKeyAndTypeOfNewInstance(value& key, Class** instanceOf) // OK
{
	QDialog d(this);
	QFormLayout* l = new QFormLayout(&d);
	QLineEdit* keyEdit = new QLineEdit();
	QComboBox* cb = new QComboBox();
	QStringList lst;
	lst << "<Empty>";
	for (const auto &n : engine->GetClassManager()->GetClasssByName())
		lst << TOQSTRING(n.first);
	cb->addItems(lst);
	QHBoxLayout* bl = new QHBoxLayout();
	QPushButton* okBtn = new QPushButton("OK");
	okBtn->setDefault(true);
	QPushButton* cancelBtn = new QPushButton("Cancel");
	bl->addWidget(okBtn);
	bl->addWidget(cancelBtn);
	l->addRow("Key:", keyEdit);
	l->addRow("Instance Type:", cb);
	l->addRow(bl);
	connect(okBtn, &QPushButton::clicked, &d, &QDialog::accept);
	connect(cancelBtn, &QPushButton::clicked, &d, &QDialog::reject);
	if (d.exec() == QDialog::Accepted) {
		bool ok = false;
		key = keyEdit->text().toDouble(&ok);
		if (!ok) return false;
		if (cb->currentIndex() != 0)
			*instanceOf = engine->GetClassManager()->GetClass(FROMQSTRING(cb->currentText()));
		return true;
	}
	return false;
}

value ClassInstanceRefByValueMapChip_Dlg::_keyAtRow(int row) // OK
{
	assert(row < ui.tableWidget->rowCount());
	return ui.tableWidget->item(row, 0)->data(KEY_COLUMN_KEY).toDouble();
}

ClassInstanceID ClassInstanceRefByValueMapChip_Dlg::_instanceIDAtRow(int row) // OK
{
	assert(row < ui.tableWidget->rowCount());
	return (ClassInstanceID)ui.tableWidget->item(row, 0)->data(KEY_COLUMN_INSTANCE_ID).toUInt();
}

bool ClassInstanceRefByValueMapChip_Dlg::_isRowValid(int row)
{
	if (row >= ui.tableWidget->rowCount())
		return false;

	QTableWidgetItem* itmKey = ui.tableWidget->item(row, KEY_COLUMN);
	if (!itmKey)
		return false;

	value key = _keyAtRow(row);
	ClassInstanceID instanceID = _instanceIDAtRow(row);

	auto n = GetChip()->GetMap().find(key);
	if (n == GetChip()->GetMap().end())
		return false; // No such key in map

	if (!n->second)
		return instanceID == InvalidClassInstanceID;

	return n->second->GetRuntimeID() == instanceID;
}

// Button clicked!
void ClassInstanceRefByValueMapChip_Dlg::addElement() // OK
{
	value key;
	Class* cg = nullptr;
	do {
		if (!_getKeyAndTypeOfNewInstance(key, &cg))
			return;
	} while (GetChip()->HasElement(key));

	if (cg == nullptr)
		GetChip()->AddElement(key, ClassInstanceRef());
	else
		GetChip()->CreateInstance(key, cg);

	auto n = GetChip()->GetMap().find(key);
	if (n == GetChip()->GetMap().end())
		return;

	auto m = n;
	std::advance(m, 1);

	int row = 0;
	if (m != GetChip()->GetMap().end()) {
		for (; row < ui.tableWidget->rowCount(); row++)
			if (_keyAtRow(row) == m->first)
				break;
	}
	else
		row = ui.tableWidget->rowCount();

	_addRows(row, 1);
	_setInstance(row, key, n->second);
	ui.tableWidget->scrollToItem(ui.tableWidget->item(row, 0));
	ui.tableWidget->selectRow(row);

	SetDirty();
	updateButtons();
}

// Button clicked!
void ClassInstanceRefByValueMapChip_Dlg::removeRows()
{
	QPair<int, int> s = _getSelection();
	if (s.first == -1)
		return;
	for (int row = s.first; row <= s.second;) {
		value key = _keyAtRow(row);
		GetChip()->RemoveElement(key);
		if (!GetChip()->HasElement(key)) {
			_removeRows(row, 1);
			s.second--;
		}
		else
			row++;
	}

	SetDirty();
	// TODO: If clearing instance, we must mark ref owners dirty?
	updateButtons();
}

// Button clicked!
void ClassInstanceRefByValueMapChip_Dlg::createInstance()
{
	QPair<int, int> s = _getSelection();
	if (s.first == -1)
		return;

	for (int i = s.first; i <= s.second; i++) {
		if (!_isRowValid(i))
			return;
		if (_instanceIDAtRow(i) != InvalidClassInstanceID)
			return;
	}

	QStringList lst;

	for (const auto &n : engine->GetClassManager()->GetClasssByName())
		lst << TOQSTRING(n.first);
	bool ok = false;
	QString result = QInputDialog::getItem(this, "New Instance", "Select Instance Type:", lst, 0, false, &ok);
	if (!ok)
		return;
	Class* cg = engine->GetClassManager()->GetClass(FROMQSTRING(result));
	if (!cg)
		return;

	for (int i = s.first; i <= s.second; i++) {
		value key = _keyAtRow(i);
		ClassInstanceRef ref = GetChip()->CreateInstance(key, cg);
		_setInstance(i, key, ref);
	}

	SetDirty();
	updateButtons();
}

// Button clicked!
void ClassInstanceRefByValueMapChip_Dlg::clearRef()
{
	QPair<int, int> s = _getSelection();
	if (s.first == -1)
		return;

	for (int row = s.first; row <= s.second; row++) {
		if (!_isRowValid(row))
			continue;
		value key = _keyAtRow(row);
		GetChip()->SetInstance(key, ClassInstanceRef());
		_setInstance(row, key, ClassInstanceRef());
	}

	SetDirty();
	updateButtons();
}

// Button clicked!
void ClassInstanceRefByValueMapChip_Dlg::copyRef()
{
	QPair<int, int> s = _getSelection();
	if (s.first == -1 || s.first != s.second || !_isRowValid(s.first))
		return;

	value key = _keyAtRow(s.first);
	if (!GetChip()->HasElement(key))
		return;

	const ClassInstanceRef* ref = GetChip()->GetValueByKey(key);

	if (ref)
		ClassInstanceRef::copiedRef = ClassInstanceRef(*ref, false);

	updateButtons();
}

// Button clicked!
void ClassInstanceRefByValueMapChip_Dlg::pasteRef()
{
	if (!ClassInstanceRef::copiedRef)
		return;

	QPair<int, int> s = _getSelection();
	if (s.first == -1 || s.first != s.second)
		return; // Single selection required!
	if (!_isRowValid(s.first))
		return;

	value key = _keyAtRow(s.first);
	GetChip()->SetInstance(key, ClassInstanceRef::copiedRef);

	SetDirty();
	updateButtons();
}

void ClassInstanceRefByValueMapChip_Dlg::updateButtons()
{
	QPair<int, int> s = _getSelection();
	bool unselected = s.first == -1;
	bool singleSelect = s.first != -1 && s.first == s.second;
	int instanceCount = 0;
	if (!unselected)
		for (int row = s.first; row <= s.second; row++)
			if (_isRowValid(row) && _instanceIDAtRow(row) != InvalidClassInstanceID)
				instanceCount++;
	ui.pushButton_removeRows->setEnabled(!unselected);
	ui.pushButton_createInstance->setEnabled(!unselected && instanceCount == 0);
	ui.pushButton_clearRef->setEnabled(instanceCount > 0);
	ui.pushButton_copyRef->setEnabled(singleSelect && instanceCount == 1);
	ui.pushButton_pasteRef->setEnabled(singleSelect && instanceCount == 0 && ClassInstanceRef::copiedRef);
}

void ClassInstanceRefByValueMapChip_Dlg::contextMenuEvent(QContextMenuEvent* event)
{/*
	QMenu *menu = createStandardContextMenu();
	menu->addSeparator();
	QAction *a = menu->addAction("Chip Dialog");
	if (menu->exec(event->globalPos()) == a) {
		dlg->showDataDialog(row, instanceID, dataID);
	}
	delete menu;*/
}
