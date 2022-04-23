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

#include "M3DEngine/Chip.h"
#include "M3DEngine/GlobalDef.h"
#include "M3DEngine/ClassInstance.h"
#include "M3DEngine/Class.h"



namespace m3d
{

	class ClassExt;

	struct CGCID
	{
		ClassID clazzID;
		ChipID chipID;
		CGCID(ClassID clazzID = InvalidClassID, ChipID chipID = InvalidChipID) : clazzID(clazzID), chipID(chipID) {}
		CGCID(Chip* chip) : clazzID(chip&& chip->GetClass() ? chip->GetClass()->GetID() : InvalidClassID), chipID(chip ? chip->GetID() : InvalidChipID) {}
		bool operator==(const CGCID& rhs) const { return clazzID == rhs.clazzID && chipID == rhs.chipID; }
		bool operator!=(const CGCID& rhs) const { return !(*this == rhs); }
	};

	struct CGIID
	{
		ClassID clazzID;
		ClassInstanceID instanceID;
		CGIID(ClassID clazzID = InvalidClassID, ClassInstanceID instanceID = InvalidClassInstanceID) : clazzID(clazzID), instanceID(instanceID) {}
		CGIID(ClassInstance* instance) : clazzID(instance&& instance->GetClass() ? instance->GetClass()->GetID() : InvalidClassID), instanceID(instance ? instance->GetRuntimeID() : InvalidClassInstanceID) {}
		bool operator==(const CGIID& rhs) const { return clazzID == rhs.clazzID && instanceID == rhs.instanceID; }
		bool operator!=(const CGIID& rhs) const { return !(*this == rhs); }
	};

	struct CallStackRecord
	{
		enum Type { CHIP, FUNCTION_CALL, STATIC_FUNCTION, NONVIRTUAL_FUNCITON, VIRTUAL_FUNCTION, PARAMETER, BREAK_POINT };
		CGCID chip;
		CGIID instance;
		Type type;
		CallStackRecord(CGCID chip = CGCID(), CGIID instance = CGIID(), Type type = CHIP) : chip(chip), instance(instance), type(type) {}
	};

	typedef QList<CallStackRecord> CallStack;

}