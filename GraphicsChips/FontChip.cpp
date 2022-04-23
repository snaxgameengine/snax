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
#include "FontChip.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "StdChips/Value.h"

using namespace m3d;


CHIPDESCV1_DEF(FontChip, MTEXT("Font"), FONTCHIP_GUID, CHIP_GUID);


FontChip::FontChip()
{
	_font = nullptr;
	_fontName = MTEXT("Arial");
	_size = 10;
	_weight = Font::Weight::NORMAL;
	_italic = false;
	_underline = false;
	_strikeout = false;

	CREATE_CHILD(0, VALUE_GUID, false, UP, MTEXT("Size"));
}

FontChip::~FontChip()
{
	SAFE_DELETE(_font);
}

bool FontChip::CopyChip(Chip *chip)
{
	FontChip *c = dynamic_cast<FontChip*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_fontName = c->_fontName;
	_size = c->_size;
	_weight = c->_weight;
	_italic = c->_italic;
	_underline = c->_underline;
	_strikeout = c->_strikeout;
	return true;
}

bool FontChip::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOAD("name", _fontName);
	LOAD("size", _size);
	LOAD("weight", _weight);
	LOAD("italic", _italic);
	LOAD("underline", _underline);
	LOAD("strikeout", _strikeout);
	return true;
}

bool FontChip::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE("name", _fontName);
	SAVE("size", _size);
	SAVE("weight", _weight);
	SAVE("italic", _italic);
	SAVE("underline", _underline);
	SAVE("strikeout", _strikeout);
	return true;
}

void FontChip::OnDestroyDevice()
{
	SAFE_DELETE(_font);
}

Font *FontChip::GetFont()
{
	uint32 size = _font ? _font->GetSize() : _size;
	if (Refresh) {
		ChildPtr<Value> ch0 = GetChild(0);
		if (ch0)
			size = (uint32)ch0->GetValue();
		size = std::min(std::max(size, 1u), 256u);
	}

	if (_font) {
		if (_font->GetSize() != size) {
			SAFE_DELETE(_font);
		}
		else 
			return _font; 
	}
	_font = new Font();
	if (!_font->Init(_fontName, size, _weight, _italic, _underline, _strikeout)) {
		SAFE_DELETE(_font);
	}
	return _font;
}

void FontChip::SetParameters(String font, uint32 size, Font::Weight weight, bool italic, bool underline, bool strikeout)
{
	_fontName = font;
	_size = size;
	_weight = weight;
	_italic = italic;
	_underline = underline;
	_strikeout = strikeout;
	SAFE_DELETE(_font);
}

