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
#include "ClassInstanceRef.h"
#include "ClassInstance.h"
#include "DocumentSaveLoadUtil.h"
#include "StdChips/InstanceData.h"
#include "Class.h"
#include "Engine.h"
#include "ClassManager.h"
#include "DocumentManager.h"
#include "Document.h"
#include "Environment.h"

using namespace m3d;

enum class __SerializationType { NIL, REF, OWNER };


ClassInstanceRef::ClassInstanceRef() : _instance(nullptr), _owner(false) 
{
}

ClassInstanceRef::ClassInstanceRef(ClassInstance *instance, bool owner) : _instance(instance), _owner(owner && _instance != nullptr)
{ 
	if (_instance) 
		_instance->_addRef(this); 
}

ClassInstanceRef::ClassInstanceRef(const ClassInstanceRef &rhs) : _instance(rhs._instance), _owner(rhs._owner)
{ 
	if (_instance) 
		_instance->_addRef(this); 
}

ClassInstanceRef::ClassInstanceRef(const ClassInstanceRef &rhs, bool owner) : _instance(rhs._instance), _owner(owner && _instance != nullptr)
{
	if (_instance) 
		_instance->_addRef(this); 
}

ClassInstanceRef::~ClassInstanceRef() 
{ 
	if (_instance) 
		_instance->_removeRef(this); 
}

ClassInstanceRef &ClassInstanceRef::operator=(const ClassInstanceRef &rhs) 
{ 
	if (this == &rhs)
		return *this;
	if (_instance) 
		_instance->_removeRef(this); 
	_instance = rhs._instance; 
	_owner = rhs._owner;
	if (_instance) 
		_instance->_addRef(this); 
	return *this; 
}

void ClassInstanceRef::SetOwner(bool owner)
{
	if (!_instance)
		return;
	if (owner) {
		for (const auto &n : _instance->_ref)
			n->_owner = false;
	}
	_owner = owner;
}

void ClassInstanceRef::Reset() 
{ 
	if (_instance) {
		ClassInstance *tmp = _instance; // Got to do it like this because our destructor may be called by _removeRef(this)
		_instance = nullptr;
		_owner = false;
		tmp->_removeRef(this); 
	}
}

bool ClassInstanceRef::Serialize(DocumentSaver &saver) const
{
	assert(saver.GetCurrentChip());

	__SerializationType st = _instance ? (_owner ? __SerializationType::OWNER : __SerializationType::REF) : __SerializationType::NIL;

	SAVE(MTEXT("type"), (uint32)st);

	if (!_instance)
		return true;

	assert(saver.GetCurrentChip()->GetClass());

	Path filename;
	Path ourFilename = saver.GetCurrentChip()->GetClass()->GetDocument()->GetFileName();

	bool owner = st == __SerializationType::OWNER;

	if (owner)
		SAVE(MTEXT("name"), _instance->GetName());

	SAVE(MTEXT("id"), _instance->GetID());
	if (_instance->GetSerialization()) { // Is the instance only serialized?
		SAVE(MTEXT("cgid"), _instance->GetSerialization()->clazzid);
		if (owner) { // are we the owner?
			assert(_instance->GetOwner());
			SAVE(MTEXT("data"), _instance->GetSerialization()->data);
			filename = _instance->GetSerialization()->filename; // filename is now the filename of the class of the instance
		}
		else {
			if (_instance->GetOwner()) {
				filename = _instance->GetOwner()->GetClass()->GetDocument()->GetFileName(); // filename is now the filename of the owner of the instance.
			}
			else {
				filename = _instance->GetSerialization()->filename; // filename is now the filename of the owner of the instance.
			}
		}
	}
	else {
		SAVE(MTEXT("cgid"), _instance->GetClass()->GetGuid());
		if (owner) { // If we are the owner we have to serialize the data.
			ChipPtrByGUIDMap dataMap;
			for (const auto &n : _instance->GetData())
				dataMap.insert(std::make_pair(n.first->GetGlobalID(), n.second));
			SAVE(MTEXT("data"), dataMap);
			filename = _instance->GetClass()->GetDocument()->GetFileName(); // filename is now the filename of the class of the instance
		}
		else {
			assert(_instance->GetOwner());
			filename = _instance->GetOwner()->GetClass()->GetDocument()->GetFileName(); // filename is now the filename of the owner of the instance.
		}
	}

	String filenameStr;

	if (filename.IsFile())
		filenameStr = saver.GetEnvironment()->CreateDocumentPath(filename, ourFilename);

	SAVE(MTEXT("filename"), filenameStr);

	return true;
}



