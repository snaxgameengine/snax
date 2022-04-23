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
#include "Class.h"
#include "DocumentSaver.h"
#include "DocumentLoader.h"
#include "M3DCore/GuidUtil.h"
#include "Engine.h"
#include "ClassManager.h"
#include "ChipManager.h"
#include "ClassInstance.h"
#include "StdChips/FunctionCall.h"
#include "StdChips/Parameter.h"
#include "StdChips/InstanceData.h"
#include "StdChips/ClassChip.h"
#include "ProjectDependencies.h"
#include "DocumentManager.h"
#include "Document.h"

using namespace m3d;



Class *ClassFactory::Create()
{
	return mmnew Class();
}

ClassFactory m3d::DefaultClassFactory;



std::atomic<ClassID> cgids;

Class::Class() : _guid(NullGUID), _clazzid(++cgids), _chStart(nullptr), _functionDataIDs(0), _loadInfo(nullptr), _doc(nullptr), _eventListener(nullptr)
{
}

Class::~Class()
{
	// Call Clear() before deleting the class!
	assert(_subClasses.empty());
	assert(_baseClasses.empty());
	assert(_allBaseClasses.empty());
	assert(_chips.empty());
	assert(_functions.empty());
	assert(_vFunctions.empty());
	assert(_parameters.empty());
	assert(_instanceData.empty());
	assert(_instances.empty());
	assert(_instancesByGlobalID.empty());
	assert(_clazzChips.empty());
	assert(_loadInfo == nullptr);
}

void Class::Clear()
{
	// This stuff can't be in dtor because many of our virtual methods are being called!

	DeleteLoadInfo();

	// Remove all chips!
	// Removing the chip will cause its function to be deleted and removed from us!
	// All parameters, instanceData and functionCalls will also be removed!
	while (_chips.size())
		RemoveChip(_chips.begin()->second); 
}

void Class::SetName(String name)
{
	if (name == _name)
		return;
	_name = name;

	// Notify all function calls about the rename...
	for (const auto &n :  _functions) {
		for (const auto &m : n->GetFunctionCalls()) {
			m->OnClassRenamed(GetName());
		}
	}
}


bool Class::RegisterInstance(ClassInstance *instance)
{
	const auto n = _instancesByGlobalID.find(instance->GetID());
	if (n != _instancesByGlobalID.end()) {
		assert(false);	// 27/7/19: Not really critical, I think.. It means the instance registered twice, so we should probably check why..
		return false; // global id already exist.
	}
	if (!_instances.insert(instance).second) {
		assert(false);
		return false; // instance already exist.. this should really not happen...
	}
	_instancesByGlobalID.insert(std::make_pair(instance->GetID(), instance));
	if (_eventListener)
		_eventListener->OnInstanceRegistered(instance);
	return true;
}

void Class::UnregisterInstance(ClassInstance *instance)
{
	_instancesByGlobalID.erase(instance->GetID());
	_instances.erase(instance);
	if (_eventListener)
		_eventListener->OnInstanceUnregistered(instance);
}

void Class::OnInstanceNameChanged(ClassInstance *instance)
{
	if (_eventListener)
		_eventListener->OnInstanceNameChanged(instance);
}

ClassInstance *Class::FindInstance(const Guid &instanceID)
{
	auto itr = _instancesByGlobalID.find(instanceID);
	return itr != _instancesByGlobalID.end() ? itr->second : nullptr;
}


void Class::RemoveChipAsChildOrParameter(Chip *ch)
{
	for (const auto &n : _chips) {
		n.second->RemoveChild(ch);
	}
	if (ch->AsParameter()) {
		for (const auto &n : _functions)
			n->RemoveParameter(ch->AsParameter());
	}
}

Chip *Class::AddChip(ChipTypeIndex chipGuid, bool initChip)
{
	Chip *chip = engine->GetChipManager()->CreateChip(chipGuid);

	if (!chip)
		return nullptr;

	chip->SetClass(this);

	if (initChip && !chip->InitChip()) {
		chip->Release();
		return nullptr;
	}

	_chips.insert(std::make_pair(chip->GetID(), chip)); // should ALWAYS be ok!

	return chip;
}

bool Class::RemoveChip(Chip *ch)
{
	if (!ch)
		return false;

	const auto &n = _chips.find(ch->GetID());
	if (n == _chips.end())
		return false;

	RemoveChipAsChildOrParameter(ch);

	_chips.erase(n);

	if (ch->AsParameter())
		_parameters.erase(ch->AsParameter());

	if (ch->AsInstanceData() && _instanceData.erase(ch->AsInstanceData()) > 0)
		_onInstanceDataRemoved(ch->AsInstanceData());

	if (_chStart == ch)
		_chStart = nullptr;

	ch->Release();

	return true;
}

