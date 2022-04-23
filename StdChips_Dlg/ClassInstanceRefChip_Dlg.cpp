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
#include "ClassInstanceRefChip_Dlg.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"
#include "ChipDialogs/ChipDialogManager.h"
#include "M3DEngine/ClassInstance.h"
#include "StdChips/InstanceData.h"
#include "StdChips/Text.h"
#include "StdChips/Value.h"
#include "StdChips/VectorChip.h"

using namespace m3d;


DIALOGDESC_DEF(ClassInstanceRefChip_Dlg, CLASSINSTANCEREFCHIP_GUID);


ClassInstanceRefChip_Dlg::ClassInstanceRefChip_Dlg()
{
	ui.setupUi(this);
	_instanceID = 0;

	ui.tableWidget->setColumnWidth(0, 200);
	ui.tableWidget->setColumnWidth(1, 150);
	ui.tableWidget->setColumnWidth(2, 150);

	ui.tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
}

ClassInstanceRefChip_Dlg::~ClassInstanceRefChip_Dlg()
{
}

void ClassInstanceRefChip_Dlg::Init()
{
	GetChip()->GetInstance().Prepare(GetChip());
	_fillData();
	_enableButtons();
	_initSaveRef = GetChip()->IsSaveRef();
	_initPreload = GetChip()->isPreload();
	ui.checkBox_saveInstance->setChecked(_initSaveRef);
}

void ClassInstanceRefChip_Dlg::OnCancel()
{
	GetChip()->SetSaveRef(_initSaveRef);
	GetChip()->setPreload(_initPreload);
}

void ClassInstanceRefChip_Dlg::AfterApply()
{
	_initSaveRef = GetChip()->IsSaveRef();
	_initPreload = GetChip()->isPreload();
}

void ClassInstanceRefChip_Dlg::Refresh()
{
	ClassInstanceRef instance = GetChip()->GetInstance();

	if (instance) { // Do chip have an instance?
		if (instance->GetRuntimeID() == _instanceID) { // Is it the same as our current instance?
			// Update name
			if (!ui.lineEdit_name->hasFocus() && ui.lineEdit_name->text() != TOQSTRING(instance->GetName()))
				ui.lineEdit_name->setText(TOQSTRING(instance->GetName()));

			// Update owner check box
			if (instance.IsOwner() != ui.checkBox_owner->isChecked())
				ui.checkBox_owner->setChecked(instance.IsOwner());

			// Update value, vector and text data...

			bool editing = ui.tableWidget->editing();

			for (int row = 0; row < ui.tableWidget->rowCount(); row++) {
				if (editing && ui.tableWidget->currentRow() == row)
					continue; // Editing this row
				Chip *c = _getDataChip(ui.tableWidget->item(row, 0)->data(Qt::UserRole).toUInt());
				Text *txt = dynamic_cast<Text*>(c);
				if (txt) {
					QTableWidgetItem *itm = ui.tableWidget->item(row, 2);
					if (itm) {
						UpdateStamp extra = itm->data(Qt::UserRole).toUInt();
						UpdateStamp us = txt->GetUpdateStamp();
						if (extra != us) {
							itm->setData(Qt::UserRole, us);
							itm->setText(TOQSTRING(txt->GetChipText()));
						}
					}
					continue;
				}
				Value *v = dynamic_cast<Value*>(c);
				if (v) {
					QTableWidgetItem *itm = ui.tableWidget->item(row, 2);
					if (itm) {
						value extra = itm->data(Qt::UserRole).toDouble();
						value val = v->GetChipValue();
						if (extra != val) {
							itm->setData(Qt::UserRole, val);
							itm->setText(QString::number(val));
						}
					}
					continue;
				}
				VectorChip *vec = dynamic_cast<VectorChip*>(c);
				if (vec) {
					QTableWidgetItem *itm = ui.tableWidget->item(row, 2);
					if (itm) {
						XMFLOAT4 extra = itm->data(Qt::UserRole).value<XMFLOAT4>();
						XMFLOAT4 val = vec->GetChipVector();
						if (extra.x != val.x || extra.y != val.y || extra.z != val.z || extra.w != val.w) {
							itm->setText(QString::number(val.x) + "; " + QString::number(val.y) + "; " + QString::number(val.z) + "; " + QString::number(val.w));
							itm->setData(Qt::UserRole, QVariant::fromValue<XMFLOAT4>(val));
						}
					}		
					continue;
				}
			}
		}
		else {
			_fillData(); // Refill the dialog!
		}
	}
	else { // No instance?
		if (_instanceID != 0) { // Do we have an instance in the dialog?
			_fillData(); // Clear the dialog!
		}
	}
	_enableButtons();
}

