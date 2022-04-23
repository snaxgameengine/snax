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
#include "ClassChip_dlg.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"


using namespace m3d;


DIALOGDESC_DEF(ClassChip_Dlg, CLASSCHIP_GUID);


ClassChip_Dlg::ClassChip_Dlg()
{
	_checkBox_preload = new QCheckBox("Preload Target", this);
	gridLayout->addWidget(_checkBox_preload, 1, 0, 1, 1, Qt::AlignRight);
}

ClassChip_Dlg::~ClassChip_Dlg()
{
}

void ClassChip_Dlg::Init()
{
	const ClassPtrByStringMap &m = engine->GetClassManager()->GetClasssByName();

	ClassChip *ch = GetChip();

	listWidget->clear();

	Class *cg = ch->GetChipCG();
	ClassID cgid = cg ? cg->GetID() : InvalidClassID;

	for (const auto &n : m) {
		QListWidgetItem *i = new QListWidgetItem(TOQSTRING(n.first), listWidget);
		i->setData(Qt::UserRole, n.second->GetID());
		if (cgid == n.second->GetID())
			listWidget->setCurrentItem(i);
	}

	_checkBox_preload->setChecked(ch->IsPreloadEnabled());
}

void ClassChip_Dlg::OnOK()
{
	ClassChip *ch = GetChip();
	QList<QListWidgetItem*> i = listWidget->selectedItems();
	if (i.size() > 0) {
		Class *cg = ch->GetChipCG();
		ClassID cgid2 = cg ? cg->GetID() : InvalidClassID;

		ClassID cgid = (ClassID)i[0]->data(Qt::UserRole).toInt();
		if (cgid != cgid2) {
			Class *c = engine->GetClassManager()->GetClass(cgid);
			ch->SetCG(c);
			SetDirty();
		}
	}
	if (ch->IsPreloadEnabled() != _checkBox_preload->isChecked()) {
		ch->EnablePreload(_checkBox_preload->isChecked());
		SetDirty();
	}
}
