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
#include "FontChip_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(FontChip_Dlg, FONTCHIP_GUID);


FontChip_Dlg::FontChip_Dlg()
{
    gridLayout = new QGridLayout(this);
    pushButton = new QPushButton("Select Font", this);
    gridLayout->addWidget(pushButton, 0, 0, 1, 1);
    connect(pushButton, &QPushButton::clicked, this, &FontChip_Dlg::onSelectFont);
}

FontChip_Dlg::~FontChip_Dlg()
{
}

void FontChip_Dlg::Init()
{
	FontChip *t = GetChip();
	int w;
	switch (t->GetWeight())
	{
	case Font::Weight::EXTRALIGHT: w = QFont::Light; break;
	case Font::Weight::NORMAL: w = QFont::Normal; break;
	case Font::Weight::SEMIBOLD: w = QFont::DemiBold; break;
	case Font::Weight::BOLD: w = QFont::Bold; break;
	case Font::Weight::HEAVY: w = QFont::Black; break;
	default: w = QFont::Normal; break;
	}
	_font = QFont(TOQSTRING(t->GetFontName()), t->GetSize(), w, t->IsItalic());
	_font.setUnderline(t->IsUnderline());
	_font.setStrikeOut(t->IsStrikeout());
}

void FontChip_Dlg::OnOK()
{
	bool underline = _font.underline();
	bool strikeout = _font.strikeOut();
	int size = _font.pointSize();
	bool italic = _font.italic();
	String name = FROMQSTRING(_font.family());
	Font::Weight weight;
	switch (_font.weight())
	{
	case QFont::Light: weight = Font::Weight::EXTRALIGHT; break;
	case QFont::Normal: weight = Font::Weight::NORMAL; break;
	case QFont::DemiBold: weight = Font::Weight::SEMIBOLD; break;
	case QFont::Bold: weight = Font::Weight::BOLD; break;
	case QFont::Black: weight = Font::Weight::HEAVY; break;
	default: weight = Font::Weight::NORMAL; break;
	}
	GetChip()->SetParameters(name, size, weight, italic, underline, strikeout);
}

void FontChip_Dlg::OnCancel()
{
}

void FontChip_Dlg::onSelectFont()
{
	bool ok = false;
	QFont font = QFontDialog::getFont(&ok, _font, this);
	if (ok)
		_font = font;
}
