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
#include "ChipDef.h"
#include "ChildPtr.h"
#include "FunctionStack.h"
#include "M3DCore/DestructionObserver.h"
#include "M3DCore/GUIDUtil.h"
#include "M3DCore/Path.h"



namespace m3d
{

static const Guid CHIP_GUID = { 0x11111111, 0x1111, 0x1111, { 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 } };


class M3DENGINE_API Chip : public DestructionObservable
{
	template<typename T> friend void mmdelete(const T*);
public:
	// These are some common messages.
	CHIPMSG(NoInstanceException, WARN, MTEXT("No instance set!"))
	CHIPMSGV(WrongInstanceException, FATAL, strUtils::ConstructString(MTEXT("The provided instance of class \'%1\' is incompatible with the excepted type \'%2\'!")).arg(given).arg(expected), String given, String expected)
	CHIPMSG(InfiniteLoopException, FATAL, MTEXT("Infinite loop detected. Execution truncated. If this is wrong, please increase the reference count limit."))
	CHIPMSG(StackOverflowException, FATAL, MTEXT("Stack overflow!"))
	CHIPMSG(UninitializedException, WARN, MTEXT("This Chip is not initialized correctly!"))
	CHIPMSGV(MissingChildException, WARN, strUtils::ConstructString(MTEXT("A required child connection (%1) is empty!")).arg(index), uint32 index)
	CHIPMSGV(NotImplementedException, WARN, strUtils::ConstructString(MTEXT("This functionality (%1) is not implemented!")).arg(name), String name)
	CHIPMSG(UnsupportedOperationException, WARN, MTEXT("This operation is not supported!"))
	CHIPMSGV(ClassNotFoundException, WARN, strUtils::ConstructString(MTEXT("Class (%1) not found!")).arg(filename.IsFile() ? filename.GetName() : GuidToString(id)), Guid id, Path filename)
	CHIPMSGV(InstanceNotFoundException, WARN, strUtils::ConstructString(MTEXT("Instance (%1) not found! Last seen in file \'%2\'.")).arg(GuidToString(id)).arg(ownerFilename.GetName()), Guid id, Path ownerFilename)
	CHIPMSGV(DocumentNotLoadedException, WARN, strUtils::ConstructString(MTEXT("Document could not be loaded! Last seen in file \'%1\'.")).arg(fileName.GetName()), Path fileName)
	CHIPMSG(GetChildNotAllowedException, FATAL, MTEXT("A Chip is not allowed to ask for its children from operations performed in the dialog pages."))
	CHIPMSG(DeprecatedFeatureException, WARN, MTEXT("This feature is deprecated and should not be used!"))
	CHIPMSG(ReinitPreventedException, WARN, MTEXT("This Chip is not initialized correctly, and initialization was prevented by refresh mode."))
	CHIPMSGV(FileNotFoundException, WARN, strUtils::ConstructString(MTEXT("This file was not found \'%1\'.")).arg(fileName.GetName()), Path fileName)

		
private:
	// _id is unique for this chip instance in the current process.
	const ChipID _id;
	// _globalID is globally unique for this chip instance. It is used to identify the chip through save/load.
	Guid _globalID;
	// The class this chip belongs to.
	// The class does not neccesarily contain the chip. It may be stored in a chip that is located in the class.
	// _clazz should always be set, except for special cases like global singletons.
	Class *_clazz;
	// The owner of this chip. Normally it will be itself, but if it is contained in another chip, it will be the containing chip. Never nullptr!
	Chip *_owner;
	// Meta data about the chip. Used by the editor.
	ChipEditorData *_editorData;
	// Chip name
	String _name;
	// The ChipTypeIndex corresponding to the type given by GetChipType().
	mutable ChipTypeIndex _typeIndex;
	// Connected children. NOTE: If _childProvider != this, _children
	// will not be used when getting/setting children, because the ChildConnectionList
	// will be provided by another chip. It is not allowed to tweak (remove/add)
	// connections given by the provider, so such changes will still be performed on the
	// local list.
	ChildConnectionList _children;
	// Provider of the ChildConnectionList to be used when setting/getting children.
	// It's const because we're not allowed to tweak the connections.
	// It defaults to "this", but when the chip is used in eg a ClassInstance, it is
	// another chip (the InstanceData) that will provide the children we should use. 
	const Chip *_childProvider;
	// Timestamp of the last time our GetChip() was called. It is used to provide
	// visual feedback of chip activity in the editor.
	int32 _lastHit;
	// The function this chip is/has if any.
	Function *_function;
	// A unique value that a derived chip can set when it is updated. This way, it's easy to see when a chip is updated.
	UpdateStamp _updateStamp;
	// List of chips (ie ShellChip) supposed to have exact the same connections as us. Any connection-modifications are relied to these.
	ChipList _connectionClones;
	// Chip Messages. NOTE: Will probably contain only one or two (or very few!) messages. Preferring list over map for searching! (mutable to be able to add messages from const func.)
	ChipMessageList *_messages;
	
