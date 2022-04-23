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
#include "FunctionStack.h"
#include "Chip.h" // For Chip->Release() only!
#include "Engine.h" // for msg only!
#include "Function.h" // for performance monitoring!
#include "ClassInstance.h" // For SetDelayDestruction()
#include "FunctionStackRecord.h"

using namespace m3d;



FunctionStack m3d::functionStack = FunctionStack();


FunctionStack::FunctionStack()
{
	_recordnrs = 0;
	_stackptr = 0;
	_stackEnd = -1;
	_refLimit = 1000;

	// For performance monitoring!
	_perfMon = PerfMon::PERF_NONE;
	_perfTime = 0;
	_perfCPPHitCount = 0;
	_perfFrame = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&_qFreq);

	_functionStack = new FunctionStackRecord[FUNCTION_STACK_SIZE];

	_functionStack[0].refCount = -1;
}

FunctionStack::~FunctionStack()
{
	delete[] _functionStack;
}

// On function call.
uint32 FunctionStack::AddFunctionCallRecord(FunctionCall *functionCall, Function *function, ClassInstance *instance)
{
	assert(CanAddRecord()); // Stack overflow?

	FunctionStackRecord &r = _functionStack[++_stackEnd]; // Push a new record on the stack.
	r.refCount = 1;
	r.recordnr = ++_recordnrs;
	r.prevRecord = _stackptr;
	r.original = _stackEnd; // Note: Always original>prevRecord!
	r.f.functionCall = functionCall;
	r.f.function = function;
	r.instance = instance;
	r.accum = 0;
	r.subAccum = 0;
	r.ccpHitCount = 0;
	if (instance)
		instance->SetDelayDestruction(true); // We have to avoid the instance being Released while on the stack!

	assert(r.prevRecord<r.original);

	return _stackEnd; // Return index to the new record.
}

bool FunctionStack::CanIncrementRecordRef() const
{
	return _functionStack[_stackptr].refCount < _refLimit; 
}

// On ChildPtr copy/assignment.
uint32 FunctionStack::IncrementRecordRef(uint32 stackptr)
{
	if (stackptr == -1)
		return -1; // This happens for a null ChildPtr.
	_functionStack[stackptr].refCount++;
	return stackptr;
}

// On parameter call.
uint32 FunctionStack::AddParameterCallRecord(Parameter *p, Chip *lp)
{
	assert(CanAddRecord()); // Stack overflow?
	assert(_stackptr != 0);

	FunctionStackRecord &r = _functionStack[++_stackEnd]; // Push a new record on the stack.
	r.refCount = 1;
	r.recordnr = ++_recordnrs;
	r.prevRecord = _stackptr;
	r.original = _functionStack[_functionStack[_functionStack[_stackptr].original].prevRecord].original; // Current records original (because current record can be a parameter call), then the previous records orginal (because it can be a parameter call).
	r.p.parameter = p;
	r.p.lparameter = lp;
	r.accum = 0;
	r.subAccum = 0;
	r.ccpHitCount = 0;

	assert(r.prevRecord>r.original);

	return _stackEnd;
}

void FunctionStack::_popStack()
{
	FunctionStackRecord &fsr = _functionStack[_stackEnd];
	while (fsr.activeDataCount) {
		Chip *&c = fsr.data[fsr.activeData[--fsr.activeDataCount]];
		assert(c != nullptr);
		c->Release();
		c = nullptr;
	}

	if (fsr.instance)
		fsr.instance->SetDelayDestruction(false);
	fsr.instance = nullptr;

	_stackEnd--; // Pop stack!
}

// Called on destruction of a ChildPtr.
void FunctionStack::DecrementRecordRef(uint32 stackptr)
{
	if (stackptr == -1)
		return; // This happens for a null ChildPtr.
	assert(stackptr <= _stackEnd); // stackptr not out of bounds?
	assert(_functionStack[stackptr].refCount > 0); // refCount valid?
	FunctionStackRecord &fsr = _functionStack[stackptr];
	if (--fsr.refCount == 0) { // No more references to this record?

		fsr.refCount++; // Added 1/2/13: We need to increment temprorarly to avoid us being poped from stack. The parameter stack casued problems without this.

		while (fsr.parameterCount > 0)
			fsr.parameters[--fsr.parameterCount] = ChipChildPtr(); // We have no more references to us so we can clear our parameters freeing references to other records.

		fsr.refCount--; // Added 1/2/13

		assert(_stackEnd >= stackptr);

		// For performance monitoring!
		// We are now destroying a ChildPtr we got from a FunctionCall. We therefore add the time accumulated in this record to the FunctionCall.
		if (_perfMon != PerfMon::PERF_NONE) {
			_functionStack[fsr.prevRecord].subAccum += fsr.accum;
			if (fsr.original > fsr.prevRecord) // Function call?
				fsr.f.function->AddCallTime(fsr.accum, fsr.accum - fsr.subAccum, fsr.ccpHitCount);	
			_perfCPPHitCount += fsr.ccpHitCount;
		}

		//_functionStack[_functionStack[stackptr].prevRecord].refCount--;
		while (_functionStack[_stackEnd].refCount == 0) { // No more references to top of stack?
			_popStack();
		}
	}
}

