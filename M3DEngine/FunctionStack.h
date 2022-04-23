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
#include "GlobalDef.h"
#include "ChipDef.h"


// MAx size of the function call stack
#define FUNCTION_STACK_SIZE 1024 


namespace m3d
{

class Chip;
class FunctionCall;
class Function;

template<typename T>
class ChildPtr;

typedef ChildPtr<Chip> ChipChildPtr;


// This is for debugging.
struct FunctionStackTraceRecord
{
	Chip *caller; // Function call or parameter chip. nullptr for first record only.
	Chip *called; // Function or connected parameter. Can also be project start chip for first record. Not nullptr!
	ClassInstance *instance; // Current instance. Can be nullptr if static function or first record.

	FunctionStackTraceRecord(Chip *caller = nullptr, Chip *called = nullptr, ClassInstance *instance = nullptr) : caller(caller), called(called), instance(instance) {}
};

typedef List<FunctionStackTraceRecord> FunctionStackTrace;

struct FunctionStackRecord;

class M3DENGINE_API FunctionStack
{
public:
	enum class PerfMon { PERF_NONE, PERF_FRAME, PERF_ACCUM };
private:
	// Counter incremented for each new record added to stack. Each record will get its unique id from this.
	uint32 _recordnrs;
	// Index into _functionStack for current record.
	uint32 _stackptr;
	// Index into top of the stack.
	uint32 _stackEnd;
	// The maximum reference count within a record. To prevent infinite loops!
	uint32 _refLimit;
	// The function stack.
	FunctionStackRecord *_functionStack; // I'm using a plain c-array for maximum performance!

	// For performance monitoring!
	PerfMon _perfMon;
	uint32 _perfFrame;
	uint32 _perfFrameCount;
	int64 _perfTime;
	int64 _qFreq;
	uint32 _perfCPPHitCount;

	void _popStack();

public:
	FunctionStack();
	~FunctionStack();

	// Check if stack overflow. If it returns false, no function call or parameter call can be made!
	inline bool CanAddRecord() const { return _stackEnd < FUNCTION_STACK_SIZE - 1; }
	// Adds a new function-call record on top of the stack.
	// Returns it's stackptr. The function does not change the current stackptr.
	uint32 AddFunctionCallRecord(FunctionCall *functionCall, Function *function, ClassInstance *instance);
	// Check for infinit loops within a function. 
	// NOTE: When _stackptr is 0, refcount will be -1 if outside a frame. This will effectively stop calls to GetChild(...) from dialog pages in the editor.
	bool CanIncrementRecordRef() const;
	// Set ref count limit.
	void SetReferenceCountLimit(uint32 l) { _refLimit = l; }
	// Get the ref count limit.
	uint32 GetReferenceCountLimit() const { return _refLimit; }
	// Are we between calls to StartOfFrame() and EndOfFrame()?
	bool IsRunning() const { return _stackEnd != -1; }
	// Increments the refcount of the record at the given stackptr.
	// Returns the given stackptr.
	uint32 IncrementRecordRef(uint32 stackptr);
	inline uint32 IncrementRecordRef() { return IncrementRecordRef(_stackptr); }
	// Adds a new parameter-call record on top of the stack.
	// Returns it's stackptr. The function does not change the current stackptr.
	uint32 AddParameterCallRecord(Parameter *p, Chip *lp);
	// Decrements the refcount on stackptr. If it's on top of the stack, and the counter
	// is 0 then the stack is poped recursivly as int32 as the top refcount is 0.
	void DecrementRecordRef(uint32 stackptr);
	// Sets the current stackptr. Returns the old stackptr. Called from ChildPtr::Call::Call(), this is, before a method call (eg CallChip()).
	uint32 SetStackPtr(uint32 stackptr);
	// Sets the current stackptr. Called from ChildPtr::Call::~Call(), that is, after a method call (eg CallChip()).
	void ResetStackPtr(uint32 stackptr);
	// Returns the current stack frame. NOTE: It always returns a function-call record.
	const FunctionStackRecord& GetCurrentRecord() const;
	// Gets data from current record.
	Chip *GetData(uint32 functionDataID);
	// Sets data on current record. Assumes GetData() is called and cid does not already exist (avoids these checks for performance)!
	void SetData(uint32 functionDataID, Chip *chip);
	// Adds a parameter to the parameter map of current record. (Parameter caching!)
	void SetParameter(uint32 index, const ChipChildPtr &p);
	// Called at the start of each frame
	void StartOfFrame();
	// Called at end of frame to validate stack and prepare new frame! Return false if stack corruption!
	bool EndOfFrame();
	// Dumps current stack trace. For debugging!
	void DumpStackTrace(FunctionStackTrace &trace);

	// Returns type of performance monitoring enabled
	inline PerfMon GetPrefMon() const { return _perfMon; }
	// Sets if performance monitoring should be enabled!
	inline void SetPerfMon(PerfMon perfMon) { _perfMon = perfMon; }
	// Only for PERF_ACCUM: On next frame, counters are reset. Should NOT be called between StartOfFrame() and EndOfFrame()!
	void ResetPerfFrame();
	// For PERF_FRAME: Incremented every frame. 
	// For PERF_ACCUM: Only incremented at ResetPerfAccum().
	inline uint32 GetPerfFrame() const { return _perfFrame; }
	// Returns the number of frame values are accumulated from. Should be 1 for PERF_FRAME!
	inline uint32 GetPerfFrameCount() const { return _perfFrameCount; }
	// if PERF_FRAME: Returns the time the last frame took (time between call to StartOfFrame() and EndOfFrame())
	// If PERF_ACCUM: Same as above, but values are not reset every frame 
	inline int64 GetPerfTime() const { return _perfTime; }
	// The number of ChildPtr-> done last frame.
	inline uint32 GetPerfCPPCount() const { return _perfCPPHitCount; }
	// Returns the system query freq. To get time in seconds use: (stop-end)/GetQFreq()
	inline int64 GetQFreq() const { return _qFreq; }

};

extern FunctionStack M3DENGINE_API functionStack;

}