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
#include "Textout_Dlg.h"


using namespace m3d;

DIALOGDESC_DEF(Textout_Dlg, TEXTOUT_GUID);


void Textout_Dlg::Init()
{
	ComboBoxInitList hAlign, vAlign;
	hAlign.push_back(std::make_pair(String(MTEXT("Left")), 0));
	hAlign.push_back(std::make_pair(String(MTEXT("Center")), 1));
	hAlign.push_back(std::make_pair(String(MTEXT("Right")), 2));
	vAlign.push_back(std::make_pair(String(MTEXT("Top")), 0));
	vAlign.push_back(std::make_pair(String(MTEXT("Center")), 1));
	vAlign.push_back(std::make_pair(String(MTEXT("Bottom")), 2));

	AddComboBox(MTEXT("Horizontal Alignment:"), hAlign, GetChip()->GetHorizontalAlignment(), [this](Id id, RVariant v) {
		SetDirty(); GetChip()->SetHorizontalAlignment((TextWriter::HAlign)v.ToUInt());
	});

	AddComboBox(MTEXT("Vertical Alignment:"), vAlign, GetChip()->GetVerticalAlignment(), [this](Id id, RVariant v) {
		SetDirty(); GetChip()->SetVerticalAlignment((TextWriter::VAlign)v.ToUInt());
	});
}

