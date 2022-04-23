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
#include "Text_Dlg.h"


using namespace m3d;


DIALOGDESC_DEF(Text_Dlg, TEXT_GUID);


Text_Dlg::Text_Dlg()
{
	ui.setupUi(this);
	_us = 0;
	_typed = false;
}

Text_Dlg::~Text_Dlg()
{
}

void Text_Dlg::Init()
{
	ui.plainTextEdit->blockSignals(true);
	ui.plainTextEdit->document()->setPlainText(TOQSTRING(GetChip()->GetChipText()));
	ui.plainTextEdit->blockSignals(false);
	_us = GetChip()->GetUpdateStamp();
}

void Text_Dlg::OnOK()
{
	String s = FROMQSTRING(ui.plainTextEdit->document()->toPlainText());
	if (s != GetChip()->GetChipText()) {
		GetChip()->SetText(s);
		SetDirty();
		_us = GetChip()->GetUpdateStamp();
		_typed = false;
	}
}

void Text_Dlg::AfterApply()
{
}

void Text_Dlg::Refresh()
{
	if (!_typed && !ui.plainTextEdit->hasFocus() && _us != GetChip()->GetUpdateStamp()) {
		QString s = ui.plainTextEdit->document()->toPlainText(), t = TOQSTRING(GetChip()->GetChipText());
		if (s != t) {
			ui.plainTextEdit->blockSignals(true);
			ui.plainTextEdit->document()->setPlainText(t);
			ui.plainTextEdit->blockSignals(false);
			_us = GetChip()->GetUpdateStamp();
		}
	}
}

void Text_Dlg::hasTyped()
{
	_typed = true;
}