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
#include "ValueContainerElement.h"
#include "ValueArray.h"
#include "SetChip.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"

using namespace m3d;


CHIPDESCV1_DEF(ValueContainerElement, MTEXT("Value Container Element"), VALUECONTAINERELEMENT_GUID, VALUE_GUID);


ValueContainerElement::ValueContainerElement()
{
	ClearConnections();
}

ValueContainerElement::~ValueContainerElement()
{
}

bool ValueContainerElement::CopyChip(Chip *chip)
{
	ValueContainerElement *c = dynamic_cast<ValueContainerElement*>(chip);
	(Value::CopyChip(c)); 
	SetOperatorType(c->_ot);
	return true;
}

bool ValueContainerElement::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Value::LoadChip(loader)); 
	OperatorType ot = OperatorType::NONE;
	LOAD(MTEXT("operatorType|ot"), ot);
	SetOperatorType(ot);
	return true;
}

bool ValueContainerElement::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Value::SaveChip(saver)); 
	SAVE(MTEXT("operatorType"), _ot);
	return true;
}

value ValueContainerElement::GetValue()
{
	RefreshT refresh(Refresh);
	if (!refresh)
		return _value;
	_value = 0.0;
	switch (_ot) 
	{
	case OperatorType::ARRAY:
		{
			ChildPtr<ValueArray> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				value idx = ch1->GetValue();
				uint32 index = (uint32)std::max(idx, 0.0);
				if (index < ch0->GetContainerSize())
					_value = ch0->GetValue(index);
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::SET_INDEX:
		{
			ChildPtr<ValueSetChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				value idx = ch1->GetValue();
				uint32 index = (uint32)std::max(idx, 0.0);
				if (index < ch0->GetContainerSize())
					_value = *ch0->GetElementByIndex(index);
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	default:
		AddMessage(UninitializedException());
		break;
	}
	return _value;
}

void ValueContainerElement::SetValue(value v)
{
	switch (_ot) 
	{
	case OperatorType::ARRAY:
		{
			ChildPtr<ValueArray> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				value idx = ch1->GetValue();
				uint32 index = (uint32)std::max(idx, 0.0);
				if (index < ch0->GetContainerSize())
					ch0->SetValue(index, _value = v);
				else
					AddMessage(ArrayChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::SET_INDEX:
		AddMessage(UnsupportedOperationException());
		break;
	default:
		AddMessage(UninitializedException());
		break;
	}
}

void ValueContainerElement::SetOperatorType(OperatorType ot)
{
	if (_ot == ot)
		return;
	_ot = ot;
	switch (_ot) 
	{
	case OperatorType::ARRAY:
		CREATE_CHILD_KEEP(0, VALUEARRAY_GUID, false, BOTH, MTEXT("Value Array"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, BOTH, MTEXT("Array Index"));
		ClearConnections(2);
		break;
	case OperatorType::SET_INDEX:
		CREATE_CHILD_KEEP(0, VALUESET_GUID, false, BOTH, MTEXT("Value Set"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, BOTH, MTEXT("Element Index"));
		ClearConnections(2);
		break;
	default:
		break;
	}
	RemoveMessage(UninitializedException());
}



