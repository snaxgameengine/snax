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


namespace m3d
{

class ClassInstance;
class DocumentSaver;
class DocumentLoader;
class Chip;

class M3DENGINE_API ClassInstanceRefOwner
{
public:
	ClassInstanceRefOwner() : _preload(true) {}
	virtual bool isPreload() const { return _preload; }
	virtual void setPreload(bool b) { _preload = b; }
protected:
	bool _preload;
};

// The ClassInstance have control over all references. When it is destroyed, all references are cleard!
// This also happens if the class is destroyed.
// NOTE: Reference counting will delete the instance when it contains no more references. 
// The instance can also be deleted at any time using the Release()-method. All references will then be cleared!
class M3DENGINE_API ClassInstanceRef
{
protected:
	ClassInstance *_instance;
	// true if this reference is supposed to be the owner of the instance. It will check this flag when serializing. 
	// There should only be one owning reference, but when assigning the reference, the flag will follow.
	// Its up to the chips using this to not mess it up!
	bool _owner;

public:
	ClassInstanceRef();
	ClassInstanceRef(ClassInstance *instance, bool owner);
	ClassInstanceRef(const ClassInstanceRef &rhs);
	ClassInstanceRef(const ClassInstanceRef &rhs, bool owner);
	~ClassInstanceRef();

	ClassInstanceRef &operator=(const ClassInstanceRef &rhs);

	bool IsOwner() const { return _owner; }
	void SetOwner(bool owner);

	void Reset();

	bool Prepare(Chip *msgChip = nullptr) const;

	// Note: Before using the instance, Prepare() should be called. If it return false, the instance could not be deserialized yet!
	operator bool() const { return _instance != nullptr; }
	bool operator==(const ClassInstanceRef &rhs) const { return _instance == rhs._instance; }
	bool operator!=(const ClassInstanceRef &rhs) const { return _instance != rhs._instance; }
	ClassInstance *operator->() const { return _instance; }

	// Serialize will save as owner if _owner=true.
	bool Serialize(DocumentSaver &saver) const;
	bool Deserialize(DocumentLoader &loader);

	// Do not access. It is to be used by performance critical code in FunctionStack!
	ClassInstance *GetRawPtr() { return _instance; }

	// In the editor, the user can copy/paste references. The copied reference is stored here.
	static ClassInstanceRef copiedRef;

};

static bool SerializeDocumentData(DocumentSaver &saver, const ClassInstanceRef &data) { return data.Serialize(saver); }
static bool DeserializeDocumentData(DocumentLoader &loader, ClassInstanceRef &data) { return data.Deserialize(loader); }




}