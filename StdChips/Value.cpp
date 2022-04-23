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
#include "Value.h"
#include "M3DEngine/DocumentSaver.h"
#include "M3DEngine/DocumentLoader.h"

using namespace m3d;


CHIPDESCV1_DEF(Value, MTEXT("Value"), VALUE_GUID, NUMERIC_GUID);


Value::Value() : _value(0.0f)
{
	CREATE_CHILD(0, VALUE_GUID, false, UP, MTEXT("Value"));
}

Value::~Value()
{
}

bool Value::CopyChip(Chip *chip)
{
	Value *c = dynamic_cast<Value*>(chip);
	B_RETURN(Numeric::CopyChip(c));
	_value = c->_value;
	return true;
}

bool Value::LoadChip(DocumentLoader &loader) 
{ 
	B_RETURN(Numeric::LoadChip(loader));
	LOADDEF(MTEXT("value"), _value, 0);
	return true;
}

bool Value::SaveChip(DocumentSaver &saver) const 
{
	B_RETURN(Numeric::SaveChip(saver));
	SAVEDEF(MTEXT("value"), _value, 0);
	return true;
}

void Value::CallChip()
{
	GetValue();
}

value Value::GetValue()
{
	RefreshT refresh(Refresh);
	if (refresh) {
		ChildPtr<Value> ch = GetChild(0);
		if (ch)
			_value = ch->GetValue();
	}
	return _value;
}

void Value::SetValue(value v) 
{ 
	_value = v; 
}

bool Value::GetValueAsBool()
{
	value d = GetValue();
	return VALUE_TO_BOOL(d);
}

String Value::GetValueAsString() const
{
	return strUtils::fromNum(_value, MTEXT("%g"));
}

