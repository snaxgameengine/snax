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

#include "StdAfx.h"
#include "VectorChip_Dlg.h"
#include "ChipDialogs/NumberToQString.h"
#include <qcolordialog.h>

using namespace m3d;


DIALOGDESC_DEF(VectorChip_Dlg, VECTORCHIP_GUID);


VectorChip_Dlg::VectorChip_Dlg()
{
	ui.setupUi(this);
	_initVector = _currentVector = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);

	connect(ui.pushButton_palette, &QPushButton::clicked, [this]() {
		XMFLOAT4 v = getColorFromWidgets();
		QColor c = QColor::fromRgbF(qBound(0.0f, v.x, 1.0f), qBound(0.0f, v.y, 1.0f), qBound(0.0f, v.z, 1.0f), qBound(0.0f, v.w, 1.0f));
		QColorDialog* dlg = new QColorDialog(c, this);
		connect(dlg, &QColorDialog::currentColorChanged, [this](QColor c) {
			XMFLOAT4 v((float32)c.redF(), (float32)c.greenF(), (float32)c.blueF(), (float32)c.alphaF());
			ui.lineEdit_1->setText(NumberToString(_currentVector.x = v.x));
			ui.lineEdit_2->setText(NumberToString(_currentVector.y = v.y));
			ui.lineEdit_3->setText(NumberToString(_currentVector.z = v.z));
			ui.lineEdit_4->setText(NumberToString(_currentVector.w = v.w));
			onValueChange();
			});
		connect(dlg, &QDialog::rejected, [this, v]() {
			ui.lineEdit_1->setText(NumberToString(_currentVector.x = v.x));
			ui.lineEdit_2->setText(NumberToString(_currentVector.y = v.y));
			ui.lineEdit_3->setText(NumberToString(_currentVector.z = v.z));
			ui.lineEdit_4->setText(NumberToString(_currentVector.w = v.w));
			onValueChange();
			});
		dlg->show();
		});
}

VectorChip_Dlg::~VectorChip_Dlg()
{
}

void VectorChip_Dlg::Init()
{
	_initVector = GetChip()->GetChipVector();
	ui.lineEdit_1->setText(NumberToString(_currentVector.x = _initVector.x));
	ui.lineEdit_2->setText(NumberToString(_currentVector.y = _initVector.y));
	ui.lineEdit_3->setText(NumberToString(_currentVector.z = _initVector.z));
	ui.lineEdit_4->setText(NumberToString(_currentVector.w = _initVector.w));
}

void VectorChip_Dlg::OnCancel()
{
	if (IsEdited())
		GetChip()->SetVector(_initVector);
}

void VectorChip_Dlg::AfterApply()
{
	_initVector = GetChip()->GetChipVector();
}

void VectorChip_Dlg::Refresh()
{
	XMFLOAT4 v = GetChip()->GetChipVector();
	QLineEdit *_le[4] = {ui.lineEdit_1, ui.lineEdit_2, ui.lineEdit_3, ui.lineEdit_4};
	for (uint32 i = 0; i < 4; i++) {
		if (((float32*)&v.x)[i] != ((float32*)&_currentVector.x)[i] && !_le[i]->hasFocus())
			_le[i]->setText(NumberToString(((float32*)&_currentVector.x)[i] = ((float32*)&v.x)[i]));
	}

}

void VectorChip_Dlg::onValueChange()
{
	GetChip()->SetVector(_currentVector = getColorFromWidgets());
	SetDirty();
}

XMFLOAT4 VectorChip_Dlg::getColorFromWidgets()
{
	XMFLOAT4 f;

	QLineEdit* _le[4] = { ui.lineEdit_1, ui.lineEdit_2, ui.lineEdit_3, ui.lineEdit_4 };
	for (uint32 i = 0; i < 4; i++) {
		((float32*)&f)[i] = _le[i]->text().toFloat(); // Returns 0 if failed. That is ok!
	}

	return f;
}