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
#include "ShellChip.h"
#include "M3DEngine/Class.h"


using namespace m3d;


// NOTE: Shell-chip is not used! Maybe later!
// CHIPDESCV1_DEF(ShellChip, MTEXT("Shell Chip"), SHELLCHIP_GUID, PROXYCHIP_GUID);


ShellChip::ShellChip()
{
	_original = nullptr;
}

ShellChip::~ShellChip()
{
}

bool ShellChip::InitChip()
{
	_original = GetClass()->GetChips().begin()->second;
	SetChipType(_original->GetChipType());
	_original->AddConnectionClone(this);
	return true;
}

bool ShellChip::CopyChip(Chip *chip)
{
	ShellChip *c = dynamic_cast<ShellChip*>(chip);
	B_RETURN(ProxyChip::CopyChip(c));
	return true;
}

bool ShellChip::LoadChip(DocumentLoader &loader)
{
	B_RETURN(ProxyChip::LoadChip(loader));
	return true;
}

bool ShellChip::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(ProxyChip::SaveChip(saver));
	return true;
}

ChipChildPtr ShellChip::GetChip()
{
	//Touch();
	return _original ? ChipChildPtr(_original, this) : ChipChildPtr();
}

void ShellChip::SetTwinChip(Chip *c)
{
}

