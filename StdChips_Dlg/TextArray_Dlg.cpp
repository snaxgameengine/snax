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
#include "TextArray_Dlg.h"
#include <qregularexpression.h>

using namespace m3d;



DIALOGDESC_DEF(TextArray_Dlg, TEXTARRAY_GUID);


TextArray_Dlg::TextArray_Dlg()
{
	_arraySizeLabel = new QLabel(this);
	gridLayout->addWidget(_arraySizeLabel, 1, 0, 1, 1);

	QFont font("Consolas");
	font.setPointSize(10);
	plainTextEdit->setFont(font);
	plainTextEdit->setWordWrapMode(QTextOption::NoWrap);
}

TextArray_Dlg::~TextArray_Dlg()
{
}

void TextArray_Dlg::Init()
{
	TextArray::ArrayType initArray = GetChip()->GetArray();

	QString txt;

	for (size_t i = 0; i < initArray.size(); i++) {
		QString str = TOQSTRING(initArray[i]);
		str.replace("\\r", "\\\\r").replace("\\n", "\\\\n");
		str.replace("\r", "\\r").replace("\n", "\\n");
		txt += str + "\n";
	}

	plainTextEdit->setPlainText(txt);

	_arraySizeLabel->setText(QString("Initial Array Size: %1").arg(initArray.size()));

	ResetEditFlag();
}

void TextArray_Dlg::OnOK()
{
	if (!IsEdited())
		return;
	QString txt = plainTextEdit->toPlainText();
	QStringList items = txt.split(QRegularExpression("[\n]|[\r\n]"), Qt::KeepEmptyParts);
	if (!items.empty() && items.last().isEmpty())
		items.pop_back();
	TextArray::ArrayType a;
	a.reserve(items.size());
	bool ok;
	for (int i = 0; i < items.size(); i++) {
		QString d = items[i];
		d.replace(QRegularExpression("(?<!\\\\)\\\\n"), "\n").replace(QRegularExpression("(?<!\\\\)\\\\r"), "\r");
		d.replace("\\\\r", "\\r").replace("\\\\n", "\\n");
		a.push_back(FROMQSTRING(d));
	}
	_arraySizeLabel->setText(QString("Initial Array Size: %1").arg(a.size()));
	GetChip()->SetArray(std::move(a));
	SetDirty();
	ResetEditFlag();
}
