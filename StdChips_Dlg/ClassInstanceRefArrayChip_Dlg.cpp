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
#include "ClassInstanceRefArrayChip_Dlg.h"
#include "M3DEngine/Class.h"
#include "StdChips/InstanceData.h"
#include "StdChips/Text.h"
#include "StdChips/Value.h"
#include "StdChips/VectorChip.h"
#include "ChipDialogs/ChipDialogManager.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"




using namespace m3d;

DIALOGDESC_DEF(ClassInstanceRefArrayChip_Dlg, CLASSINSTANCEREFARRAYCHIP_GUID);


#define DATA_START 2

#define HORIZONTAL_HEADER_UNDERSCORED Qt::UserRole + 0
#define HORIZONTAL_HEADER_BASE_COUNT Qt::UserRole + 1
#define HORIZONTAL_HEADER_REF_COUNT Qt::UserRole + 2

#define COLUMN0_INSTANCE_RUNTIME_ID Qt::UserRole + 0

#define DATA_COLUMN_DATA_ID Qt::UserRole + 0
#define DATA_COLUMN_REFRESH_INFO Qt::UserRole + 1


ClassInstanceRefArrayChip_Dlg::ClassInstanceRefArrayChip_Dlg()
{
	ui.setupUi(this);
	QPalette p = ui.tableWidget->viewport()->palette();
	p.setColor(QPalette::Base,Qt::gray);
    ui.tableWidget->viewport()->setPalette(p);

	ui.tableWidget->setColumnWidth(0, 170);
	ui.tableWidget->setColumnWidth(1, 150);
	ui.tableWidget->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);
	ui.tableWidget->horizontalHeaderItem(1)->setTextAlignment(Qt::AlignLeft);
}

ClassInstanceRefArrayChip_Dlg::~ClassInstanceRefArrayChip_Dlg()
{
}

