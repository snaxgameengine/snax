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

#include "GlobalDef.h"
#include "M3DCore/Path.h"
#include "M3DCore/Containers.h"



namespace m3d
{

class Class;
class ClassFactory;
class Document;

typedef List<Class*> ClassPtrList;

class M3DENGINE_API DocumentEventListener
{
public:
	virtual void ClassMoved(Class *clazz, Document *oldDoc, Document *newDoc) = 0;
	virtual void DirtyFlagChanged(Document *doc) = 0;

};


class M3DENGINE_API Document
{
public:
	Document(ClassFactory *factory);
	~Document();

	void SetEventListener(DocumentEventListener *listener) { _eventListener = listener; }

	Class *CreateClass();
	bool MoveClass(Class *clazz);
	bool RemoveClass(Class *clazz);

	const ClassPtrList &GetClasss() const { return _clazzList; }

	bool IsDirty() const { return _dirty; }
	void SetDirty(bool dirty = true);

	bool IsReadOnly() const { return _readOnly; }
	void SetReadOnly(bool readOnly) { _readOnly = readOnly; }

	Path GetFileName() const { return _filename; }
	void SetFileName(Path filename);

	Class *GetStartClass() const { return _startClass; }
	bool SetStartClass(Class *clazz);

	int32 GetRecoverySaveTime() const { return _recoverySaveTime; }
	void ClearRecoveryFlag() { _recoverySaveTime = 0; }
	Path GetRecoveryFileName() const { return _recoveryFile; }
	void SetRecoveryFileName(Path p) { _recoveryFile = p; }

	bool IsAllowLibraryUpdate() const { return _allowLibraryUpdate; }
	void SetAllowLibraryUpdate(bool b) { _allowLibraryUpdate = b; }

private:
	// Event listener
	DocumentEventListener *_eventListener;
	// Factory for creating classes.
	ClassFactory *_factory;
	// List of classes in this document. Load/save is in order of list.
	ClassPtrList _clazzList;
	// File name to be accossiated with document.
	Path _filename;
	// Read-only flag to indicate if we can save to _filename. If a doc loads unnormally (not loading all classes) we should set this to true to warn user.
	bool _readOnly;
	// true if we we contain changes that need to be saved. All classes have their own flags as well, but we have a local flag here in case we remove a class.
	bool _dirty;
	// The entry point for this document.
	Class *_startClass;
	// In N seconds from this time, we should do a recovery save.
	int32 _recoverySaveTime;
	// File that recovery is saved to. Delete when exiting normally and after successful save of class!
	// When loading a document in editor, we first check for a xxx.backup-file and ask if user wanna load it.
	Path _recoveryFile;
	// When in editor, and this is a library file, wether or not the user has allowed to save this file (ie edit a library).
	bool _allowLibraryUpdate = false;

	bool _removeClass(Class *clazz);

};



}