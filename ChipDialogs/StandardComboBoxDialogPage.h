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
#include <qformlayout.h>
#include <qcombobox>
#include <QSpacerItem>


namespace m3d
{


class CHIPDIALOGS_EXPORT StandardComboBoxDialogPage : public StandardDialogPage
{
	Q_OBJECT
public:
	StandardComboBoxDialogPage(bool instantUpdateMode = true, bool sorted = false);
	~StandardComboBoxDialogPage();

	virtual QSize preferredSize() const { return QSize(300, 0); }

	virtual void AddItem(QString label, QVariant data);
	virtual void SetInit(QVariant data, QVariant defaultData);
	virtual QVariant GetCurrent();
	virtual void sort(Qt::SortOrder order = Qt::AscendingOrder);

	virtual void OnOK();
	virtual void OnCancel();
	virtual void AfterApply();

protected:
	QFormLayout *_layout;
	QComboBox *_comboBox;

	QVariant _init;
	QVariant _defaultData;

	bool _instantUpdateMode;

	virtual bool onSelectionChanged(QVariant data) = 0;

protected:
	void _onSelectionChanged(int32 index);
};

}
