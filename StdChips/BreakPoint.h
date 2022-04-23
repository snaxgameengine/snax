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

#include "Exports.h"
#include "M3DEngine/Chip.h"
#include "ValueDef.h"

namespace m3d
{


static const Guid BREAKPOINT_GUID = { 0x11f371e2, 0x53a8, 0x43be, { 0x9e, 0xb3, 0xbb, 0xe5, 0x17, 0x4e, 0x79, 0x32 } };

class STDCHIPS_API BreakPoint : public Chip
{
	CHIPDESC_DECL;
public:
	BreakPoint();
	virtual ~BreakPoint();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual bool CanCreateFunction() const override { return false; }

	virtual void CallChip() override;

	virtual String GetValueAsString() const override;

	enum class HitCountCondition {HCC_DISABLED, HCC_EQUAL, HCC_GREATEREQUAL, HCC_POWEROF };
	enum class ChildValueCondition { CVC_DISABLED, CVC_EQUAL, CVC_NOTEQUAL, CVC_LESS, CVC_LESSEQUAL, CVC_GREATER, CVC_GREATEREQUAL, CVC_CHANGED };

	virtual bool IsEnabled() const { return _enabled; }
	virtual void SetEnabled(bool b) { _enabled = b; }
	virtual HitCountCondition GetHitCountCondition() const { return _hitCountCondition; }
	virtual void SetHitCountCondition(HitCountCondition hcc) { _hitCountCondition = hcc; }
	virtual uint32 GetHitCount() const { return _hitCount; }
	virtual void SetHitCount(uint32 h) { _hitCount = h; }
	virtual uint32 GetCurrentHitCount() const { return _currentHitCount; }
	virtual void SetCurrentHitCount(uint32 h) { _currentHitCount = h; }
	virtual bool IsResetHitCountPrFrame() const { return _resetHitCountPrFrame; }
	virtual void SetResetHitCountPrFrame(bool b) { _resetHitCountPrFrame = b; }
	virtual ChildValueCondition GetChildValueCondition() const { return _childValueCondition; }
	virtual void SetChildValueCondition(ChildValueCondition cvc) { _childValueCondition = cvc; }
	virtual value GetChildValue() const { return _childValue; }
	virtual void SetChildValue(value v) { _childValue = v; }
	virtual ClassInstanceID GetClassInstanceID() const { return _instanceID; }
	virtual void SetClassInstanceID(ClassInstanceID cid) { _instanceID = cid; }

protected:
	bool _enabled;
	HitCountCondition _hitCountCondition;
	uint32 _hitCount;
	uint32 _currentHitCount;
	bool _resetHitCountPrFrame;
	uint32 _lastFrameNbr;
	ChildValueCondition _childValueCondition;
	value _childValue;
	bool _lastChildValueValid;
	ClassInstanceID _instanceID;

};

}