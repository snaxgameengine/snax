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
#include "SimpleFormDialogPage.h"
#include <qspinbox.h>
#include <QDoubleSpinBox>
#include <qpushbutton.h>


using namespace m3d;

static SimpleFormDialogPage::Id AutoIdCounter = 0;


SimpleFormDialogPage::SimpleFormDialogPage(bool instantUpdateMode) : _instantUpdateMode(instantUpdateMode)
{
	_layout = new QFormLayout(GetWidget());
}

SimpleFormDialogPage::~SimpleFormDialogPage()
{
}

void SimpleFormDialogPage::OnOK()
{
	if (_instantUpdateMode)
		return;
	for (const auto &itr : _elements) {
		if (IsUpdated(itr.first)) {
			switch (itr.second.type)
			{
			case CHECKBOX:
				CheckBoxUpdated(itr.first, (RCheckState)itr.second.value.ToUInt());
				break;
			case SPINBOX:
				SpinBoxUpdated(itr.first, itr.second.value.ToInt());
				break;
			case DOUBLESPINBOX:
				DoubleSpinBoxUpdated(itr.first, itr.second.value.ToDouble());
				break;
			case COMBOBOX:
				ComboBoxUpdated(itr.first, itr.second.value);
				break;
			case LINEEDIT:
				LineEditUpdated(itr.first, itr.second.value.ToString());
				break;
			}
		}
	}
}

void SimpleFormDialogPage::OnCancel()
{
	if (!_instantUpdateMode)
		return;
	for (const auto &itr : _elements) {
		if (IsUpdated(itr.first)) {
			switch (itr.second.type)
			{
			case CHECKBOX:
				CheckBoxUpdated(itr.first, (RCheckState)itr.second.initValue.ToUInt());
				break;
			case SPINBOX:
				SpinBoxUpdated(itr.first, itr.second.initValue.ToInt());
				break;
			case DOUBLESPINBOX:
				DoubleSpinBoxUpdated(itr.first, itr.second.initValue.ToDouble());
				break;
			case COMBOBOX:
				ComboBoxUpdated(itr.first, itr.second.initValue);
				break;
			case LINEEDIT:
				LineEditUpdated(itr.first, itr.second.initValue.ToString());
				break;
			}
		}
	}
}

void SimpleFormDialogPage::AfterApply()
{
	for (auto &itr : _elements) {
		if (IsUpdated(itr.first)) {
			switch (itr.second.type)
			{
			case CHECKBOX:
				itr.second.initValue = itr.second.value;
				break;
			case SPINBOX:
				itr.second.initValue = itr.second.value;
				break;
			case DOUBLESPINBOX:
				itr.second.initValue = itr.second.value;
				break;
			case COMBOBOX:
				itr.second.initValue = itr.second.value;
				break;
			case LINEEDIT:
				itr.second.initValue = itr.second.value;
				break;
			}
		}
	}
}

void SimpleFormDialogPage::Activate(Id id)
{
	auto a = _elements.find(id);
	if (a == _elements.end())
		return;
	_actives[a->second.widget] = id;
	QSignalBlocker b(static_cast<QWidget*>(a->second.widget));
	switch (a->second.type)
	{
	case CHECKBOX: (static_cast<QCheckBox*>(a->second.widget))->setCheckState((Qt::CheckState)a->second.value.ToUInt()); break;
	case SPINBOX: (static_cast<QSpinBox*>(a->second.widget))->setValue(a->second.value.ToInt()); break;
	case DOUBLESPINBOX: (static_cast<QDoubleSpinBox*>(a->second.widget))->setValue(a->second.value.ToDouble()); break;
	case COMBOBOX: (static_cast<QComboBox*>(a->second.widget))->setCurrentIndex(static_cast<QComboBox*>(a->second.widget)->findData(*static_cast<QVariant*>(a->second.value.ptr()))); break;
	case LINEEDIT: (static_cast<QLineEdit*>(a->second.widget))->setText(TOQSTRING(a->second.value.ToString())); break;
	}
	static_cast<QWidget*>(a->second.widget)->setEnabled(a->second.isEnabled);
}

bool SimpleFormDialogPage::IsUpdated() const
{
	for (const auto &itr : _elements) {
		if (IsUpdated(itr.first))
			return true;
	}
	return false;
}

bool SimpleFormDialogPage::IsUpdated(Id id) const
{
	auto itr = _elements.find(id);
	if (itr == _elements.end())
		return false;
	switch (itr->second.type)
	{
	case CHECKBOX:
	case SPINBOX:
	case DOUBLESPINBOX:
	case COMBOBOX:
	case LINEEDIT:
		return itr->second.initValue != itr->second.value;
	}
	return false;
}

