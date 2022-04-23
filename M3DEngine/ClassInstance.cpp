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
#include "ClassInstance.h"
#include "Engine.h"
#include "ClassManager.h"
#include "ChipManager.h"
#include "Class.h"
#include "StdChips/InstanceData.h"
#include "ClassInstanceRef.h"
#include "DocumentManager.h"

using namespace m3d;

std::atomic<ClassInstanceID> runtimeIDs = 0;

ClassInstance::ClassInstance(const Guid &id, const Guid &clazzid, Path filename) : _id(id), _runtimeID(++runtimeIDs), _owner(nullptr), _clazz(nullptr), _delayDestruction(false), _releaseCallback(nullptr)
{
	_serialization = mmnew ClassInstanceSerialization();
	_serialization->clazzid = clazzid;
	_serialization->filename = filename;
	_serialization->atManager = false;
}

ClassInstance::ClassInstance(const Guid &id, const Guid & clazzid, Path filename, ChipPtrByGUIDMap &&data, Chip *owner, String name) : _id(id), _runtimeID(++runtimeIDs), _owner(owner), _clazz(nullptr), _delayDestruction(false), _name(name), _releaseCallback(nullptr)
{
	assert(owner);
	_serialization = mmnew ClassInstanceSerialization();
	_serialization->clazzid = clazzid;
	_serialization->filename = filename;
	_serialization->data = std::move(data);
	_serialization->atManager = false;
}

ClassInstance::ClassInstance(Class *cg, Chip *owner) : _runtimeID(++runtimeIDs), _clazz(cg), _owner(owner), _serialization(nullptr), _delayDestruction(false), _releaseCallback(nullptr)
{
	assert(cg);
	assert(owner);
	GenerateGuid(_id);
	_initInstanceData(_clazz); // create our instance data!
}

ClassInstance::ClassInstance(ClassInstance *original, Chip *ownerOfNewInstance) : _runtimeID(++runtimeIDs), _clazz(original->_clazz), _owner(ownerOfNewInstance), _serialization(nullptr), _delayDestruction(false), _releaseCallback(nullptr)
{
	GenerateGuid(_id);
	if (original->_serialization) {
		_serialization = mmnew ClassInstanceSerialization();
		_serialization->clazzid = original->_serialization->clazzid;
		_serialization->filename = original->_serialization->filename;
		_serialization->atManager = original->_serialization->atManager;
		
		for (const auto &n : original->_serialization->data) {
			Chip *data = engine->GetChipManager()->CreateChip(n.second->GetChipDesc().type);
			if (!data)
				continue;
			if (!data->CopyChip(n.second)) {
				data->Release();
				continue;
			}
			_serialization->data.insert(std::make_pair(n.first, data));
		}
		if (_serialization->atManager)
			engine->GetClassManager()->RegisterSerializedInstance(this);
	}
	else {
		assert(_clazz != nullptr);

		ChipPtrByGUIDMap instanceData;
		for (const auto &n : original->_instanceData) {
			Chip *data = engine->GetChipManager()->CreateChip(n.second->GetChipDesc().type);
			if (!data)
				continue;
			if (!data->CopyChip(n.second)) {
				data->Release();
				continue;
			}
			instanceData.insert(std::make_pair(n.first->GetGlobalID(), data));
		}
		_initInstanceData(_clazz, &instanceData);
	}

	if (_clazz)
		_clazz->RegisterInstance(this);
}

ClassInstance::~ClassInstance()
{
	if (_clazz)
		_clazz->UnregisterInstance(this);

	for (const auto &n : _instanceData)
		n.second->Release(); // Release all instance data!

	if (_serialization) { // Destroy the serialization!
		if (_serialization->atManager)
			engine->GetClassManager()->UnregisterSerializedInstance(this);

		for (const auto &n : _serialization->data)
			n.second->Release(); // Release all serialized instance data.
		mmdelete(_serialization);
	}
}

void ClassInstance::Release() const
{
	if (_ref.empty()) {
		if (_releaseCallback)
			_releaseCallback->OnRelease((ClassInstance*)this);
		return mmdelete(this);
	}
	while (_ref.size() > 1)
		(*_ref.begin())->Reset();
	(*_ref.begin())->Reset();
}

void ClassInstance::_addRef(ClassInstanceRef *ref) 
{ 
	_ref.insert(ref); 
}

void ClassInstance::_removeRef(ClassInstanceRef *ref) 
{ 
	_ref.erase(ref); 
	if (_ref.empty() && !_delayDestruction)
		Release();
}