Chip *Class::GetChip(ChipID id)
{
	const auto &n = _chips.find(id);
	return n != _chips.end() ? n->second : nullptr; // could be null if not found!
}

void Class::OnFunctionCreate(Function *function)
{
	assert(function);
	if (GetChip(function->GetChip()->GetID()) == nullptr) {
		assert(false);
		return; // This function doesn't belong to us!
	}
	if (_functions.insert(function).second) {
		for (const auto &n : _functions) {
			if (n->GetChip()->GetName() == function->GetChip()->GetName() && function != n) {
				msg(WARN, MTEXT("Another function with this name already exist in this Class. Function Calls may connect to wrong function!"), function->GetChip());
				break;
			}
		}
		if (function->GetType() == Function::Type::Virtual)
			_updateVirtualFunction(function->GetSignature());

		if (_eventListener)
			_eventListener->OnFunctionCreate(function);
	}
	else
		assert(false);
}

void Class::OnFunctionRemove(Function *function)
{
	assert(function);
	if (_functions.erase(function) > 0 && function->GetType() == Function::Type::Virtual)
		_updateVirtualFunction(function->GetSignature());
	if (_eventListener)
		_eventListener->OnFunctionRemove(function);
}

void Class::OnFunctionChange(Function *function, FunctionSignatureID oldSignature)
{
	assert(function);
	assert(GetChip(function->GetChip()->GetID()) != 0);
	if (oldSignature != InvalidFunctionSignatureID && oldSignature != function->GetSignature())
		_updateVirtualFunction(oldSignature);
	if (function->GetType() == Function::Type::Virtual)
		_updateVirtualFunction(function->GetSignature());
	if (_eventListener)
		_eventListener->OnFunctionChange(function);
}

void Class::_updateVirtualFunction(FunctionSignatureID signature)
{
	assert(signature != InvalidFunctionSignatureID);
	auto n = _vFunctions.find(signature); 

	Function *vFunc = 0;
	for (const auto &m : _functions) { // Iterate local functions...
		if (m->GetType() == Function::Type::Virtual && m->GetSignature() == signature) { // Matching virtual function?
			if (vFunc) { // Function found already? Ambiguous!
				if (!(n != _vFunctions.end() && n->second == 0)) { // Are we entering an ambiguous state?
					msg(WARN, MTEXT("Another virtual function with the same signature exist in this Class. Functions will be unaccessible!"), m->GetChip());
					if (n == _vFunctions.end())
						n = _vFunctions.insert(std::make_pair(signature, (Function*)nullptr)).first;
					else {
						n->second = nullptr;
						for (const auto &m : _subClasses)
							m->_updateVirtualFunction(signature);
					}
				}
				return;
			}
			vFunc = m;
		}
	}
	if (vFunc == 0) { // No local function found... look in bases...
		for (const auto &m : _baseClasses) {
			auto itr = m->_vFunctions.find(signature);
			if (itr == m->_vFunctions.end())
				continue;
			Function *f = itr->second;
			assert(f);
			if (vFunc) {
				// Situation: We do not have any local function with this signature. We have now found it in more than one base: ambiguous!
				if (n != _vFunctions.end()) {
					msg(WARN, String(MTEXT("Two or more Base Classes contain the same virtual function (")) + f->GetChip()->GetName() + MTEXT("). Override to resolve!"), this);
				}
				vFunc = 0;
				break;
			}
			vFunc = f;
		}
	}

	if (vFunc == 0) { // Still no function found?
		if (n != _vFunctions.end()) {
			bool isChange = n->second != nullptr;
			_vFunctions.erase(n);
			if (isChange) {
				for (const auto &m : _subClasses)
					m->_updateVirtualFunction(signature);
			}
		}
	}
	else {
		if (n == _vFunctions.end())
			n = _vFunctions.insert(std::make_pair(signature, (Function*)nullptr)).first;
		if (n->second != vFunc) {
			n->second = vFunc;
			for (const auto &m : _subClasses)
				m->_updateVirtualFunction(signature);
		}
	}
}

Function *Class::GetFunction(const String &name, ChipTypeIndex typeRequired)
{
	if (typeRequired == InvalidChipTypeIndex)
		typeRequired = engine->GetChipManager()->GetChipTypeIndex(CHIP_GUID);
	for (const auto &n : _functions) {
		// Note: there could be more than one function with the same name. Thats an ambiguousy the user is warned about.
		if (n->GetChip()->GetName() == name && engine->GetChipManager()->IsChipTypeSupported(typeRequired, n->GetChip()->GetChipTypeIndex()))
			return n;
	}
	return nullptr;
}


