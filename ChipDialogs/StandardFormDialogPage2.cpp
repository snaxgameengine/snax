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
#include "StandardFormDialogPage2.h"

using namespace m3d;


StandardFormDialogPage2::StandardFormDialogPage2(bool instantUpdateMode) : _instantUpdateMode(instantUpdateMode)
{
	_layout = new QFormLayout(this);
}

StandardFormDialogPage2::~StandardFormDialogPage2()
{
}

void StandardFormDialogPage2::OnCancel()
{
	if (_instantUpdateMode) {
		for (QMap<Id, Element>::const_iterator itr = _elements.begin(); itr != _elements.end(); itr++) {
			if (IsUpdated(itr.key())) {
				switch (itr->type)
				{
				case CHECKBOX:
					CheckBoxUpdated(itr.key(), (Qt::CheckState)itr->initValue.toInt());
					break;
				case SPINBOX:
					SpinBoxUpdated(itr.key(), itr->initValue.toInt());
					break;
				case DOUBLESPINBOX:
					DoubleSpinBoxUpdated(itr.key(), itr->initValue.toDouble());
					break;
				case COMBOBOX:
					ComboBoxUpdated(itr.key(), itr->initValue);
					break;
				case LINEEDIT:
					LineEditUpdated(itr.key(), itr->initValue.toString());
					break;
				}
			}
		}
	}
}

void StandardFormDialogPage2::AfterApply()
{
	for (QMap<Id, Element>::iterator itr = _elements.begin(); itr != _elements.end(); itr++) {
		if (IsUpdated(itr.key())) {
			switch (itr->type)
			{
			case CHECKBOX:
				itr->initValue = itr->value;
				break;
			case SPINBOX:
				itr->initValue = itr->value;
				break;
			case DOUBLESPINBOX:
				itr->initValue = itr->value;
				break;
			case COMBOBOX:
				itr->initValue = itr->value;
				break;
			case LINEEDIT:
				itr->initValue = itr->value;
				break;
			}
		}
	}
}

void StandardFormDialogPage2::Activate(Id id)
{
	auto a = _elements.find(id);
	if (a == _elements.end())
		return;
	_actives[a->widget] = id;
	QSignalBlocker b(a->widget);
	switch (a->type) 
	{
	case CHECKBOX: ((QCheckBox*)a->widget)->setCheckState((Qt::CheckState)a->value.toUInt()); break;
	case SPINBOX: ((QSpinBox*)a->widget)->setValue(a->value.toInt()); break;
	case DOUBLESPINBOX: ((QDoubleSpinBox*)a->widget)->setValue(a->value.toDouble()); break;
	case COMBOBOX: ((QComboBox*)a->widget)->setCurrentIndex(((QComboBox*)a->widget)->findData(a->value)); break;
	case LINEEDIT: ((QLineEdit*)a->widget)->setText(a->value.toString()); break;
	}
}

bool StandardFormDialogPage2::IsUpdated() const
{
	for (QMap<Id, Element>::const_iterator itr = _elements.begin(); itr != _elements.end(); itr++) {
		if (IsUpdated(itr.key()))
			return true;
	}
	return false;
}

bool StandardFormDialogPage2::IsUpdated(Id id) const
{
	auto itr = _elements.find(id);
	if (itr == _elements.end())
		return false;
	switch (itr->type)
	{
	case CHECKBOX:
	case SPINBOX:
	case DOUBLESPINBOX:
	case COMBOBOX:
	case LINEEDIT:
		return itr->initValue != itr->value;
	}
	return false;
}

QCheckBox *StandardFormDialogPage2::AddCheckBox(Id id, QString label, Qt::CheckState value, Callback cb)
{
	if (_elements.contains(id))
		return nullptr;
	QCheckBox *w = new QCheckBox(label, this);
	w->setCheckState(value);
	_elements.insert(id, Element(w, value, cb));
	_layout->addRow("", w);
	_actives[w] = id;
	QObject::connect(w, &QCheckBox::stateChanged, this, [this, id, w](int32 v) { Id id = _actives[w]; _elements[id].value = v; CheckBoxUpdated(id, (Qt::CheckState)v); });
	return w;
}

