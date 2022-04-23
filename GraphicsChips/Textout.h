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
#include "TextWriter.h"
#include "Graphics.h"

namespace m3d
{


static const Guid TEXTOUT_GUID = { 0xd6779b90, 0xeca6, 0x4e29, { 0xa3, 0x09, 0xa7, 0x8d, 0x5c, 0x51, 0xc1, 0xf7 } };

class GRAPHICSCHIPS_API Textout : public Chip, public GraphicsUsage
{
	CHIPDESC_DECL;
public:
	Textout();
	virtual ~Textout();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual void CallChip() override;

	virtual void OnDestroyDevice() override; 

	virtual TextWriter::HAlign GetHorizontalAlignment() const { return _hAlign; }
	virtual TextWriter::VAlign GetVerticalAlignment() const { return _vAlign; }

	virtual void SetHorizontalAlignment(TextWriter::HAlign a) { _hAlign = a; }
	virtual void SetVerticalAlignment(TextWriter::VAlign a) { _vAlign = a; }

protected:
	TextWriter *_tw;
	bool _isInit;
	TextWriter::HAlign _hAlign;
	TextWriter::VAlign _vAlign;
};


}