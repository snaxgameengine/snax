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
#include "Function.h"
#include "FunctionSignatures.h"
#include "Engine.h"
#include "Class.h"
#include "StdChips/Parameter.h"
#include "StdChips/FunctionCall.h"



using namespace m3d;


Function::Function(Chip *chip) : _chip(chip), _vFunction(0), _type(Type::Static), _access(Access::Public), _signature(InvalidFunctionSignatureID), _callTime(0), _callTimeExclSubFunc(0), _lastFrame(0), _hitCount(0), _cppHitCount(0)
{
	_chip->GetClass()->OnFunctionCreate(this);
}

Function::~Function()
{
	_chip->GetClass()->OnFunctionRemove(this);
	while (_functionCalls.size())
		(*_functionCalls.begin())->RemoveFunction(); // Calls our UnregisterFunctionCall()
}

uint32 Function::GetParameterIndex(Parameter *param)
{
	auto itr = _parameters.find(param);
	return itr != _parameters.end() ? itr->c : -1; // Can return -1 if not found..
}

Parameter *Function::GetParameterFromIndex(uint32 index)
{
	for (const auto &n : _parameters)
		if (n.c == index)
			return n.p;
	return nullptr;
}

void Function::Set(const String &name, Type type, Access access, const ParameterConnectionSet &parameters)
{
	static bool ignoreChange = false;

	if (ignoreChange)
		return;

	if (name == _chip->GetName() && _type == type && _access == access && _parameters == parameters)
		return; // No change

	ParameterConnectionSet oldParameters = _parameters;
	FunctionSignatureID oldSignature = _signature;

	String oldName = _chip->GetName();

	if (name != _chip->GetName()) {
		ignoreChange = true;
		_chip->SetName(name); // This will call our Set(name)
		ignoreChange = false;
	}
	
	_type = type;
	_access = access;
	_parameters = parameters;

	if (_type == Type::Virtual) {
		FunctionSignature fs;
		fs.functionName = _chip->GetName();
		fs.returnType = _chip->GetChipTypeIndex();

		for (const auto &n : _parameters) {
			if (n.c >= fs.arguments.size())
				fs.arguments.resize(n.c + 1, InvalidChipTypeIndex);
			fs.arguments[n.c] = n.p->GetChipTypeIndex();
		}

		_signature = engine->GetFunctionSignatureManager()->RegisterSignature(fs);

		if (_access == Access::Private)
			_access = Access::Protected; // virtual functions can't be private!
	}
	else
		_signature = InvalidFunctionSignatureID; // Signature only needed for virtuals..

	// If we use signature for all functions, we could do without this one.. (it is used only for marking classes dirty...)
//	bool nameOrParameterChange = oldParameters != parameters || oldName != name;

	_chip->GetClass()->OnFunctionChange(this, oldSignature);//, nameOrParameterChange);

	FunctionCallSet functionCalls = _functionCalls; // Need to copy this because _functionCalls may be altered during the process.
	for (const auto &n : functionCalls)
		n->OnFunctionChange(oldParameters); // This function may call our UnregisterFunctionCall()
}

void Function::Set(const String &name)
{
	Set(name, _type, _access, _parameters);
}

void Function::RemoveParameter(Parameter *param)
{
	uint32 i = GetParameterIndex(param);
	if (i == -1)
		return;
	ParameterConnectionSet parameters;
	for (const auto &n : _parameters) {
		if (n.c < i)
			parameters.insert(n);
		else if (n.c > i)
			parameters.insert(ParameterConnection(n.p, n.c - 1));
	}
	Set(_chip->GetName(), _type, _access, parameters);	
}

void Function::RegisterFunctionCall(FunctionCall *fc)
{
	_functionCalls.insert(fc);
}

void Function::UnregisterFunctionCall(FunctionCall *fc)
{
	_functionCalls.erase(fc);
}

void Function::AddCallTime(int64 callTime, int64 callTimeExclSubFunc, uint32 ccpHitCount) 
{ 
	if (_lastFrame != functionStack.GetPerfFrame()) {
		_lastFrame = functionStack.GetPerfFrame();
		_callTime = 0;
		_callTimeExclSubFunc = 0;
		_cppHitCount = 0;
		_hitCount = 0;
	}

	_callTime += callTime; 
	_callTimeExclSubFunc += callTimeExclSubFunc;
	_cppHitCount += ccpHitCount; 
	_hitCount++; 
}


