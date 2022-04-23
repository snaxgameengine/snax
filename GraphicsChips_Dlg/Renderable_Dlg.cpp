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
#include "Renderable_Dlg.h"
#include "GraphicsChips/Geometry.h"



using namespace m3d;

DIALOGDESC_DEF(Renderable_Dlg, RENDERABLE_GUID);


Renderable_Dlg::Renderable_Dlg()
{
	ui.setupUi(this);
}

Renderable_Dlg::~Renderable_Dlg()
{
}

void Renderable_Dlg::Init()
{
	GetChip()->GetSpecificSubsets(_initSpecificSubsets);

	ui.checkBox_culling->setChecked(_initWholeObjectCulling = GetChip()->IsUsingWholeObjectCulling());
	ui.checkBox_subsetCulling->setChecked(_initSubsetCulling = GetChip()->IsUsingSubsetCulling());

	_initBB = GetChip()->GetBoundingBox();
	_fillBB(_initBB);

	ui.listWidget_subsets->clear();
	if (!_initSpecificSubsets.empty()) {
		for (size_t i = 0; i < _initSpecificSubsets.size(); i++) {
			QListWidgetItem *itm = new QListWidgetItem(TOQSTRING(_initSpecificSubsets[i].second));
			itm->setFlags(itm->flags() | Qt::ItemIsUserCheckable);
			itm->setCheckState(Qt::Checked); // Only checked if there was no items in the list!
			itm->setData(Qt::UserRole, _initSpecificSubsets[i].first);
			itm->setData(Qt::UserRole + 1, itm->checkState() == Qt::Checked ? 1 : 0);
			ui.listWidget_subsets->addItem(itm);
		}
	}
}

void Renderable_Dlg::OnOK()
{
}

void Renderable_Dlg::OnCancel()
{
	GetChip()->SetUseWholeObjectCulling(_initWholeObjectCulling);
	GetChip()->SetUseSubsetCulling(_initSubsetCulling);
	GetChip()->SetBoundingBox(_initBB);
	GetChip()->MakeSpecific(_initSpecificSubsets);
}

void Renderable_Dlg::AfterApply()
{
	_initWholeObjectCulling = GetChip()->IsUsingWholeObjectCulling();
	_initSubsetCulling = GetChip()->IsUsingSubsetCulling();
	_initBB = GetChip()->GetBoundingBox();
	GetChip()->GetSpecificSubsets(_initSpecificSubsets);
}

void Renderable_Dlg::updateFromDrawable()
{
	List<std::pair<uint32, String>> subsets;
	_getSubsets(subsets);

	Geometry *d = dynamic_cast<Geometry*>(GetChip()->GetRawChild(0));

	if (!d || d->GetSubsets().empty()) {
		ui.listWidget_subsets->clear();
		if (subsets.size() > 0) {
			GetChip()->MakeUniversal();
			SetDirty();
		}
		return;
	}

	Map<uint32, QListWidgetItem*> old;
	for (int32 i = 0; i < ui.listWidget_subsets->count(); i++) {
		QListWidgetItem *itm = ui.listWidget_subsets->item(i);
		old.insert(std::make_pair(itm->data(Qt::UserRole).toUInt(), itm));
	}
	bool hasOldItems = old.size() > 0;

	const GeometrySubsetList &ssl = d->GetSubsets();
	for (uint32 i = 0; i < ssl.size(); i++) {
		auto n = old.find(i);
		String s = MTEXT("Subset ") + strUtils::fromNum(i) + MTEXT(": ") + (ssl[i].name.empty() ? MTEXT("Unnamed") : ssl[i].name);
		if (n != old.end()) { // Subset exist in ui?
			n->second->setText(TOQSTRING(s)); // Update name!
			old.erase(n);
		}
		else {
			QListWidgetItem *itm = new QListWidgetItem(TOQSTRING(s));
			itm->setFlags(itm->flags() | Qt::ItemIsUserCheckable);
			itm->setCheckState(hasOldItems ? Qt::Unchecked : Qt::Checked); // Only checked if there was no items in the list!
			itm->setData(Qt::UserRole, i);
			itm->setData(Qt::UserRole + 1, itm->checkState() == Qt::Checked ? 1 : 0);
			int32 j = 0;
			for (; j < ui.listWidget_subsets->count(); j++)
				if (ui.listWidget_subsets->item(j)->data(Qt::UserRole).toUInt() > i)
					break;
			ui.listWidget_subsets->insertItem(j, itm); // Insert in the most logical place..
		}
	}

	// Remove subsets from ui that does not exist in drawable's subset list.
	for (const auto &n : old)
		delete n.second; 

	List<std::pair<uint32, String>> newSubsets;
	_getSubsets(newSubsets);

	if (subsets != newSubsets) {
		GetChip()->MakeSpecific(newSubsets);
		SetDirty();
	}
}

void Renderable_Dlg::_getSubsets(List<std::pair<uint32, String>> &subsets)
{
	subsets.clear();
	for (int32 i = 0; i < ui.listWidget_subsets->count(); i++) {
		QListWidgetItem *itm = ui.listWidget_subsets->item(i);
		if (itm->checkState() == Qt::Checked) {
			subsets.push_back(std::make_pair(itm->data(Qt::UserRole).toUInt(), FROMQSTRING(itm->text())));
		}
	}
}

