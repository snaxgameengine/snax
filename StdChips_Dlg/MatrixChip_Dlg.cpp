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
#include "MatrixChip_dlg.h"
#include "ChipDialogs/NumberToQString.h"


using namespace m3d;


DIALOGDESC_DEF(MatrixChip_Dlg, MATRIXCHIP_GUID);


MatrixChip_Dlg::MatrixChip_Dlg()
{
	ui.setupUi(this);

	_le[0][0] = ui.lineEdit_1;
	_le[1][0] = ui.lineEdit_2;
	_le[2][0] = ui.lineEdit_3;
	_le[3][0] = ui.lineEdit_4;
	_le[0][1] = ui.lineEdit_5;
	_le[1][1] = ui.lineEdit_6;
	_le[2][1] = ui.lineEdit_7;
	_le[3][1] = ui.lineEdit_8;
	_le[0][2] = ui.lineEdit_9;
	_le[1][2] = ui.lineEdit_10;
	_le[2][2] = ui.lineEdit_11;
	_le[3][2] = ui.lineEdit_12;
	_le[0][3] = ui.lineEdit_13;
	_le[1][3] = ui.lineEdit_14;
	_le[2][3] = ui.lineEdit_15;
	_le[3][3] = ui.lineEdit_16;

}

MatrixChip_Dlg::~MatrixChip_Dlg()
{
}

void MatrixChip_Dlg::Init()
{
	_initMatrix = GetChip()->GetChipMatrix();
	for (uint32 i = 0; i < 4; i++)
		for (uint32 j = 0; j < 4; j++)
			_le[i][j]->setText(NumberToString(_currentMatrix(i, j) = _initMatrix(i, j)));
}

void MatrixChip_Dlg::OnCancel()
{
	if (IsEdited())
		GetChip()->SetMatrix(_initMatrix);
}

void MatrixChip_Dlg::AfterApply()
{
	_initMatrix = GetChip()->GetChipMatrix();
}

void MatrixChip_Dlg::Refresh()
{
	XMFLOAT4X4 v = GetChip()->GetChipMatrix();
	for (uint32 i = 0; i < 4; i++) {
		for (uint32 j = 0; j < 4; j++) {
			if (v(i, j) != _currentMatrix(i, j) && !_le[i][j]->hasFocus())
				_le[i][j]->setText(NumberToString(_currentMatrix(i, j) = v(i, j)));
		}
	}
}

void MatrixChip_Dlg::onValueChange()
{
	XMFLOAT4X4 m;
	for (uint32 i = 0; i < 4; i++)
		for (uint32 j = 0; j < 4; j++) {
			 m(i, j) = _le[i][j]->text().toFloat(); // Returns 0 if failed. That is ok!
		}
	GetChip()->SetMatrix(_currentMatrix = m);
	SetDirty();
}
