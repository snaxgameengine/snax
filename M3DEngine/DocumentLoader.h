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
#include "M3DCore/GuidUtil.h"
#include "M3DCore/Containers.h"
#include "M3DCore/Path.h"
#include "DocumentTags.h"
#include "Function.h"
#include "Class.h"

namespace m3d
{

template<typename T>
struct LoadDataT
{
	static bool Deserialize(DocumentLoader& loader, T& t);
};


template<typename T>
struct LoadDataT<T*>
{
	static bool Deserialize(DocumentLoader& loader, T*& t);
	static bool Deserialize(DocumentLoader& loader, T* t, uint32 size);
};

class ClassFactory;
class DataBuffer;

struct FunctionMeta
{
	Guid type;
	String name;
	Function::Type ft;
	Function::Access fa;
	String comment;
};

struct ClassMeta
{
	String name;
	bool hasStartchip;
	String description;
	List<FunctionMeta> functions;
};

typedef List<ClassMeta> ClassMetaList;

// Loads data and return false if failed/not found.
#define LOAD(name, data) if (!loader.LoadData(name, data, false)) return false;
// Loads an array and returns false if failed/not found.
#define LOADARRAY(name, data, size) if (!loader.LoadData(name, data, size)) return false; 
// Assigns ddata to data, then try to load and returns false only on error. (If not found, ddata is already set!)
#define LOADDEF(name, data, ddata) if (!loader.LoadData(name, data=ddata, true)) return false;
// Loads data, and returns false only on error. If not found, nothing is set and data is assumed to already contain a default value.
#define LOADPREDEF(name, data) if (!loader.LoadData(name, data, true)) return false;

typedef List<Guid> ParameterList;

struct FunctionDesc
{
	Function::Type ft;
	Function::Access fa;
	ParameterList parameters;
};

struct LMsg
{
	MessageSeverity severity;
	String message;
};

typedef Map<Guid, ChipID> ChipIDByGUIDMap;

struct LoadedConnection
{
	Guid guid;
	bool isMulticonnection;
};

typedef Map<std::pair<uint32, uint32>, LoadedConnection> ConnectionMap;
typedef Map<ChipID, ConnectionMap> ConnectionMapByChipIDMap;
typedef Map<ChipID, FunctionDesc> FunctionDescByChipIDMap;
typedef Map<Shortcut*, Guid> ShortcutPtrByGUIDMap;
typedef Map<Guid, Version> GUIDByVersionMap;
typedef List<LMsg> LMsgList;


class M3DENGINE_API DocumentLoader
{
private:
	// true if we should try to get through loading even if some stuff fails.
	bool _allowIrregularLoading;
	// true if we should keep loaded global id's. false good for copy/paste operation.
	bool _keepGlobalIDs;
	// true if we should optimize away shortcuts.
	bool _optimize;
	// true if we should ask the document manager to start loading releated documents when encountered.
	bool _loadRelatedDocumentsAsync;
	// Updated in LoadChip(). Translation from (loaded) global id to runtime id.
	ChipIDByGUIDMap _idTranslationMap;
	// Updated in LoadChip(). Contains how chips are to be connected to each other. 
	ConnectionMapByChipIDMap _connections;
	// Updated in LoadChip(). Contain which chips are to have function.
	FunctionDescByChipIDMap _functions;
	// Updated indirectly in LoadChip(). It is set to allow instances to register themself in the correct clazz.
	ClassInstancePtrByGUIDMap* _instances;
	// Shortcuts found during loading of a clazz.
	ShortcutPtrByGUIDMap _loadedShortcuts;
	// The version of the document being loaded.
	Version _version;
	// The version of the chip types loaded.
	GUIDByVersionMap _chipVersions;

	bool _readAttr;
	DocumentTags::Tag _attrTag;

	Chip* _currentChip;

	DocumentEncryptionLevel _encLevel;


	// List of messages found during loading.
	LMsgList _messages;

	bool _readConnections(Chip* ch, Guid globalID);
	bool _readFunction(Chip* ch, Guid globalID); 

protected:
	DocumentLoader() : _allowIrregularLoading(true), _keepGlobalIDs(true), _optimize(false), _loadRelatedDocumentsAsync(false), _readAttr(false), _attrTag((DocumentTags::Tag)0), _instances(0), _currentChip(nullptr), _version(0), _encLevel(DocumentEncryptionLevel::ENCLEVEL0) {}