	void _removeChild(Chip *child);
	bool _setChild(Chip *child, uint32 index, uint32 subIndex, bool insert = false);
	void _clearConnections(uint32 fromIndex = 0);
	void _clearConnection(uint32 index);
	void _setConnection(uint32 index, const ChildConnectionDesc &connection, bool keepChildren);
	void _clearMessages();

protected:
	RefreshManager Refresh;

	// Resets the _typeIndex.
	void ResetTypeIndex() { _typeIndex = InvalidChipTypeIndex; }



	// Function called by GetChildren() to get the ChildConnectionList to use.
	// Normally this is our _children, but chips like the TemplateChip
	// overrides it to get the list from its template.
	virtual const ChildConnectionList &ProvideChildren() const { return _children; }

	virtual ~Chip();
	virtual void OnRelease() {}

public:
	Chip();

	// Call release when done with the chip.
	void Release() const;

	// To be overridden by all new chips!
//	virtual const ChipDesc &GetChipDesc() const { return CHIP_DESC; } 

	// These are defined for all chips using the CHIPDESC_DECL macro.
	static const ChipDesc DESC;
	virtual const ChipDesc &GetChipDesc() const { return DESC; }

	// To be overridden by special chips like the ProxyChip. 
	// Remember to call ResetTypeIndex() when changing this.
	// No not change more than once!
	virtual const Guid &GetChipType() const { return GetChipDesc().type; } 

	// Gets the ChipTypeIndex (type of fast runtime guid) for this chip.
	virtual ChipTypeIndex GetChipTypeIndex() const; 

	// Returns the runtime id of this chip.
	ChipID GetID() const { return _id; }

	// Returns the global id of this chip. It is unchanged through save/load.
	const Guid &GetGlobalID() const { return _globalID; }

	// Only to be used by loading functionality!
	void SetGlobalID(const Guid &globalID) { _globalID = globalID; }

	// Get/Set the class this chip belongs to.
	Class *GetClass() const { return _clazz; }
	virtual void SetClass(Class* clazz); // This can be overridden in classes that hold instances.

	// Get/Set the owner of this chip.
	Chip *GetOwner() const { return _owner; }
	void SetOwner(Chip *owner) { _owner = owner; }

	// Editor data used by the editor.
	ChipEditorData *GetChipEditorData() const { return _editorData; }
	void SetChipEditorData(ChipEditorData *editorData) { _editorData = editorData; }

	// Function methods.
	virtual bool CanCreateFunction() const;
	Function *CreateFunction();
	Function *GetFunction() const { return _function; }
	void RemoveFunction();

	// Fast casting functions for selected types.
	virtual ProxyChip *AsProxyChip() { return nullptr; }
	virtual Parameter *AsParameter() { return nullptr; }
	virtual FunctionCall *AsFunctionCall() { return nullptr; }
	virtual FunctionData *AsFunctionData() { return nullptr; }
	virtual InstanceData *AsInstanceData() { return nullptr; }
	virtual Shortcut *AsShortcut() { return nullptr; }

	// Get/Set chip name.
	virtual String GetName() const { return _name; }
	virtual void SetName(String name);

	// After a chip is created, ONE of InitChip(), CopyChip() OR LoadChip() 
	// should be called to finalize initialization of the chip!
	// Therefore, move heavy initialization code from the constructor to these methods!
	virtual bool InitChip();
	virtual bool CopyChip(Chip *chip);
	virtual bool LoadChip(DocumentLoader &loader) { return true; }
	virtual bool SaveChip(DocumentSaver &saver) const { return true; }

