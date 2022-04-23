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
#include "Value_dlg.h"
#include "ChipDialogs/NumberToQString.h"

using namespace m3d;


DIALOGDESC_DEF(Value_Dlg, VALUE_GUID);




Value_Dlg::Value_Dlg()
{
	ui.setupUi(this);
	_initValue = _currentValue = 0.0f;
}

Value_Dlg::~Value_Dlg()
{
}

void Value_Dlg::Init()
{
	_initValue = GetChip()->GetChipValue();
	ui.lineEdit->setText(NumberToString(_currentValue = _initValue));
}

void Value_Dlg::OnCancel()
{
	if (IsEdited())
		GetChip()->SetValue(_initValue);
}

void Value_Dlg::AfterApply()
{
	_initValue = GetChip()->GetChipValue();
}

void Value_Dlg::Refresh()
{
	value v = GetChip()->GetChipValue();
	if (v != _currentValue && !ui.lineEdit->hasFocus()) {
		ui.lineEdit->setText(NumberToString(_currentValue = v));
	}
}

void Value_Dlg::onValueChange(const QString &v)
{
	value f = (value)v.toDouble(); // Returns 0 if failed. That is ok!
	GetChip()->SetValue(_currentValue = f);
	SetDirty();
}


