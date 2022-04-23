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


#include "Value.h"


namespace m3d
{

static const Guid TRIGGER_GUID = { 0x9cbfe861, 0x4a4c, 0x4f63, { 0x95, 0x82, 0x6c, 0xa8, 0x39, 0x89, 0x44, 0xf9 } };

class STDCHIPS_API Trigger : public Value
{
	CHIPDESC_DECL;
public:
	enum class Type { VALUE_CHANGE, VALUE_INC, VALUE_DEC, AT_VALUE_BOTH, AT_VALUE_INC, AT_VALUE_DEC, DELAY_MS };

	Trigger();
	virtual ~Trigger();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual value GetValue() override;
	virtual void SetValue(value value) override {}

	virtual Type GetType() const { return _type; }
	virtual void SetType(Type type);
	virtual value GetTriggerAtValue() const { return _at; }
	virtual void SetTriggerAtValue(value at) { _at = at; }
	virtual bool IsTriggerAtFirstCall() const { return _triggerAtFirstCall; }
	virtual void SetIsTriggerAtFirstCall(bool triggetAtFirstCall) { _triggerAtFirstCall = triggetAtFirstCall; }

protected:
	Type _type = Type::AT_VALUE_INC;
	value _at = 1.0f;
	bool _triggerAtFirstCall = false;
	value _lastTriggerValue = 0.0f;
	int64 _lastTriggerTime = 0;
	bool _firstCall = false;

	bool _checkTrigger(value v);
	void _trigger(uint32 conn = 1);
};



}