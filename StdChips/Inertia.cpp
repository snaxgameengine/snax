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
#include "Inertia.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Engine.h"

using namespace m3d;


CHIPDESCV1_DEF(Inertia, MTEXT("Inertia"), INERTIA_GUID, VALUE_GUID);


Inertia::Inertia()
{
	CREATE_CHILD(1, VALUE_GUID, false, UP, MTEXT("Damping Value"));
	_dir = Direction::Both;
	_isSet = false;
}

Inertia::~Inertia()
{
}

bool Inertia::CopyChip(Chip *chip)
{
	Inertia *c = dynamic_cast<Inertia*>(chip);
	B_RETURN(Value::CopyChip(c));
	_dir = c->_dir;
	_isSet = false;
	return true;
}

bool Inertia::LoadChip(DocumentLoader &loader) 
{ 
	B_RETURN(Value::LoadChip(loader));
	LOAD(MTEXT("direction|dir"), _dir);
	_isSet = false;
	return true;
}

bool Inertia::SaveChip(DocumentSaver &saver) const 
{
	B_RETURN(Value::SaveChip(saver));
	SAVE(MTEXT("direction"), _dir);
	return true;
}

value Inertia::GetValue()
{
	RefreshT refresh(Refresh);
	if (refresh) {
		ChildPtr<Value> ch0 = GetChild(0);

		if (!ch0)
			return _value;

		value newValue = ch0->GetValue();

		if (!_isSet) {
			_value = newValue;
			_isSet = true;
			return _value;
		}

		ChildPtr<Value> ch1 = GetChild(1);
		value damping = ch1 ? ch1->GetValue() : 0.5f;

		value d = (newValue - _value) * engine->GetDt() * 25.0 / 1000000.0;

		if(_value > newValue) {
			if(_value + d * damping < newValue) {
				_value = newValue;
				d = 0.0f;
			}
		} 
		else if(_value < newValue) {
			if(_value + d * damping > newValue) {
				_value = newValue;
				d = 0.0f;
			}
		}

		switch(_dir) 
		{
		case Direction::Both:
			_value += d * damping;	
			break;
		case Direction::Down: // only up
			if (newValue > _value) {
				_value = newValue;
			} else {
				_value += d * damping;	
			}
			break;
		case Direction::Up: // only down
			if (newValue < _value) {
				_value = newValue;
			} else {
				_value += d * damping;	
			}
			break;
		}
	}

	return _value;
}