void ClassInstance::_initInstanceData(Class *cg, ChipPtrByGUIDMap *fromSerialization)
{
	for (const auto &n : cg->GetInstanceData()) {
		if (_instanceData.find(n) != _instanceData.end())
			continue; // We already have this data. This may be because of multiple inheritance (diamond pattern).
		if (fromSerialization) {
			const auto m = fromSerialization->find(n->GetGlobalID());
			if (m != fromSerialization->end()) {
				if (m->second->GetChipDesc().type == n->GetTemplate()->GetChipDesc().type) {

					// Added 28/7/19 because class was missing for some instance data chips...
					Chip *data = m->second;
					assert(_owner);
					data->SetClass(_owner->GetClass());
					data->SetOwner(_owner);

					bool b = _instanceData.insert(std::make_pair(n, data)).second; // should always return true!
					assert(b);
					m->second->SetChildProvider(n->GetTemplate());
					fromSerialization->erase(m);
					continue;
				}
				else {
					msg(WARN, MTEXT("The type of the loaded instance data (") + cg->GetName() + MTEXT("::") + n->GetName() + MTEXT(") does not match the current type."), _owner);
					m->second->Release();
					fromSerialization->erase(m);
				}
			}
			else {
				msg(WARN, MTEXT("Instance data for ") + cg->GetName() + MTEXT("::") + n->GetName() + MTEXT(" not found in loaded instance."), _owner);
			}
		}
		// No data from backing store.. we have to copy from instance data template!
		_addInstanceData(n);
	}
	for (const auto &n : cg->GetBaseClasses())
		_initInstanceData(n, fromSerialization); // Init instance data for base graphs!
	if (cg == _clazz) { // Recursion ended?
		if (fromSerialization) {
			for (const auto &n : *fromSerialization) {
				msg(WARN, MTEXT("Removing excessive instance data member (") + n.second->GetName() + MTEXT(") loaded."), _owner);
				n.second->Release();
			}
			fromSerialization->clear();
		}
	}
}

void ClassInstance::_addInstanceData(InstanceData *instanceData)
{
	if (_serialization)
		return; // We can ignore this if we are serialized
	assert(instanceData);
	if (_instanceData.find(instanceData) != _instanceData.end())
		return; // We already have this data. This may be because of multiple inheritance (diamond pattern).
	Chip *data = instanceData->CreateChipFromTemplate();
	if (!data) { // Why should this happend?
		msg(WARN, MTEXT("Failed to add instance data (") + instanceData->GetName() + MTEXT(")."), _owner);
		return; 
	}
	assert(_owner && _owner->GetClass());
	data->SetClass(_owner->GetClass());
	data->SetOwner(_owner);
	_instanceData.insert(std::make_pair(instanceData, data));
}

void ClassInstance::_removeInstanceData(InstanceData *instanceData)
{
	if (_serialization)
		return; // We can ignore this if we are serialized
	const auto n = _instanceData.find(instanceData);
	if (n != _instanceData.end()) {
		if (n->second != nullptr)
			n->second->Release();
	}
	_instanceData.erase(n);
}

ClassInstanceRef ClassInstance::Create(Class *cg, Chip *owner)
{
	assert(cg);
	assert(owner);

	ClassInstance *instance = mmnew ClassInstance(cg, owner);
	if (cg->RegisterInstance(instance))
		return ClassInstanceRef(instance, true);
	mmdelete(instance);
	return ClassInstanceRef();
}

ClassInstanceRef ClassInstance::Clone(Chip *ownerOfNewInstance)
{
	return ClassInstanceRef(mmnew ClassInstance(this, ownerOfNewInstance), true);
}

bool ClassInstance::_deserialize()
{
	assert(_serialization);
	assert(_owner);

	if (!_clazz) { // Do we know about our cg yet? not?
		Class *cg = engine->GetClassManager()->GetClass(_serialization->clazzid);

		if (!cg) { // No class? try to load!
			if (_serialization->filename.IsFile())
				engine->GetDocumentManager()->GetDocument(_serialization->filename); // Do not use the return value! No guarantee it is our cg that just loaded!

			if (!_serialization)
				return true; // It could happen that while waiting in GetDocment, we could be deserialized by others. If no serialization, just return happily!

			cg = engine->GetClassManager()->GetClass(_serialization->clazzid); // Try to find it now!

			if (!cg) {
				_owner->AddMessage(Chip::ClassNotFoundException(_serialization->clazzid, _serialization->filename));
				return false; // Failed to load cg!
			}
		}
		_clazz = cg;
	}

	ClassInstanceSerialization *ser = _serialization;
	_serialization = nullptr;
	_initInstanceData(_clazz, &ser->data);
	mmdelete(ser);

	return true;
}