	// Message system.
	virtual bool HasMessages() const { return _messages != nullptr; }
	virtual const ChipMessageList *GetMessages() const { return _messages; }
	virtual uint32 GetMessageHitCount(const ChipMessage &msg) const;
	virtual void AddMessage(const ChipMessage &msg);
	virtual void RemoveMessage(const ChipMessage &msg);
	virtual void ClearMessages();
	// This will add the message to the chip given in the exception. If none, it will add it to us!
	virtual void AddException(const ChipException &exp);

	// Get/Set the update-stamp.
	UpdateStamp GetUpdateStamp() const { return _updateStamp; }
	UpdateStamp SetUpdateStamp() { return _updateStamp = GenerateUpdateStamp(); }

	virtual void OnDestroyDevice() {}
	virtual void OnReleasingBackBuffer(RenderWindow *rw) {}


	virtual const ChildConnectionList &GetChildren() const { return _childProvider->ProvideChildren(); }
	inline const Chip *GetChildProvider() const { return _childProvider; }
	inline void SetChildProvider(const Chip *chip) { _childProvider = chip; }
	inline const Chip *ReplaceChildProvider(const Chip *chip) { const Chip *c = _childProvider; _childProvider = chip; return c; }

	// Removes all child connections in our _children list.
	// It does NOT use the ChildConnectionList returned by our _childProvider!
	// The fromIndex parameters lets you clear exessive connections.
	virtual void ClearConnections(uint32 fromIndex = 0);

	// Sets a child connection in our _children list.
	// It does NOT use the ChildConnectionList returned by our _childProvider!
	virtual void SetConnection(uint32 index, const ChildConnectionDesc &connection, bool keepChildren = false);

	virtual bool SetChild(Chip *child, uint32 index, uint32 subIndex);
	virtual bool InsertChild(Chip *child, uint32 index, uint32 subIndex);
	virtual bool MoveChild(uint32 index, uint32 fromSubIndex, uint32 toSubIndex);
	virtual void RemoveChild(Chip *child);
	virtual ChipChildPtr GetChild(uint32 index, uint32 subIndex = 0) const;
	virtual Chip *GetRawChild(uint32 index, uint32 subIndex = 0) const;
	virtual uint32 GetConnectionCount() const;
	virtual uint32 GetSubConnectionCount(uint32 index) const;
	virtual void RemoveEmptyConnections();

	void AddConnectionClone(Chip *chip);
	void RemoveConnectionClone(Chip *chip);

	virtual String GetValueAsString() const { return String(); }

	virtual void CallChip() {}
	virtual ChipChildPtr GetChip();


	virtual uint32 GetMultiConnectionChildCount() const { return 1; }
	virtual ChipChildPtr GetMultiConnectionChip(uint32 subIndex) { return subIndex == 0 ? GetChip() : nullptr; }

	virtual void Run();

	virtual Chip *FindChip(ChipID chipID) { return _id == chipID ? this : nullptr; }

	// Updates _lastHit.
	void Touch();

	// Returns the Refresh Manager used to limit chip updates.
	virtual RefreshManager &GetRefreshManager() { return Refresh; }
	virtual const RefreshManager &GetRefreshManager() const { return Refresh; }

	virtual int32 GetLastHitTime() const { return _lastHit; }

	virtual void RestoreChip() {}

	virtual void AddDependencies(ProjectDependencies &deps) {}

	// Hidden chips (aka global chips) will be called at the start of each frame. 
	// This is a single event that later may be part of a more advance event system.
	virtual void OnNewFrame() {}

	virtual List<Shortcut*> GetShortcuts() const;

	// Returns the platforms that are supported by this chip.
	// This function is to be used by the publishing process.
	// The publishing will fail or give a warning (and excluding the chip) if the
	// chip is not supported by the target platform.
	// It is implemented as a virtual function instead of being defined in the ChipDesc
	// for the chip to be able to select supported platform based on its configuration.
	// This also means that if the chip is a container or the creator of global chips,
	// is must call GetSupportedPlatforms() on them as well and merge the result!
	// Note: There is a similar function defined at packet level.
	virtual uint32 GetSupportedPlatforms() const { return PLATFORM_all_platforms; }
};

#ifdef DEVCHECKS
#define CHECK_CHILD(c, idx) if (c == nullptr) throw MissingChildException(idx);
#else
#define CHECK_CHILD(c, idx)
#endif

}