void Class::Run()
{
	if (_chStart)
		_chStart->Run();
}

bool Class::SetStartChip(Chip *chip)
{
	if (chip && !GetChip(chip->GetID()))
		return false;
	_chStart = chip;
	return true;
}

void Class::OnDestroyDevice()
{
	for (const auto &n : _chips)
		n.second->OnDestroyDevice();
}

void Class::OnReleasingBackBuffer(RenderWindow *rw)
{
	for (const auto &n : _chips)
		n.second->OnReleasingBackBuffer(rw);
}

void Class::_findInstanceData(Class *base, Set<InstanceData*> &instanceData, uint32 nBaseOccurences) const
{
	if (_findBaseCount(base) == nBaseOccurences) { // ==1 means that it's new because it's only derived once! ==0 means it's removed.
		for (const auto &n : base->GetInstanceData())
			instanceData.insert(n);
		
		for (const auto &n : base->_baseClasses)
			_findInstanceData(n, instanceData, nBaseOccurences); // reqursively iterate all inherited bases!
	}
}

uint32 Class::_findBaseCount(Class *base) const
{
	if (base == this)
		return 1;
	uint32 i = 0;
	for (const auto &n : _baseClasses)
		i += n->_findBaseCount(base);
	return i;
}

void Class::_onBaseClassAdded(Class *base, Class *directBase)
{
	// Notify objects
	// they must add new data members for base and all its bases.
	// NOTE: both base and all its bases may already be bases of us
	// because of multiple inheritance.
	// Therefore: Iterate base and all its bases, and add only missing data.

	Set<InstanceData*> newInstanceData;
	_findInstanceData(base, newInstanceData, 1);

	for (const auto &n : _instances)
		for (const auto m : newInstanceData)
			n->_addInstanceData(m);

	for (const auto &n : directBase->_vFunctions)
		_updateVirtualFunction(n.first); // Update all inherited virtual functions

	for (const auto &n : _subClasses)
		n->_onBaseClassAdded(base, this); // Notify all derived classes
}

void Class::_onBaseClassRemoved(Class *base, Class *directBase)
{
	// Notify objects
	// they must remove data members for base and all its bases.
	// NOTE: both base and all its bases may still be bases of us
	// because of multiple inheritance.
	// Therefore: Iterate base and all its bases, check if it still exist,
	// and remove only data for non-existing!

	Set<InstanceData*> oldInstanceData;
	_findInstanceData(base, oldInstanceData, 0);
	
	for (const auto &n : _instances)
		for (const auto &m : oldInstanceData)
			n->_removeInstanceData(m);

	for (const auto &n : directBase->_vFunctions)
		_updateVirtualFunction(n.first); // Update all inherited virtual functions

	for (const auto n : _subClasses)
		n->_onBaseClassRemoved(base, this); // Notify all derived classes

	// TODO: Iterate all functions and their calls to ensure that they are still valid!
}

bool Class::AddBaseClass(Class *base)
{
	if (_baseClasses.find(base) != _baseClasses.end())
		return true; // Already one of our direct bases
	if (base->IsBaseClass(this)) 
		return false; // Avoid cyclic inheritance
	_baseClasses.insert(base);
	base->_subClasses.insert(this);
	_onBaseClassAdded(base, base);
	if (_eventListener)
		_eventListener->OnBaseClassAdded(this, base);
	return true;
}

bool Class::RemoveBaseClass(Class *base)
{
	if (_baseClasses.erase(base) == 0)
		return false; // Not found
	base->_subClasses.erase(this);
	_onBaseClassRemoved(base, base);
	if (_eventListener)
		_eventListener->OnBaseClassRemoved(this, base);
	return true;
}

bool Class::IsBaseClass(const Class *base) const
{
	if (base == this)
		return true;
	for (const auto &n : _baseClasses)
		if (n->IsBaseClass(base))
			return true;
	return false;
}

uint32 Class::IsBaseClassN(const Class *base) const
{
	if (base == this)
		return 1;
	uint32 r = 0;
	for (const auto& n : _baseClasses)
		if ((r = n->IsBaseClass(base)) != 0)
			return r + 1;
	return 0;
}

bool Class::IsDirectBaseClass(const Class *base) const
{
	for (const auto &n : _baseClasses)
		if (n == base)
			return true;
	return false;
}

uint32 Class::GetBaseCount() const
{
	uint32 i = 0;
	for (const auto &n : _baseClasses)
		i += n->GetBaseCount() + 1;
	return i;
}

