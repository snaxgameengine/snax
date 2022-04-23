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

#include "pch.h"
#include "Textout.h"
#include "StdChips/VectorChip.h"
#include "StdChips/MatrixChip.h"
#include "StdChips/Text.h"
#include "FontChip.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Engine.h"


using namespace m3d;



CHIPDESCV1_DEF(Textout, MTEXT("Textout"), TEXTOUT_GUID, CHIP_GUID);


Textout::Textout()
{
	CREATE_CHILD(0, TEXT_GUID, false, UP, MTEXT("Text"));
	CREATE_CHILD(1, VECTORCHIP_GUID, false, UP, MTEXT("Position (x, y)"));
	CREATE_CHILD(2, VECTORCHIP_GUID, false, UP, MTEXT("Color"));
	CREATE_CHILD(3, FONTCHIP_GUID, false, UP, MTEXT("Font"));
	//CREATE_CHILD(4, MATRIXCHIP_GUID, false, UP, MTEXT("Transform"));

	_isInit = false;
	_tw = nullptr;

	_hAlign = TextWriter::LEFT;
	_vAlign = TextWriter::BOTTOM;
}

Textout::~Textout()
{
	SAFE_DELETE(_tw);
}

bool Textout::CopyChip(Chip *chip)
{
	Textout *c = dynamic_cast<Textout*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_hAlign = c->_hAlign;
	_vAlign = c->_vAlign;
	SAFE_DELETE(_tw);
	_isInit = false;
	return true;
}

bool Textout::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOADDEF("halign", (uint32&)_hAlign, TextWriter::LEFT);
	LOADDEF("valign", (uint32&)_vAlign, TextWriter::BOTTOM);
	return true;
}

bool Textout::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVEDEF("halign", (uint32)_hAlign, TextWriter::LEFT);
	SAVEDEF("valign", (uint32)_vAlign, TextWriter::BOTTOM);
	return true;
}

void Textout::CallChip()
{
	ChildPtr<Text> ch0 = GetChild(0);
	if (!ch0)
		return;
	String txt = ch0->GetText();

	XMFLOAT4 p(0.0f, 0.0f, 0.0f, 0.0f);
	ChildPtr<VectorChip> ch1 = GetChild(1);
	if (ch1)
		p = ch1->GetVector();

	XMFLOAT4 c(1.0f, 1.0f, 1.0f, 1.0f);
	ChildPtr<VectorChip> ch2 = GetChild(2);
	if (ch2)
		c = ch2->GetVector();

	ChildPtr<FontChip> ch3 = GetChild(3);
	// NOTE: There is no safe ref to the font. The only reason it is safe, 
	// is because we reset it every time we come here! 
	// DO NOT use TextWriter without ensuring the font is OK!
	Font *f = nullptr;
	if (ch3)
		f = ch3->GetFont();
	if (!f)
		f = graphics()->GetDefaultFont();
	if (!f)
		return;

	if (!_isInit) {
		_tw = new TextWriter();
		if (FAILED(_tw->Init(graphics(), f)))
			return;
		_isInit = true;
	}
	else
		_tw->SetFont(f);

	ChildPtr<MatrixChip> ch4 = GetChild(4);
	if (ch4) {
		_tw->SetTransform(ch4->GetMatrix());
		_tw->SetUseTransform(true);
	}
	else {
		_tw->SetTransform(MatrixChip::IDENTITY);
		_tw->SetUseTransform(false);
	}
	

	_tw->Write(graphics(), XMFLOAT2(p.x, p.y), _hAlign, _vAlign, true, c, txt);
}

void Textout::OnDestroyDevice()
{
	SAFE_DELETE(_tw);
	_isInit = false;
}
