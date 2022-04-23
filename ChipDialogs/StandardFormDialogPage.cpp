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
#include "StandardFormDialogPage.h"


using namespace m3d;


StandardFormDialogPage::StandardFormDialogPage(bool instantUpdateMode) : _instantUpdateMode(instantUpdateMode)
{
	_layout = new QFormLayout(this);
}

StandardFormDialogPage::~StandardFormDialogPage() 
{
}

void StandardFormDialogPage::OnCancel()
{
	if (_instantUpdateMode) {
		for (QMap<QWidget*, Element>::const_iterator itr = _elements.begin(); itr != _elements.end(); itr++) {
			if (IsUpdated(itr.key())) {
				switch (itr->type)
				{
				case CHECKBOX:
					CheckBoxUpdated((QCheckBox*)itr.key(), itr->initValue.toBool());
					break;
				case SPINBOX:
					SpinBoxUpdated((QSpinBox*)itr.key(), itr->initValue.toInt());
					break;
				case DOUBLESPINBOX:
					DoubleSpinBoxUpdated((QDoubleSpinBox*)itr.key(), itr->initValue.toDouble());
					break;
				case COMBOBOX:
					ComboBoxUpdated((QComboBox*)itr.key(), itr->initValue);
					break;
				case LINEEDIT:
					LineEditUpdated((QLineEdit*)itr.key(), itr->initValue.toString());
					break;
				}
			}
		}
	}
}

void StandardFormDialogPage::AfterApply()
{
	for (QMap<QWidget*, Element>::iterator itr = _elements.begin(); itr != _elements.end(); itr++) {
		if (IsUpdated(itr.key())) {
			switch (itr->type)
			{
			case CHECKBOX:
				itr->initValue = CheckBoxValue(itr.key());
				break;
			case SPINBOX:
				itr->initValue = SpinBoxValue(itr.key());
				break;
			case DOUBLESPINBOX:
				itr->initValue = DoubleSpinBoxValue(itr.key());
				break;
			case COMBOBOX:
				itr->initValue = ComboBoxValue(itr.key());
				break;
			case LINEEDIT:
				itr->initValue = LineEditValue(itr.key());
				break;
			}
		}
	}
}

bool StandardFormDialogPage::IsUpdated() const
{
	for (QMap<QWidget*, Element>::const_iterator itr = _elements.begin(); itr != _elements.end(); itr++) {
		if (IsUpdated(itr.key()))
			return true;
	}
	return false;
}

bool StandardFormDialogPage::IsUpdated(QWidget *widget) const
{
	auto itr = _elements.find(widget);
	if (itr == _elements.end())
		return false;
	switch (itr->type)
	{
	case CHECKBOX:
		return itr->initValue != CheckBoxValue(widget);
	case SPINBOX:
		return itr->initValue != SpinBoxValue(widget);
	case DOUBLESPINBOX:
		return itr->initValue != DoubleSpinBoxValue(widget);
	case COMBOBOX:
		return itr->initValue != ComboBoxValue(widget);
	case LINEEDIT:
		return itr->initValue != LineEditValue(widget);
	}
	return false;
}

QCheckBox *StandardFormDialogPage::AddCheckBox(QString label, bool value)
{
	QCheckBox *w = new QCheckBox(label, this);
	w->setChecked(value);
	_elements.insert(w, Element(w, value));
	_layout->addRow(w);
	connect(w, &QCheckBox::clicked, this, &StandardFormDialogPage::_onCheckBoxClicked);
	return w;
}

bool StandardFormDialogPage::CheckBoxValue(QWidget *widget) const
{
	return ((QCheckBox*)widget)->isChecked();
}

void StandardFormDialogPage::SetCheckBoxValue(QWidget *widget, bool value)
{
	QSignalBlocker b(widget);
	((QCheckBox*)widget)->setChecked(value);
}

void StandardFormDialogPage::SetSpinBoxValue(QWidget *widget, int32 value)
{
	QSignalBlocker b(widget);
	((QSpinBox*)widget)->setValue(value);
}

void StandardFormDialogPage::SetDoubleSpinBoxValue(QWidget *widget, float64 value)
{
	QSignalBlocker b(widget);
	((QDoubleSpinBox*)widget)->setValue(value);
}

void StandardFormDialogPage::SetComboBoxValue(QWidget *widget, QVariant value)
{
	QSignalBlocker b(widget);
	((QComboBox*)widget)->setCurrentIndex(((QComboBox*)widget)->findData(value));
}

