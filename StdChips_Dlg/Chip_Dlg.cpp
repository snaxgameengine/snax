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
#include "Chip_Dlg.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ChipManager.h"
#include "StdChips/Parameter.h"
#include "M3DEngine/Function.h"

using namespace m3d;

DIALOGDESC_DEF(Chip_Dlg, CHIP_GUID);


Chip_Dlg::Chip_Dlg()
{
	ui.setupUi(this);
	_nameIsEdited = false;
	connect(ui.lineEdit, &QLineEdit::textEdited, [=](const QString&) { _nameIsEdited = true; });
}

Chip_Dlg::~Chip_Dlg()
{
}

void Chip_Dlg::Init()
{
	_initName = GetChip()->GetName();
	ui.lineEdit->setText(TOQSTRING(_initName));

	Function *function = GetChip()->GetFunction();

	_initRM = GetChip()->GetRefreshManager().GetRefreshMode();

	ui.radioButton_rm0->setChecked(_initRM == RefreshManager::RefreshMode::Once);
	ui.radioButton_rm1->setChecked(_initRM == RefreshManager::RefreshMode::OncePerFrame);
	ui.radioButton_rm2->setChecked(_initRM == RefreshManager::RefreshMode::OncePerFunctionCall);
	ui.radioButton_rm3->setChecked(_initRM == RefreshManager::RefreshMode::Always);
	ui.pushButton_resetRM->setEnabled(_initRM == RefreshManager::RefreshMode::Once);
	ui.radioButton_4->setChecked(function == NULL);
	ui.radioButton_5->setChecked(function == NULL ? false : (function->GetType() == Function::Type::Static));
	ui.radioButton_6->setChecked(function == NULL ? false : (function->GetType() == Function::Type::NonVirtual));
	ui.radioButton_7->setChecked(function == NULL ? false : (function->GetType() == Function::Type::Virtual));
	ui.radioButton_8->setChecked(function == NULL ? true : (function->GetAccess() == Function::Access::Public));
	ui.radioButton_9->setChecked(function == NULL ? false : (function->GetAccess() == Function::Access::Protected));
	ui.radioButton_10->setChecked(function == NULL ? false : (function->GetAccess() == Function::Access::Private));

	ui.groupBox_2->setEnabled(function != NULL);
	ui.groupBox_3->setEnabled(function != NULL);
	ui.radioButton_10->setEnabled(!ui.radioButton_7->isChecked());

	if (!GetChip()->CanCreateFunction()) {
		ui.groupBox_4->setEnabled(false);
		return;
	}

	Map<Parameter*, uint32> m;

	if (function) {
		for (const auto &n : function->GetParameters()) {
			m.insert(std::make_pair(n.p, n.c));
		}
	}

	Class *cg = GetChip()->GetClass();
	for (const auto &n : cg->GetParameters())
		m.insert(std::make_pair(n, -1));

	QList<QPair<uint32, QPair<String, Parameter*>>> l;
	for (const auto &n : m)
		l.push_back(qMakePair(n.second, qMakePair(n.first->GetName(), n.first)));

	std::sort(l.begin(), l.end());

	ui.treeWidget->setIndentation(0);
	for (uint32 i = 0; i < l.size(); i++) {
		QStringList sl;
		const ChipInfo *ci = engine->GetChipManager()->GetChipInfo(l[i].second.second->GetChipTypeIndex());
		sl << TOQSTRING(l[i].second.first) << (ci ? TOQSTRING(ci->chipDesc.name) : "Unknown");
		QTreeWidgetItem *item = new QTreeWidgetItem(ui.treeWidget, sl);
		item->setCheckState(0, l[i].first != -1 ? Qt::Checked : Qt::Unchecked);
		item->setData(0, Qt::UserRole, (uint32)l[i].second.second->GetID());
	}
}

