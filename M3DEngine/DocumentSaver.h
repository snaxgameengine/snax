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
#include "M3DCore/MString.h"
#include "M3DCore/GuidUtil.h"
#include "M3DCore/Containers.h"
#include "M3DCore/Path.h"
#include "DocumentTags.h"
#include "Function.h"
#include "Class.h"


namespace m3d
{

class DataBuffer;

template<typename T>
struct SaveDataT
{
	static bool Serialize(DocumentSaver& saver, const T& t);
};

template<typename T>
struct SaveDataT<T*>
{
	static bool Serialize(DocumentSaver& saver, const T* t);
	static bool Serialize(DocumentSaver& saver, const T* t, uint32 size);
};


// Saves the data. Can be used togehter with LOAD.
#define SAVE(name, value) if (!saver.SaveData(name, value)) return false;
// Saves the data (array). Can be used together with LOADARRAY.
#define SAVEARRAY(name, value, size) if (!saver.SaveData(name, value, size)) return false;
// Saves the data if not equal to dvalue. Can be used together with LOADDEF or LOADPREDEF.
#define SAVEDEF(name, value, dvalue) if (value != dvalue && !saver.SaveData(name, value)) return false;
// Saves the data IF saving editor data only. (Can be used for saving data only needed in the editor (eg shader source code.).  Can be used together with LOADDEF or LOADPREDEF.
#define SAVEOPT(name, value) if (saver.IsSaveEditorData() && !saver.SaveData(name, value)) return false;
// Saves the data IF saving editor data AND not equal to dvalue only. Can be used together with LOADDEF or LOADPREDEF.
#define SAVEDEFOPT(name, value, dvalue) if (saver.IsSaveEditorData() && value != dvalue && !saver.SaveData(name, value)) return false;

typedef Set<ChipTypeIndex> ChipTypeIndexSet;

class M3DENGINE_API DocumentSaver
{
private:
	// true if this is a 'chip'-saver (for parallel execution) of another master saver.
	const bool _chipSaver; 
	// true if we are to save editor data.
	bool _saveEditorData;
	// true if we should use multithreading.
	bool _useMultithreading;
	// The compression level to be used.
	DocumentCompressionLevel _compression;
	// This set contains all chip types encountered.
	ChipTypeIndexSet _chipTypes;
	// true if we currently are writing an attribute. (Pr thread variable)
	bool _writeAttr;
	// the attribute we are currently writing. (Pr thread variable)
	DocumentTags::Tag _attrTag;
	// The Chip currently being loaded. (Pr thread variable)
	Chip *_currentChip;

protected:
	DocumentSaver() : _chipSaver(false), _saveEditorData(true), _useMultithreading(true), _compression(DocumentCompressionLevel::DCL_NONE), _writeAttr(false), _attrTag((DocumentTags::Tag)0), _currentChip(nullptr) {}

	DocumentSaver(DocumentSaver *parent) : _chipSaver(true), _saveEditorData(parent->_saveEditorData), _useMultithreading(false), _compression(parent->_compression), _writeAttr(false), _attrTag((DocumentTags::Tag)0), _currentChip(nullptr) {}

	bool _finalize();

	inline bool IsWriteAttribute() const { return _writeAttr; }
	inline DocumentTags::Tag GetAttributeTag() const { return _attrTag; }
	inline ChipTypeIndexSet &GetChipTypes() { return _chipTypes; }

public:
	virtual ~DocumentSaver() { }

	// true if we are to save editor data like formatting, comments, shortcuts etc. 
	// Chips can use this flag to skip saving stuff not neccessary outside the editor (eg. shader source code)
	bool IsSaveEditorData() const { return _saveEditorData; }
	void SetSaveEditorData(bool doSave) { _saveEditorData = doSave; }

	// Get/Set if we should use multithreading to speed up things (default)!
	bool IsUsingMultithreading() const { return _useMultithreading; }
	void SetUseMultithreading(bool b) { _useMultithreading = b; }

	// Get/Set if we should apply compression to the saved document. For XML-docs, compression is applied to binary data.
	void SetCompressionLevel(DocumentCompressionLevel cl) { _compression = cl; }
	DocumentCompressionLevel GetCompressionLevel() const { return _compression; }

	class Environment *GetEnvironment() const;

	// true if this is a temporary saver run in a worker thread for saving one single chip only.
	bool IsChipSaver() const { return _chipSaver; }

	bool SaveClasss(Document *doc);
	bool SaveClass(Class *clazz);
	// Save the chips in the given map. See also comment for SaveChip().
	// NOTE: This function is virtual so that derived savers can create their own parallel implementation!
	virtual bool SaveChips(const ChipPtrByChipIDMap&chips);
	// Saves the given chip. Does ONLY try to save the children IF chips is not null. Saves a child IF included in the chips-map.
	bool SaveChip(Chip *chip, const ChipPtrByChipIDMap*chips);

