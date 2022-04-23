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
#include "ChildPtr.h"

// Max number of cached parameters
#define MAX_PARAMETERS 16 
// Max number of function data allowed in a class
#define MAX_FUNCTION_DATA 128
// Max number of function data currently in used by a function
#define MAX_ACTIVE_FUNCTION_DATA 64


namespace m3d
{


struct M3DENGINE_API FunctionStackRecord
{
	// Number of references to this record.
	uint32 refCount = 0;
	// Unique id for this record.
	uint32 recordnr = 0;
	// The index of the record in _functionStack that we were called from.
	uint32 prevRecord = 0;
	// For a parameter call (original<prevRecord), this is the original recordIndex.
	uint32 original = 0;

	union
	{
		// For function call (original>prevRecord)
		struct
		{
			// The function call that caused this stack record. Not null for all but the first record.
			FunctionCall* functionCall;
			// Function called. Not null for all but the first record.
			Function* function;
		} f = { nullptr, nullptr };
		// For parameter call (original<prevRecord)
		struct
		{
			// The parameter that caused this stack record. Never null.
			Parameter* parameter;
			// The chip linked to a function call as parameter. Never null.
			Chip* lparameter;
		} p;
	};

	// The rest of this is only applicapable for a function call record.
	// A parameter call record uses the 'original' index to look up the record with these data.

	// The instance. May be nullptr if static function.
	ClassInstance* instance = nullptr;

	// Function data. Do not access directly.
	Chip* data[MAX_FUNCTION_DATA] = {};
	// indices into data currently in use.
	uint32 activeData[MAX_ACTIVE_FUNCTION_DATA] = {};
	// Number of active datas.
	uint32 activeDataCount = 0;

	// Parameter cache. The index is the Parameter index.
	ChipChildPtr parameters[MAX_PARAMETERS] = {};
	// Number of parameters.
	uint32 parameterCount = 0;

	// For performance monitoring!
	int64 start = 0;
	int64 accum = 0;
	int64 subAccum = 0;
	uint32 ccpHitCount = 0;
};

}
