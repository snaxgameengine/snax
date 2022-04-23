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
#include "M3DCore/Containers.h"
#include "M3DCore/GuidUtil.h"
#include "M3DCore/Path.h"
#include "Class.h"



namespace m3d
{

typedef Map<String, Class*> ClassPtrByStringMap;
typedef Map<Guid, Class*> ClassPtrByGUIDMap;
typedef Map<ClassID, Class*> ClassPtrByClassIDMap;
typedef Map<Guid, ClassInstancePtrByGUIDMap> ClassInstancePtrByGUIDMapByGUIDMap;


class M3DENGINE_API ClassManagerEventListener
{
public:
	virtual void OnClassAdded(Class *clazz) = 0;
	virtual void OnClassRemoved(Class *clazz) = 0;
	virtual void OnClassNameChanged(Class *clazz) = 0;
	virtual void OnStartClassSet(Class *clazzStart) = 0;
	virtual bool ResolveNameConflict(String &clazzName) = 0;
	virtual bool ConfirmGuidChange(String clazzName) = 0;
};


class M3DENGINE_API ClassManager
{
public:
	ClassManager();
	~ClassManager();

	// Removes everything from the manager.
	void Clear2();
	// Sets the event listener. 
	void SetEventListener(ClassManagerEventListener *listener) { _eventListener = listener; }

	// Set the class factory. 
	void SetClassFactory(ClassFactory *f) { _factory = f; }
	// Get the class factory.
	ClassFactory *GetClassFactory() const { return _factory; }

	// Returns the classes by name.
	const ClassPtrByStringMap &GetClasssByName() const { return _clazzByName; }
	// Gets a class with the given name, or nullptr if none exist.
	Class *GetClass(String name) const;
	// Gets a class with the given guid, or nullptr if none exist.
	Class *GetClass(const Guid &guid) const;
	// Gets a class with the given id, or nullptr if none exist.
	Class *GetClass(ClassID id) const;
	// Adds a new class with the given name to the manager. Guid will be generated. Event Listener will be called if conflict. Returns nullptr if fail.
	Class *CreateClass(String name, Document *doc);
	// Adds a new class with the given name and guid. Event Listener will be called if conflict. Returns nullptr if fail.
	Class *CreateClass(String name, Guid guid, Document *doc);
	// Removes the given class from the manager.
	bool RemoveClass(Class *clazz);
	// Gives a class a new name.
	bool SetClassName(Class *clazz, String name);

	// Returns the class, and finishes loading if the document is not yet finished loading.
	Class* GetClassAndFinishLoading(const Guid& guid);
	// Return true if the class exist, and updates the file-name for the document.
	bool HasClass(String name, Path &fileName) const;
	// Return true if the class exist, and updates the file-name for the document.
	bool HasClass(const Guid &guid, Path& fileName) const;


	// Gets the class where project is to start running.
	Class *GetStartClass() const { return _clazzStart; }
	// Sets the class where project is to start running.
	bool SetStartClass(Class *clazz);
	// Runs the project!
	void Run();
	// Called by engine when device is destroyed. Distributed to classes.
	void DestroyDeviceObjects();
	// Called by engine when back buffer is released. Distributed to classes.
	void OnReleasingBackBuffer(RenderWindow *rw, uint32 index = 0);
	// Called by ClassInstance only!
	bool RegisterSerializedInstance(ClassInstance *instance);
	// Called by ClassInstance only!
	void UnregisterSerializedInstance(ClassInstance *instance);
	// Called by ClassInstance only!
	ClassInstance *GetSerializedInstance(const Guid &clazzID, const Guid &instanceID);
	// Called by Class only!
	ClassInstancePtrByGUIDMap CaptureSerializedInstances(const Guid &clazzID);

	// Called by DocumentManager when a new document loaded. We are to extract the classes!
	bool OnDocumentLoaded(Document *doc);

	// Searches for a chip. ClassID is optional.
	Chip *FindChip(ChipID chipID, ClassID clazzID = InvalidClassID);
	// Searches for an instance. ClassID is optional.
	ClassInstance *FindInstance(ClassInstanceID instanceID, ClassID clazzID = InvalidClassID);

private:
	// The factory used when creating classes!
	ClassFactory *_factory;
	// Classes ordered by name.
	ClassPtrByStringMap _clazzByName;
	// Classes ordered by guid.
	ClassPtrByGUIDMap _clazzByGuid;
	// Classes ordered by id.
	ClassPtrByClassIDMap _clazzById;
	// The start clazz of the project.
	Class *_clazzStart;

	// This is a map containing serialized instances of classes not completly loaded.
	ClassInstancePtrByGUIDMapByGUIDMap _serializedInstances;

	ClassManagerEventListener *_eventListener;

	Set<Class*> _classesBeingLoaded;

	bool _confirmNameAndGuid(String &name, Guid &guid);
	void _addClazz(Class *clazz);


};

}


