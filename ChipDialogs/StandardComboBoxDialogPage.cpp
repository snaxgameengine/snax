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
#include "M3DEngine/Chip.h"
#include "StandardComboBoxDialogPage.h"


using namespace m3d;

StandardComboBoxDialogPage::StandardComboBoxDialogPage(bool instantUpdateMode, bool sorted) : _instantUpdateMode(instantUpdateMode)
{
	_layout = new QFormLayout(this);
	_comboBox = new QComboBox(this);
	_layout->addRow(_comboBox);
	if (sorted) {
		QSortFilterProxyModel* proxy = new QSortFilterProxyModel(_comboBox);
		proxy->setSourceModel(_comboBox->model());
		_comboBox->model()->setParent(proxy);
		_comboBox->setModel(proxy);
	}
}

StandardComboBoxDialogPage::~StandardComboBoxDialogPage() 
{
}

void StandardComboBoxDialogPage::AddItem(QString label, QVariant data)
{
	_comboBox->addItem(label, data);
}

void StandardComboBoxDialogPage::SetInit(QVariant data, QVariant defaultData)
{
	_init = data;
	_defaultData = defaultData;
	_comboBox->setCurrentIndex(_comboBox->findData(_init));
	if (_instantUpdateMode)
		connect(_comboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &StandardComboBoxDialogPage::_onSelectionChanged);
}

QVariant StandardComboBoxDialogPage::GetCurrent()
{
	if (_comboBox->currentIndex() != -1)
		return _comboBox->itemData(_comboBox->currentIndex());
	return _defaultData;
}

void StandardComboBoxDialogPage::sort(Qt::SortOrder order)
{
	_comboBox->model()->sort(order);
}

void StandardComboBoxDialogPage::OnOK()
{
	if (!_instantUpdateMode && _init != GetCurrent())
		_onSelectionChanged(_comboBox->currentIndex());
}

void StandardComboBoxDialogPage::OnCancel()
{
	if (_instantUpdateMode && _init != GetCurrent())
		if (onSelectionChanged(_init))
			SetDirty();
}

void StandardComboBoxDialogPage::AfterApply()
{
	_init = GetCurrent();
}

void StandardComboBoxDialogPage::_onSelectionChanged(int32 index)
{
	if (onSelectionChanged(index != -1 ? _comboBox->itemData(index) : _defaultData)) {
		GetChip()->SetName(index != -1 ? FROMQSTRING(_comboBox->currentText()) : GetChip()->GetChipDesc().name);
		SetDirty();
	}
}