RPtr SimpleFormDialogPage::AddCheckBox(String label, RCheckState value, RVariant defaultValue, Callback cb) { return AddCheckBox(--AutoIdCounter, label, value, defaultValue, cb); }

RPtr SimpleFormDialogPage::AddCheckBox(Id id, String label, RCheckState value, RVariant defaultValue, Callback cb)
{
	if (_elements.find(id) != _elements.end())
		return nullptr;
	QCheckBox* w = new QCheckBox(TOQSTRING(label), GetWidget());
	w->setCheckState((Qt::CheckState)value);
	_elements.insert(std::make_pair(id, Element(w, CHECKBOX, value, cb)));
	static_cast<QFormLayout*>(_layout)->addRow("", w);
	_actives[w] = id;
	QObject::connect(w, &QCheckBox::stateChanged, GetWidget(), [this, id, w](int32 v) { Id id = _actives[w]; _elements[id].value = v; if (_instantUpdateMode) CheckBoxUpdated(id, (RCheckState)v); });
	return w;
}

void SimpleFormDialogPage::OverloadCheckBox(Id id, RPtr w, RCheckState value, Callback cb)
{
	if (_elements.find(id) != _elements.end())
		return;
	_elements.insert(std::make_pair(id, Element(w, CHECKBOX, value, cb)));
}

RCheckState SimpleFormDialogPage::CheckBoxValue(Id id) const
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->second.type != CHECKBOX)
		return RCheckState::Unchecked;
	return (RCheckState)a->second.value.ToUInt();
}

void SimpleFormDialogPage::SetCheckBoxValue(Id id, RCheckState value)
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->second.type != CHECKBOX)
		return;
	a->second.value = value;
	QSignalBlocker b(static_cast<QWidget*>(a->second.widget));
	if (_actives[a->second.widget] == id)
		((QCheckBox*)a->second.widget)->setCheckState((Qt::CheckState)value);
}

RPtr SimpleFormDialogPage::AddSpinBox(String label, int32 value, int32 minimum, int32 maximum, int32 singleStep, RVariant defaultValue, Callback cb) { return AddSpinBox(--AutoIdCounter, label, value, minimum, maximum, singleStep, defaultValue, cb); }

RPtr SimpleFormDialogPage::AddSpinBox(Id id, String label, int32 value, int32 minimum, int32 maximum, int32 singleStep, RVariant defaultValue, Callback cb)
{
	if (_elements.find(id) != _elements.end())
		return nullptr;
	QSpinBox* w = new QSpinBox(GetWidget());
	w->setRange(minimum, maximum);
	w->setValue(value);
	w->setSingleStep(singleStep);
	_elements.insert(std::make_pair(id, Element(w, SPINBOX, value, cb)));
	static_cast<QFormLayout*>(_layout)->addRow(TOQSTRING(label), w);
	_actives[w] = id;
	QObject::connect(w, static_cast<void(QSpinBox::*)(int32)>(&QSpinBox::valueChanged), GetWidget(), [this, id, w](int32 i) { Id id = _actives[w]; _elements[id].value = i; if (_instantUpdateMode) SpinBoxUpdated(id, i); });
	return w;
}

void SimpleFormDialogPage::OverloadSpinBox(Id id, RPtr w, int32 value, Callback cb)
{
	if (_elements.find(id) != _elements.end())
		return;
	_elements.insert(std::make_pair(id, Element(w, SPINBOX, value, cb)));
}

int32 SimpleFormDialogPage::SpinBoxValue(Id id) const
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->second.type != SPINBOX)
		return 0;
	return a->second.value.ToInt();
}

void SimpleFormDialogPage::SetSpinBoxValue(Id id, int32 value)
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->second.type != SPINBOX)
		return;
	a->second.value = value;
	QSignalBlocker b(static_cast<QWidget*>(a->second.widget));
	if (_actives[a->second.widget] == id)
		static_cast<QSpinBox*>(a->second.widget)->setValue(value);
}

RPtr SimpleFormDialogPage::AddDoubleSpinBox(String label, float64 value, float64 minimum, float64 maximum, float64 singleStep, RVariant defaultValue, Callback cb) { return AddDoubleSpinBox(--AutoIdCounter, label, value, minimum, maximum, singleStep, defaultValue, cb); }

