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
#include "FunctionCall.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/ChipManager.h"
#include "Parameter.h"
#include "M3DEngine/DocumentSaver.h"
#include "M3DEngine/DocumentLoader.h"
#include "ClassInstanceRefChip.h"
#include "M3DEngine/Function.h"
#include "M3DEngine/FunctionStack.h"
#include "M3DEngine/ClassInstance.h"
#include "M3DEngine/DocumentManager.h"
#include "M3DEngine/Document.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/FunctionStackRecord.h"
#include "M3DEngine/Environment.h"

using namespace m3d;


CHIPDESCV1_DEF(FunctionCall, MTEXT("Function Call"), FUNCTIONCALL_GUID, PROXYCHIP_GUID)


FunctionCall::FunctionCall() : _function(nullptr), _preload(true), _callByName(false)
{
	ClearConnections();

	CREATE_CHILD(0, CLASSINSTANCEREFCHIP_GUID, false, BOTH, MTEXT("Instance"));
	CREATE_CHILD(1, CHIP_GUID, true, BOTH, MTEXT("Parameters"));
}

FunctionCall::~FunctionCall()
{
	if (_function)
		_function->UnregisterFunctionCall(this);
}

bool FunctionCall::CopyChip(Chip *chip)
{
	FunctionCall *c = dynamic_cast<FunctionCall*>(chip);
	B_RETURN(ProxyChip::CopyChip(c));
	_preload = c->_preload;
	SetFunction(c->_clazzName, c->_funcName, c->_callByName, false);
	_filename = c->_filename;
	return true;
}

bool FunctionCall::LoadChip(DocumentLoader &loader) 
{ 
	B_RETURN(ProxyChip::LoadChip(loader));
	String s1, s2;
	bool callByName = false;
	String filenameStr;
	LOAD(MTEXT("classId|cg"), s1);
	LOAD(MTEXT("functionName|func"), s2);
	LOAD(MTEXT("callByName|cbn"), callByName);
	if (loader.GetDocumentVersion() >= Version(1,2,1,0)) // TODO: Remove!
		LOAD(MTEXT("preload"), _preload);
	LOAD(MTEXT("filename"), filenameStr);
	SetFunction(s1, s2, callByName, false);

	if (!filenameStr.empty())
		_filename = loader.GetEnvironment()->ResolveDocumentPath(filenameStr, GetClass()->GetDocument()->GetFileName());

	if (_filename.IsFile() && _preload && loader.IsLoadRelatedDocumentsAsync()) {
		Path fn2 = engine->GetDocumentManager()->PreloadDocument(_filename, _clazzName); // Start loading document containing target function. NOTE: filename can be changed by the function call!
		if (fn2.IsValid() && fn2 != _filename && GetClass()) {
			msg(WARN, strUtils::ConstructString(MTEXT("FunctionCall-chip in class \'%1\' (%4) tried find class \'%3\' from \'%2\', but found it in \'%5\'. Please resave!")).arg(GetClass()->GetName()).arg(_filename.AsString()).arg(_clazzName).arg(GetClass()->GetDocument()->GetFileName().GetName()).arg(fn2.AsString()));
			_filename = fn2;
			GetClass()->SetDirty(); // Mark class as dirty so we can encourage user to save it with the updated filename!
		}
	}

	return true; 
}

bool FunctionCall::SaveChip(DocumentSaver &saver) const 
{ 
	B_RETURN(ProxyChip::SaveChip(saver));
	SAVE(MTEXT("classId"), _clazzName);
	SAVE(MTEXT("functionName"), _funcName);
	SAVE(MTEXT("callByName"), _callByName);
	SAVE(MTEXT("preload"), _preload);

	Path p;
	if (_function) {
		if (_function->GetChip()->GetClass()->GetDocument()->GetFileName().IsValid())
			p = _function->GetChip()->GetClass()->GetDocument()->GetFileName(); // update filename
	}
	else
		p = _filename;
	String filenameStr;
	if (p.IsValid())
		filenameStr = saver.GetEnvironment()->CreateDocumentPath(p, GetClass()->GetDocument()->GetFileName());

	SAVE(MTEXT("filename"), filenameStr);

	return true; 
}

