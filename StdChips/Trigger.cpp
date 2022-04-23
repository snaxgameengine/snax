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
#include "Trigger.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"

using namespace m3d;


CHIPDESCV1_DEF(Trigger, MTEXT("Trigger"), TRIGGER_GUID, VALUE_GUID);


Trigger::Trigger()
{
	CREATE_CHILD(0, VALUE_GUID, false, UP, MTEXT("Trigger Value"));
	CREATE_CHILD(1, CHIP_GUID, true, DOWN, MTEXT("Chips to Call"));
}

Trigger::~Trigger()
{
}

bool Trigger::CopyChip(Chip *chip)
{
	Trigger *c = dynamic_cast<Trigger*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_at = c->_at;
	_type = c->_type;
	_triggerAtFirstCall = c->_triggerAtFirstCall;
	return true;
}

bool Trigger::LoadChip(DocumentLoader &loader) 
{ 
	B_RETURN(Chip::LoadChip(loader));
	LOAD(MTEXT("type"), _type);
	LOAD(MTEXT("at"), _at);
	LOADDEF(MTEXT("triggerAtFirstCall"), _triggerAtFirstCall, false);
	return true;
}

bool Trigger::SaveChip(DocumentSaver &saver) const 
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE(MTEXT("type"), _type);
	SAVE(MTEXT("at"), _at);
	SAVEDEF(MTEXT("triggerAtFirstCall"), _triggerAtFirstCall, false);
	return true;
}

value Trigger::GetValue()
{
	_value = 0.0f;

	RefreshT refresh(Refresh);
	if (refresh) {
		ChildPtr<Value> ch0 = GetChild(0);
		if (!ch0)
			return _value;

		value f = ch0->GetValue();

		if (_checkTrigger(f))
			_trigger();
	}

	return _value;
}

void Trigger::SetType(Type type)
{
	if (_type == type)
		return;
	_type = type;
	_firstCall = false;
	_lastTriggerTime = 0;
}

bool Trigger::_checkTrigger(value v)
{
	bool trigger = false;

	if (_firstCall) {
		switch (_type)
		{
		case Type::VALUE_CHANGE:
			trigger = v != _lastTriggerValue;
			break;
		case Type::VALUE_INC:
			trigger = v > _lastTriggerValue;
			break;
		case Type::VALUE_DEC:
			trigger = v < _lastTriggerValue;
			break;
		case Type::AT_VALUE_BOTH:
			trigger = _lastTriggerValue < _at && v >= _at || _lastTriggerValue > _at && v <= _at;
			break;
		case Type::AT_VALUE_INC:
			trigger = _lastTriggerValue < _at && v >= _at;
			break;
		case Type::AT_VALUE_DEC:
			trigger = _lastTriggerValue > _at && v <= _at;
			break;
		case Type::DELAY_MS:
			trigger = (engine->GetAppTime() - int64(v * 1000)) >= _lastTriggerTime;
			break;
		default:
			trigger = false;
		}
	}
	else if (_triggerAtFirstCall) {
		trigger = true;
		switch (_type)
		{
		case Type::AT_VALUE_INC:
			if (v < _at)
				trigger = false;
			break;
		case Type::AT_VALUE_DEC:
			if (v > _at)
				trigger = false;
			break;
		default:;
		}
	}

	_lastTriggerValue = v;
	_firstCall = true;

	return trigger;
}

void Trigger::_trigger(uint32 conn)
{
	_lastTriggerTime = engine->GetAppTime();
	_value = 1.0f;
	for (uint32 i = 0, j = GetSubConnectionCount(conn); i < j; i++) {
		ChipChildPtr ch1 = GetChild(conn, i);
		if (ch1)
			ch1->CallChip();
	}
}