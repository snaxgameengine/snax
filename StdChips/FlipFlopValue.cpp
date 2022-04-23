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
#include "FlipFlopValue.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"


using namespace m3d;


CHIPDESCV1_DEF(FlipFlopValue, MTEXT("Flip-Flop Value"), FLIPFLOPVALUE_GUID, VALUE_GUID);


FlipFlopValue::FlipFlopValue()
{
	_ot = OperatorType::NONE;
	_old = false;
	_valid = false;
}

FlipFlopValue::~FlipFlopValue()
{
}

bool FlipFlopValue::CopyChip(Chip *chip)
{
	FlipFlopValue *c = dynamic_cast<FlipFlopValue*>(chip);
	B_RETURN(Value::CopyChip(c));
	SetOperatorType(c->_ot);
	return true;
}

bool FlipFlopValue::LoadChip(DocumentLoader &loader) 
{ 
	B_RETURN(Value::LoadChip(loader));
	OperatorType ot;
	LOAD(MTEXT("operatorType|ot"), ot);
	SetOperatorType(ot);
	return true;
}

bool FlipFlopValue::SaveChip(DocumentSaver &saver) const 
{
	B_RETURN(Value::SaveChip(saver));
	SAVE(MTEXT("operatorType"), _ot);
	return true;
}

value FlipFlopValue::GetValue()
{
	RefreshT refresh(Refresh);
	if (refresh) {
		ChildPtr<Value> ch0 = GetChild(0);
		if (!ch0)
			return _value;

		bool b = ch0->GetValueAsBool();

		bool me = VALUE_TO_BOOL(_value);

		switch (_ot) 
		{
		case OperatorType::LOW:
			me = b ? me : !me;
			break;
		case OperatorType::HIGH:
			me = b ? !me : me;
			break;
		case OperatorType::INC:
			me = _valid && !_old && b ? !me : me;
			break;
		case OperatorType::DEC:
			me = _valid && _old && !b ? !me : me;
			break;
		default:
			AddMessage(UninitializedException());
			break;
		}

		_old = b;
		_valid = true;
		_value = me ? 1.0 : 0.0;
	}
	return _value;
}

void FlipFlopValue::SetOperatorType(OperatorType ot)
{ 
	if (_ot == ot)
		return;
	_ot = ot; 
	RemoveMessage(UninitializedException());
}
