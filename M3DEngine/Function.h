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

namespace m3d
{

class Parameter;

struct ParameterConnection
{
	Parameter *p; 
	uint32 c;
	ParameterConnection() : p(0), c(-1) {}
	ParameterConnection(Parameter *p, uint32 c = -1) : p(p), c(c) {}
	bool operator<(const ParameterConnection &rhs) const { return p < rhs.p && c != rhs.c; }
	bool operator==(const ParameterConnection &rhs) const { return p == rhs.p && c == rhs.c; }
	bool operator!=(const ParameterConnection& rhs) const { return !(*this == rhs); }
};

typedef Set<ParameterConnection> ParameterConnectionSet;
typedef Set<FunctionCall*> FunctionCallSet;

class Chip;

class M3DENGINE_API Function
{
public:
	enum class Type { Static, NonVirtual, Virtual };
	enum class Access { Public, Protected, Private };

protected:
	Chip *_chip;
	Function *_vFunction;

	Type _type;
	Access _access;
	ParameterConnectionSet _parameters;
	FunctionSignatureID _signature;
	FunctionCallSet _functionCalls;

	// For performance monitoring
	int64 _callTime;
	int64 _callTimeExclSubFunc;
	uint32 _lastFrame;
	uint32 _hitCount;
	uint32 _cppHitCount;

public:
	Function(Chip *chip);
	~Function();

	Chip *GetChip() const { return _chip; }

	Type GetType() const { return _type; }
	Access GetAccess() const { return _access; }
	const ParameterConnectionSet &GetParameters() const { return _parameters; }
	uint32 GetParameterIndex(Parameter *param);
	Parameter *GetParameterFromIndex(uint32 index);
	FunctionSignatureID GetSignature() const { return _signature; }
	const FunctionCallSet &GetFunctionCalls() const { return _functionCalls; }

	void Set(const String &name, Type type, Access access, const ParameterConnectionSet &parameters);
	void Set(const String &name);
	void RemoveParameter(Parameter *param);

	void RegisterFunctionCall(FunctionCall *fc);
	void UnregisterFunctionCall(FunctionCall *fc);

	// For performance monitoring
	void AddCallTime(int64 callTime, int64 callTimeExclSubFunc, uint32 ccpHitCount);
	inline int64 GetCallTime() const { return _callTime; }
	inline int64 GetCallTimeExclSubFunc() const { return _callTimeExclSubFunc; }
	inline uint32 GetLastFrame() const { return _lastFrame; }
	inline uint32 GetHitCount() const { return _hitCount; }
	inline uint32 GetCPPHitCount() const { return _cppHitCount; }

};



}