void Chip_Dlg::OnOK()
{
	String name = _nameIsEdited ? FROMQSTRING(ui.lineEdit->text()) : _initName;
	name = name != _initName ? name : GetChip()->GetName();

	bool isFunction = !ui.radioButton_4->isChecked();
	Function::Type ft = ui.radioButton_5->isChecked() ? Function::Type::Static : (ui.radioButton_6->isChecked() ? Function::Type::NonVirtual : Function::Type::Virtual);
	Function::Access fa = ui.radioButton_9->isChecked() ? Function::Access::Protected : (ui.radioButton_10->isChecked() ? Function::Access::Private : Function::Access::Public);

	if (isFunction) {
		if (!GetChip()->GetFunction())
			SetDirty();
		Function *function = GetChip()->CreateFunction();
		if (!function)
			ft = (Function::Type)-1;
		else {
			ParameterConnectionSet parameters;
			for (uint32 i = 0, j = 0; i < ui.treeWidget->topLevelItemCount(); i++) {
				QTreeWidgetItem *item = ui.treeWidget->topLevelItem(i);
				if (item->checkState(0) == Qt::Checked) {
					ChipID cid = (ChipID)item->data(0, Qt::UserRole).toUInt();
					Chip *ch = GetChip()->GetClass()->GetChip(cid);
					if (ch) {
						assert(ch->AsParameter());
						parameters.insert(ParameterConnection(ch->AsParameter(), j++));
					}
				}
			}
			if (name != GetChip()->GetName() || ft != function->GetType() || fa != function->GetAccess() || parameters != function->GetParameters()) {
				bool nameOrParameterChange = name != GetChip()->GetName() || parameters != function->GetParameters();
				function->Set(name, ft, fa, parameters);

				SetDirty();

				if (nameOrParameterChange) {
					// We need to mark function call's classes dirty!
					for (const auto &m : function->GetFunctionCalls()) { // Iterate the function calls...
						SetDirty(((Chip*)m)->GetClass());
					}
				}
			}
		}
	}
	if (!isFunction) {
		if (GetChip()->GetFunction()) {
			GetChip()->RemoveFunction();
			SetDirty();
		}
		if (GetChip()->GetName() != name) {
			GetChip()->SetName(name);
			SetDirty();
		}
	}
}

void Chip_Dlg::OnCancel()
{
	if (_initRM != GetChip()->GetRefreshManager().GetRefreshMode()) {
		SetDirty();
		GetChip()->GetRefreshManager().SetRefreshMode(_initRM);
	}

	// Strictly speaking, it would be more correct to have pages changing the name, resetting the name on cancel as well, but it seems simpler to have it here.
	if (_initName != GetChip()->GetName()) {
		SetDirty();
		GetChip()->SetName(_initName);
	}
}

void Chip_Dlg::AfterApply()
{
	_initRM = GetChip()->GetRefreshManager().GetRefreshMode();
	_initName = GetChip()->GetName();
}

void Chip_Dlg::onRefreshModeChange(QAbstractButton *btn)
{
	RefreshManager::RefreshMode rm = (btn == ui.radioButton_rm2 ? RefreshManager::RefreshMode::OncePerFunctionCall : (btn == ui.radioButton_rm1 ? RefreshManager::RefreshMode::OncePerFrame : (btn == ui.radioButton_rm0 ? RefreshManager::RefreshMode::Once : RefreshManager::RefreshMode::Always)));
	ui.pushButton_resetRM->setEnabled(rm == RefreshManager::RefreshMode::Once);
	if (GetChip()->GetRefreshManager().GetRefreshMode() == rm)
		return;
	GetChip()->GetRefreshManager().SetRefreshMode(rm);
	SetDirty();
}

void Chip_Dlg::onFunctionTypeChange(QAbstractButton *btn)
{
	ui.groupBox_2->setEnabled(btn != ui.radioButton_4);
	ui.groupBox_3->setEnabled(btn != ui.radioButton_4);
	ui.radioButton_10->setEnabled(!ui.radioButton_7->isChecked());
	if (ui.radioButton_7->isChecked() && ui.radioButton_10->isChecked())
		ui.radioButton_9->setChecked(true); // cant be virtual and private!
}

void Chip_Dlg::moveSelectedUp()
{
	QTreeWidgetItem *item = ui.treeWidget->currentItem();
	if (item) {
		int i = ui.treeWidget->indexOfTopLevelItem(item);
		if (i > 0) {
			ui.treeWidget->takeTopLevelItem(i);
			ui.treeWidget->insertTopLevelItem(i - 1, item);
			ui.treeWidget->setCurrentItem(item);
		}
	}
}

void Chip_Dlg::moveSelectedDown()
{
	QTreeWidgetItem *item = ui.treeWidget->currentItem();
	if (item) {
		int i = ui.treeWidget->indexOfTopLevelItem(item);
		if (i < ui.treeWidget->topLevelItemCount() - 1) {
			ui.treeWidget->takeTopLevelItem(i);
			ui.treeWidget->insertTopLevelItem(i + 1, item);
			ui.treeWidget->setCurrentItem(item);
		}
	}
}

void Chip_Dlg::selectAll()
{
	for (uint32 i = 0; i < ui.treeWidget->topLevelItemCount(); i++)
		ui.treeWidget->topLevelItem(i)->setCheckState(0, Qt::Checked);
}

void Chip_Dlg::unselectAll()
{
	for (uint32 i = 0; i < ui.treeWidget->topLevelItemCount(); i++)
		ui.treeWidget->topLevelItem(i)->setCheckState(0, Qt::Unchecked);
}

void Chip_Dlg::resetRefreshMode()
{
	GetChip()->GetRefreshManager().Reset();
}