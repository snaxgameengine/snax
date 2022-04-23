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
#include "M3DCore/Containers.h"
#include "M3DCore/Path.h"



namespace m3d
{

class Class;
class ClassInstanceRef;
class Chip;
class InstanceData;
class ProjectDependencies;
class RenderWindow;

typedef Map<InstanceData*, Chip*> ChipPtrByInstanceDataPtrMap;
typedef Map<Guid, Chip*> ChipPtrByGUIDMap;

struct ClassInstanceSerialization
{
	// global id of the class we're instance of
	Guid clazzid;
	// file name to the owner (if refOnly, ie no owner) or the class we're instance of
	Path filename;
	// serialized data (only if not refOnly)
	ChipPtrByGUIDMap data;
	// true if registered at clazz-manager.	
	bool atManager;
};

class ClassInstance;

typedef Set<ClassInstanceRef*> ClassInstanceRefPtrSet;

class M3DENGINE_API ClassInstanceReleaseCallback
{
public:
	virtual void OnRelease(ClassInstance *instance) = 0;
};


class M3DENGINE_API ClassInstance
{
	friend class Class;
	friend class ClassInstanceRef;
protected:
	// This is the id used to identify this instance through save/load.
	Guid _id;
	// A unique id used to identify this instance at runtime.
	ClassInstanceID _runtimeID;
	// Name of object to make it easier for user to identify it.
	String _name;
	// The class this instance represent. May be nullptr if serialized.
	Class *_clazz;
	// Mapping from the classes InstanceData-chips to our instances of them. Only if not serialized!
	ChipPtrByInstanceDataPtrMap _instanceData;
	// All the references to this instance.
	ClassInstanceRefPtrSet _ref;
	// The chip that is the owner of the instance. It is used when serializing references. (To get the owners class file name)
	Chip *_owner;
	// The serialized version of this instance. Should only be non-null after deserialization.
	ClassInstanceSerialization *_serialization;
	// true if we shoud wait with destruction even if refCount is 0. Used by FunctionStack.
	bool _delayDestruction;
	// Callback for release notification. Used by editor.
	ClassInstanceReleaseCallback *_releaseCallback;

	// Internal!
	void _initInstanceData(Class *clazz, ChipPtrByGUIDMap *fromSerialization = nullptr);
	// Class will call these methods when instance data is added.
	void _addInstanceData(InstanceData *instanceData);
	// Class will call these methods when instance data is removed.
	void _removeInstanceData(InstanceData *instanceData);
	// Called by ClassInstanceRef only!
	void _addRef(ClassInstanceRef *ref);
	// Called by ClassInstanceRef only!
	void _removeRef(ClassInstanceRef *ref);
	// Called by ClassInstanceRef only!
	bool _deserialize();

	// Creates a serialized instance (refOnly).
	ClassInstance(const Guid &id, const Guid &clazzid, Path filename);
	// Creates a serialized instance.
	ClassInstance(const Guid &id, const Guid &clazzid, Path filename, ChipPtrByGUIDMap &&data, Chip *owner, String name);
	// Creates the instance. Is the Create()-method!
	ClassInstance(Class *clazz, Chip *owner);
	// Clone!
	ClassInstance(ClassInstance *original, Chip *ownerOfNewInstance);
public:
	// Use Release() instead of mmdelete!
	~ClassInstance();
	// Creates an instance of the given class.
	static ClassInstanceRef Create(Class *clazz, Chip *owner);
	// Call on destruction! This will clear all references to us!
	void Release() const;
	// Makes a clone of this instance.
	ClassInstanceRef Clone(Chip *ownerOfNewInstance);
	// Returns the id.
	const Guid &GetID() const { return _id; }
	// returns the runtime id.
	ClassInstanceID GetRuntimeID() const { return _runtimeID; }
	// Name of object.
	String GetName() const { return _name; }
	// Set name of object.
	void SetName(String name);
	// Returns the class we represent. May be nullptr if serialized.
	Class *GetClass() const { return _clazz; }
	// Gets our instance data for the given InstanceData-chip.
	Chip* GetData(InstanceData* instanceData) { auto itr = _instanceData.find(instanceData); return itr != _instanceData.end() ? itr->second : nullptr; }
	// Returns map of all instance data.
	const ChipPtrByInstanceDataPtrMap &GetData() const { return _instanceData; }
	// Gets the chip that owns this instance.
	Chip *GetOwner() const { return _owner; }
	// Sets the owner of this instance. Should not be null!
	void SetOwner(Chip* owner);
	// Gets the serialization for this instance, if it exist.
	const ClassInstanceSerialization *GetSerialization() const { return _serialization; }
	// true if serialized. 
	bool IsSerialized() const { return _serialization != nullptr; }
	// Called when owning class is done loading and added to manager. NOTE: The instance may be deleted during this process!
	void CompleteLoading(bool changeID);
	// Used by FunctionStack.
	void SetDelayDestruction(bool b);
	// Set callback for release notification. Used by editor.
	void SetReleaseCallback(ClassInstanceReleaseCallback *cb) { _releaseCallback = cb; }
	
	// Functions for notifying our instance data.
	void OnDestroyDevice();
	void OnReleasingBackBuffer(RenderWindow *rw);
	void RestoreChip();
	void AddDependencies(ProjectDependencies &deps);
	Chip *FindChip(ChipID chipID);

};





}