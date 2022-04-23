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
#include "ThisChip.h"
#include "M3DEngine/FunctionStack.h"
#include "M3DEngine/FunctionStackRecord.h"

using namespace m3d;


CHIPDESCV1_DEF(ThisChip, MTEXT("Instance THIS"), THISCHIP_GUID, CLASSINSTANCEREFCHIP_GUID);


ThisChip::ThisChip()
{
}

ThisChip::~ThisChip()
{
}

bool ThisChip::CopyChip(Chip *chip)
{
	ThisChip *c = dynamic_cast<ThisChip*>(chip);
	B_RETURN(Chip::CopyChip(c)); // Skip ClassInstanceRefChip
	return true;
}

bool ThisChip::LoadChip(DocumentLoader &loader)
{
	return Chip::LoadChip(loader); // Skip ClassInstanceRefChip
}

bool ThisChip::SaveChip(DocumentSaver &saver) const
{
	return Chip::SaveChip(saver); // Skip ClassInstanceRefChip
}

ClassInstanceRef ThisChip::GetInstance()
{
	// TODO: What about update stamp for this chip?
	return ClassInstanceRef(functionStack.GetCurrentRecord().instance, false); // Can be nullptr
}
