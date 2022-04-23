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
#include "TextItemDialog.h"
#include <qcolordialog.h>

using namespace m3d;

TextItemDialog::TextItemDialog(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	ui.setupUi(this);
}

TextItemDialog::~TextItemDialog()
{
}

bool TextItemDialog::show(QWidget *parent, QString &text, float &size, QColor &color)
{
	TextItemDialog d(parent);

	d.ui.textEdit_Text->setText(text);
	d.ui.doubleSpinBox_Size->setValue(size);

	d._color = color;
	d.ui.pushButton_Color->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(color.red()).arg(color.green()).arg(color.blue()));

	int r = d.exec();
	if (r != QDialog::Accepted) 
		return false;
	color = d._color;
	size = (float)d.ui.doubleSpinBox_Size->value();
	text = d.ui.textEdit_Text->toPlainText();
	return true;
}

void TextItemDialog::selectColor()
{
	QColor c = QColorDialog::getColor(_color, this, "Text Color", QColorDialog::ShowAlphaChannel);
	if (c.isValid()) {
		_color = c;
		ui.pushButton_Color->setStyleSheet(QString("background-color: rgb(%1, %2, %3);").arg(_color.red()).arg(_color.green()).arg(_color.blue()));
	}
}