bool ClassInstanceRef::Deserialize(DocumentLoader &loader)
{
	__SerializationType st;

	Reset();

	LOAD(MTEXT("type"), (uint32&)st);
	if (st == __SerializationType::NIL)
		return true;

	Chip *chip = loader.GetCurrentChip();
	assert(chip);
	ClassInstanceRefOwner *ow = dynamic_cast<ClassInstanceRefOwner*>(chip);
	//assert(ow);
	Class *cg = chip->GetClass();
	assert(cg);

	Guid id, cgid;
	String filenameStr;
	Path filename;
	Path ourFilename = cg->GetDocument()->GetFileName();
	ChipPtrByGUIDMap dataMap;
	String name;

	bool owner = st == __SerializationType::OWNER;

	LOAD(MTEXT("id"), id);
	LOAD(MTEXT("cgid"), cgid);
	LOAD(MTEXT("filename"), filenameStr);
	if (owner) {
		LOAD(MTEXT("name"), name);
		LOAD(MTEXT("data"), dataMap);
	}
	
	*this = ClassInstanceRef(loader.GetInstance(id), owner);

	if (!filenameStr.empty())
		filename = loader.GetEnvironment()->ResolveDocumentPath(filenameStr, ourFilename);

	// 3/20: Moved here from bottom of file!
	if (filename.IsFile() && loader.IsLoadRelatedDocumentsAsync() && ow && ow->isPreload()) {
		// cgid is the id of the class the instance is a type of. Only provide it to PreloadDocument if we are the owner, as the filename is for the owning reference...
		Path fn = engine->GetDocumentManager()->PreloadDocument(filename, owner ? &cgid : nullptr); // Start loading of target...
		if (fn.IsFile() && fn != filename) {// The file we are actually loading is different from the one we expected?
			if (owner)
				msg(WARN, strUtils::ConstructString(MTEXT("ClassInstanceRef in class \'%1\' (%4) tried to load class \'%3\' for instance from \'%2\', but found it in \'%5\'. Please resave!")).arg(cg->GetName()).arg(filename.AsString()).arg(GuidToString(cgid)).arg(ourFilename.GetName()).arg(fn.AsString()));
			else
				msg(WARN, strUtils::ConstructString(MTEXT("ClassInstanceRef in class \'%1\' (%4) tried to load owning reference from \'%2\', but found it in \'%5\'. Please resave!")).arg(cg->GetName()).arg(filename.AsString()).arg(GuidToString(cgid)).arg(ourFilename.GetName()).arg(fn.AsString()));
			filename = fn;
			cg->SetDirty(); // Mark class as dirty so we can encourage user to save it with the updated filename!
		}
		else if (!fn.IsFile()) {
			chip->AddMessage(Chip::FileNotFoundException(filename));
		}
	}

	if (_instance) { // instance already exist? (we found it earlier while loading this class. It may be ref-only though!)
		if (owner) { // are we an owner?
			if (_instance->GetOwner()) {
				Reset();
				return false; // This should not happen! Two owners! Something must be wrong in the document we are loading!
			}
			_instance->_serialization->data = std::move(dataMap);
			_instance->SetOwner(chip);
			_instance->SetName(name);
			_instance->_serialization->filename = filename; // Update filename to the target class we're instance of!
		}
		else  {
			// Update filename
			if (!_instance->GetOwner() && filename.IsFile() && ourFilename.IsValid() && !_instance->GetSerialization()->filename.IsValid()) 
				_instance->_serialization->filename = filename; // Update filename! It should now be the filename of the owner of the instance!
		}
	}
	else {
		if (owner)
			*this = ClassInstanceRef(mmnew ClassInstance(id, cgid, filename, std::move(dataMap), chip, name), true);
		else
			*this = ClassInstanceRef(mmnew ClassInstance(id, cgid, filename), false);
		
		assert(_instance != nullptr);

		loader.AddInstance(_instance);
	}

	return true;
}

bool ClassInstanceRef::Prepare(Chip *msgChip) const
{
	// NOTE: I've placed this stuff in the reference because when dealing with serialized instances, they may be deleted during this process, which complicates stuff if this is placed in the instance itself.
	if (!_instance)
		return false; // No instance!
	if (_instance->IsSerialized()) { // Instance is serialized?
		if (_instance->GetOwner()) // Serialized with owner?
			return _instance->_deserialize(); // try to deserialize. 
		if (_instance->GetSerialization()->filename.IsFile()) { // Got file name to owner?
			Document *doc = engine->GetDocumentManager()->GetDocument(_instance->GetSerialization()->filename); // Try to load owner
			if (!doc)
				msgChip->AddMessage(Chip::DocumentNotLoadedException(_instance->GetSerialization()->filename));
			if (_instance && _instance->IsSerialized() && _instance->GetOwner())
				return _instance->_deserialize(); // Owner loaded. Try to deserialize. 
		}
		if (msgChip)
			msgChip->AddMessage(Chip::InstanceNotFoundException(_instance->GetID(), _instance->GetSerialization()->filename));
		return false; // Serialized by ref-only, and we do not have file name of owner!
	}
	return true; // Instance is not serialized. Ready to use!
}


ClassInstanceRef ClassInstanceRef::copiedRef = ClassInstanceRef();

