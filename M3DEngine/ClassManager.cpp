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
#include "ClassManager.h"
#include "Class.h"
#include "Engine.h"
#include "ClassInstance.h"
#include "ClassInstanceRef.h"
#include "DocumentManager.h"
#include "Document.h"


using namespace m3d;



ClassManager::ClassManager() : _clazzStart(nullptr), _factory(&DefaultClassFactory), _eventListener(nullptr)
{
}

ClassManager::~ClassManager()
{
	msg(DINFO, MTEXT("ClassManager::~ClassManager() called."));

	assert(_clazzByName.empty());
	assert(_clazzByGuid.empty());
	assert(_clazzById.empty());
	assert(_serializedInstances.empty());
}

void ClassManager::Clear2()
{
	ClassInstanceRef::copiedRef.Reset();
}

bool ClassManager::OnDocumentLoaded(Document *doc)
{
	assert(doc);
	bool ok = true;
	// Now, iterate loaded classes to check their name&guid.
	List<Class*> cgList = doc->GetClasss();
	for (size_t i = 0; i < cgList.size(); i++) { // Iterate classes
		String name = cgList[i]->GetName();
		Guid guid = cgList[i]->GetGuid();
		if (!_confirmNameAndGuid(name, guid)) { // name and guid ok?
			doc->RemoveClass(cgList[i]);
			doc->SetReadOnly(true);
			// Note: If irregular loading not allowed, we should probably break here!
			continue;
		}
		if (name != cgList[i]->GetName())
			cgList[i]->SetName(name);
		if (guid != cgList[i]->GetGuid())
			cgList[i]->SetGuid(guid);
	}
	if (ok) {
		const List<Class*> &cgList = doc->GetClasss();
		
		// Add the new classes to a list of classes ready to be added.
		// Because a class being loaded can ask for a base class that
		// is not yet processed, we can process them "on demand", in the
		// GetClassAndFinishLoading-function below.
		for (size_t i = 0; i < cgList.size(); i++)
			_classesBeingLoaded.insert(cgList[i]);

		for (size_t i = 0; i < cgList.size(); i++) {
			if (_classesBeingLoaded.erase(cgList[i]) == 1) { // Check if the class was processed already...
				_addClazz(cgList[i]);
			}
		}
	}

	return true;
}

Class *ClassManager::GetClass(String name) const
{
	auto itr = _clazzByName.find(name);
	if (itr != _clazzByName.end())
		return itr->second;
	ClassManager* cm = const_cast<ClassManager*>(this);
	for (const auto &n : cm->_classesBeingLoaded) {
		if (n->GetName() == name) {
			Class* c = n;
			cm->_classesBeingLoaded.erase(n);
			cm->_addClazz(c);
			return c;
		}
	}
	return nullptr;
}

Class *ClassManager::GetClass(const Guid &guid) const
{
	auto itr = _clazzByGuid.find(guid);
	if (itr != _clazzByGuid.end())
		return itr->second;
	ClassManager* cm = const_cast<ClassManager*>(this);
	for (const auto &n : cm->_classesBeingLoaded) {
		if (n->GetGuid() == guid) {
			Class *c = n;
			cm->_classesBeingLoaded.erase(n);
			cm->_addClazz(c);
			return c;
		}
	}
	return nullptr;
}

Class *ClassManager::GetClass(ClassID id) const
{
	auto itr = _clazzById.find(id);
	if (itr != _clazzById.end())
		return itr->second;
	ClassManager* cm = const_cast<ClassManager*>(this);
	for (const auto &n : cm->_classesBeingLoaded) {
		if (n->GetID() == id) {
			Class *c = n;
			cm->_classesBeingLoaded.erase(n);
			cm->_addClazz(c);
			return c;
		}
	}
	return nullptr;
}

Class* ClassManager::GetClassAndFinishLoading(const Guid& guid)
{
	auto itr = _clazzByGuid.find(guid);
	if (itr != _clazzByGuid.end())
		return itr->second;
	for (const auto &n : _classesBeingLoaded) {
		if (n->GetGuid() == guid) {
			Class* c = n;
			_classesBeingLoaded.erase(n);
			_addClazz(c);
			return c;
		}
	}
	return nullptr;
}