ChipChildPtr FunctionCall::GetChip()
{
	//Touch();

	if (!ConnectToFunction())
		return ChipChildPtr(); // No function!

	if (_function->GetType() != Function::Type::Static) {
		ClassInstanceRef instance;
		bool isVirtualFuncCall = _function->GetType() == Function::Type::Virtual;
		if (_callByName) { 
			instance = ClassInstanceRef(functionStack.GetCurrentRecord().instance, false); // Read instance on top of the stack. If we're not within a non-static function-call the instance will be null.
			isVirtualFuncCall = false; // Ignoring virtual function like in c++: Base::someVirtualFunction();
		}
		else {
			ChildPtr<ClassInstanceRefChip> chInstance = GetChild(0);
			if (!chInstance) {
				AddMessage(NoInstanceException());
				return ChipChildPtr(); // No instance
			}
			instance = chInstance->GetInstance();
		}

//		if (!(instance && instance->Prepare())) {
		if (!instance.Prepare(this)) {
			AddMessage(NoInstanceException());
			return ChipChildPtr(); // No instance set/available
		}

		Class *funcCG = _function->GetChip()->GetClass();
		if (!instance->GetClass()->IsBaseClass(funcCG)) {
			AddMessage(WrongInstanceException(instance->GetClass()->GetName(), funcCG->GetName()));
			return ChipChildPtr(); // Wrong instance type!
		}

		if (isVirtualFuncCall) { // Is virtual function?
			const auto &f = instance->GetClass()->GetVirtualFunctions();
			auto itr = f.find(_function->GetSignature());
			Function* vFunction = itr != f.end() ? itr->second : nullptr;
			if (!vFunction)
				return ChipChildPtr(); // No virtual function found!
	
			if (!functionStack.CanAddRecord()) {
				AddMessage(StackOverflowException());
				return ChipChildPtr();
			}

			//ClearError();
			vFunction->GetChip()->Touch();
			return ChipChildPtr(vFunction, this, instance.GetRawPtr())->GetChip();
		}
		else { // else is non-virtual call...
			if (!functionStack.CanAddRecord()) {
				AddMessage(StackOverflowException());
				return ChipChildPtr();
			}

			//ClearError();
			_function->GetChip()->Touch();
			return ChipChildPtr(_function, this, instance.GetRawPtr())->GetChip();
		}
	}

	if (!functionStack.CanAddRecord()) {
		AddMessage(StackOverflowException());
		return ChipChildPtr();
	}

	//ClearError();

	_function->GetChip()->Touch();
	return ChipChildPtr(_function, this, nullptr)->GetChip();
}

bool FunctionCall::SetChild(Chip *child, uint32 index, uint32 subIndex)
{
	// During loading we may have to remap the children becuause we're not connected to a function yet!
	if (!_function && index > 1) {
		subIndex = index - 1;
		index = 1;
	}
	return Chip::SetChild(child, index, subIndex);
}

void FunctionCall::SetFunction(String cgName, String funcName, bool callByName, bool connect)
{
	if (cgName != _clazzName || funcName != _funcName || callByName != _callByName) { // Change?
		RemoveFunction();
	
		if (_clazzName != cgName)
			_filename = Path(); // Clear filename!

		_clazzName = cgName;
		_funcName = funcName;
		_callByName = callByName;

		SetName(_clazzName + MTEXT("->") + _funcName);
	}
	if (connect)
		ConnectToFunction();
}

bool FunctionCall::ConnectToFunction()
{
	if (_function)
		return true; // function OK!
	if (_clazzName.empty() || _funcName.empty())
		return false; // No function is set.
	Class *cg = engine->GetClassManager()->GetClass(_clazzName);
	if (!cg) {
//		if (_autoLoad == AUTOLOAD) {
			if (_filename.IsFile()) {
				engine->GetDocumentManager()->GetDocument(_filename);
				cg = engine->GetClassManager()->GetClass(_clazzName);
			}
//		}
		if (!cg) {
			AddMessage(FunctionConnectionFailedException());
			return false; // No class!
		}
	}
	_function = cg->GetFunction(_funcName, GetChipTypeIndex()); // locate function based on name and type!
	
	if (!_isFunctionValid())
		_function = nullptr;

	if (!_function) {
		AddMessage(FunctionConnectionFailedException());
		return false; // function not found or unable to call!
	}

	_function->RegisterFunctionCall(this); // register ourself at the function (to be notified about changes!)

	// Make sure to keep the connected children when setting the interface to that of the function!
	Chip *instance = GetRawChild(0);
	Map<uint32, Chip*> cm;
	for (uint32 i = 0; i < GetSubConnectionCount(1); i++)
		cm.insert(std::make_pair(i, GetRawChild(1, i)));
	SetParameters(instance, cm);

	return true;
}

