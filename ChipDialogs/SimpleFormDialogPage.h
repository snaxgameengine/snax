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
#include "SimpleDialogPage.h"
#include <functional>

namespace m3d
{


class CHIPDIALOGS_EXPORT SimpleFormDialogPage : public SimpleDialogPage
{
public:
	typedef List<std::pair<String, RVariant>> ComboBoxInitList;
	typedef int32 Id;

	SimpleFormDialogPage(bool instantUpdateMode = true);
	~SimpleFormDialogPage();

	typedef std::function<void(Id, RVariant)> Callback;

	// Setup
	virtual RPtr AddCheckBox(Id id, String label, RCheckState value, RVariant defaultValue, Callback cb = nullptr);
	virtual RPtr AddCheckBox(String label, RCheckState value, RVariant defaultValue, Callback cb = nullptr);
	virtual RPtr AddCheckBox(Id id, String label, RCheckState value, Callback cb = nullptr) { return AddCheckBox(id, label, value, RVariant(), cb); }
	virtual RPtr AddCheckBox(String label, RCheckState value, Callback cb = nullptr) { return AddCheckBox(label, value, RVariant(), cb); }

	virtual RPtr AddSpinBox(Id id, String label, int32 value, int32 minimum, int32 maximum, int32 singleStep, RVariant defaultValue, Callback cb = nullptr);
	virtual RPtr AddSpinBox(String label, int32 value, int32 minimum, int32 maximum, int32 singleStep, RVariant defaultValue, Callback cb = nullptr);
	virtual RPtr AddSpinBox(Id id, String label, int32 value, int32 minimum, int32 maximum, int32 singleStep = 1, Callback cb = nullptr) { return AddSpinBox(id, label, value, minimum, maximum, singleStep, RVariant(), cb); }
	virtual RPtr AddSpinBox(String label, int32 value, int32 minimum, int32 maximum, int32 singleStep = 1, Callback cb = nullptr) { return AddSpinBox(label, value, minimum, maximum, singleStep, RVariant(), cb); }

	virtual RPtr AddDoubleSpinBox(Id id, String label, float64 value, float64 minimum, float64 maximum, float64 singleStep, RVariant defaultValue, Callback cb = nullptr);
	virtual RPtr AddDoubleSpinBox(String label, float64 value, float64 minimum, float64 maximum, float64 singleStep, RVariant defaultValue, Callback cb = nullptr);
	virtual RPtr AddDoubleSpinBox(Id id, String label, float64 value, float64 minimum, float64 maximum, float64 singleStep = 1.0, Callback cb = nullptr) { return AddDoubleSpinBox(id, label, value, minimum, maximum, singleStep, RVariant(), cb); }
	virtual RPtr AddDoubleSpinBox(String label, float64 value, float64 minimum, float64 maximum, float64 singleStep = 1.0, Callback cb = nullptr) { return AddDoubleSpinBox(label, value, minimum, maximum, singleStep, RVariant(), cb); }

	virtual RPtr AddPushButton(Id id, String label, Callback cb = nullptr);
	virtual RPtr AddPushButton(String label, Callback cb = nullptr);

	virtual RPtr AddComboBox(Id id, String label, ComboBoxInitList elements, RVariant value, RVariant defaultValue, Callback cb = nullptr);
	virtual RPtr AddComboBox(String label, ComboBoxInitList elements, RVariant value, RVariant defaultValue, Callback cb = nullptr);
	virtual RPtr AddComboBox(Id id, String label, ComboBoxInitList elements, RVariant value, Callback cb = nullptr) { return AddComboBox(id, label, elements, value, RVariant(), cb); }
	virtual RPtr AddComboBox(String label, ComboBoxInitList elements, RVariant value, Callback cb = nullptr) { return AddComboBox(label, elements, value, RVariant(), cb); }