bool ClassManager::HasClass(String name, Path &fileName) const
{
	auto itr = _clazzByName.find(name);
	if (itr != _clazzByName.end()) {
		fileName = itr->second->GetDocument()->GetFileName();
		return true;
	}

	for (const auto &n : _classesBeingLoaded) {
		if (n->GetName() == name) {
			fileName = n->GetDocument()->GetFileName();
			return true;
		}
	}
	return false;
}

bool ClassManager::HasClass(const Guid& guid, Path& fileName) const
{
	auto itr = _clazzByGuid.find(guid);
	if (itr != _clazzByGuid.end()) {
		fileName = itr->second->GetDocument()->GetFileName();
		return true;
	}

	for (const auto &n : _classesBeingLoaded) {
		if (n->GetGuid() == guid) {
			fileName = n->GetDocument()->GetFileName();
			return true;
		}
	}
	return false;
}

Class *ClassManager::CreateClass(String name, Document *doc)
{
	assert(doc);
	Guid guid;
	if (!GenerateGuid(guid)) {
		msg(FATAL, MTEXT("GenerateGuid() failed."));
		return nullptr;
	}
	return CreateClass(name, guid, doc);
}

Class *ClassManager::CreateClass(String name, Guid guid, Document *doc)
{
	assert(doc);
	if (!_confirmNameAndGuid(name, guid))
		return nullptr;

	Class *cg = doc->CreateClass();
	if (!cg) {
		msg(FATAL, MTEXT("Failed to create Class."));
		return nullptr; // Factory returned null
	}

	cg->SetGuid(guid);
	cg->SetName(name);

	_addClazz(cg);

	return cg;
}


bool ClassManager::_confirmNameAndGuid(String &name, Guid &guid)
{
	while (_clazzByGuid.find(guid) != _clazzByGuid.end()) {
		if (!_eventListener || !_eventListener->ConfirmGuidChange(name) || !GenerateGuid(guid)) {
			msg(FATAL, MTEXT("Failed to create Class because of Guid conflict (") + GuidToString(guid) + MTEXT(")."));
			return false; // Guid already exist
		}
	}

	while (_clazzByName.find(name) != _clazzByName.end()) {
		if (!_eventListener || !_eventListener->ResolveNameConflict(name)) {
			msg(FATAL, MTEXT("Failed to create Class because of name conflict (") + name + MTEXT(")."));
			return false; // Name already exist
		}
	}
	return true;
}

bool ClassManager::RemoveClass(Class *cg)
{
	assert(cg);
	auto n = _clazzById.find(cg->GetID());
	if (n == _clazzById.end())
		return false;

	if (cg == _clazzStart)
		SetStartClass(nullptr);

	_clazzById.erase(n);
	_clazzByName.erase(cg->GetName());
	_clazzByGuid.erase(cg->GetGuid());

	cg->OnRemovedFromClassManager();

	if (_eventListener)
		_eventListener->OnClassRemoved(cg);

	Document *doc = cg->GetDocument();
	assert(doc);
	bool b = doc->RemoveClass(cg);
	assert(b);

	return true;
}

bool ClassManager::SetClassName(Class *cg, String name) 
{ 
	assert(cg);

	name = strUtils::trim(name);
	if (name.empty() || name.length() > 64)
		return false; // Invalid name!

	auto m = _clazzByName.find(name);
	if (m != _clazzByName.end()) {
		if (m->second == cg)
			return true; // No change!
		return false; // Name taken!
	}

	auto n = _clazzByName.find(cg->GetName());

	if (n == _clazzByName.end())
		return false; // Should not happen!

	if (_clazzByName.insert(std::make_pair(name, cg)).first->second != cg)
		return false; // Should not happen!

	_clazzByName.erase(n);

	cg->SetName(name);

	if (_eventListener)
		_eventListener->OnClassNameChanged(cg);

	return true; 
}

