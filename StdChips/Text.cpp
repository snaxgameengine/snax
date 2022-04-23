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
#include "Text.h"
#include "M3DEngine/DocumentSaver.h"
#include "M3DEngine/DocumentLoader.h"


using namespace m3d;


CHIPDESCV1_DEF(Text, MTEXT("Text"), TEXT_GUID, CHIP_GUID);


Text::Text()
{
}

Text::~Text()
{
}

bool Text::CopyChip(Chip *chip)
{
	Text *c = dynamic_cast<Text*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_text = c->_text;
	return true;
}

bool Text::LoadChip(DocumentLoader &loader) 
{ 
	B_RETURN(Chip::LoadChip(loader));
	LOADDEF(MTEXT("text"), _text, String());
	return true;
}

bool Text::SaveChip(DocumentSaver &saver) const 
{
	B_RETURN(Chip::SaveChip(saver));
	SAVEDEF(MTEXT("text"), _text, String());
	return true;
}

void Text::CallChip() 
{ 
	GetText(); 
}

void Text::SetText(String text) 
{ 
	_text = text; 
	SetUpdateStamp();
}

String Text::GetValueAsString() const
{
	String str = _text.substr(0, std::min(_text.length(), (size_t)32));
	if (_text.length() > 32)
		str += MTEXT("...");
	for (uint32 i = 0; i < str.length(); i++)
		if (str[i] == L'\n' || str[i] == L'\t')
			str[i] = L' ';
	return str;
}