void Renderable_Dlg::calculateBB()
{
	GetChip()->CalculateBoundingBox();
	_fillBB(GetChip()->GetBoundingBox());
	SetDirty();
}

void Renderable_Dlg::moveSsUp()
{
	QList<QListWidgetItem*> sel = ui.listWidget_subsets->selectedItems();
	if (sel.empty() || ui.listWidget_subsets->row(sel.front()) == 0)
		return;
	QListWidgetItem *itm = ui.listWidget_subsets->takeItem(ui.listWidget_subsets->row(sel.front()) - 1);
	ui.listWidget_subsets->insertItem(ui.listWidget_subsets->row(sel.back()) + 1, itm);
	for (int32 i = 0; i < sel.size(); i++) {
		if (sel[i]->checkState() != itm->checkState()) {
			List<std::pair<uint32, String>> newSubsets;
			_getSubsets(newSubsets);
			GetChip()->MakeSpecific(newSubsets);
			SetDirty();
			break;
		}
	}
}

void Renderable_Dlg::moveSsDown()
{
	QList<QListWidgetItem*> sel = ui.listWidget_subsets->selectedItems();
	if (sel.empty() || ui.listWidget_subsets->row(sel.back()) == ui.listWidget_subsets->count() - 1)
		return;
	QListWidgetItem *itm = ui.listWidget_subsets->takeItem(ui.listWidget_subsets->row(sel.back()) + 1);
	ui.listWidget_subsets->insertItem(ui.listWidget_subsets->row(sel.front()), itm);
	for (int32 i = 0; i < sel.size(); i++) {
		if (sel[i]->checkState() != itm->checkState()) {
			List<std::pair<uint32, String>> newSubsets;
			_getSubsets(newSubsets);
			GetChip()->MakeSpecific(newSubsets);
			SetDirty();
			break;
		}
	}
}

void Renderable_Dlg::_fillBB(const AxisAlignedBox &aabb)
{
	XMFLOAT3 a[2] = { aabb.GetMin(), aabb.GetMax() };
	QLineEdit *bbEdit[6] = { ui.lineEdit_bbxa, ui.lineEdit_bbya, ui.lineEdit_bbza, ui.lineEdit_bbxb, ui.lineEdit_bbyb, ui.lineEdit_bbzb };
	for (int32 i = 0; i < 6; i++)
		if (aabb.IsInfinite())
			bbEdit[i]->setText(i < 3 ? "-INF" : "+INF");
		else
			bbEdit[i]->setText(QString::number(((float32*)a)[i]));
}

void Renderable_Dlg::bbEdited()
{
	XMFLOAT3 a[2];
	QLineEdit *bbEdit[6] = { ui.lineEdit_bbxa, ui.lineEdit_bbya, ui.lineEdit_bbza, ui.lineEdit_bbxb, ui.lineEdit_bbyb, ui.lineEdit_bbzb };
	for (int32 i = 0; i < 6; i++) {
		float32 f = 0.0f;
		QString s = bbEdit[i]->text().toLower();
		if (s == "inf" || s == "+inf")
			f = std::numeric_limits<float32>::max();
		else if (s == "-inf")
			f = -std::numeric_limits<float32>::max();
		else
			f = s.toFloat();
		((float32*)a)[i] = f;
	}
	AxisAlignedBox aabb;
	if (a[0].x == -std::numeric_limits<float32>::max() && a[0].y == -std::numeric_limits<float32>::max() && a[0].y == -std::numeric_limits<float32>::max() && a[1].x == std::numeric_limits<float32>::max() && a[1].y == std::numeric_limits<float32>::max() && a[1].y == std::numeric_limits<float32>::max())
		aabb.SetInfinite();
	else
		aabb.Set(a[0], a[1]);
	GetChip()->SetBoundingBox(aabb);
	SetDirty();
}

void Renderable_Dlg::ssSelectionChanged()
{
	QList<QListWidgetItem*> sel = ui.listWidget_subsets->selectedItems();
	ui.pushButton_ssUp->setEnabled(sel.size() > 0 && ui.listWidget_subsets->row(sel.front()) > 0);
	ui.pushButton_ssDown->setEnabled(sel.size() > 0 && ui.listWidget_subsets->row(sel.back()) < ui.listWidget_subsets->count() - 1);
}

void Renderable_Dlg::ssItemChanged(QListWidgetItem*itm)
{
	if (itm->data(Qt::UserRole + 1).toUInt() != itm->checkState()) { // Check state changed?
		List<std::pair<uint32, String>> newSubsets;
		_getSubsets(newSubsets);
		GetChip()->MakeSpecific(newSubsets);
		SetDirty();
	}
}

void Renderable_Dlg::cullingChanged()
{
	if (GetChip()->IsUsingWholeObjectCulling() != ui.checkBox_culling->isChecked()) {
		GetChip()->SetUseWholeObjectCulling(ui.checkBox_culling->isChecked());
		SetDirty();
	}
	if (GetChip()->IsUsingSubsetCulling() != ui.checkBox_subsetCulling->isChecked()) {
		GetChip()->SetUseSubsetCulling(ui.checkBox_subsetCulling->isChecked());
		SetDirty();
	}
}