RPtr SimpleFormDialogPage::AddDoubleSpinBox(Id id, String label, float64 value, float64 minimum, float64 maximum, float64 singleStep, RVariant defaultValue, Callback cb)
{
	if (_elements.find(id) != _elements.end())
		return nullptr;
	QDoubleSpinBox* w = new QDoubleSpinBox(GetWidget());
	w->setRange(minimum, maximum);
	w->setValue(value);
	w->setSingleStep(singleStep);
	_elements.insert(std::make_pair(id, Element(w, DOUBLESPINBOX, value, cb)));
	static_cast<QFormLayout*>(_layout)->addRow(TOQSTRING(label), w);
	_actives[w] = id;
	QObject::connect(w, static_cast<void(QDoubleSpinBox::*)(float64)>(&QDoubleSpinBox::valueChanged), GetWidget(), [this, id, w](float64 d) { Id id = _actives[w]; _elements[id].value = d; if (_instantUpdateMode) DoubleSpinBoxUpdated(id, d); });
	return w;
}

void SimpleFormDialogPage::OverloadDoubleSpinBox(Id id, RPtr w, float64 value, Callback cb)
{
	if (_elements.find(id) != _elements.end())
		return;
	_elements.insert(std::make_pair(id, Element(w, DOUBLESPINBOX, value, cb)));
}

float64 SimpleFormDialogPage::DoubleSpinBoxValue(Id id) const
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->second.type != DOUBLESPINBOX)
		return 0.0;
	return a->second.value.ToDouble();
}

void SimpleFormDialogPage::SetDoubleSpinBoxValue(Id id, float64 value)
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->second.type != DOUBLESPINBOX)
		return;
	a->second.value = value;
	QSignalBlocker b(static_cast<QWidget*>(a->second.widget));
	if (_actives[a->second.widget] == id)
		static_cast<QDoubleSpinBox*>(a->second.widget)->setValue(value);
}

RPtr SimpleFormDialogPage::AddPushButton(String label, Callback cb) { return AddPushButton(--AutoIdCounter, label, cb); }

RPtr SimpleFormDialogPage::AddPushButton(Id id, String label, Callback cb)
{
	if (_elements.find(id) != _elements.end())
		return nullptr;
	QPushButton* w = new QPushButton(TOQSTRING(label), GetWidget());
	_elements.insert(std::make_pair(id, Element(w, cb)));
	static_cast<QFormLayout*>(_layout)->addRow(w);
	_actives[w] = id;
	QObject::connect(w, &QPushButton::clicked, GetWidget(), [this, id]() { PushButtonPressed(id); });
	return w;
}

void SimpleFormDialogPage::OverloadPushButton(Id id, RPtr w, Callback cb)
{
	if (_elements.find(id) != _elements.end())
		return;
	_elements.insert(std::make_pair(id, Element(w, cb)));
}

RPtr SimpleFormDialogPage::AddComboBox(String label, ComboBoxInitList elements, RVariant value, RVariant defaultValue, Callback cb) { return AddComboBox(--AutoIdCounter, label, elements, value, defaultValue, cb); }

RPtr SimpleFormDialogPage::AddComboBox(Id id, String label, ComboBoxInitList elements, RVariant value, RVariant defaultValue, Callback cb)
{
	if (_elements.find(id) != _elements.end())
		return nullptr;
	QComboBox* w = new QComboBox(GetWidget());
	for (int32 i = 0; i < elements.size(); i++)
		w->addItem(TOQSTRING(elements[i].first), *static_cast<QVariant*>(elements[i].second.ptr()));
	w->setCurrentIndex(w->findData(*static_cast<QVariant*>(value.ptr())));
	_elements.insert(std::make_pair(id, Element(w, COMBOBOX, value, cb)));
	static_cast<QFormLayout*>(_layout)->addRow(TOQSTRING(label), w);
	_actives[w] = id;
	QObject::connect(w, static_cast<void(QComboBox::*)(int32)>(&QComboBox::currentIndexChanged), GetWidget(), [this, id, w](int32 i) { QVariant v = w->currentData(); Id id = _actives[w]; _elements[id].value = RVariant(&v); if (_instantUpdateMode) ComboBoxUpdated(id, RVariant(&v)); });
	return w;
}

void SimpleFormDialogPage::OverloadComboBox(Id id, RPtr w, RVariant value, Callback cb)
{
	if (_elements.find(id) != _elements.end())
		return;
	_elements.insert(std::make_pair(id, Element(w, COMBOBOX, value, cb)));
}