// Button clicked!
void ClassInstanceRefChip_Dlg::onNewInstance()
{
	if (GetChip()->GetInstance().IsOwner())
		if (QMessageBox::question(this, "Instance Ref", "Are you sure you want to discard the current instance? This is the owning reference!", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
			return;

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

	GetChip()->CreateInstance(cg);

	SetDirty(); // TODO: Should we mark lost references dirty also?
	_fillData();
	_enableButtons();
}

// Button clicked!
void ClassInstanceRefChip_Dlg::onClearReference()
{
	if (!GetChip()->GetInstance())
		return;

	if (GetChip()->GetInstance().IsOwner())
		if (QMessageBox::question(this, "Instance Ref", "Are you sure you want to discard the current instance? This is the owning reference!", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
			return;

	GetChip()->ClearInstance(); // NOTE: This may delete us!
	
	if (GetChip()) { // Chip will be set to nullptr if we are to be deleted.
		SetDirty(); // TODO: Should we mark lost references dirty also?
		_fillData();
		_enableButtons();
	}
}

// Button clicked!
void ClassInstanceRefChip_Dlg::onCopyReference()
{
	ClassInstanceRef::copiedRef = ClassInstanceRef(GetChip()->GetInstance(), false);
	_enableButtons();
}

// Button clicked!
void ClassInstanceRefChip_Dlg::onPasteReference()
{
	if (ClassInstanceRef::copiedRef != ClassInstanceRef() && ClassInstanceRef::copiedRef != GetChip()->GetInstance()) {
		if (GetChip()->GetInstance().IsOwner())
			if (QMessageBox::question(this, "Instance Ref", "Are you sure you want to discard the current instance? This is the owning reference!", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
				return;

		GetChip()->SetInstance(ClassInstanceRef::copiedRef);
		SetDirty();
		_fillData();
	}
	_enableButtons();
}

void ClassInstanceRefChip_Dlg::onTableDoubleClick(int row, int column)
{
	Chip *chip = _getDataChipFromRow(row);
	if (!chip)
		return; // this data may have been removed!

	if (column == 2 && (dynamic_cast<Text*>(chip) || dynamic_cast<Value*>(chip) || dynamic_cast<VectorChip*>(chip)))
		return; // Double click triggers edit for these types!
	 
	GetDialogManager()->OpenChipDialog(chip);
}

Chip *ClassInstanceRefChip_Dlg::_getDataChipFromRow(int row)
{
	return _getDataChip((ChipID)ui.tableWidget->item(row, 0)->data(Qt::UserRole).toUInt());
}

Chip *ClassInstanceRefChip_Dlg::_getDataChip(ChipID cid)
{
	ClassInstanceRef instance = GetChip()->GetInstance();

	if (!instance)
		return nullptr;

	if (instance->IsSerialized()) {
		const ClassInstanceSerialization *s = instance->GetSerialization();

		for (const auto &n : s->data)
			if (n.second->GetID() == cid)
				return n.second;
	}
	else {
		const ChipPtrByInstanceDataPtrMap &m = instance->GetData();
		for (const auto &n : m)
			if (n.second->GetID() == cid)
				return  n.second;
	}
	return nullptr;
}


void ClassInstanceRefChip_Dlg::_enableButtons()
{
	ui.pushButton_2->setEnabled(_instanceID != 0); // Clear Reference only if reference is not null.
	ui.pushButton_3->setEnabled(_instanceID != 0); // Copy Reference only if reference is not null.
	ui.pushButton_4->setEnabled(ClassInstanceRef::copiedRef); // Paste Reference only if a ref is available.
	ui.checkBox_owner->setEnabled(_instanceID != 0); // Owner checkbox enabled only if instance.
}

void ClassInstanceRefChip_Dlg::_fillData()
{
	static QIcon f(":/StdChips_Dialogs/Resources/dialog.png");
	static QPixmap formPixMap = f.pixmap(16, 16);

	ClassInstanceRef instance = GetChip()->GetInstance();

	ui.tableWidget->clearContents();
	ui.tableWidget->setRowCount(0);
	ui.lineEdit_instanceType->setText("");
	ui.lineEdit_name->setText(instance ? TOQSTRING(instance->GetName()) : "");
	ui.lineEdit_name->setEnabled(instance && instance->GetOwner() != nullptr);
	ui.lineEdit_id->setText(instance ? QString::number(instance->GetRuntimeID()) : "");
	ui.checkBox_owner->setChecked(instance.IsOwner());

	_instanceID = instance ? instance->GetRuntimeID() : 0;

	if (!instance)
		return;

	MultiMap<std::pair<uint32, String>, std::pair<Chip*, bool>> m;

	if (instance->IsSerialized()) {
		const ClassInstanceSerialization *s = instance->GetSerialization();
		ui.lineEdit_instanceType->setText(TOQSTRING(GuidToString(s->clazzid)));

		for (const auto &n : s->data)
			m.insert(std::make_pair(std::make_pair(0, GuidToString(n.first)), std::make_pair(n.second, false)));
	}
	else {
		ui.lineEdit_instanceType->setText(TOQSTRING(instance->GetClass()->GetName()));

		for (const auto &n : instance->GetData()) {
			bool b = n.first->GetName().size() && n.first->GetName()[0] == MCHAR('_');
			m.insert(std::make_pair(std::make_pair(instance->GetClass()->IsBaseClassN(n.first->GetClass()), strUtils::ConstructString(MTEXT("%1 (%2)")).arg(n.first->GetName()).arg(n.first->GetClass()->GetName()).string), std::make_pair(n.second, b)));
		} 
	}

	ui.tableWidget->setRowCount(m.size());

	uint32 i = 0;

	for (auto n = m.begin(); n != m.end(); n++, i++) {
		QTableWidgetItem *itm0 = new QTableWidgetItem(TOQSTRING(n->first.second));
		itm0->setFlags(itm0->flags() & ~Qt::ItemIsEditable);
		itm0->setData(Qt::UserRole, n->second.first->GetID());
		ui.tableWidget->setItem(i, 0, itm0);
		QTableWidgetItem *itm1 = new QTableWidgetItem(TOQSTRING(String(n->second.first->GetChipDesc().name)));
		itm1->setFlags(itm0->flags() & ~Qt::ItemIsEditable);
		itm1->setData(Qt::UserRole, n->second.second);
		ui.tableWidget->setItem(i, 1, itm1);
		if (ui.checkBox_hideMembers->isChecked() && n->second.second)
			ui.tableWidget->hideRow(i);

		QString txt;
		int align = Qt::AlignLeft;
		QVariant extra;

		Text *t = dynamic_cast<Text*>(n->second.first);
		if (t) {
			txt = TOQSTRING(t->GetChipText());
			extra = t->GetUpdateStamp();
		}
		else {
			Value *v = dynamic_cast<Value*>(n->second.first);
			if (v) {
				value val = v->GetChipValue();
				txt = QString::number(val);
				align = Qt::AlignRight;
				extra = val;
			}
			else {
				VectorChip *vec = dynamic_cast<VectorChip*>(n->second.first);
				if (vec) {
					XMFLOAT4 v = vec->GetChipVector();
					txt = QString::number(v.x) + "; " + QString::number(v.y) + "; " + QString::number(v.z) + "; " + QString::number(v.w);
					align = Qt::AlignRight;
					extra.fromValue<XMFLOAT4>(v);
				}
				else {
					QLabel *l = new QLabel();
					l->setPixmap(formPixMap);
					l->setAlignment(Qt::AlignCenter);
					ui.tableWidget->setCellWidget(i, 2, l);
					continue;
				}
			}
		}
		QTableWidgetItem *itm = new QTableWidgetItem(txt);
		itm->setTextAlignment(align | Qt::AlignVCenter);
		itm->setData(Qt::UserRole, extra);
		ui.tableWidget->setItem(i, 2, itm);
	}
}

void ClassInstanceRefChip_Dlg::onCellChanged(int row, int column)
{
	if (!ui.tableWidget->editing())
		return;

	Chip *c = _getDataChip(ui.tableWidget->item(row, 0)->data(Qt::UserRole).toUInt());
	if (!c)
		return;

	QTableWidgetItem *w = ui.tableWidget->item(row, column);
	if (!w)
		return;

	bool setDirty = false;

	ui.tableWidget->blockSignals(true);

	Text *t = dynamic_cast<Text*>(c);
	if (t) {
		t->SetText(FROMQSTRING(w->text()));
		w->setData(Qt::UserRole, t->GetUpdateStamp());
		setDirty = true;
	}
	else {
		Value *v = dynamic_cast<Value*>(c);
		if (v) {
			bool ok = false;
			value val = w->text().toDouble(&ok);
			if (ok) {
				v->SetValue(val);
				w->setData(Qt::UserRole, val);
				setDirty = true;
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
					setDirty = true;
				}
				if (c < 4)
					w->setText(QString::number(n.x) + "; " + QString::number(n.y) + "; " + QString::number(n.z) + "; " + QString::number(n.w));
				w->setData(Qt::UserRole, QVariant::fromValue<XMFLOAT4>(n));
			}
		}
	}

	ui.tableWidget->blockSignals(false);

	ClassInstanceRef instance = GetChip()->GetInstance();
	if (instance && setDirty)
		SetDirty(instance->GetOwner()->GetClass());
}

void ClassInstanceRefChip_Dlg::hideMembersClicked()
{
	bool b = ui.checkBox_hideMembers->isChecked();
	for (int i = 0; i < ui.tableWidget->rowCount(); i++) {
		if (ui.tableWidget->item(i, 1)->data(Qt::UserRole).toBool()) {
			if (b)
				ui.tableWidget->hideRow(i);
			else
				ui.tableWidget->showRow(i);
		}
	}
}

void ClassInstanceRefChip_Dlg::saveInstanceClicked()
{
	GetChip()->SetSaveRef(ui.checkBox_saveInstance->isChecked());
	SetDirty();
}

void ClassInstanceRefChip_Dlg::preloadClicked()
{
	GetChip()->setPreload(ui.checkBox_preload->isChecked());
	SetDirty();
}

void ClassInstanceRefChip_Dlg::ownerClicked()
{
	if (!GetChip()->GetInstance().IsOwner() && ui.checkBox_owner->isChecked()) {
		if (QMessageBox::question(this, "Instance Ref", "Are you sure you want this reference to take ownership of the instance?", QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
			return;
		GetChip()->MakeOwner();
		SetDirty(); // TODO: Mark old owner dirty?
	}
	ui.checkBox_owner->setChecked(GetChip()->GetInstance().IsOwner());
}

void ClassInstanceRefChip_Dlg::nameChanged()
{
	ClassInstanceRef instance = GetChip()->GetInstance();
	if (instance) {
		instance->SetName(FROMQSTRING(ui.lineEdit_name->text()));
		if (instance->GetOwner())
			SetDirty(instance->GetOwner()->GetClass());
	}
}