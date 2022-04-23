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
#include "Parameter.h"
#include "FunctionCall.h"
#include "M3DEngine/Function.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/FunctionStackRecord.h"

using namespace m3d;


CHIPDESCV1_DEF(Parameter, MTEXT("Parameter"), PARAMETER_GUID, PROXYCHIP_GUID);

Parameter::Parameter()
{
}

Parameter::~Parameter()
{
}

ChipChildPtr Parameter::GetChip()
{
	//Touch();

	const FunctionStackRecord &f = functionStack.GetCurrentRecord();

//	// Check the parameter cache to see if we did already register.
//	Map<ChipID, ChipChildPtr>::cNode p = f.parameters.find(GetID());
//	if (p.valid())
//		return p->second;

	if (IsChipTypeSet() && f.f.function && f.f.functionCall) {

		if (!functionStack.CanAddRecord()) {
			AddMessage(StackOverflowException());
			return ChipChildPtr();
		}

		uint32 i = f.f.function->GetParameterIndex(this);
		if (i != -1) { // This function does not have this parameter!
			//ClearError();

			// Check the parameter cache to see if we did already register.
			if (i < f.parameterCount && f.parameters[i])
				return f.parameters[i];//ChipChildPtr(f.parameters[i], this)->GetChip();

			ChipChildPtr ch = ChipChildPtr(f.f.functionCall->GetParameter(i), this);

			if (!ch)
				return ChipChildPtr(); // Function call has no parameter connected.

			ch = ch->GetChip();

//			ChipChildPtr c = ChipChildPtr(f.functionCall, i)->AsFunctionCall()->GetParameter(i);
//			ChipChildPtr c = ChipChildPtr(ch, this)->GetChip();

//			if (ch && i < MAX_PARAMETERS)
//				functionStack.SetParameter(i, ch); // Register us at current stack record. TODO: Incorrect! must use c.

			// NOTE: The idea about the parameter cache is to avoid traversing through *possibly* several function calls each time 
			// we ask for a parameter. If some function does deliver a different parameter each time it is asked, the cache system
			// will always return the first one. This may be a weakness. Be aware of it!
			// TODO: We may add an option in the parameter dialog to enable/disable caching.
			
			return ch;
		}
	}

	AddMessage(IsChipTypeSet() ? (ChipMessage)ParameterNotSetException() : (ChipMessage)ChipTypeNotSetException());

	return ChipChildPtr();
}

bool Parameter::SetChipType(const Guid &type)
{
	B_RETURN(ProxyChip::SetChipType(type));
	ClearConnections();

	if (GetClass())
		GetClass()->OnParameterTypeSet(this);

	return true;
}
