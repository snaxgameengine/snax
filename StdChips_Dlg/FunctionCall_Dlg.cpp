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
#include "FunctionCall_dlg.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/ChipManager.h"
#include "M3DEngine/Function.h"

using namespace m3d;

DIALOGDESC_DEF(FunctionCall_Dlg, FUNCTIONCALL_GUID);


FunctionCall_Dlg::FunctionCall_Dlg()
{
	ui.setupUi(this);
	ui.treeWidget->header()->setSectionHidden(2, true);
	ui.treeWidget->header()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
	ui.treeWidget->header()->setStretchLastSection(true);
	ui.treeWidget->sortByColumn(0, Qt::AscendingOrder);
	ui.treeWidget->setHeaderHidden(true);

	_isSetOnInit = false;
}

FunctionCall_Dlg::~FunctionCall_Dlg()
{
}

void FunctionCall_Dlg::Init()
{
	const ClassPtrByStringMap &cg = engine->GetClassManager()->GetClasssByName();
	_isSetOnInit = GetChip()->IsChipTypeSet();
	ui.treeWidget->clear();
	for (const auto &n : cg) {
		List<Function*> funcList;
		const FunctionPtrSet &functions = n.second->GetFunctions();
		ChipTypeIndex gi = _isSetOnInit ? GetChip()->GetChipTypeIndex() : engine->GetChipManager()->GetChipTypeIndex(CHIP_GUID);
		for (const auto &m : functions)
			if (engine->GetChipManager()->IsChipTypeSupported(gi, m->GetChip()->GetChipTypeIndex()))
				funcList.push_back(m);
		if (funcList.size() == 0)
			continue;
		QStringList sl;
		sl << (TOQSTRING(n.first) + "    ") << "" << "";
		QTreeWidgetItem *item = new QTreeWidgetItem(ui.treeWidget, sl);
		for (uint32 i = 0; i < funcList.size(); i++) {
			QStringList sl;
			const ChipInfo *ci = engine->GetChipManager()->GetChipInfo(funcList[i]->GetChip()->GetChipTypeIndex());
			assert(ci);
			sl << (TOQSTRING(funcList[i]->GetChip()->GetName()) + "    ") << TOQSTRING(ci->chipDesc.name) << (TOQSTRING(n.first) + "::" + TOQSTRING(funcList[i]->GetChip()->GetName()));
			QTreeWidgetItem *itm = new QTreeWidgetItem(item, sl);
		}
	}
	ui.lineEdit_1->setText(TOQSTRING(GetChip()->GetClassName()));
	ui.lineEdit_2->setText(TOQSTRING(GetChip()->GetFunctionName()));
	ui.checkBox->setChecked(GetChip()->IsCallByName());
	ui.checkBox_preload->setChecked(GetChip()->IsPreload());
	funcTextChanged();
}

void FunctionCall_Dlg::OnOK()
{
	if (!GetChip()->IsChipTypeSet()) {
		Class *cg = engine->GetClassManager()->GetClass(FROMQSTRING(ui.lineEdit_1->text()));
		if (cg) {
			Function *function = cg->GetFunction(FROMQSTRING(ui.lineEdit_2->text()));
			if (function) {
				if (GetChip()->SetChipType(function->GetChip()->GetChipType()))
					SetDirty();
			}
		}
	}
	String s1 = FROMQSTRING(ui.lineEdit_1->text());
	String s2 = FROMQSTRING(ui.lineEdit_2->text());
	bool callByName = ui.checkBox->isChecked();
	if (GetChip()->GetClassName() != s1 || GetChip()->GetFunctionName() != s2 || GetChip()->IsCallByName() != callByName) {
		GetChip()->SetFunction(s1, s2, callByName, false);
		SetDirty();
	}
	if (GetChip()->IsPreload() != ui.checkBox_preload->isChecked()) {
		GetChip()->SetPreload( ui.checkBox_preload->isChecked());
		SetDirty();
	}
	GetChip()->ConnectToFunction();
}

void FunctionCall_Dlg::AfterApply()
{
	if (_isSetOnInit != GetChip()->IsChipTypeSet())
		Init();
}

void FunctionCall_Dlg::funcTextChanged()
{
	QList<QTreeWidgetItem *> ql = ui.treeWidget->findItems(ui.lineEdit_1->text() + "::" + ui.lineEdit_2->text(), Qt::MatchExactly | Qt::MatchRecursive, 2);

	if (ql.size() > 0) {
		ui.treeWidget->setCurrentItem(ql[0]);
	}
	else
		ui.treeWidget->setCurrentItem(0);
}

void FunctionCall_Dlg::funcTreeItemChanged()
{
	QTreeWidgetItem *item = ui.treeWidget->currentItem();
	if (item->parent() == 0)
		return;
	ui.lineEdit_1->setText(item->parent()->text(0).trimmed());
	ui.lineEdit_2->setText(item->text(0).trimmed());
}