void StandardFormDialogPage2::OverloadCheckBox(Id id, QCheckBox *w, Qt::CheckState value, Callback cb)
{
	if (_elements.contains(id))
		return;
	_elements.insert(id, Element(w, value, cb));
}

Qt::CheckState StandardFormDialogPage2::CheckBoxValue(Id id) const
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->type != CHECKBOX)
		return Qt::Unchecked;
	return (Qt::CheckState)a->value.toUInt();
}

void StandardFormDialogPage2::SetCheckBoxValue(Id id, Qt::CheckState value)
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->type != CHECKBOX)
		return;
	a->value = value;
	QSignalBlocker b(a->widget);
	if (_actives[a->widget] == id)
		((QCheckBox*)a->widget)->setCheckState(value);
}

QSpinBox *StandardFormDialogPage2::AddSpinBox(Id id, QString label, int32 value, int32 minimum, int32 maximum, int32 singleStep, Callback cb)
{
	if (_elements.contains(id))
		return nullptr;
	QSpinBox *w = new QSpinBox(this);
	w->setRange(minimum, maximum);
	w->setValue(value);
	w->setSingleStep(singleStep);
	_elements.insert(id, Element(w, value, cb));
	_layout->addRow(label, w);
	_actives[w] = id;
	QObject::connect(w, static_cast<void(QSpinBox::*)(int32)>(&QSpinBox::valueChanged), this, [this, id, w](int32 i) { Id id = _actives[w]; _elements[id].value = i; SpinBoxUpdated(id, i); });
	return w;
}

void StandardFormDialogPage2::OverloadSpinBox(Id id, QSpinBox *w, int32 value, Callback cb)
{
	if (_elements.contains(id))
		return;
	_elements.insert(id, Element(w, value, cb));
}

int32 StandardFormDialogPage2::SpinBoxValue(Id id) const
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->type != SPINBOX)
		return 0;
	return a->value.toInt();
}

void StandardFormDialogPage2::SetSpinBoxValue(Id id, int32 value)
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->type != SPINBOX)
		return;
	a->value = value;
	QSignalBlocker b(a->widget);
	if (_actives[a->widget] == id)
		((QSpinBox*)a->widget)->setValue(value);
}

QDoubleSpinBox *StandardFormDialogPage2::AddDoubleSpinBox(Id id, QString label, float64 value, float64 minimum, float64 maximum, float64 singleStep, Callback cb)
{
	if (_elements.contains(id))
		return nullptr;
	QDoubleSpinBox *w = new QDoubleSpinBox(this);
	w->setRange(minimum, maximum);
	w->setValue(value);
	w->setSingleStep(singleStep);
	_elements.insert(id, Element(w, value, cb));
	_layout->addRow(label, w);
	_actives[w] = id;
	QObject::connect(w, static_cast<void(QDoubleSpinBox::*)(float64)>(&QDoubleSpinBox::valueChanged), this, [this, id, w](float64 d) { Id id = _actives[w]; _elements[id].value = d; DoubleSpinBoxUpdated(id, d); });
	return w;
}

void StandardFormDialogPage2::OverloadDoubleSpinBox(Id id, QDoubleSpinBox *w, float64 value, Callback cb)
{
	if (_elements.contains(id))
		return;
	_elements.insert(id, Element(w, value, cb));
}

float64 StandardFormDialogPage2::DoubleSpinBoxValue(Id id) const
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->type != DOUBLESPINBOX)
		return 0.0;
	return a->value.toDouble();
}

void StandardFormDialogPage2::SetDoubleSpinBoxValue(Id id, float64 value)
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->type != DOUBLESPINBOX)
		return;
	a->value = value;
	QSignalBlocker b(a->widget);
	if (_actives[a->widget] == id)
		((QDoubleSpinBox*)a->widget)->setValue(value);
}