void StandardFormDialogPage::SetLineEditValue(QWidget *widget, QString value)
{
	QSignalBlocker b(widget);
	((QLineEdit*)widget)->setText(value);
}


void StandardFormDialogPage::_onCheckBoxClicked(bool checked)
{
	CheckBoxUpdated((QCheckBox*)QObject::sender(), checked);
}

QSpinBox *StandardFormDialogPage::AddSpinBox(QString label, int32 value, int32 minimum, int32 maximum, int32 singleStep)
{
	QSpinBox *w = new QSpinBox(this);
	w->setRange(minimum, maximum);
	w->setValue(value);
	w->setSingleStep(singleStep);
	_elements.insert(w, Element(w, value));
	_layout->addRow(label, w);
	connect(w, qOverload<int>(&QSpinBox::valueChanged), this, &StandardFormDialogPage::_onSpinBoxValueChanged);
	return w;
}

int32 StandardFormDialogPage::SpinBoxValue(QWidget *widget) const
{
	return ((QSpinBox*)widget)->value();
}

void StandardFormDialogPage::_onSpinBoxValueChanged(int32 v)
{
	SpinBoxUpdated((QSpinBox*)QObject::sender(), v);
}

QDoubleSpinBox *StandardFormDialogPage::AddDoubleSpinBox(QString label, float64 value, float64 minimum, float64 maximum, float64 singleStep)
{
	QDoubleSpinBox *w = new QDoubleSpinBox(this);
	w->setRange(minimum, maximum);
	w->setValue(value);
	w->setSingleStep(singleStep);
	_elements.insert(w, Element(w, value));
	_layout->addRow(label, w);
	connect(w, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &StandardFormDialogPage::_onDoubleSpinBoxValueChanged);
	return w;
}

float64 StandardFormDialogPage::DoubleSpinBoxValue(QWidget *widget) const
{
	return ((QDoubleSpinBox*)widget)->value();
}

void StandardFormDialogPage::_onDoubleSpinBoxValueChanged(float64 v)
{
	DoubleSpinBoxUpdated((QDoubleSpinBox*)QObject::sender(), v);
}

QPushButton *StandardFormDialogPage::AddPushButton(QString label)
{
	QPushButton *w = new QPushButton(label, this);
	_layout->addRow(w);
	connect(w, &QPushButton::clicked, this, &StandardFormDialogPage::_onPushButtonPressed);
	return w;
}

void StandardFormDialogPage::_onPushButtonPressed()
{
	PushButtonPressed((QPushButton*)QObject::sender());
}

QComboBox *StandardFormDialogPage::AddComboBox(QString label, ComboBoxInitList elements, QVariant value)
{
	QComboBox *w = new QComboBox(this);
	for (int32 i = 0; i < elements.size(); i++)
		w->addItem(elements[i].first, elements[i].second);
	w->setCurrentIndex(w->findData(value));
	_elements.insert(w, Element(w, value));
	_layout->addRow(label, w);
	connect(w, qOverload<int>(&QComboBox::currentIndexChanged), this, &StandardFormDialogPage::_onComboBoxChanged);
	return w;
}


QVariant StandardFormDialogPage::ComboBoxValue(QWidget *widget) const
{
	int32 i = ((QComboBox*)widget)->currentIndex();
	return i != -1 ? ((QComboBox*)widget)->itemData(i) : QVariant();
}

void StandardFormDialogPage::_onComboBoxChanged(int32 i)
{
	QComboBox *s = (QComboBox*)QObject::sender();
	return ComboBoxUpdated(s, i == -1 ? QVariant() : s->itemData(i));
}

QLineEdit *StandardFormDialogPage::AddLineEdit(QString label, QString value)
{
	QLineEdit *w = new QLineEdit(this);
	w->setText(value);
	_elements.insert(w, Element(w, value));
	_layout->addRow(label, w);
	connect(w, &QLineEdit::textEdited, this, &StandardFormDialogPage::_onLineEditEditingUpdated);
	connect(w, &QLineEdit::editingFinished, this, &StandardFormDialogPage::_onLineEditEditingFinished);
	return w;
}

QString StandardFormDialogPage::LineEditValue(QWidget *widget) const
{
	return ((QLineEdit*)widget)->text();
}

void StandardFormDialogPage::_onLineEditEditingUpdated()
{
	QLineEdit *w = (QLineEdit*)QObject::sender();
	LineEditUpdated(w, w->text());
}

void StandardFormDialogPage::_onLineEditEditingFinished()
{
	QLineEdit *w = (QLineEdit*)QObject::sender();
	LineEditFinished(w, w->text());
}