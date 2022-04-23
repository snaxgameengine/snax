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

#pragma once

#include "Exports.h"
#include "StandardDialogPage.h"
#include <QFormLayout>
#include <qcombobox>
#include <qspinbox>
#include <qcheckbox>
#include <qpushbutton>
#include <qlineedit.h>

namespace m3d
{


class CHIPDIALOGS_EXPORT StandardFormDialogPage : public StandardDialogPage
{
	Q_OBJECT
public:
	typedef QList<QPair<QString, QVariant>> ComboBoxInitList;

	StandardFormDialogPage(bool instantUpdateMode = false);
	~StandardFormDialogPage();

	// Setup
	virtual QCheckBox *AddCheckBox(QString label, bool value);
	virtual QSpinBox *AddSpinBox(QString label, int32 value, int32 minimum, int32 maximum, int32 singleStep = 1);
	virtual QDoubleSpinBox *AddDoubleSpinBox(QString label, float64 value, float64 minimum, float64 maximum, float64 singleStep = 1.0);
	virtual QPushButton *AddPushButton(QString label);
	virtual QComboBox *AddComboBox(QString label, ComboBoxInitList elements, QVariant value);
	virtual QLineEdit *AddLineEdit(QString label, QString value);

	virtual bool CheckBoxValue(QWidget *widget) const;
	virtual int32 SpinBoxValue(QWidget *widget) const;
	virtual float64 DoubleSpinBoxValue(QWidget *widget) const;
	virtual QVariant ComboBoxValue(QWidget *widget) const;
	virtual QString LineEditValue(QWidget *widget) const;

	virtual void SetCheckBoxValue(QWidget *widget, bool value);
	virtual void SetSpinBoxValue(QWidget *widget, int32 value);
	virtual void SetDoubleSpinBoxValue(QWidget *widget, float64 value);
	virtual void SetComboBoxValue(QWidget *widget, QVariant value);
	virtual void SetLineEditValue(QWidget *widget, QString value);

	virtual bool IsUpdated() const;
	virtual bool IsUpdated(QWidget *widget) const;

	virtual void OnCancel();
	virtual void AfterApply();

protected:
	enum ElementType { CHECKBOX, SPINBOX, DOUBLESPINBOX, COMBOBOX, LINEEDIT };
	struct Element
	{
		ElementType type;
		QVariant initValue;
		QWidget *widget;
		Element(QCheckBox *widget, bool init) : type(CHECKBOX), initValue(init), widget(widget) {}
		Element(QSpinBox *widget, int32 init) : type(SPINBOX), initValue(init), widget(widget) {}
		Element(QDoubleSpinBox *widget, float64 init) : type(DOUBLESPINBOX), initValue(init), widget(widget) {}
		Element(QComboBox *widget, QVariant init) : type(COMBOBOX), initValue(init), widget(widget) {}
		Element(QLineEdit *widget, QVariant init) : type(LINEEDIT), initValue(init), widget(widget) {}
	};
	QMap<QWidget*, Element> _elements;

	QFormLayout *_layout;

	bool _instantUpdateMode;

	// callbacks
	virtual void CheckBoxUpdated(QCheckBox *widget, bool value) {}
	virtual void SpinBoxUpdated(QSpinBox *widget, int32 value) {}
	virtual void DoubleSpinBoxUpdated(QDoubleSpinBox *widget, float64 value) {}
	virtual void PushButtonPressed(QPushButton *widget) {}
	virtual void ComboBoxUpdated(QComboBox *widget, QVariant value) {}
	virtual void LineEditUpdated(QLineEdit* widget, QString value) {}
	virtual void LineEditFinished(QLineEdit* widget, QString value) {}

protected:
	virtual void _onCheckBoxClicked(bool checked);
	virtual void _onSpinBoxValueChanged(int32 v);
	virtual void _onDoubleSpinBoxValueChanged(float64 v);
	virtual void _onPushButtonPressed();
	virtual void _onComboBoxChanged(int32 i);
	virtual void _onLineEditEditingUpdated();
	virtual void _onLineEditEditingFinished();


};

}