void Class::OnParameterTypeSet(Parameter *parameter)
{
	assert(parameter);
	if (GetChip(parameter->GetID()) == 0 || !parameter->IsChipTypeSet())
		return; // Not one of ours or not set!
	_parameters.insert(parameter);
}

void Class::OnInstanceDataTypeSet(InstanceData *data)
{
	assert(data);
	if (GetChip(data->GetID()) == 0 || !data->IsChipTypeSet())
		return; // Not one of ours or not set!
	_instanceData.insert(data);
	_onInstanceDataAdded(data);
}

void Class::_onInstanceDataAdded(InstanceData *data)
{
	for (const auto &n : _instances)
		n->_addInstanceData(data);
	for (const auto &n : _subClasses)
		n->_onInstanceDataAdded(data);
}

void Class::_onInstanceDataRemoved(InstanceData *data)
{
	for (const auto &n : _instances)
		n->_removeInstanceData(data);
	for (const auto &n : _subClasses)
		n->_onInstanceDataRemoved(data);
}


void Class::RestoreChips()
{
	// Make sure the class' document is actually fully loaded, and not only preloaded!
	// This was added to handle the case where chip messages are added while loading,
	// and when the user clicks on it, the class and the document must have been fully loaded!
	{
		Document* doc = GetDocument();
		if (doc) { // No doc for class diagrams...
			if (doc->GetFileName().IsValid())
				engine->GetDocumentManager()->GetDocument(doc->GetFileName());
		}
	}

	for (const auto &n : _chips)
		n.second->RestoreChip();
}

void Class::AddDependencies(ProjectDependencies &packets, ProjectDependencies &third)
{
	Set<Packet*> packetSet;
	for (const auto &n : _chips) {
		packetSet.insert(engine->GetChipManager()->GetChipInfo(n.second->GetChipDesc().type)->packet);
		n.second->AddDependencies(third);
	}
	for (const auto &n : packetSet) {
		packets.AddDependency(n->filename.AsString());
		if (n->addDependenciesFunc)
			(*n->addDependenciesFunc)(third);
	}
}

void Class::OnAddedToClassManager()
{
	// Iterate all base classes found during loading, load them if neccessary, and add them as base classes!
	if (_loadInfo) {
		for (const auto &n : _loadInfo->instances)
			n.second->CompleteLoading(false);
		_loadInfo->instances.clear();

		for (const auto &n : _loadInfo->loadedBaseClasses) {
			Class *base = engine->GetClassManager()->GetClass(n.first);
			if (!base)
				engine->GetDocumentManager()->GetDocument(n.second); // Try to load
			base = engine->GetClassManager()->GetClassAndFinishLoading(n.first);
			if (base) {
				bool b = AddBaseClass(base);
			}
		}
		_loadInfo->loadedBaseClasses.clear();

		DeleteLoadInfo();
	}

	// Iterate all serialized instances of us registered in the manager, and register them at us. These instances are owned by chips in other classes.
	ClassInstancePtrByGUIDMap si = engine->GetClassManager()->CaptureSerializedInstances(GetGuid());
	for (const auto &m : si) {
		m.second->_clazz = this; // temporary
		m.second->_serialization->atManager = false;
		RegisterInstance(m.second);
	}
}

void Class::OnRemovedFromClassManager()
{
	while (_clazzChips.size())
		(*_clazzChips.begin())->SetCG(nullptr);

	// Remove all instances
	while (_instances.size())
		(*_instances.begin())->Release(); // This will unregister at us.

	// Remove all sub classes!
	while (_subClasses.size())
		(*_subClasses.begin())->RemoveBaseClass(this);

	// Remove all base classes!
	while (_baseClasses.size())
		RemoveBaseClass(*_baseClasses.begin());
}

ClassLoadInfo *Class::GetLoadInfo(bool create)
{
	if (_loadInfo || !create)
		return _loadInfo;
	return _loadInfo = CreateLoadInfo();
}

void Class::DeleteLoadInfo()
{
	if (_loadInfo)
		DeleteLoadInfo(_loadInfo);
	_loadInfo = nullptr;
}

ClassLoadInfo *Class::CreateLoadInfo() const
{
	return mmnew ClassLoadInfo();
}

void Class::DeleteLoadInfo(ClassLoadInfo *nfo) const
{
	mmdelete(nfo);
}

Chip *Class::FindChip(ChipID chipID) const
{
	const auto &n = _chips.find(chipID);
	if (n != _chips.end())
		return n->second;
	for (const auto &m : _chips) {
		Chip *c = m.second->FindChip(chipID);
		if (c)
			return c;
	}
	return nullptr;
}
