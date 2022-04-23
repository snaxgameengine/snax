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
#include "BreakPoint.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/Application.h"
#include "Value.h"
#include "M3DEngine/ClassInstance.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/FunctionStackRecord.h"

using namespace m3d;

CHIPDESCV1_DEF(BreakPoint, MTEXT("Break Point"), BREAKPOINT_GUID, CHIP_GUID);



BreakPoint::BreakPoint()
{
	_enabled = true;
	_hitCountCondition = HitCountCondition::HCC_DISABLED;
	_hitCount = 1;
	_currentHitCount = 0;
	_resetHitCountPrFrame = false;
	_lastFrameNbr = 0;
	_childValueCondition = ChildValueCondition::CVC_DISABLED;
	_childValue = 0.0f;
	_lastChildValueValid = false;
	_instanceID = InvalidClassInstanceID;

	CREATE_CHILD(0, VALUE_GUID, false, UP, MTEXT("Condition"));
}

BreakPoint::~BreakPoint()
{
}

bool BreakPoint::CopyChip(Chip *chip)
{
	BreakPoint *c = dynamic_cast<BreakPoint*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_enabled = c->_enabled;
	_hitCountCondition = c->_hitCountCondition;
	_hitCount = c->_hitCount;
	_currentHitCount = 0;
	_resetHitCountPrFrame = c->_resetHitCountPrFrame;
	_lastFrameNbr = 0;
	_childValueCondition = c->_childValueCondition;
	_childValue = c->_childValue;
	_lastChildValueValid = false;
	_instanceID = c->_instanceID;
	return true;
}

bool BreakPoint::LoadChip(DocumentLoader &loader) 
{ 
	B_RETURN(Chip::LoadChip(loader));
	LOADDEF(MTEXT("enabled|Enabled"), _enabled, true);
	LOADDEF(MTEXT("hitCountCondition|HitCountCondition"), _hitCountCondition, HitCountCondition::HCC_DISABLED);
	LOADDEF(MTEXT("hitCount|HitCount"), _hitCount, 0);
	LOADDEF(MTEXT("resetHitCountPrFrame|ResetHitCountPrFrame"), _resetHitCountPrFrame, false);
	LOADDEF(MTEXT("childValueCondition|ChildValueCondition"), _childValueCondition, ChildValueCondition::CVC_DISABLED);
	LOADDEF(MTEXT("childValue|ChildValue"), _childValue, 0.0f);
	LOADDEF(MTEXT("instanceID|InstanceID"), _instanceID, InvalidClassInstanceID);
	return true;
}

bool BreakPoint::SaveChip(DocumentSaver &saver) const 
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE(MTEXT("enabled"), _enabled);
	SAVE(MTEXT("hitCountCondition"), _hitCountCondition);
	SAVE(MTEXT("hitCount"), _hitCount);
	SAVE(MTEXT("resetHitCountPrFrame"), _resetHitCountPrFrame);
	SAVE(MTEXT("childValueCondition"), _childValueCondition);
	SAVE(MTEXT("childValue"), _childValue);
	SAVE(MTEXT("instanceID"), _instanceID);
	return true;
}

void BreakPoint::CallChip()
{
	if (!engine->GetApplication()->IsBreakPointsEnabled() || !_enabled)
		return; // Disabled
	
	if (_childValueCondition != ChildValueCondition::CVC_DISABLED) {
		ChildPtr<Value> ch0 = GetChild(0);
		if (!ch0)
			return;

		value v = ch0->GetValue();
		bool passed = false;

		switch (_childValueCondition)
		{
		case ChildValueCondition::CVC_EQUAL: passed = v == _childValue; break;
		case ChildValueCondition::CVC_NOTEQUAL: passed = v != _childValue; break;
		case ChildValueCondition::CVC_LESS: passed = v < _childValue; break;
		case ChildValueCondition::CVC_LESSEQUAL: passed = v <= _childValue; break;
		case ChildValueCondition::CVC_GREATER: passed = v > _childValue; break;
		case ChildValueCondition::CVC_GREATEREQUAL: passed = v >= _childValue; break;
		case ChildValueCondition::CVC_CHANGED: 
			passed = _lastChildValueValid && v != _childValue;
			_childValue = v;
			_lastChildValueValid = true;
		default:;
		}

		if (!passed)
			return;
	}

	if (_instanceID != InvalidClassInstanceID) {
		ClassInstance *instance = functionStack.GetCurrentRecord().instance;
		if (!instance || instance->GetRuntimeID() != _instanceID)
			return;
	}

	if (_resetHitCountPrFrame && _lastFrameNbr != engine->GetFrameNr())
		_currentHitCount = 0;

	_currentHitCount++;
	_lastFrameNbr = engine->GetFrameNr();
	
	if (_hitCountCondition != HitCountCondition::HCC_DISABLED) {

		bool passed = false;

		switch (_hitCountCondition)
		{
		case HitCountCondition::HCC_EQUAL: passed = _hitCount == _currentHitCount; break;
		case HitCountCondition::HCC_GREATEREQUAL: passed = _hitCount >= _currentHitCount; break;
		case HitCountCondition::HCC_POWEROF: passed = (_currentHitCount % _hitCount) == 0; break;
		default:;
		}

		if (!passed)
			return;
	}

	engine->Break(this);
}

String BreakPoint::GetValueAsString() const
{
	if (_enabled) {
		if (engine->GetApplication()->IsBreakPointsEnabled()) {
			if (_hitCountCondition != HitCountCondition::HCC_DISABLED || _childValueCondition != ChildValueCondition::CVC_DISABLED || _instanceID != InvalidClassInstanceID)
				return MTEXT("ENABLED (Conditions)");
			return MTEXT("ENABLED");
		}
		return MTEXT("Globally Disabled");
	}
	return MTEXT("Disabled");
}