RVariant SimpleFormDialogPage::ComboBoxValue(Id id) const
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->second.type != COMBOBOX)
		return RVariant();
	return a->second.value;
}

void SimpleFormDialogPage::SetComboBoxValue(Id id, RVariant value)
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->second.type != COMBOBOX)
		return;
	a->second.value = value;
	QSignalBlocker b(static_cast<QWidget*>(a->second.widget));
	if (_actives[a->second.widget] == id)
		static_cast<QComboBox*>(a->second.widget)->setCurrentIndex(static_cast<QComboBox*>(a->second.widget)->findData(*static_cast<QVariant*>(value.ptr())));
}

RPtr SimpleFormDialogPage::AddLineEdit(String label, String value, bool signalImmediately, RVariant defaultValue, Callback cb) { return AddLineEdit(--AutoIdCounter, label, value, signalImmediately, defaultValue, cb); }

RPtr SimpleFormDialogPage::AddLineEdit(Id id, String label, String value, bool signalImmediately, RVariant defaultValue, Callback cb)
{
	if (_elements.find(id) != _elements.end())
		return nullptr;
	QLineEdit* w = new QLineEdit(GetWidget());
	w->setText(TOQSTRING(value));
	_elements.insert(std::make_pair(id, Element(w, LINEEDIT, value, cb)));
	static_cast<QFormLayout*>(_layout)->addRow(TOQSTRING(label), w);
	_actives[w] = id;
	if (signalImmediately)
		QObject::connect(w, &QLineEdit::textEdited, GetWidget(), [this, id, w](QString s) { Id id = _actives[w]; _elements[id].value = FROMQSTRING(s); if (_instantUpdateMode) LineEditUpdated(id, FROMQSTRING(s)); });
	else
		QObject::connect(w, &QLineEdit::editingFinished, GetWidget(), [this, id, w]() { Id id = _actives[w]; _elements[id].value = FROMQSTRING(w->text()); if (_instantUpdateMode) LineEditUpdated(id, FROMQSTRING(w->text())); });
	return w;
}

void SimpleFormDialogPage::OverloadLineEdit(Id id, RPtr w, String value, Callback cb)
{
	if (_elements.find(id) != _elements.end())
		return;
	_elements.insert(std::make_pair(id, Element(w, LINEEDIT, value, cb)));
}

String SimpleFormDialogPage::LineEditValue(Id id) const
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->second.type != LINEEDIT)
		return String();
	return a->second.value.ToString();
}

void SimpleFormDialogPage::SetLineEditValue(Id id, String value)
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->second.type != LINEEDIT)
		return;
	a->second.value = value;
	QSignalBlocker b(static_cast<QWidget*>(a->second.widget));
	if (_actives[a->second.widget] == id)
		static_cast<QLineEdit*>(a->second.widget)->setText(TOQSTRING(value));
}

RPtr SimpleFormDialogPage::AddLine()
{
	QFrame* f = new QFrame(GetWidget());
	f->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
	f->setFrameShape(QFrame::HLine);
	f->setFrameShadow(QFrame::Sunken);
	static_cast<QFormLayout*>(_layout)->addRow(f);
	return f;
}

RPtr SimpleFormDialogPage::AddReadOnlyLineEdit(String label, String value) { return AddReadOnlyLineEdit(--AutoIdCounter, label, value); }

RPtr SimpleFormDialogPage::AddReadOnlyLineEdit(Id id, String label, String value)
{
	if (_elements.find(id) != _elements.end())
		return nullptr;
	QLineEdit* w = new QLineEdit(GetWidget());
	w->setReadOnly(true);
	w->setText(TOQSTRING(value));
	_elements.insert(std::make_pair(id, Element(w, LINEEDIT, value)));
	static_cast<QFormLayout*>(_layout)->addRow(TOQSTRING(label), w);
	_actives[w] = id;
	return w;
}

void SimpleFormDialogPage::_doCallback(Id id, RVariant value)
{
	auto a = _elements.find(id);
	if (a == _elements.end() && a->second.cb)
		a->second.cb(id, value);
}

RVariant SimpleFormDialogPage::GetValueFromWidget(Id id) const
{
	auto a = _elements.find(id);
	if (a == _elements.end())
		return RVariant();
	return a->second.value;
}

void SimpleFormDialogPage::SetWidgetEnabled(Id id, bool enabled)
{
	auto a = _elements.find(id);
	if (a == _elements.end())
		return;
	a->second.isEnabled = enabled;
	if (_actives[a->second.widget] == id)
		static_cast<QWidget*>(a->second.widget)->setEnabled(enabled);
}