uint32 FunctionStack::SetStackPtr(uint32 stackptr)
{
	assert(stackptr <= _stackEnd); // stackptr not out of bounds?
	assert(_functionStack[stackptr].refCount > 0); // refCount valid?

	// For performance monitoring!
	if (_perfMon != PerfMon::PERF_NONE) {
		if (stackptr != _stackptr) {
			// We are now doing a c++ function call on a chip we got from a FunctionCall-chip as a ChildPtr<>.
			// This means we should start monitoring how long that FunctionCall is taking us.			
			QueryPerformanceCounter((LARGE_INTEGER*)&_functionStack[stackptr].start);
		}
		_functionStack[stackptr].ccpHitCount++;
	}

	std::swap(_stackptr, stackptr); // Set stackptr to be current record.

	return stackptr; // return index of old record.
}

void FunctionStack::ResetStackPtr(uint32 stackptr)
{
	assert(stackptr <= _stackEnd); // stackptr not out of bounds?
	assert(_functionStack[stackptr].refCount > 0); // refCount valid?

	if (_perfMon != PerfMon::PERF_NONE) {
		if (stackptr != _stackptr) {
			// We are now leaving the c++ function call we did on a chip we got from a FunctionCall-chip.
			// We now accumulate the time we spent on the call to the FunctionCalls stack-record.
			int64 stop;
			QueryPerformanceCounter((LARGE_INTEGER*)&stop);
			_functionStack[_stackptr].accum += stop - _functionStack[_stackptr].start;
		}
	}

	_stackptr = stackptr;
}

const FunctionStackRecord& FunctionStack::GetCurrentRecord() const 
{ 
	return _functionStack[_functionStack[_stackptr].original]; 
}

Chip *FunctionStack::GetData(uint32 functionDataID)
{
	assert(functionDataID < MAX_FUNCTION_DATA);
	FunctionStackRecord &fsr = _functionStack[_functionStack[_stackptr].original];
	return fsr.data[functionDataID]; // Returns nullptr if no data. FunctionData will then call SetData(..)
}

void FunctionStack::SetData(uint32 functionDataID, Chip *chip)
{
	assert(functionDataID < MAX_FUNCTION_DATA);
	FunctionStackRecord &fsr = _functionStack[_functionStack[_stackptr].original];
	assert(fsr.data[functionDataID] == nullptr);
	assert(fsr.activeDataCount < MAX_ACTIVE_FUNCTION_DATA);
	fsr.data[functionDataID] = chip;
	fsr.activeData[fsr.activeDataCount++] = functionDataID;
}

void FunctionStack::SetParameter(uint32 index, const ChipChildPtr &p)
{
	assert(_stackptr > 0);
	assert(index < MAX_PARAMETERS);

	FunctionStackRecord &fsr = _functionStack[_functionStack[_stackptr].original]; // We need to look up original record.
	assert(fsr.original > fsr.prevRecord);
	fsr.parameterCount = std::max(index + 1, fsr.parameterCount);
	fsr.parameters[index] = p;
}

void FunctionStack::StartOfFrame()
{
	assert(_stackEnd == -1);
	assert(_functionStack[0].refCount == -1);

	_stackEnd = 0;
	_functionStack[0].refCount = 1;
	_functionStack[0].recordnr = ++_recordnrs;

	QueryPerformanceCounter((LARGE_INTEGER*)&_functionStack[0].start);
	_functionStack[0].accum = 0;
	_functionStack[0].subAccum = 0;
	_functionStack[0].ccpHitCount = 0;
	if (_perfMon != PerfMon::PERF_ACCUM) {
		ResetPerfFrame();
	}
	_perfFrameCount++;
}

bool FunctionStack::EndOfFrame()
{
	if (_stackEnd != 0) {
		msg(FATAL, MTEXT("Function stack corruption detected. Size is ") + strUtils::fromNum(_stackEnd + 1) + MTEXT(" instead of 1."));
		return false;
	}
	if (_functionStack[0].refCount != 1) {
		msg(FATAL, MTEXT("Function stack corruption detected. Final reference count is ") + strUtils::fromNum(_functionStack[0].refCount) + MTEXT(" instead of 1."));
		return false;
	}
	if (_stackptr != 0) {
		msg(FATAL, MTEXT("Function stack corruption detected. Stack pointer is ") + strUtils::fromNum(_stackptr) + MTEXT(" instead of 0."));
		return false;
	}

	_popStack();

	if (_perfMon != PerfMon::PERF_NONE) {
		int64 stop;
		QueryPerformanceCounter((LARGE_INTEGER*)&stop);
		_perfTime += stop - _functionStack[0].start;
		_perfCPPHitCount += _functionStack[0].ccpHitCount;
	}

	_functionStack[0].refCount = -1;

	return true;
}


void FunctionStack::DumpStackTrace(FunctionStackTrace &trace)
{
	trace.clear();
	if (_stackEnd == -1)
		return;
	
	for (uint32 i = _stackptr; true;) {
		const FunctionStackRecord &fsr = _functionStack[i];
		if (fsr.original > fsr.prevRecord) { // Function call
			trace.push_back(FunctionStackTraceRecord((Chip*)fsr.f.functionCall, fsr.f.function->GetChip(), fsr.instance));
		}
		else if (fsr.original < fsr.prevRecord) { // Parameter call
			trace.push_back(FunctionStackTraceRecord((Chip*)fsr.p.parameter, fsr.p.lparameter, _functionStack[fsr.original].instance));
		}
		else {
			trace.push_back(FunctionStackTraceRecord(nullptr, nullptr, nullptr));
			break;
		}

		i = fsr.prevRecord;
	}
}

void FunctionStack::ResetPerfFrame() 
{ 
	_perfFrameCount = 0; 
	_perfFrame++; 
	_perfTime = 0; 
	_perfCPPHitCount = 0; 
}