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
#include "SimpleComboBoxDialogPage.h"
#include "M3DEngine/Chip.h"

using namespace m3d;



SimpleComboBoxDialogPage::SimpleComboBoxDialogPage(bool instantUpdateMode) : _instantUpdateMode(instantUpdateMode)
{
	_initData = (RData)0;
	_defaultData = (RData)0;
	_isInit = false;
	QWidget* w = static_cast<QWidget*>(_ptr);
	QFormLayout *layout = new QFormLayout(w);
	QComboBox *comboBox = new QComboBox(w);
	layout->addRow(comboBox);
	_comboBox = comboBox;

	QSortFilterProxyModel* proxy = new QSortFilterProxyModel(comboBox);
	proxy->setSourceModel(comboBox->model());
	comboBox->model()->setParent(proxy);
	comboBox->setModel(proxy);

	if (instantUpdateMode)
		QObject::connect(comboBox, static_cast<void (QComboBox::*)(int32)>(&QComboBox::currentIndexChanged), [this](int32 index) { _onSelectionChanged(); });

	_onSelectedChangedCallback = [this](RData data) -> bool { return this->onSelectionChanged(data); };
}

SimpleComboBoxDialogPage::~SimpleComboBoxDialogPage()
{
}

void SimpleComboBoxDialogPage::AddItem(String text, RData data)
{
	static_cast<QComboBox*>(_comboBox)->addItem(TOQSTRING(text), data);
}

void SimpleComboBoxDialogPage::SetInit(RData data, RData defaultData)
{
	_initData = data;
	_defaultData = defaultData;
	static_cast<QComboBox*>(_comboBox)->setCurrentIndex(static_cast<QComboBox*>(_comboBox)->findData(_initData));
	_isInit = true;
}

RData SimpleComboBoxDialogPage::GetCurrent()
{
	if (static_cast<QComboBox*>(_comboBox)->currentIndex() != -1)
		return (RData)static_cast<QComboBox*>(_comboBox)->itemData(static_cast<QComboBox*>(_comboBox)->currentIndex()).toUInt();
	return _defaultData;
}

void SimpleComboBoxDialogPage::sort(RSortOrder order)
{
	static_cast<QComboBox*>(_comboBox)->model()->sort((Qt::SortOrder)order);
}

void SimpleComboBoxDialogPage::OnOK()
{
	RData current = GetCurrent();
	if (!_instantUpdateMode && _initData != current)
		_onSelectionChanged();
}

void SimpleComboBoxDialogPage::OnCancel()
{
	if (_instantUpdateMode && _initData != GetCurrent())
		if (_onSelectedChangedCallback)
			if (_onSelectedChangedCallback(_initData))
				SetDirty();
}

void SimpleComboBoxDialogPage::AfterApply()
{
	_initData = GetCurrent();
}

void SimpleComboBoxDialogPage::_onSelectionChanged()
{
	if (!_isInit)
		return;
	QComboBox* comboBox = static_cast<QComboBox*>(_comboBox);
	int32 index = comboBox->currentIndex();
	if (_onSelectedChangedCallback) {
		if (_onSelectedChangedCallback(index != -1 ? comboBox->itemData(index).toUInt() : _defaultData)) {
			GetChip()->SetName(index != -1 ? FROMQSTRING(comboBox->currentText()) : GetChip()->GetChipDesc().name);
			SetDirty();
		}
	}
}