	void _setEncryptionLevel(DocumentEncryptionLevel encLevel) { _encLevel = encLevel; }

	bool _init();

	void lmsg(MessageSeverity severity, String message, const Chip *chip = nullptr);

	// Returns true if we are currently loading an attribute. Opposite to loading a regular data field.
	bool IsReadAttribute() const { return _readAttr; }
	// Returns current attribute tag being loaded.
	DocumentTags::Tag GetAttributeTag() const { return _attrTag; }

public:
	virtual ~DocumentLoader() {}
	// true if noncritical issues are ignored.
	bool AllowingIrregularLoading() const { return _allowIrregularLoading; }
	// set to true if noncritical issues are ignored. false if everything must be according to protocol.
	void SetAllowIrregularLoading(bool allow) { _allowIrregularLoading = allow; }
	// true if global ids (for chips and instances) are to be kept. false are used for copy/paste.
	bool IsKeepGlobalIDs() const { return _keepGlobalIDs; }
	// set to true if global ids (for chips and instances) are to be kept.
	void SetKeepGlobalIDs(bool keep) { _keepGlobalIDs = keep; }
	// Set if we should load base classes async. false by default.
	void SetLoadRelatedDocumentsAsync(bool b) { _loadRelatedDocumentsAsync = b; }
	// true if we should ask document manager to load other documents when references are found.
	bool IsLoadRelatedDocumentsAsync() const { return _loadRelatedDocumentsAsync; }
	// The level of encryption.
	DocumentEncryptionLevel GetEncryptionLevel() const { return _encLevel; }

	class Environment* GetEnvironment() const;

	Version GetDocumentVersion() const { return _version; }
	Version GetChipTypeVersion(const Guid &type) const;

	// Searches for classes and return a list of clazz-names.
	bool SearchForClasses(ClassMetaList& classList, bool includeFunctions);
	// Load chips classes into the given document. 
	bool LoadDocument(Document *doc);
	// Load a class into the given document. *clazz is the loaded class. Can be null if no more found.
	bool LoadClass(Class **clazz, Document *doc);

	// Load classes into a list.
//	bool LoadClasss(List<Class*> &clazzs, ClassFactory *factory, Path filename = Path()); 
	// Fills the given class with the data loaded. filename is the filename to be accossiated with the clazz. 
	// Relative filenames will be relative to it - not to the file the loader loaded from. returns true on success.
//	bool LoadClass(Class *clazz); // TO BE REMOVED
	// Load chips into the class by entering the <chips>-group. Returns true on success.
	// instances is a map that is to be filled with instances found.
	bool LoadChips(Class *clazz, ClassInstancePtrByGUIDMap *instances);
	// Tries to load a chip by entering the next <chip>-group. If clazz!=nullptr the chip will be added to the class.
	// Returns true on success. Note: *chip can be nullptr on success if no more chips are found.
	bool LoadChip(Chip **chip, Class *clazz = nullptr);
	// Translate a loaded chip's *saved* global id to the current chipID. (During copy/paste the saved global id will be replaced!)
	ChipID TranslateChipID(const Guid &globalChipID);
	// Called by ClassInstanceRef during loading.
	void AddInstance(ClassInstance *instance);
	// Called by ClassInstanceRef during loading.
	ClassInstance *GetInstance(const Guid &originalID) const;
	// Returns the current chip being loaded.
	Chip *GetCurrentChip() const { return _currentChip; } 
	// During loading, shortcuts register themself here.
	void RegisterShortcut(Shortcut *sc, Guid guid) { _loadedShortcuts.insert(std::make_pair(sc, guid)); }

	// This template allows for loading of any user defined type.
	template<typename T>
	bool LoadData(String id, T &data, bool optional = false)
	{
		if (id.find_first_of(MCHAR('|')) != String::npos)
		{
			// When refactoring, we can change id by using the '|' symbol, to use multiple identifiers: "ot|operatorType" to check for both "ot" and "operatorType".
			std::basic_stringstream<Char, std::char_traits<Char>, Allocator<Char>> s(id);
			String tmp_id;
			while (std::getline(s, tmp_id, MCHAR('|')))
			{
				if (LoadData(tmp_id, data, false))
					return true;
			}
		}
		else
		{
			if (EnterGroup(DocumentTags::Data, DocumentTags::id, id)) {
				bool readDataOK = ReadData(data);
				return LeaveGroup(DocumentTags::Data) && readDataOK;
			}
		}
		return optional;
	}

