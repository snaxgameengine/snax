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
#include "ChipDef.h"
#include "Engine.h"
#include "FunctionStack.h"
#include "FunctionStackRecord.h"

using namespace m3d;

Chip *m3d::ChipExceptionScope::currentChip = nullptr;

// BIG NOTE (14/1/15):
// ---------
// I've discovered a flaw with the RefreshManager! :(
// Imagen a recursive function getting a value from an Expression Value.
// The chip passes the refresh check, sets the _lastStack member, and starts to calculate.
// The chips need input causing the recursive function to enter the chip again.
// The refresh check is performed, _lastStack is updated, calculation is done and the value set.
// Now, when returning to the first use of the chip, calculation is finalizing and the value written.
// What just happend? The value corresponing to the last setting of _lastStack is overwritten! :'(
// That makes value set to the chip is no longer valid with regards to _lastStack!! Thats no good!!!
// I realized this examing the Fibonacci recursive function...
// SOLUTION: I've created the RefreshT struct to overcome this using RAII.
//           Not as beautiful maybe, but it works! And it is only neccessary where a function update a chip variable!

RefreshManager::operator bool()
{
	if (_rm == RefreshManager::RefreshMode::Always)
		return true;
//	if (_rm == NEVER)
//		return false;

	if (engine->GetFrameNr() != _lastFrame) {
		if (_rm == RefreshManager::RefreshMode::Once && _lastFrame != 0)
			return false;
		_lastFrame = engine->GetFrameNr();
		_lastStack = functionStack.GetCurrentRecord().recordnr;
		return true;
	}

	if (_rm == RefreshManager::RefreshMode::OncePerFrame)
		return false;

	if (_lastStack != functionStack.GetCurrentRecord().recordnr) {
		_lastStack = functionStack.GetCurrentRecord().recordnr;
		return true;
	}
	
	return false;
};

static UpdateStamp __us = 0;

m3d::UpdateStamp m3d::GenerateUpdateStamp() { return ++__us; }


