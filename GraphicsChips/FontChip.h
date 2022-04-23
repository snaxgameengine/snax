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

#pragma once

#include "Exports.h"
#include "M3DEngine/Chip.h"
#include "Font.h"
#include "Graphics.h"

namespace m3d
{


static const Guid FONTCHIP_GUID = { 0x15f2f068, 0x95b5, 0x4367, { 0xb9, 0x67, 0x4d, 0xf3, 0x56, 0x0f, 0x84, 0x57 } };


class GRAPHICSCHIPS_API FontChip : public Chip, protected GraphicsUsage
{
	CHIPDESC_DECL;
public:
	FontChip();
	~FontChip();

	bool CopyChip(Chip *chip) override;
	bool LoadChip(DocumentLoader &loader) override;
	bool SaveChip(DocumentSaver &saver) const override;

	void OnDestroyDevice() override; 

	virtual Font *GetFont();

	virtual String GetFontName() const { return _fontName; }
	virtual uint32 GetSize() const { return _size; }
	virtual Font::Weight GetWeight() const { return _weight; }
	virtual bool IsItalic() const { return _italic; }
	virtual bool IsUnderline() const { return _underline; }
	virtual bool IsStrikeout() const { return _strikeout; }
	virtual void SetParameters(String font, uint32 size, Font::Weight weight, bool italic, bool underline, bool strikeout);
	

protected:
	Font *_font;
	String _fontName;
	uint32 _size;
	Font::Weight _weight;
	bool _italic;
	bool _underline;
	bool _strikeout;
};

}