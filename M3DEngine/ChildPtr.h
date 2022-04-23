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

#include "FunctionStack.h"
#include "Function.h"

namespace m3d
{

class Parameter;
class ClassInstance;
class ShellChip;

template<typename T>
class ChildPtr
{
	template<typename S> friend class ChildPtr;
private:
	T* t;
	//	const Chip *childProvider; // <= 2/5/12: Highly experimental stuff for the idea with the Shell-Chip...
	uint32 stackptr;

public:
	ChildPtr() : t(nullptr)/*, childProvider(nullptr)*/, stackptr(-1) {}
	ChildPtr(const ChildPtr& s) : t(s.t)/*, childProvider(s.childProvider)*/, stackptr(functionStack.IncrementRecordRef(s.stackptr)) {}
	ChildPtr(T* t) : t(t)/*, childProvider(t ? t->GetChildProvider() : nullptr)*/, stackptr(functionStack.IncrementRecordRef()) {}
	ChildPtr(Function* f, FunctionCall* functionCall, ClassInstance* instance = nullptr) : t(f->GetChip())/*, childProvider(t ? t->GetChildProvider() : nullptr)*/, stackptr(functionStack.AddFunctionCallRecord(functionCall, f, instance)) {}
	ChildPtr(T* t, Parameter* p) : t(t)/*, childProvider(t ? t->GetChildProvider() : nullptr)*/, stackptr(functionStack.AddParameterCallRecord(p, t)) {}
	ChildPtr(T* t, ShellChip* shell) : t(t)/*, childProvider(shell)*/, stackptr(functionStack.IncrementRecordRef()) {}
	template<typename S>
	ChildPtr(const ChildPtr<S>& s) : t(dynamic_cast<T*>(s.t))/*, childProvider(t ? s.childProvider : nullptr)*/, stackptr(functionStack.IncrementRecordRef(s.stackptr)) {}
	~ChildPtr() { functionStack.DecrementRecordRef(stackptr); }

	ChildPtr& operator=(const ChildPtr& rhs)
	{
		functionStack.DecrementRecordRef(stackptr);
		t = rhs.t;
		//childProvider = rhs.childProvider;
		stackptr = functionStack.IncrementRecordRef(rhs.stackptr); // Changed 25/7/12 because I think the previous assignment was wrong: functionStack.IncrementRecordRef();
		return *this;
	}

	template<typename S>
	ChildPtr& operator=(const ChildPtr<S>& rhs)
	{
		functionStack.DecrementRecordRef(stackptr);
		t = dynamic_cast<T*>(rhs.t);
		//childProvider = t ? rhs.childProvider : nullptr;
		stackptr = functionStack.IncrementRecordRef(rhs.stackptr); // Changed 25/7/12 because I think the previous assignment was wrong: functionStack.IncrementRecordRef();
		return *this;
	}

	operator bool() const { return t != nullptr; }
	bool operator==(T* s) const { return t == s; }
	bool operator!=(T* s) const { return t != s; }

	template<typename S>
	bool operator==(const ChildPtr<S>& s) { return t == s.t; }
	template<typename S>
	bool operator!=(const ChildPtr<S>& s) { return t != s.t; }

	// For explanation about this technique, see [Stroustrup]: Wrapping C++ Member Function Calls
	class Call
	{
	public:
		T* const t;
		//const Chip *const oldChildProvider;
		const uint32 oldstackptr;

		Call(T* const t/*, const Chip *const childProvider*/, uint32 stackptr) : t(t)/*, oldChildProvider(t->ReplaceChildProvider(childProvider))*/, oldstackptr(functionStack.SetStackPtr(stackptr))
		{
			// Enter function call! Called right before ANY function call on child (eg 'someChild->CallChip()') is executed. 
			// Adjust the function stack frame.
		}

		~Call()
		{
			// Leave function call! Called right after a function call on child has returned!
			// Adjust the stack back!
			//t->SetChildProvider(oldChildProvider);
			//functionStack.SetStackPtr(oldstackptr);
			functionStack.ResetStackPtr(oldstackptr);
		}

		T* operator->() const { return t; }
	};


	// NOTE, IMPORTANT:
	// 1) You should NEVER do a function call (B) on an object returned by a function call (A) (using ->) on ChildPtr.
	//    Example:
	//    ChildPtr<SomeChip> ch0 = GetChild(0);
	//    ch0->SomeFunction()->SomeOtherFunction(); // DO NOT DO THIS!
	//    Do this instead:
	//    SomeObject *obj = ch0->SomeFunction();
	//    obj->SomeOtherFunction();
	// 2) Do NOT use the result from a function call (A) (using ->) on ChildPtr as a parameter to another function (B).
	//    Example:
	//    ChildPtr<SomeChip> ch0 = GetChild(0);
	//    SomeFunction(ch0->SomeOtherFunction()); // DO NOT DO THIS!
	//    Do this instead:
	//    int32 result = ch0->SomeOtherFunction();
	//    SomeFunction(result);
	// 3) Also, is you violate 2), absolutely do NOT do this for multiple parameters or if function (B) is also a function call on a ChildPtr.
	const Call operator->() const { return Call(t/*, childProvider*/, stackptr); }
};

typedef ChildPtr<Chip> ChipChildPtr;

template class M3DENGINE_API ChildPtr<Chip>;

}