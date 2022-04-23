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
#include "InstanceData.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/ClassInstance.h"
#include "M3DEngine/ClassInstanceRef.h"
#include "M3DEngine/FunctionStackRecord.h"

using namespace m3d;


CHIPDESCV1_DEF(InstanceData, MTEXT("Instance Data"), INSTANCEDATA_GUID, TEMPLATECHIP_GUID);



InstanceData::InstanceData()
{
}

InstanceData::~InstanceData()
{
}

ChipChildPtr InstanceData::GetChip()
{
	//Touch();

	ClassInstanceRef instance = ClassInstanceRef(functionStack.GetCurrentRecord().instance, false);

//	if (!(instance && instance->Prepare())) {
	if (!instance.Prepare(this)) {
		AddMessage(NoInstanceException());
		return ChipChildPtr(); // No instance set/available
	}

	Chip *chip = instance->GetData(this);
	
	if (!chip) {
		AddMessage(WrongInstanceException(instance->GetClass()->GetName(), GetClass()->GetName()));
		return ChipChildPtr(); // No data found for this chip
	}
	
	//ClearError();

	return chip->GetChip(); // Need to call GetChip() because it may return a different chip. It may eg be a chip switch!
}

bool InstanceData::SetChipType(const Guid &type)
{
	B_RETURN(TemplateChip::SetChipType(type));
	if (GetClass())
		GetClass()->OnInstanceDataTypeSet(this);
	return true;
}

bool InstanceData::SetChipTypeAndCreateTemplate(const Guid &type, Chip *copyTemplateFrom)
{
	B_RETURN(TemplateChip::SetChipType(type));
	CreateTemplate(copyTemplateFrom); // What about return type from this?
	if (GetClass())
		GetClass()->OnInstanceDataTypeSet(this);
	return true;
}