bool FunctionCall::_isFunctionValid()
{
	if (!_function)
		return false;

	Class *cg = _function->GetChip()->GetClass();

	if (_function->GetAccess() == Function::Access::Private && cg != GetClass())
		return false; // Can't access private function outside cg.
	bool isBase = GetClass() && GetClass()->IsBaseClass(cg);
	if (_function->GetAccess() == Function::Access::Protected && !isBase)
		return false; // Can't access protected function outside cg or derived class.
	if (_callByName && !isBase)
		return false; // Can't call function by name if it is outside cg or derived classes. (In addition, we must be within a non-static function when calling it, but we can't check for that now)

	return true;
}

void FunctionCall::Validate()
{
	if (!_function)
		return;
	if (!_isFunctionValid())
		return RemoveFunction();
	if (_funcName != _function->GetChip()->GetName() || _clazzName != _function->GetChip()->GetClass()->GetName()) {
		_funcName = _function->GetChip()->GetName();
		_clazzName = _function->GetChip()->GetClass()->GetName();
		SetName(_clazzName + MTEXT("->") + _funcName);
	}
}

void FunctionCall::RemoveFunction()
{
	if (!_function)
		return; // No function!

	// Makes sure to keep children!
	Chip *instance = GetRawChild(0);
	Map<uint32, Chip*> cm;
	for (uint32 i = 1; i < GetConnectionCount(); i++) {
		Chip *ch = GetRawChild(i);
		if (ch)
			cm.insert(std::make_pair(i - 1, ch));
	}

	ClearConnections();

	CREATE_CHILD(0, CLASSINSTANCEREFCHIP_GUID, false, BOTH, MTEXT("Instance"));
	CREATE_CHILD(1, CHIP_GUID, true, BOTH, MTEXT("Parameters"));

	SetChild(instance, 0, 0);
	for (const auto &n : cm)
		SetChild(n.second, 1, n.first);

	_function->UnregisterFunctionCall(this); // Unregister at function
	_function = nullptr;
}

void FunctionCall::OnFunctionChange(const ParameterConnectionSet &oldParameters)
{
	Validate();

	if (!_function)
		return;

	Map<uint32, Chip*> cm;

	Chip *instance = GetRawChild(0);

	for (const auto &n : oldParameters) {
		uint32 i = _function->GetParameterIndex(n.p);
		if (i != -1) // If -1 the parameter was removed.
			cm.insert(std::make_pair(i, GetRawChild(n.c + 1))); 
	}

	SetParameters(instance, cm);
}

void FunctionCall::OnClassRenamed(String cgName)
{
	_clazzName = cgName;
	SetName(_clazzName + MTEXT("->") + _funcName);
}

void FunctionCall::SetParameters(Chip *instance, const Map<uint32, Chip*> &parameterMapping)
{	
	ClearConnections();

	if (_function->GetType() != Function::Type::Static && _callByName == false) {
		CREATE_CHILD(0, CLASSINSTANCEREFCHIP_GUID, false, BOTH, MTEXT("Instance"));
		SetChild(instance, 0, 0);
	}

	for (const auto &n : _function->GetParameters()) {
		CREATE_CHILD(n.c + 1, n.p->GetChipType(), false, BOTH, n.p->GetName());
		auto m = parameterMapping.find(n.c);
		if (m != parameterMapping.end())
			SetChild(m->second, n.c + 1, 0);
	}
}

Chip *FunctionCall::GetParameter(uint32 index)
{
	// NOTE: 10/3/13: Now using get RawChild(...)

	// NOTE: If the number of parameters is 0 or 1, the two cases below are equal!
	if (GetConnectionCount() > 2)
		return GetRawChild(index + 1); // This is the normal case where our parameter connections are set!
	// This can be used for special cases like when we're using the function call as an InstanceData. 
	// Then the function call can be used even if the templates function (and paramter connections!) is not set. Remember, it is the template that provide our children!
	// This enable us to have instances containting function calls to various functions.
	return GetRawChild(1, index); 
}

bool FunctionCall::SetChipType(const Guid &type)
{
	B_RETURN(ProxyChip::SetChipType(type));
	ClearConnections();
	CREATE_CHILD(0, CLASSINSTANCEREFCHIP_GUID, false, BOTH, MTEXT("Instance"));
	CREATE_CHILD(1, CHIP_GUID, true, BOTH, MTEXT("Parameters"));
	return true;
}