	bool AddInstance(const Guid &instanceID);


//	Class *GetCurrentClass() const { return _currentClazz; }
	Chip *GetCurrentChip() const { return _currentChip; } 

	// This template allows for saving any type user defined data type.
	template<typename T>
	bool SaveData(String id, const T &data)
	{
		bool ok = true;
		ok = ok && PushGroup(DocumentTags::Data);
		ok = ok && SetAttribute(DocumentTags::id, id);
		ok = ok && WriteData(data); 
		ok = ok && PopGroup(DocumentTags::Data);
		return ok;
	}

	// This template allows for saving of array of any user defined type.
	template<typename T>
	bool SaveData(String id, const T *data, uint32 size)
	{
		bool ok = true;
		ok = ok && PushGroup(DocumentTags::Data);
		ok = ok && SetAttribute(DocumentTags::id, id);
		ok = ok && WriteData(data, size);
		ok = ok && PopGroup(DocumentTags::Data);
		return ok;
	}

	template<typename T>
	bool SetAttribute(DocumentTags::Tag tag, const T &data)
	{
		_attrTag = tag; 
		_writeAttr = true;
		bool ok = WriteData(data);
		_writeAttr = false;
		return ok;
	}

	template<typename T>
	bool SetAttribute(DocumentTags::Tag tag, const T *data, uint32 size)
	{
		_attrTag = tag; 
		_writeAttr = true;
		bool ok = WriteData(data, size);
		_writeAttr = false;
		return ok;
	}

	// Returns true if the saver is binary. This allows for certain optimizations when saving user defined types.
	virtual bool IsBinary() const = 0;

	virtual bool IsJson() const { return false; }

	virtual bool Initialize() = 0;

	virtual bool SaveToFile(Path fileName, DocumentEncryptionLevel encryptionLevel = DocumentEncryptionLevel::ENCLEVEL0, Guid signature = NullGUID) = 0;
	virtual bool SaveToMemory(DataBuffer &databuffer, DocumentEncryptionLevel encryptionLevel = DocumentEncryptionLevel::ENCLEVEL0, Guid signature = NullGUID) = 0;

	virtual bool PushGroup(DocumentTags::Tag group) = 0;
	virtual bool PopGroup(DocumentTags::Tag group) = 0;

	// Primitive data types supported
	virtual bool WriteData(String data) = 0;
	virtual bool WriteData(bool data) = 0;
	virtual bool WriteData(float32 data) = 0;
	virtual bool WriteData(float64 data) = 0;
	virtual bool WriteData(int8 data) = 0;
	virtual bool WriteData(uint8 data) = 0;
	virtual bool WriteData(int16 data) = 0;
	virtual bool WriteData(uint16 data) = 0;
	virtual bool WriteData(int32 data) = 0;
	virtual bool WriteData(uint32 data) = 0;
	virtual bool WriteData(int64 data) = 0;
	virtual bool WriteData(uint64 data) = 0;
	virtual bool WriteData(const void *data, uint32 sizeInBytes) = 0;
	virtual bool WriteData(const float32 *data, uint32 numberOfEntries) = 0;
	virtual bool WriteData(const float64 *data, uint32 numberOfEntries) = 0;
	virtual bool WriteData(const int8 *data, uint32 numberOfEntries) = 0;
	virtual bool WriteData(const uint8 *data, uint32 numberOfEntries) = 0;
	virtual bool WriteData(const int16 *data, uint32 numberOfEntries) = 0;
	virtual bool WriteData(const uint16 *data, uint32 numberOfEntries) = 0;
	virtual bool WriteData(const int32 *data, uint32 numberOfEntries) = 0;
	virtual bool WriteData(const uint32 *data, uint32 numberOfEntries) = 0;

	template<typename T>
	bool WriteData(const T &t) { return SaveDataT<T>::Serialize(*this, t); }

	template<typename T>
	bool WriteData(const T *t, uint32 size) { return SaveDataT<T*>::Serialize(*this, t, size); } 
 
};


template<typename T>
bool SaveDataT<T>::Serialize(DocumentSaver& saver, const T& t) { return SerializeDocumentData(saver, t); }

template<typename T>
bool SaveDataT<T*>::Serialize(DocumentSaver& saver, const T* t)
{
	if (t)
		return saver.PushGroup(DocumentTags::Ptr) && SerializeDocumentData(saver, t) && saver.PopGroup(DocumentTags::Ptr);
	return true;
}

template<typename T>
bool SaveDataT<T*>::Serialize(DocumentSaver& saver, const T* t, uint32 size) { return SerializeDocumentData(saver, t, size); }


}