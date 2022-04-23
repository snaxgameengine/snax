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
#include "MatrixArray_Dlg.h"

using namespace m3d;



DIALOGDESC_DEF(MatrixArray_Dlg, MATRIXARRAY_GUID);


MatrixArray_Dlg::MatrixArray_Dlg()
{
	_arraySizeLabel = new QLabel(this);
	gridLayout->addWidget(_arraySizeLabel, 1, 0, 1, 1);

	QFont font("Consolas");
	font.setPointSize(10);
	plainTextEdit->setFont(font);
	plainTextEdit->setWordWrapMode(QTextOption::NoWrap);
}

MatrixArray_Dlg::~MatrixArray_Dlg()
{
}

void MatrixArray_Dlg::Init()
{
	MatrixArray::ArrayType initArray = GetChip()->GetArray();

	QString txt;

	for (size_t i = 0; i < initArray.size(); i++) {
		XMFLOAT4X4 f = initArray[i];
		txt += QString("%1;  %2;  %3;  %4\n")
			.arg(QString("%1; %2; %3; %4").arg(f(0,0)).arg(f(0,1)).arg(f(0,2)).arg(f(0,3)))
			.arg(QString("%1; %2; %3; %4").arg(f(1,0)).arg(f(1,1)).arg(f(1,2)).arg(f(1,3)))
			.arg(QString("%1; %2; %3; %4").arg(f(2,0)).arg(f(2,1)).arg(f(2,2)).arg(f(2,3)))
			.arg(QString("%1; %2; %3; %4").arg(f(3,0)).arg(f(3,1)).arg(f(3,2)).arg(f(3,3)));
	}

	plainTextEdit->setPlainText(txt);

	_arraySizeLabel->setText(QString("Initial Array Size: %1").arg(initArray.size()));

	ResetEditFlag();
}

void MatrixArray_Dlg::OnOK()
{
	if (!IsEdited())
		return;
	QString txt = plainTextEdit->toPlainText();
	QStringList items = txt.split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts);
	MatrixArray::ArrayType a;
	a.reserve(items.size());
	bool ok;
	for (int i = 0; i < items.size(); i++) {
		QStringList vItems = items[i].split(';', Qt::KeepEmptyParts);
		if (vItems.isEmpty())
			continue;
		float32 f[16] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
		for (int j = 0, k = std::min((qsizetype)16, vItems.size()); j < k; j++) {
			f[j] = vItems[j].toFloat(&ok);
			if (!ok)
				f[j] = 0.0f;
		}
		a.push_back(XMFLOAT4X4(f));
	}
	_arraySizeLabel->setText(QString("Initial Array Size: %1").arg(a.size()));
	GetChip()->SetArray(std::move(a));
	SetDirty();
	ResetEditFlag();
}