bool ClassManager::SetStartClass(Class *cg) 
{ 
	if (cg == _clazzStart)
		return true; // No change!
	if (cg && GetClass(cg->GetID()) != cg)
		return false; // No valid cg!
	_clazzStart = cg; 
	if (cg) {
		bool b = cg->GetDocument()->SetStartClass(cg);
		assert(b);
	}
	if (_eventListener)
		_eventListener->OnStartClassSet(_clazzStart);
	return true;
}

void ClassManager::Run()
{
	if (_clazzStart)
		_clazzStart->Run();
}

void ClassManager::DestroyDeviceObjects()
{
	for (const auto &n : _clazzById)
		n.second->OnDestroyDevice();
}

void ClassManager::OnReleasingBackBuffer(RenderWindow *rw, uint32 index)
{
	for (const auto &n : _clazzById)
		n.second->OnReleasingBackBuffer(rw);
}

bool ClassManager::RegisterSerializedInstance(ClassInstance *instance)
{
	const ClassInstanceSerialization *s = instance->GetSerialization();
	assert(s);

	auto n = _serializedInstances.find(s->clazzid);
	if (n == _serializedInstances.end())
		n = _serializedInstances.insert(std::make_pair(s->clazzid, ClassInstancePtrByGUIDMap())).first;
	auto m = n->second.find(instance->GetID());
	if (m != n->second.end())
		return false; // instance-id already exist...
	n->second.insert(std::make_pair(instance->GetID(), instance));
	return true;
}

void ClassManager::UnregisterSerializedInstance(ClassInstance *instance)
{
	const ClassInstanceSerialization *s = instance->GetSerialization();
	assert(s);
	auto n = _serializedInstances.find(s->clazzid);
	if (n != _serializedInstances.end()) {
		n->second.erase(instance->GetID());
		if (n->second.empty())
			_serializedInstances.erase(n);
	}
}

ClassInstance *ClassManager::GetSerializedInstance(const Guid &cgID, const Guid &instanceID)
{
	auto n = _serializedInstances.find(cgID);
	if (n != _serializedInstances.end()) {
		auto m = n->second.find(instanceID);
		if (m != n->second.end())
			return m->second;
	}
	return nullptr;
}

ClassInstancePtrByGUIDMap ClassManager::CaptureSerializedInstances(const Guid &cgID)
{
	auto n = _serializedInstances.find(cgID);
	if (n != _serializedInstances.end()) { 
		ClassInstancePtrByGUIDMap r = std::move(n->second);
		_serializedInstances.erase(n);
		return r; // perfect forwarding!
	}
	return ClassInstancePtrByGUIDMap();
}

void ClassManager::_addClazz(Class *cg)
{
	bool b1 = _clazzByName.insert(std::make_pair(cg->GetName(), cg)).second; // Should always return true!
	bool b2 = _clazzByGuid.insert(std::make_pair(cg->GetGuid(), cg)).second; // Should always return true!
	bool b3 = _clazzById.insert(std::make_pair(cg->GetID(), cg)).second; // Should always return true!
	assert(b1 && b2 && b3);

	cg->OnAddedToClassManager(); // Let the cg know it was added!

	if (_eventListener)
		_eventListener->OnClassAdded(cg);
}

Chip *ClassManager::FindChip(ChipID chipID, ClassID cgID)
{
	if (chipID != InvalidChipID) {
		if (cgID == InvalidClassID) {
			for (const auto &n : _clazzById) {
				Chip *c = n.second->FindChip(chipID);
				if (c)
					return c;
			}
		}
		else {
			Class *cg = GetClass(cgID);
			if (cg)
				return cg->FindChip(chipID);
		}
	}
	return nullptr;
}

ClassInstance *ClassManager::FindInstance(ClassInstanceID instanceID, ClassID cgID)
{
	Class *c = _clazzById.find(cgID)->second;
	if (!c)
		return nullptr; // Class not found!

	for (const auto &n : c->GetInstances())
		if (n->GetRuntimeID() == instanceID)
			return n;

	return nullptr;
}
