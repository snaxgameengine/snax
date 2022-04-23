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
#include "FunctionData.h"
#include "Value.h"
#include "M3DEngine/Class.h"


using namespace m3d;


CHIPDESCV1_DEF(FunctionData, MTEXT("Function Data"), FUNCTIONDATA_GUID, TEMPLATECHIP_GUID);


FunctionData::FunctionData()
{
	_functionDataID = -1;
}

FunctionData::~FunctionData()
{
}

ChipChildPtr FunctionData::GetChip()
{
	//Touch();

	// See if data is already on the stack...
	Chip *chip = functionStack.GetData(_functionDataID);

	if (!chip) {
		// Data not found on stack => create
		chip = CreateChipFromTemplate();

		if (!chip)
			return ChipChildPtr();

		chip->SetClass(GetClass());
		chip->SetOwner(this); 

		// Set chip on stack!
		functionStack.SetData(_functionDataID, chip);
	}

	//ClearError();

	return chip->GetChip(); // Need to call GetChip() because it may return a different chip. It may eg be a chip switch!
}

bool FunctionData::SetChipType(const Guid &type)
{
	B_RETURN(TemplateChip::SetChipType(type));

	if (GetClass())
		_functionDataID = GetClass()->GetNewFunctionDataID();

	return true;
}