	// This template allows for loading of an array of any user defined type.
	template<typename T>
	bool LoadData(String id, T *data, uint32 size, bool optional = false)
	{
		if (id.find_first_of(MCHAR('|')) != String::npos)
		{
			// When refactoring, we can change id by using the '|' symbol, to use multiple identifiers: "ot|operatorType" to check for both "ot" and "operatorType".
			std::basic_stringstream<Char, std::char_traits<Char>, Allocator<Char>> s(id);
			String tmp_id;
			while (std::getline(s, tmp_id, MCHAR('|')))
			{
				if (LoadData(tmp_id, data, size, false))
					return true;
			}
		}
		else
		{
			if (EnterGroup(DocumentTags::Data, DocumentTags::id, id)) {
				bool readDataOK = ReadData(data, size);
				return LeaveGroup(DocumentTags::Data) && readDataOK;
			}
		}
		return optional;
	}

	// This template allows for loading of an attribute of any user defined type.
	template<typename T>
	bool GetAttribute(DocumentTags::Tag tag, T &data)
	{
		struct B // RAII
		{
			bool &b;
			B(bool &b) : b(b = true) {}
			~B() { b = false; }
		};
		B b(_readAttr);
		_attrTag = tag;
		return ReadData(data);
	}

	// This template allows for loading of an attribute of an array of any user defined type.
	template<typename T>
	bool GetAttribute(DocumentTags::Tag tag, T *data, uint32 size)
	{
		struct B // RAII
		{
			bool &b;
			B(bool &b) : b(b = true) {}
			~B() { b = false; }
		};
		B b(_readAttr);
		_attrTag = tag;
		return ReadData(data, size);
	}

	virtual bool IsBinary() const = 0;
	virtual bool IsJson() const { return false; }

	virtual bool OpenFile(Path fileName) = 0;
	virtual bool OpenMemory(DataBuffer &&data) = 0;

	virtual bool Reset();

	virtual bool VerifyGroup(DocumentTags::Tag group) = 0;
	virtual bool EnterGroup(DocumentTags::Tag group) = 0;
	virtual bool EnterGroup(DocumentTags::Tag group, DocumentTags::Tag attribute, String attrValue) = 0;
	virtual bool LeaveGroup(DocumentTags::Tag group) = 0;

	virtual bool ReadData(String &data) = 0;
	virtual bool ReadData(bool &data) = 0;
	virtual bool ReadData(float32 &data) = 0;
	virtual bool ReadData(float64 &data) = 0;
	virtual bool ReadData(int8 &data) = 0;
	virtual bool ReadData(uint8 &data) = 0;
	virtual bool ReadData(int16 &data) = 0;
	virtual bool ReadData(uint16 &data) = 0;
	virtual bool ReadData(int32 &data) = 0;
	virtual bool ReadData(uint32 &data) = 0;
	virtual bool ReadData(int64 &data) = 0;
	virtual bool ReadData(uint64 &data) = 0;
	virtual bool ReadData(void *data, uint32 size) = 0;
	virtual bool ReadData(float32 *data, uint32 size) = 0;
	virtual bool ReadData(float64 *data, uint32 size) = 0;
	virtual bool ReadData(int8 *data, uint32 size) = 0;
	virtual bool ReadData(uint8 *data, uint32 size) = 0;
	virtual bool ReadData(int16 *data, uint32 size) = 0;
	virtual bool ReadData(uint16 *data, uint32 size) = 0;
	virtual bool ReadData(int32 *data, uint32 size) = 0;
	virtual bool ReadData(uint32 *data, uint32 size) = 0;

	template<typename T>
	bool ReadData(T &t) { return LoadDataT<T>::Deserialize(*this, t); }

	template<typename T>
	bool ReadData(T *t, uint32 size) { return LoadDataT<T*>::Deserialize(*this, t, size); }

	static void SetKey(void* key);

};


template<typename T>
bool LoadDataT<T>::Deserialize(DocumentLoader& loader, T& t) { return DeserializeDocumentData(loader, t); }

template<typename T>
bool LoadDataT<T*>::Deserialize(DocumentLoader& loader, T*& t)
{
	if (loader.EnterGroup(DocumentTags::Ptr))
		return DeserializeDocumentData(loader, t) && loader.LeaveGroup(DocumentTags::Ptr);
	return true;
}

template<typename T>
bool LoadDataT<T*>::Deserialize(DocumentLoader& loader, T* t, uint32 size) { return DeserializeDocumentData(loader, t, size); }

}