	virtual RPtr AddLineEdit(Id id, String label, String value, bool signalImmediately, RVariant defaultValue, Callback cb = nullptr);
	virtual RPtr AddLineEdit(String label, String value, bool signalImmediately, RVariant defaultValue, Callback cb = nullptr);
	virtual RPtr AddLineEdit(Id id, String label, String value, bool signalImmediately, Callback cb = nullptr) { return AddLineEdit(id, label, value, signalImmediately, RVariant(), cb); }
	virtual RPtr AddLineEdit(String label, String value, bool signalImmediately, Callback cb = nullptr) { return AddLineEdit(label, value, signalImmediately, RVariant(), cb); }

	virtual RPtr AddReadOnlyLineEdit(Id id, String label, String value);
	virtual RPtr AddReadOnlyLineEdit(String label, String value);

	virtual RPtr AddLine();

	virtual void OverloadCheckBox(Id id, RPtr w, RCheckState value, Callback cb = nullptr);
	virtual void OverloadSpinBox(Id id, RPtr w, int32 value, Callback cb = nullptr);
	virtual void OverloadDoubleSpinBox(Id id, RPtr w, float64 value, Callback cb = nullptr);
	virtual void OverloadPushButton(Id id, RPtr w, Callback cb = nullptr);
	virtual void OverloadComboBox(Id id, RPtr w, RVariant value, Callback cb = nullptr);
	virtual void OverloadLineEdit(Id id, RPtr w, String value, Callback cb = nullptr);

	virtual RCheckState CheckBoxValue(Id id) const;
	virtual int32 SpinBoxValue(Id id) const;
	virtual float64 DoubleSpinBoxValue(Id id) const;
	virtual RVariant ComboBoxValue(Id id) const;
	virtual String LineEditValue(Id id) const;

	virtual void SetCheckBoxValue(Id id, RCheckState value);
	virtual void SetSpinBoxValue(Id id, int32 value);
	virtual void SetDoubleSpinBoxValue(Id id, float64 value);
	virtual void SetComboBoxValue(Id id, RVariant value);
	virtual void SetLineEditValue(Id id, String value);

	virtual void Activate(Id id);

	virtual bool IsUpdated() const;
	virtual bool IsUpdated(Id id) const;

	RVariant GetValueFromWidget(Id id) const;
	void SetWidgetEnabled(Id id, bool enabled);

	void OnOK() override;
	void OnCancel() override;
	void AfterApply() override;

protected:
	enum ElementType { CHECKBOX, SPINBOX, DOUBLESPINBOX, PUSHBUTTON, COMBOBOX, LINEEDIT };
	struct Element
	{
		ElementType type;
		RVariant initValue;
		RVariant value;
		RPtr widget;
		bool isEnabled;
		Callback cb;
		Element() {} // For map[]...
		Element(RPtr widget, ElementType type, RVariant init, Callback cb = nullptr) : type(type), initValue(init), value(init), widget(widget), isEnabled(true), cb(cb) {}
		Element(RPtr buttonWidget, Callback cb = nullptr) : type(PUSHBUTTON), widget(buttonWidget), isEnabled(true), cb(cb) {}
	};
	Map<Id, Element> _elements;
	Map<RPtr, Id> _actives;

	RPtr _layout;

	bool _instantUpdateMode;

	// callbacks
	virtual void CheckBoxUpdated(Id id, RCheckState value) { _doCallback(id, value); }
	virtual void SpinBoxUpdated(Id id, int32 value) { _doCallback(id, value); }
	virtual void DoubleSpinBoxUpdated(Id id, float64 value) { _doCallback(id, value); }
	virtual void PushButtonPressed(Id id) { _doCallback(id, RVariant()); }
	virtual void ComboBoxUpdated(Id id, RVariant value) { _doCallback(id, value); }
	virtual void LineEditUpdated(Id id, String value) { _doCallback(id, value); }

private:
	void _doCallback(Id id, RVariant value);

};


}
