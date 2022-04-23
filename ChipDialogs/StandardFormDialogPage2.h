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


class CHIPDIALOGS_EXPORT StandardFormDialogPage2 : public StandardDialogPage
{
	Q_OBJECT
public:
	typedef QList<QPair<QString, QVariant>> ComboBoxInitList;
	typedef int32 Id;

	StandardFormDialogPage2(bool instantUpdateMode = false);
	~StandardFormDialogPage2();

	typedef std::function<void(Id, QVariant)> Callback;

	// Setup
	virtual QCheckBox *AddCheckBox(Id id, QString label, Qt::CheckState value, Callback cb = nullptr);
	virtual QSpinBox *AddSpinBox(Id id, QString label, int32 value, int32 minimum, int32 maximum, int32 singleStep = 1, Callback cb = nullptr);
	virtual QDoubleSpinBox *AddDoubleSpinBox(Id id, QString label, float64 value, float64 minimum, float64 maximum, float64 singleStep = 1.0, Callback cb = nullptr);
	virtual QPushButton *AddPushButton(Id id, QString label, Callback cb = nullptr);
	virtual QComboBox *AddComboBox(Id id, QString label, ComboBoxInitList elements, QVariant value, Callback cb = nullptr);
	virtual QLineEdit *AddLineEdit(Id id, QString label, QString value, bool signalImmediately, Callback cb = nullptr);
	virtual QFrame *AddLine();

	virtual void OverloadCheckBox(Id id, QCheckBox *w, Qt::CheckState value, Callback cb = nullptr);
	virtual void OverloadSpinBox(Id id, QSpinBox *w, int32 value, Callback cb = nullptr);
	virtual void OverloadDoubleSpinBox(Id id, QDoubleSpinBox *w, float64 value, Callback cb = nullptr);
	virtual void OverloadPushButton(Id id, QPushButton *w, Callback cb = nullptr);
	virtual void OverloadComboBox(Id id, QComboBox *w, QVariant value, Callback cb = nullptr);
	virtual void OverloadLineEdit(Id id, QLineEdit *w, QString value, Callback cb = nullptr);

	virtual Qt::CheckState CheckBoxValue(Id id) const;
	virtual int32 SpinBoxValue(Id id) const;
	virtual float64 DoubleSpinBoxValue(Id id) const;
	virtual QVariant ComboBoxValue(Id id) const;
	virtual QString LineEditValue(Id id) const;

	virtual void SetCheckBoxValue(Id id, Qt::CheckState value);
	virtual void SetSpinBoxValue(Id id, int32 value);
	virtual void SetDoubleSpinBoxValue(Id id, float64 value);
	virtual void SetComboBoxValue(Id id, QVariant value);
	virtual void SetLineEditValue(Id id, QString value);

	virtual void Activate(Id id);

	virtual bool IsUpdated() const;
	virtual bool IsUpdated(Id id) const;

	virtual void OnCancel();
	virtual void AfterApply();

protected:
	enum ElementType { CHECKBOX, SPINBOX, DOUBLESPINBOX, PUSHBUTTON, COMBOBOX, LINEEDIT };
	struct Element
	{
		ElementType type;
		QVariant initValue;
		QVariant value;
		QWidget *widget;
		Callback cb;
		Element() {} // For map[]...
		Element(QCheckBox *widget, QVariant init, Callback cb = nullptr) : type(CHECKBOX), initValue(init), value(init), widget(widget), cb(cb) {}
		Element(QSpinBox *widget, QVariant init, Callback cb = nullptr) : type(SPINBOX), initValue(init), value(init), widget(widget), cb(cb) {}
		Element(QDoubleSpinBox *widget, QVariant init, Callback cb = nullptr) : type(DOUBLESPINBOX), initValue(init), value(init), widget(widget), cb(cb) {}
		Element(QPushButton *widget, Callback cb = nullptr) : type(PUSHBUTTON), widget(widget), cb(cb) {}
		Element(QComboBox *widget, QVariant init, Callback cb = nullptr) : type(COMBOBOX), initValue(init), value(init), widget(widget), cb(cb) {}
		Element(QLineEdit *widget, QVariant init, Callback cb = nullptr) : type(LINEEDIT), initValue(init), value(init), widget(widget), cb(cb) {}
	};
	QMap<Id, Element> _elements;
	QMap<QWidget*, Id> _actives;

	QFormLayout *_layout;

	bool _instantUpdateMode;

	// callbacks
	virtual void CheckBoxUpdated(Id id, Qt::CheckState value) { _doCallback(id, value); }
	virtual void SpinBoxUpdated(Id id, int32 value) { _doCallback(id, value); }
	virtual void DoubleSpinBoxUpdated(Id id, float64 value) { _doCallback(id, value); }
	virtual void PushButtonPressed(Id id) { _doCallback(id, QVariant()); }
	virtual void ComboBoxUpdated(Id id, QVariant value) { _doCallback(id, value); }
	virtual void LineEditUpdated(Id id, QString value) { _doCallback(id, value); }

private:
	void _doCallback(Id id, QVariant value);

};


}