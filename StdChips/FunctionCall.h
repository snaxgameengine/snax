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
#include "ProxyChip.h"
#include "M3DCore/Path.h"

namespace m3d
{



static const Guid FUNCTIONCALL_GUID = { 0x6e91c75a, 0xb487, 0x41b1, { 0x90, 0x56, 0xbe, 0xbb, 0x06, 0x82, 0x8b, 0x47 } };


class STDCHIPS_API FunctionCall : public ProxyChip
{
	CHIPDESC_DECL;
	CHIPMSG(FunctionConnectionFailedException, FATAL, MTEXT("Could not connect to function!"))

public:
	FunctionCall();
	virtual ~FunctionCall();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	// Function call can't be a function!
	bool CanCreateFunction() const override { return false; }
	// GetChip() is the function call!
	virtual ChipChildPtr GetChip() override;
	// Overriding basic functionality because during loading we may have to remap the children becuause we're not connected to a function yet!
	virtual bool SetChild(Chip *child, uint32 index, uint32 subIndex) override;
	// Fast casting.
	virtual FunctionCall *AsFunctionCall() override { return this; }
	// Returns connected function.
	virtual Function *GetFunction() const { return _function; } // <===== NOTE: This has the same name as the non-virtual function in Chip. This can cause problems in not careful!
	// Returns the parameter at the given index, starting at 0. NOTE: This is the raw child connected to us (GetRawChild() is used)
	virtual Chip *GetParameter(uint32 index);
	// Name of functions class.
	virtual String GetClassName() const { return _clazzName; }
	// Name of function.
	virtual String GetFunctionName() const { return _funcName; }
	// Call by Name like this: SomeClass::functionName().
	virtual bool IsCallByName() const { return _callByName; }
	// Autoload means that the document containing the targeted function will be start loaded async when the function call is loaded.
	virtual bool IsPreload() const { return _preload; }
	// Set autoload.
	virtual void SetPreload(bool b) { _preload = b; }
	// Set the function based on clazzName and function name.
	virtual void SetFunction(String clazzName, String funcName, bool callByName, bool connect);
	// Connects to function. Returns true on success.
	virtual bool ConnectToFunction();
	// Disconnects from function if connected.
	virtual void RemoveFunction();
	// '_function' calls this function when it changes for some reason. The old parameter mapping is given, allowing us to remap our connected parameters!
	virtual void OnFunctionChange(const ParameterConnectionSet &oldParameters);
	// Called by the function's class when its name changes.
	virtual void OnClassRenamed(String clazzName);

	virtual void Validate();


	virtual bool SetChipType(const Guid &type) override;

	virtual void RestoreChip() override { ConnectToFunction(); }

protected:
	bool _preload;
	Path _filename;
	bool _callByName;
	String _clazzName;
	String _funcName;

	Function *_function;

	virtual void SetParameters(Chip *instance, const Map<uint32, Chip*> &parameterMapping);
	bool _isFunctionValid();

};



}