// Adds a new data column
Map<Guid, int>::iterator ClassInstanceRefArrayChip_Dlg::_addColumn(const Guid &globalID, int baseCount, QString name, bool underscored)
{
	int column = ui.tableWidget->columnCount(); // We add it to the end of the (logical) columns
	auto p = _dataTypeMap.insert(std::make_pair(globalID, column)).first;
	ui.tableWidget->insertColumn(column);
	ui.tableWidget->setColumnWidth(column, 150);
	QTableWidgetItem *us = new QTableWidgetItem(name); // Create header item
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
void ClassInstanceRefArrayChip_Dlg::_setColumnInOrder(int column)
{
	QTableWidgetItem *us = ui.tableWidget->horizontalHeaderItem(column); // column is the logical index.
	int visualColumn = ui.tableWidget->horizontalHeader()->visualIndex(column); // find visual index
	int i = DATA_START;
	for (; i < ui.tableWidget->columnCount(); i++) {
		if (i == visualColumn)
			continue;
		QTableWidgetItem *other = ui.tableWidget->horizontalHeaderItem(ui.tableWidget->horizontalHeader()->logicalIndex(i));
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
void ClassInstanceRefArrayChip_Dlg::_addRows(int from, int count)
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
void ClassInstanceRefArrayChip_Dlg::_removeRows(int from, int count)
{
	if (from < 0 || count < 0)
		return;
	if (from == 0 && count == ui.tableWidget->rowCount()) {
		ui.tableWidget->setRowCount(0);
		// Remove ALL data columns!
		while (ui.tableWidget->columnCount() > DATA_START)
			ui.tableWidget->removeColumn(ui.tableWidget->columnCount() - DATA_START);
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

void ClassInstanceRefArrayChip_Dlg::_clearRows(int from, int count)
{
	if (from < 0 || count < 0)
		return;
	for (int i = from; i < from + count; i++) { // erase data in selected columns..
		QTableWidgetItem *itm0 = ui.tableWidget->item(i, 0);
		if (!itm0)
			continue;
		for (int j = DATA_START; j < ui.tableWidget->columnCount(); j++) { // Iterate data items...
			QTableWidgetItem *itm = ui.tableWidget->item(i, j);
			if (!itm)
				continue;
			// Decrease refCount for data header item
			QTableWidgetItem *hitm = ui.tableWidget->horizontalHeaderItem(j);
			int refCount = hitm->data(HORIZONTAL_HEADER_REF_COUNT).toInt() - 1;
			hitm->setData(HORIZONTAL_HEADER_REF_COUNT, refCount);
			ui.tableWidget->setCellWidget(i, j, nullptr);
			delete itm; // Delete data item
			if (refCount == 0) {
				ui.tableWidget->hideColumn(j); // Hide data column if no more data for it
			}
		}
		delete itm0; // Delete item for first column
		delete ui.tableWidget->item(i, 1); // Delete item for second column
	}
}

void ClassInstanceRefArrayChip_Dlg::_setInstance(int row, ClassInstanceRef ref)
{
	if (ui.tableWidget->item(row, 0) != nullptr)
		_clearRows(row, 1); // Ensure row is clean.

	if (!ref)
		return;

	QString typeName;
	if (ref->IsSerialized()) {
		const ClassInstanceSerialization *s = ref->GetSerialization();
		typeName = TOQSTRING(GuidToString(s->clazzid)); // Set name of the instance type to be the guid of the class.
				
		// Iterate data
		for (const auto &n : s->data) {
			auto p = _dataTypeMap.find(n.first); // Does column for this InstanceData exist?
			if (p == _dataTypeMap.end()) { // No?
				p = _addColumn(n.first, -1, TOQSTRING(GuidToString(n.first)), false);
			}
			else {
				QTableWidgetItem *itm = ui.tableWidget->horizontalHeaderItem(p->second);
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
				QTableWidgetItem *itm = ui.tableWidget->horizontalHeaderItem(p->second); // Item at logical index
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

	// Column 0
	QTableWidgetItem *itm0 = new QTableWidgetItem(typeName);
	itm0->setFlags((itm0->flags() & ~Qt::ItemIsEditable) | Qt::ItemIsUserCheckable);
	itm0->setData(COLUMN0_INSTANCE_RUNTIME_ID, (uint32)ref->GetRuntimeID());
	itm0->setBackground(Qt::white);
	itm0->setCheckState(ref.IsOwner() ? Qt::Checked : Qt::Unchecked);
	ui.tableWidget->setItem(row, 0, itm0);

	// Column 1
	QTableWidgetItem *itm1 = new QTableWidgetItem(TOQSTRING(ref->GetName()));
	itm1->setBackground(Qt::white);
	itm1->setToolTip(QString("ID: %1").arg(ref->GetRuntimeID()));
	if (ref->GetOwner() == nullptr)
		itm1->setFlags(itm0->flags() & ~Qt::ItemIsEditable);
	ui.tableWidget->setItem(row, 1, itm1);

}

void ClassInstanceRefArrayChip_Dlg::_setDataItem(Chip *data, ClassInstanceID instanceID, int row, int column)
{
	static QIcon f(":/StdChips_Dialogs/Resources/dialog.png");
	static QPixmap formPixMap = f.pixmap(16, 16);

	QTableWidgetItem *itm = new QTableWidgetItem("");
	itm->setData(DATA_COLUMN_DATA_ID, data->GetID());
	itm->setBackground(Qt::white);

	QString txt;
	int align = Qt::AlignLeft;
	QVariant extra;

	Text *t = dynamic_cast<Text*>(data);
	if (t) {
		txt = TOQSTRING(t->GetChipText());
		extra = t->GetUpdateStamp();
	}
	else {
		Value *v = dynamic_cast<Value*>(data);
		if (v) {
			value val = v->GetChipValue();
			txt = QString::number(val);
			extra = val;
			align = Qt::AlignRight;
		}
		else {
			VectorChip *vec = dynamic_cast<VectorChip*>(data);
			if (vec) {
				XMFLOAT4 v = vec->GetChipVector();
				txt = QString::number(v.x) + "; " + QString::number(v.y) + "; " + QString::number(v.z) + "; " + QString::number(v.w);
				align = Qt::AlignRight;
				extra.fromValue<XMFLOAT4>(v);
			}
			else {
				itm->setFlags(itm->flags() & ~Qt::ItemIsEditable);
				QLabel *l = new QLabel();
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

// Returns the instance with given id. row is just a hint about where to look first. On return, it is the actual row of the instance.
ClassInstanceRef ClassInstanceRefArrayChip_Dlg::_getInstance(uint32 &row, ClassInstanceID instanceID)
{
	ClassInstanceRef instance;

	if (GetChip()->GetContainerSize() > row) {
		instance = GetChip()->GetInstance(row);
		if (instance && instance->GetRuntimeID() != instanceID)
			instance = ClassInstanceRef();
	}
	if (!instance) {
		for (uint32 i = 0; i < GetChip()->GetContainerSize(); i++) {
			ClassInstanceRef ref = GetChip()->GetInstance(i);
			if (ref && ref->GetRuntimeID() == instanceID) {
				instance = ref;
				row = i;
				break;
			}
		}
	}
	return instance;
}

// Returns the data-chip for the given instance and with the given id.
Chip *ClassInstanceRefArrayChip_Dlg::_getDataChip(ClassInstanceRef instance, ChipID dataID)
{
	if (!instance)
		return nullptr;

	if (instance->IsSerialized()) {
		const ClassInstanceSerialization *s = instance->GetSerialization();

		for (const auto &n : s->data)
			if (n.second->GetID() == dataID)
				return n.second;
	}
	else {
		const ChipPtrByInstanceDataPtrMap &m = instance->GetData();
		for (const auto &n : m)
			if (n.second->GetID() == dataID)
				return n.second;
	}
	return nullptr;
}

void ClassInstanceRefArrayChip_Dlg::_swapRows(uint32 a, uint32 b)
{
	if (a == b || std::max(a, b) >= ui.tableWidget->rowCount())
		return;
	for (uint32 i = 0; i < ui.tableWidget->columnCount(); i++) {
		QTableWidgetItem *itmA = ui.tableWidget->takeItem(a, i);
		QTableWidgetItem *itmB = ui.tableWidget->takeItem(b, i);
		ui.tableWidget->setItem(a, i, itmB);
		ui.tableWidget->setItem(b, i, itmA);
		QWidget *wA = ui.tableWidget->cellWidget(a, i);
		QWidget *wB = ui.tableWidget->cellWidget(b, i);
		ui.tableWidget->setCellWidget(a, i, wB);
		ui.tableWidget->setCellWidget(b, i, wA);
	}
}

void ClassInstanceRefArrayChip_Dlg::Init()
{
	_addRows(0, GetChip()->GetContainerSize());
	for (uint32 i = 0; i < GetChip()->GetContainerSize(); i++) {
		ClassInstanceRef r = GetChip()->GetInstance(i);
		r.Prepare(GetChip());
		_setInstance(i, r);
	}

	ui.checkBox_saveInstance->setChecked(GetChip()->IsSaveContent());
	ui.checkBox_preload->setChecked(GetChip()->isPreload());
	_initSaveRef = GetChip()->IsSaveContent();
	_initPreload = GetChip()->isPreload();

	updateButtons();
}

void ClassInstanceRefArrayChip_Dlg::OnCancel()
{
	GetChip()->SetSaveContent(_initSaveRef);
	GetChip()->setPreload(_initPreload);
}

void ClassInstanceRefArrayChip_Dlg::AfterApply()
{
	_initSaveRef = GetChip()->IsSaveContent();
	_initPreload = GetChip()->isPreload();
}

// Refresh the UI. Called every 0.5 sec.
void ClassInstanceRefArrayChip_Dlg::Refresh()
{
	int row = ui.tableWidget->rowAt(0); 
	if (row == -1)
		return;
	int end = ui.tableWidget->rowAt(ui.tableWidget->viewport()->height());
	if (end == -1)
		end = ui.tableWidget->rowCount();

	bool editing = ui.tableWidget->editing();

	// TODO: Make refresh more advanced!

	for (; row < end; row++) { // We do only update the visible ones. Performance!
		QTableWidgetItem *itm = ui.tableWidget->item(row, 0);
		if (!itm)
			continue; // No data at this table row...
		ClassInstanceID instanceID = itm->data(COLUMN0_INSTANCE_RUNTIME_ID).toUInt();
		uint32 r = row;
		ClassInstanceRef instance = _getInstance(r, instanceID);
		if (!instance)
			continue; // No instance at this row.

		// Update owner flag
		if (itm->checkState() == Qt::Checked != instance.IsOwner())
			itm->setCheckState(instance.IsOwner() ? Qt::Checked : Qt::Unchecked);

		// Update name
		QString name = TOQSTRING(instance->GetName());
		QTableWidgetItem *nItm = ui.tableWidget->item(row, 1);
		if (!editing || ui.tableWidget->currentItem() != nItm) {
			if (name != nItm->text())
				nItm->setText(name);
		}

		for (int col = DATA_START; col < ui.tableWidget->columnCount(); col++) { // Update data columns!
			QTableWidgetItem *w = ui.tableWidget->item(row, col);
			if (!w)
				continue;

			if (editing && ui.tableWidget->currentItem() == w)
				continue; // Editing this item

			ChipID dataID = w->data(DATA_COLUMN_DATA_ID).toUInt();
		
			Chip *c = _getDataChip(instance, dataID);
			Text *txt = dynamic_cast<Text*>(c);
			if (txt) {
				UpdateStamp extra = w->data(DATA_COLUMN_REFRESH_INFO).toUInt();
				UpdateStamp us = txt->GetUpdateStamp();
				if (extra != us) {
					w->setData(DATA_COLUMN_REFRESH_INFO, us);
					w->setText(TOQSTRING(txt->GetChipText()));
				}
				continue;
			}
			Value *v = dynamic_cast<Value*>(c);
			if (v) {
				value extra = w->data(DATA_COLUMN_REFRESH_INFO).toDouble();
				value val = v->GetChipValue();
				if (extra != val) {
					w->setData(DATA_COLUMN_REFRESH_INFO, val);
					w->setText(QString::number(val));
				}
				continue;
			}
			VectorChip *vec = dynamic_cast<VectorChip*>(c);
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

void ClassInstanceRefArrayChip_Dlg::saveInstanceClicked()
{
	GetChip()->SetSaveContent(ui.checkBox_saveInstance->isChecked());
}

void ClassInstanceRefArrayChip_Dlg::showDataDialog(uint32 row, ClassInstanceID instanceID, ChipID dataID)
{
	Chip *c = _getDataChip(_getInstance(row, instanceID), dataID);
	if (c)
		GetDialogManager()->OpenChipDialog(c);
}


void ClassInstanceRefArrayChip_Dlg::onTableDoubleClick(int row, int column)
{
	if (column < DATA_START) {
		QTableWidgetItem *w = ui.tableWidget->item(row, 0);
		if (!w)
			return;
		uint32 r = row;
		ClassInstanceRef instance = _getInstance(r, w->data(COLUMN0_INSTANCE_RUNTIME_ID).toUInt());

		if (instance)
			GetDialogManager()->OpenInstanceDialog(instance.GetRawPtr());
		return;
	}

	QTableWidgetItem *w = ui.tableWidget->item(row, column);
	if (!w)
		return;
	ClassInstanceID instanceID = ui.tableWidget->item(row, 0)->data(COLUMN0_INSTANCE_RUNTIME_ID).toUInt();
	ChipID dataID = w->data(DATA_COLUMN_DATA_ID).toUInt();
	uint32 r = row;
	Chip *c = _getDataChip(_getInstance(r, instanceID), dataID);
	if (c)
		GetDialogManager()->OpenChipDialog(c);
}

void ClassInstanceRefArrayChip_Dlg::onTableCellChanged(int row, int column)
{
	if (ui.tableWidget->editCalled() && column == 0) {
		// Owner checkbox clicked
		QTableWidgetItem *w = ui.tableWidget->item(row, 0);
		if (!w)
			return;

		uint32 r = row;

		ClassInstanceRef ref = _getInstance(r, w->data(COLUMN0_INSTANCE_RUNTIME_ID).toUInt());

		if (!ref.IsOwner() && w->checkState() == Qt::Checked) {
			GetChip()->MakeOwner(r);
			SetDirty(); // TODO: Mark old owner dirty?
		}

		ref = _getInstance(r, w->data(COLUMN0_INSTANCE_RUNTIME_ID).toUInt()); // Update ref with correct owner flag!
		
		w->setCheckState(ref.IsOwner() ? Qt::Checked : Qt::Unchecked);

		return;
	}

	if (!ui.tableWidget->editing()) {
		return;
	}

	if (column == 1) { // Name changed
		QTableWidgetItem *w = ui.tableWidget->item(row, 0);
		if (!w)
			return;

		uint32 r = row;

		ClassInstanceRef instance = _getInstance(r, w->data(COLUMN0_INSTANCE_RUNTIME_ID).toUInt());

		if (!instance)
			return;

		w = ui.tableWidget->item(row, 1);
		if (!w)
			return;

		String name = FROMQSTRING(w->text());
		instance->SetName(name);
		SetDirty(instance->GetOwner()->GetClass()); // Make owner dirty!
	}
	else if (column >= DATA_START) { // Data updated...
		QTableWidgetItem *w = ui.tableWidget->item(row, column);
		if (!w)
			return;
		ClassInstanceID instanceID = ui.tableWidget->item(row, 0)->data(COLUMN0_INSTANCE_RUNTIME_ID).toUInt();
		ChipID dataID = w->data(DATA_COLUMN_DATA_ID).toUInt();
		uint32 r = row;		
		ClassInstanceRef instance = _getInstance(r, instanceID);

		Chip *c = _getDataChip(instance, dataID);

		ui.tableWidget->blockSignals(true);

		Text *t = dynamic_cast<Text*>(c);
		if (t) {
			t->SetText(FROMQSTRING(w->text()));
			w->setData(DATA_COLUMN_REFRESH_INFO, t->GetUpdateStamp());
			SetDirty(instance->GetOwner()->GetClass());
		}
		else {
			Value *v = dynamic_cast<Value*>(c);
			if (v) {
				bool ok = false;
				value val = w->text().toDouble(&ok);
				if (ok) {
					v->SetValue(val);
					w->setData(DATA_COLUMN_REFRESH_INFO, val);
					SetDirty(instance->GetOwner()->GetClass());
				}
				else
					w->setText(QString::number(v->GetChipValue()));
			}
			else {
				VectorChip *vec = dynamic_cast<VectorChip*>(c);
				if (vec) {
					XMFLOAT4 n = vec->GetChipVector();
					QStringList lst = w->text().split(';');
					bool ok;
					float32 tmp;
					int c = 0;
					if (lst.size() > 0) { tmp = lst[0].toFloat(&ok); if (ok) { n.x = tmp; c++; } }
					if (lst.size() > 1) { tmp = lst[1].toFloat(&ok); if (ok) { n.y = tmp; c++; } }
					if (lst.size() > 2) { tmp = lst[2].toFloat(&ok); if (ok) { n.z = tmp; c++; } }
					if (lst.size() > 3) { tmp = lst[3].toFloat(&ok); if (ok) { n.w = tmp; c++; } }
					if (c > 0) {
						vec->SetVector(n);
						SetDirty(instance->GetOwner()->GetClass());
					}
					if (c < 4)
						w->setText(QString::number(n.x) + "; " + QString::number(n.y) + "; " + QString::number(n.z) + "; " + QString::number(n.w));
					w->setData(DATA_COLUMN_REFRESH_INFO, QVariant::fromValue<XMFLOAT4>(n));
				}
			}
		}
		ui.tableWidget->blockSignals(false);
	}
}

void ClassInstanceRefArrayChip_Dlg::hideMembersClicked()
{
	bool b = ui.checkBox_hideMembers->isChecked();
	for (uint32 i = DATA_START, j = ui.tableWidget->columnCount(); i < j; i++) {
		if (b && ui.tableWidget->horizontalHeaderItem(i)->data(HORIZONTAL_HEADER_UNDERSCORED).toBool() || ui.tableWidget->horizontalHeaderItem(i)->data(HORIZONTAL_HEADER_REF_COUNT).toInt() == 0)
			ui.tableWidget->hideColumn(i);
		else
			ui.tableWidget->showColumn(i);
	}
}

void ClassInstanceRefArrayChip_Dlg::preloadClicked()
{
	GetChip()->setPreload(ui.checkBox_preload->isChecked());
}

// Get range of selected items.
QPair<int, int> ClassInstanceRefArrayChip_Dlg::_getSelection()
{
	QList<QTableWidgetSelectionRange> lst = ui.tableWidget->selectedRanges();
	if (lst.size() != 1 || lst.front().rowCount() == 0)
		return qMakePair(-1, -1);
	return qMakePair(lst.front().topRow(), lst.front().bottomRow());
}

// Shows a dialogbox where user can enter number and type of new instances!
bool ClassInstanceRefArrayChip_Dlg::_getNumberAndTypeOfNewInstances(int &count, Class **instanceOf)
{
	QDialog d(this);
	QFormLayout *l = new QFormLayout(&d);
	QSpinBox *spinBox = new QSpinBox();
	spinBox->setRange(1, 10000);
	QComboBox *cb = new QComboBox();
	QStringList lst;
	lst << "<Empty>";
	for (const auto &n : engine->GetClassManager()->GetClasssByName())
		lst << TOQSTRING(n.first);
	cb->addItems(lst);
	QHBoxLayout *bl = new QHBoxLayout();
	QPushButton *okBtn = new QPushButton("OK");
	okBtn->setDefault(true);
	QPushButton *cancelBtn = new QPushButton("Cancel");
	bl->addWidget(okBtn);
	bl->addWidget(cancelBtn);
	l->addRow("Number of Instances:", spinBox);
	l->addRow("Instance Type:", cb);
	l->addRow(bl);
	connect(okBtn, &QPushButton::clicked, &d, &QDialog::accept);
	connect(cancelBtn, &QPushButton::clicked, &d, &QDialog::reject);
	if (d.exec() == QDialog::Accepted) {
		count = spinBox->value();
		if (cb->currentIndex() != 0)
			*instanceOf = engine->GetClassManager()->GetClass(FROMQSTRING(cb->currentText()));
		return true;
	}
	return false;
}

void ClassInstanceRefArrayChip_Dlg::_showWarning()
{
	QMessageBox::critical(this, "Warning", "Can not complete operation. Array has been modified since dialog box was opened. Reopen dialog box to fix problem.");
}

// Button clicked!
void ClassInstanceRefArrayChip_Dlg::addRows()
{
	if (GetChip()->GetContainerSize() != ui.tableWidget->rowCount()) // different size?
		return _showWarning();

	int count;
	Class *cg = nullptr;
	if (!_getNumberAndTypeOfNewInstances(count, &cg))
		return;

	int oldSize = GetChip()->GetContainerSize();
	GetChip()->AddElements(count, cg);
	int newSize = GetChip()->GetContainerSize();
	if (newSize <= oldSize)
		return;
	_addRows(oldSize, newSize - oldSize);
	for (uint32 i = oldSize; i < newSize; i++)
		_setInstance(i, GetChip()->GetInstance(i));

	SetDirty();
	updateButtons();
}

// Button clicked!
void ClassInstanceRefArrayChip_Dlg::insertRows()
{
	QPair<int, int> s = _getSelection();
	if (s.first == -1 || s.first != s.second)
		return; // single selection required!

	if (GetChip()->GetContainerSize() != ui.tableWidget->rowCount()) // different size?
		return _showWarning();

	int count;
	Class *cg = nullptr;
	if (!_getNumberAndTypeOfNewInstances(count, &cg))
		return;

	int oldSize = GetChip()->GetContainerSize();
	GetChip()->InsertElements(s.first, count, cg);
	count = GetChip()->GetContainerSize() - oldSize;
	if (count <= 0)
		return;
	_addRows(s.first, count);
	for (int i = s.first; i < s.first + count; i++)
		_setInstance(i, GetChip()->GetInstance(i));

	SetDirty();
	updateButtons();
}

bool ClassInstanceRefArrayChip_Dlg::_ensureSelectionIsValid(int a, int b)
{
	if (GetChip()->GetContainerSize() != ui.tableWidget->rowCount())
		return false;

	for (int i = a; i <= b; i++) {
		ClassInstanceRef ref = GetChip()->GetInstance(i);
		QTableWidgetItem *itm = ui.tableWidget->item(i, 0);
		ClassInstanceID a = ref ? ref->GetRuntimeID() : 0;
		ClassInstanceID b = itm ? ClassInstanceID(itm->data(COLUMN0_INSTANCE_RUNTIME_ID).toUInt()) : 0;
		if (a != b)
			return false;
	}
	return true;
}

// Button clicked!
void ClassInstanceRefArrayChip_Dlg::removeRows()
{
	QPair<int, int> s = _getSelection();
	if (s.first == -1)
		return;

	if (!_ensureSelectionIsValid(s.first, s.second))
		return _showWarning();

	int oldSize = GetChip()->GetContainerSize();
	GetChip()->RemoveElements(s.first, s.second - s.first + 1);
	int count = oldSize - GetChip()->GetContainerSize();
	if (count <= 0)
		return;
	_removeRows(s.first, count);

	SetDirty();
	// TODO: If clearing instance, we must mark ref owners dirty?
	updateButtons();
}

// Button clicked!
void ClassInstanceRefArrayChip_Dlg::createInstance()
{
	QPair<int, int> s = _getSelection();
	if (s.first == -1)
		return;

	if (GetChip()->GetContainerSize() <= s.second)
		return _showWarning();

	for (int i = s.first; i <= s.second; i++)
		if (GetChip()->GetInstance(i))
			return _showWarning();

	QStringList lst;

	for (const auto &n : engine->GetClassManager()->GetClasssByName())
		lst << TOQSTRING(n.first);
	bool ok = false;
	QString result = QInputDialog::getItem(this, "New Instance", "Select Instance Type:", lst, 0, false, &ok);
	if (!ok)
		return;
	Class *cg = engine->GetClassManager()->GetClass(FROMQSTRING(result));
	if (!cg)
		return;

	for (int i = s.first; i <= s.second; i++) {
		GetChip()->CreateInstance(i, cg);
		_setInstance(i, GetChip()->GetInstance(i));
	}

	SetDirty();
	updateButtons();
}

// Button clicked!
void ClassInstanceRefArrayChip_Dlg::clearRef()
{
	QPair<int, int> s = _getSelection();
	if (s.first == -1)
		return;
	
	if (!_ensureSelectionIsValid(s.first, s.second))
		return _showWarning();

	for (int i = s.first; i <= s.second; i++)
		GetChip()->ClearInstance(i);

	_clearRows(s.first, s.second - s.first + 1);

	SetDirty();
	// TODO: If clearing instance, we must mark ref owners dirty?
	updateButtons();
}

// Button clicked!
void ClassInstanceRefArrayChip_Dlg::copyRef()
{
	QPair<int, int> s = _getSelection();
	if (s.first == -1 || s.first != s.second)
		return;

	QTableWidgetItem *itm = ui.tableWidget->item(s.first, 0);
	if (!itm)
		return;

	ClassInstanceRef ref;

	if (GetChip()->GetContainerSize() <= s.first || !(ref = GetChip()->GetInstance(s.first)) || ref->GetRuntimeID() != ClassInstanceID(itm->data(COLUMN0_INSTANCE_RUNTIME_ID).toUInt()))
		return _showWarning();
	
	ClassInstanceRef::copiedRef = ClassInstanceRef(ref, false);

	updateButtons();
}

// Button clicked!
void ClassInstanceRefArrayChip_Dlg::pasteRef()
{
	if (!ClassInstanceRef::copiedRef)
		return;

	QPair<int, int> s = _getSelection();
	if (s.first == -1 || s.first != s.second)
		return; // Single selection required!

	QTableWidgetItem *itm = ui.tableWidget->item(s.first, 0);
	if (itm)
		return;

	if (GetChip()->GetContainerSize() <= s.first || GetChip()->GetInstance(s.first))
		return _showWarning();

	GetChip()->SetInstance(s.first, ClassInstanceRef::copiedRef);
	_setInstance(s.first, GetChip()->GetInstance(s.first));

	SetDirty();
	updateButtons();
}

void ClassInstanceRefArrayChip_Dlg::updateButtons()
{
	QPair<int, int> s = _getSelection();
	bool unselected = s.first == -1;
	bool singleSelect = s.first != -1 && s.first == s.second;
	int instanceCount = 0; 
	if (!unselected)
		for (int i = s.first; i <= s.second; i++)
			if (ui.tableWidget->item(i, 0))
				instanceCount++;
	ui.pushButton_insertRows->setEnabled(singleSelect);
	ui.pushButton_removeRows->setEnabled(!unselected);
	ui.pushButton_createInstance->setEnabled(!unselected && instanceCount == 0);
	ui.pushButton_clearRef->setEnabled(instanceCount > 0);
	ui.pushButton_copyRef->setEnabled(singleSelect && instanceCount == 1);
	ui.pushButton_pasteRef->setEnabled(singleSelect && instanceCount == 0 && ClassInstanceRef::copiedRef);
	ui.pushButton_up->setEnabled(!unselected && s.first > 0);
	ui.pushButton_down->setEnabled(!unselected && s.second < ui.tableWidget->rowCount() - 1);
}

void ClassInstanceRefArrayChip_Dlg::contextMenuEvent(QContextMenuEvent *event)
{/*
	QMenu *menu = createStandardContextMenu();
	menu->addSeparator();
	QAction *a = menu->addAction("Chip Dialog");
	if (menu->exec(event->globalPos()) == a) {
		dlg->showDataDialog(row, instanceID, dataID);
	}
	delete menu;*/
}

// Button clicked!
void ClassInstanceRefArrayChip_Dlg::moveDown()
{
	_move(1);
}

// Button clicked!
void ClassInstanceRefArrayChip_Dlg::moveUp()
{
	_move(-1);
}

void ClassInstanceRefArrayChip_Dlg::_move(int dir)
{
	QPair<int, int> s = _getSelection();
	if (s.first == -1 || s.first + dir < 0 || s.second + dir >= ui.tableWidget->rowCount())
		return;

	if (!_ensureSelectionIsValid(s.first, s.second))
		return _showWarning();

	GetChip()->MoveElements(s.first, s.second - s.first + 1, dir);
	SetDirty();

	if (dir == -1) {
		for (int i = s.first - 1; i < s.second; i++)
			_swapRows(i, i + 1);
	}
	else {
		for (int i = s.second + 1; i > s.first; i--)
			_swapRows(i, i - 1);
	}
	ui.tableWidget->clearSelection();
	ui.tableWidget->setRangeSelected(QTableWidgetSelectionRange(s.first + dir, 0, s.second + dir, ui.tableWidget->columnCount() - 1), true);
}