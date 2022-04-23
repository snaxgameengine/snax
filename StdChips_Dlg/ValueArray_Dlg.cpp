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
#include "ValueArray_Dlg.h"

using namespace m3d;



DIALOGDESC_DEF(ValueArray_Dlg, VALUEARRAY_GUID);


ValueArray_Dlg::ValueArray_Dlg()
{
	_arraySizeLabel = new QLabel(this);
	gridLayout->addWidget(_arraySizeLabel, 1, 0, 1, 1);

	QFont font("Consolas");
	font.setPointSize(10);
	plainTextEdit->setFont(font);
	plainTextEdit->setWordWrapMode(QTextOption::NoWrap);
}

ValueArray_Dlg::~ValueArray_Dlg()
{
}

void ValueArray_Dlg::Init()
{
	ValueArray::ArrayType initArray = GetChip()->GetArray();

	QString txt;

	for (size_t i = 0; i < initArray.size(); i++) {
		txt += QString::number(initArray[i]) + "\n";
	}

	plainTextEdit->setPlainText(txt);

	_arraySizeLabel->setText(QString("Initial Array Size: %1").arg(initArray.size()));

	ResetEditFlag();
}

void ValueArray_Dlg::OnOK()
{
	if (!IsEdited())
		return;
	QString txt = plainTextEdit->toPlainText();
	QStringList items = txt.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
	ValueArray::ArrayType a;
	a.reserve(items.size());
	bool ok;
	for (int i = 0; i < items.size(); i++) {
		float64 d = items[i].toDouble(&ok);
		if (!ok) {
			if (items[i].trimmed().isEmpty())
				continue;
			d = 0.0;
		}
		a.push_back(d);
	}
	_arraySizeLabel->setText(QString("Initial Array Size: %1").arg(a.size()));
	GetChip()->SetArray(std::move(a));
	SetDirty();
	ResetEditFlag();
}