QPushButton *StandardFormDialogPage2::AddPushButton(Id id, QString label, Callback cb)
{
	if (_elements.contains(id))
		return nullptr;
	QPushButton *w = new QPushButton(label, this);
	_elements.insert(id, Element(w, cb));
	_layout->addRow(w);
	_actives[w] = id;
	QObject::connect(w, &QPushButton::clicked, this, [this, id]() { PushButtonPressed(id); });
	return w;
}

void StandardFormDialogPage2::OverloadPushButton(Id id, QPushButton *w, Callback cb)
{
	if (_elements.contains(id))
		return;
	_elements.insert(id, Element(w, cb));
}

QComboBox *StandardFormDialogPage2::AddComboBox(Id id, QString label, ComboBoxInitList elements, QVariant value, Callback cb)
{
	if (_elements.contains(id))
		return nullptr;
	QComboBox *w = new QComboBox(this);
	for (int32 i = 0; i < elements.size(); i++)
		w->addItem(elements[i].first, elements[i].second);
	w->setCurrentIndex(w->findData(value));
	_elements.insert(id, Element(w, value, cb));
	_layout->addRow(label, w);
	_actives[w] = id;
	QObject::connect(w, static_cast<void(QComboBox::*)(int32)>(&QComboBox::currentIndexChanged), this, [this, id, w](int32 i) { Id id = _actives[w]; _elements[id].value = w->currentData(); ComboBoxUpdated(id, w->currentData()); });
	return w;
}

void StandardFormDialogPage2::OverloadComboBox(Id id, QComboBox *w, QVariant value, Callback cb)
{
	if (_elements.contains(id))
		return;
	_elements.insert(id, Element(w, value, cb));
}

QVariant StandardFormDialogPage2::ComboBoxValue(Id id) const
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->type != COMBOBOX)
		return QVariant();
	return a->value;
}

void StandardFormDialogPage2::SetComboBoxValue(Id id, QVariant value)
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->type != COMBOBOX)
		return;
	a->value = value;
	QSignalBlocker b(a->widget);
	if (_actives[a->widget] == id)
		((QComboBox*)a->widget)->setCurrentIndex(((QComboBox*)a->widget)->findData(value));
}

QLineEdit *StandardFormDialogPage2::AddLineEdit(Id id, QString label, QString value, bool signalImmediately, Callback cb)
{
	if (_elements.contains(id))
		return nullptr;
	QLineEdit *w = new QLineEdit(this);
	w->setText(value);
	_elements.insert(id, Element(w, value, cb));
	_layout->addRow(label, w);
	_actives[w] = id;
	if (signalImmediately)
		QObject::connect(w, &QLineEdit::textEdited, this, [this, id, w](QString s) { Id id = _actives[w]; _elements[id].value = s; LineEditUpdated(id, s); });
	else
		QObject::connect(w, &QLineEdit::editingFinished, this, [this, id, w]() { Id id = _actives[w]; _elements[id].value = w->text(); LineEditUpdated(id, w->text()); });
	return w;
}

void StandardFormDialogPage2::OverloadLineEdit(Id id, QLineEdit *w, QString value, Callback cb)
{
	if (_elements.contains(id))
		return;
	_elements.insert(id, Element(w, value, cb));
}

QString StandardFormDialogPage2::LineEditValue(Id id) const
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->type != LINEEDIT)
		return QString();
	return a->value.toString();
}

void StandardFormDialogPage2::SetLineEditValue(Id id, QString value)
{
	auto a = _elements.find(id);
	if (a == _elements.end() || a->type != LINEEDIT)
		return;
	a->value = value;
	QSignalBlocker b(a->widget);
	if (_actives[a->widget] == id)
		((QLineEdit*)a->widget)->setText(value);
}

QFrame *StandardFormDialogPage2::AddLine()
{
	QFrame *f = new QFrame(this);
	f->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
	f->setFrameShape(QFrame::HLine);
	f->setFrameShadow(QFrame::Sunken);
	_layout->addRow(f);
	return f;
}

void StandardFormDialogPage2::_doCallback(Id id, QVariant value)
{
	auto a = _elements.find(id);
	if (a != _elements.end() && a->cb)
		a->cb(id, value);
}