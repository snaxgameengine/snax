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
#include "ClassInstanceDialog.h"
#include "M3DEngine/ClassInstance.h"
#include "M3DCore/GuidUtil.h"
#include "M3DEngineExt/D3DInclude.h"
#include "StdChips/InstanceData.h"
#include "StdChips/Text.h"
#include "StdChips/Value.h"
#include "StdChips/VectorChip.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/ClassInstanceRef.h"
#include "M3DEngine/Engine.h"
#include <qmessagebox.h>
#include <time.h>

using namespace m3d;



ClassInstanceDialog::ClassInstanceDialog(QWidget * parent, Qt::WindowFlags f) : QDialog(parent, f), _callback(nullptr)
{
	ui.setupUi(this);
	_lastRefresh = 0;

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

	ui.tableWidget->setColumnWidth(0, 200);
	ui.tableWidget->setColumnWidth(1, 150);
	ui.tableWidget->setColumnWidth(2, 150);

	ui.tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
}

ClassInstanceDialog::~ClassInstanceDialog()
{
}

void ClassInstanceDialog::Init(ClassInstance *instance)
{
	_instance = instance;
	_fillData();
	UpdateTitle();
}

void ClassInstanceDialog::_fillData()
{
	static QIcon f(":/EditorApp/Resources/form-icon.png");
	static QPixmap formPixMap = f.pixmap(16, 16);

	ui.tableWidget->clearContents();
	ui.tableWidget->setRowCount(0);
	ui.lineEdit_instanceType->setText("");
	ui.lineEdit_name->setText(TOQSTRING(_instance->GetName()));
	ui.lineEdit_name->setEnabled(_instance->GetOwner() != nullptr);
	ui.lineEdit_id->setText(QString::number(_instance->GetRuntimeID()));

	MultiMap<std::pair<unsigned, String>, std::pair<Chip*, bool>> m;

	if (_instance->IsSerialized()) {
		const ClassInstanceSerialization *s = _instance->GetSerialization();
		ui.lineEdit_instanceType->setText(TOQSTRING(GuidToString(s->clazzid)));

		for (const auto &n : s->data)
			m.insert(std::make_pair(std::make_pair(0, GuidToString(n.first)), std::make_pair(n.second, false)));
	}
	else {
		ui.lineEdit_instanceType->setText(TOQSTRING(_instance->GetClass()->GetName()));

		for (const auto &n : _instance->GetData()) {
			bool b = n.first->GetName().size() && n.first->GetName()[0] == MCHAR('_');
			m.insert(std::make_pair(std::make_pair(_instance->GetClass()->IsBaseClassN(n.first->GetClass()), n.first->GetClass()->GetName() + MTEXT("::") + n.first->GetName()), std::make_pair(n.second, b)));
		}
	}

	ui.tableWidget->setRowCount(m.size());

	unsigned i = 0;

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
		itm->setData(Qt::UserRole, extra);
		itm->setTextAlignment(align | Qt::AlignVCenter);
		ui.tableWidget->setItem(i, 2, itm);
	}
}

void ClassInstanceDialog::Refresh()
{
	// Limit rate to update at a reasonable freq. Do this here so dialogs are in async.
	long ct = engine->GetClockTime();
	if (abs(ct - _lastRefresh) / (float(CLOCKS_PER_SEC) / 1000.0f) < REFRESH_FREQ)
		return;
	_lastRefresh = ct;

	if (!ui.lineEdit_name->hasFocus() && ui.lineEdit_name->text() != TOQSTRING(_instance->GetName()))
		ui.lineEdit_name->setText(TOQSTRING(_instance->GetName()));

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

void ClassInstanceDialog::onCellChanged(int row, int column)
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
				float tmp;
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

	if (_callback && setDirty)
		_callback->SetDirty(_instance->GetOwner()->GetClass());
}

void ClassInstanceDialog::onTableDoubleClick(int row, int column)
{
	Chip *chip = _getDataChipFromRow(row);
	if (!chip)
		return; // this data may have been removed!
	 
	if (_callback)
		_callback->ShowChipDialog(chip);
}

Chip *ClassInstanceDialog::_getDataChipFromRow(int row)
{
	return _getDataChip((ChipID)ui.tableWidget->item(row, 0)->data(Qt::UserRole).toUInt());
}

Chip *ClassInstanceDialog::_getDataChip(ChipID cid)
{
	if (_instance->IsSerialized()) {
		const ClassInstanceSerialization *s = _instance->GetSerialization();

		for (const auto &n : s->data)
			if (n.second->GetID() == cid)
				return n.second;
	}
	else {
		const ChipPtrByInstanceDataPtrMap &m = _instance->GetData();
		for (const auto &n : m)
			if (n.second->GetID() == cid)
				return  n.second;
	}
	return nullptr;
}


void ClassInstanceDialog::hideMembersClicked()
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

void ClassInstanceDialog::Flash()
{
	FLASHWINFO finfo;
	finfo.cbSize = sizeof( FLASHWINFO );
	finfo.hwnd = (HWND)winId();
	finfo.uCount = 6;
	finfo.dwTimeout = 50;
	finfo.dwFlags = FLASHW_CAPTION;
	::FlashWindowEx( &finfo );
}

void ClassInstanceDialog::UpdateTitle()
{
	QString name = TOQSTRING(_instance->GetName());
	if (name.size() == 0)
		name = QString("<Unnamed Instance> (id: %1)").arg(_instance->GetRuntimeID());
	setWindowTitle(name);
}

void ClassInstanceDialog::onFinish(int)
{
	if (_callback)
		_callback->CloseDialog(this);
}

void ClassInstanceDialog::openOwner()
{
	if (_instance->GetOwner() == nullptr) {
		QMessageBox::warning(this, "No Owner", "This instance does not have an owner.", QMessageBox::Ok);
	}
	else if (_callback)
		_callback->ShowChipDialog(_instance->GetOwner());

}

void ClassInstanceDialog::nameChanged()
{
	_instance->SetName(FROMQSTRING(ui.lineEdit_name->text()));
	if (_callback)
		_callback->SetDirty(_instance->GetOwner()->GetClass());
}

void ClassInstanceDialog::copyRef()
{
	ClassInstanceRef::copiedRef = ClassInstanceRef(_instance, false);
}

void ClassInstanceDialog::destroyRef()
{
	if (_callback)
		_callback->SetDirty(_instance->GetOwner()->GetClass());
	_instance->Release();
}