void ClassInstance::CompleteLoading(bool changeID)
{
	assert(_serialization);

	if (changeID)
		GenerateGuid(_id);

	Class *cg = engine->GetClassManager()->GetClass(GetSerialization()->clazzid);

	// First, check if the manager knows about any serialized instances with our id.
	ClassInstance *i = engine->GetClassManager()->GetSerializedInstance(GetSerialization()->clazzid, _id);
	if (i == nullptr && cg) // If not, check if it is found in the cg.
		i = cg->FindInstance(_id);

	if (i != nullptr) { // found it?
		if (GetOwner() != nullptr) { // Do we have an owner?
			if (i->GetOwner() != nullptr) { // Do i have an owner? Conflict!
				msg(WARN, MTEXT("An instance with the same ID already exist. Changing ID. References may be pointing to the wrong instance."), GetOwner());
				GenerateGuid(_id); // Change our id!
			}
			else { // No, i is ref conly!
				// All references to i must be directed to us. i is not owned by anyone, should be serialized, and have no data!
				assert(i->GetSerialization());
				assert(i->GetSerialization()->data.empty());
				for (size_t k = 0, j = i->_ref.size(); k < j; k++)
					**i->_ref.begin() = ClassInstanceRef(this, false);
				// Note: when the last reference is cleared, i is destryed!
			}
		}
		else { // No, we're ref only!
			// All references to us must be directed to i. i is the instance to be used. It can be serialized or not, and have an owner or not. 
			for (size_t k = 0, j = _ref.size(); k < j; k++)
				**_ref.begin() = ClassInstanceRef(i, false);
			// Note: when the last reference is cleared, we are destroyed!
			return;
		}
	}

	if (cg) {
		_clazz = cg; // 10/11/12: moved this above next line. don't know if this has any unforeseen concequences!
		bool b = cg->RegisterInstance(this);
		assert(b);
	}
	else {
		_serialization->atManager = true;
		bool b = engine->GetClassManager()->RegisterSerializedInstance(this); // We can register our serialized instance at the manager.
		assert(b);
	}
}

void ClassInstance::OnDestroyDevice()
{
	for (const auto &n : _instanceData)
		n.second->OnDestroyDevice();
	if (_serialization)
		for (const auto &n : _serialization->data)
			n.second->OnDestroyDevice();
}

void ClassInstance::OnReleasingBackBuffer(RenderWindow *rw)
{
	for (const auto &n : _instanceData)
		n.second->OnReleasingBackBuffer(rw);
	if (_serialization)
		for (const auto &n : _serialization->data)
			n.second->OnReleasingBackBuffer(rw);
}

void ClassInstance::RestoreChip()
{
	for (const auto &n : _instanceData)
		n.second->RestoreChip();
	if (_serialization)
		for (const auto &n : _serialization->data)
			n.second->RestoreChip();
}

void ClassInstance::AddDependencies(ProjectDependencies &deps)
{
	for (const auto &n : _instanceData)
		n.second->AddDependencies(deps);
	if (_serialization)
		for (const auto &n : _serialization->data)
			n.second->AddDependencies(deps);
}

Chip *ClassInstance::FindChip(ChipID chipID)
{
	Chip *c = nullptr;
	for (const auto &n : _instanceData)
		if (c = n.second->FindChip(chipID))
			return c;
	if (_serialization)
		for (const auto &n : _serialization->data)
			if (c = n.second->FindChip(chipID))
				return c;
	return nullptr;
}

void ClassInstance::SetDelayDestruction(bool b)
{
	_delayDestruction = b;
	if (!_delayDestruction && _ref.empty())
		Release();
}

void ClassInstance::SetName(String name)
{
	if (_name == name)
		return;
	_name = name;
	if (_clazz)
		_clazz->OnInstanceNameChanged(this);
}

void ClassInstance::SetOwner(Chip* owner) 
{ 
	assert(owner != nullptr);
	// Note: Do not do an early return if _owner==owner, because we do also set class for data members!
	_owner = owner; 
	for (const auto &n : _instanceData) {
		n.second->SetOwner(_owner);
		assert(_owner->GetClass() != nullptr);
		n.second->SetClass(_owner->GetClass());
	}
}