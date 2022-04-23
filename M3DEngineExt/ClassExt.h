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


#include "M3DEngine/Class.h"
#include "M3DEngine/ClassManager.h" // only for ClassFactory
#include "GraphicsDefines.h" // only for ColorIndex
#include "D3DInclude.h" // Only for imageSRV in background item.
#include "M3DCore/DataBuffer.h" // Only for imageData in background item.
#include "Publisher.h"

namespace m3d
{

enum DragMode { DM_NONE, DM_SELECT, DM_DRAGGING, DM_TOPCONN, DM_CHILDCONN, DM_SIZING };
enum HoverMode { HM_NONE, HM_CHIP, HM_TOPCONN, HM_CHILDCONN, HM_LINK, HM_FOLDER, HM_BGITEM, HM_BGITEM_SCALE, HM_ADDCHILD };

typedef uint32 FolderID;

const FolderID InvalidFolderID = -1;
const FolderID MainFolderID = 0;

typedef std::pair<unsigned, unsigned> ConnectionID;
typedef std::pair<ChipID, ConnectionID> ChipConnectionID;

const ConnectionID InvalidConnectionID = ConnectionID(-1, -1);


// Structure containing pr chip data for use in editor.
struct ChipEditorData
{
	Chip * const chip;
	String comment;
	FolderID folder;
	Vector2 pos;
	ColorScheme::ColorIndex colorIndex;

	// Updated pr view
	unsigned cCount;
	float32 width;
	HoverMode hm;
	bool selected;

	ChipEditorData(Chip *chip) : chip(chip), folder(MainFolderID), pos(0.0f, 0.0f), colorIndex(ColorScheme::CI_CHIP), cCount(0), width(1.0f), hm(HM_NONE), selected(false) {}

	unsigned GetNumberOfChildConnections() const;
	ConnectionID GetConnectionID(unsigned connectionIdx) const;
	ChipID GetChild(const ConnectionID &cid) const;
	void FindAllChildren(Set<Chip*> &s) const;
};


// Structure containing information about folders in a class.
struct Folder
{
	FolderID parentFolder;
	Vector2 pos;
	Vector2 inPos;
	String name;

	// Updated pr view
	mutable bool hover;
	mutable bool selected;
	mutable FolderID nearFolder;
};

typedef List<Folder> FolderList;

struct CGBackgroundItem
{
	static unsigned Counter;

	enum Type  { INVALID, RECT, IMAGE, TEXT };
	const Type type;
	Vector2 pos;
	Vector2 size;
	FolderID folder;
	// Not for image
	Vector4 color; 
	// For image only
	DataBuffer imageData;
	mutable SID3D11ShaderResourceView imageSRV; // Updated by renderer.
	// For text only
	String text;
	float32 textSize;

	// Updated pr view
	mutable bool hover;
	mutable bool selected;
	mutable bool showHandles;

	CGBackgroundItem() : type(INVALID), pos(0.0f, 0.0f), size(0.0f, 0.0f), folder(InvalidFolderID), color(0.0f, 0.0f, 0.0f, 0.0f), textSize(0.0f) {}
	CGBackgroundItem(Vector2 pos, Vector2 size, FolderID folder, Vector4 color) : type(RECT), pos(pos), size(size), folder(folder), color(color), textSize(0.0f) {}
	CGBackgroundItem(Vector2 pos, Vector2 size, FolderID folder, const DataBuffer &imageData, ID3D11ShaderResourceView *srv = nullptr) : type(IMAGE), pos(pos), size(size), folder(folder), color(0.0f, 0.0f, 0.0f, 0.0f), imageData(imageData), textSize(0.0f), imageSRV(srv) {}
	CGBackgroundItem(Vector2 pos, Vector2 size, FolderID folder, Vector4 color, String text, float32 textSize) : type(TEXT), pos(pos), size(size), folder(folder), color(color), text(text), textSize(textSize) {}
};

typedef std::pair<CGBackgroundItem::Type, unsigned> CGBackgroundItemID;

const CGBackgroundItemID InvalidCGBackgroundItemID = CGBackgroundItemID(CGBackgroundItem::INVALID, 0);

typedef Map<CGBackgroundItemID, CGBackgroundItem> CGBackgroundItemMap;


class ClassExt;
class ClassView;
class ClassDiagramChip;

struct ClassExtLoadInfo : public ClassLoadInfo
{
	String comment;
	List<std::pair<Guid, Vector2>> rgChips;
	Map<Guid, std::pair<unsigned, Guid>> baseClassMapping;
};

class ClassExtEventListener : public ClassEventListener
{
public:
	virtual void DirtyFlagChanged(Class *clazz) = 0;
	virtual void OnFolderRemoved(ClassExt *clazz, FolderID folder) = 0;
	virtual void OnDescriptionChanged(ClassExt* clazz) = 0;
};

class ClassExtFactory : public ClassFactory
{
public:
	ClassExtFactory() {}
	~ClassExtFactory() {}

	virtual Class *Create() override;
};


struct PublishSettings;

// DR_DERIVED: Marks classes directly derived from us dirty!
// DR_FUNCTION_CALLS: For all our functions, mark the classes of their function calls dirty!
// DR_INSTANCES: Mark classes containing instances of us dirty!
// DR_ALL_INSTANCES: Mark classes containing instances of us or derived classes dirty!
// DR_CG: Mark classes containing Class-chips dirty!
enum DirtyRelationsMask { DR_DERIVED = 0x01, DR_FUNCTION_CALLS = 0x02, DR_INSTANCES = 0x04, DR_ALL_INSTANCES = 0x0C, DR_CG = 0x10, DR_ALL = 0xFFFFFFFF };

class ClassExt : public Class
{
public:
	ClassExt();
	virtual ~ClassExt();

	bool IsDirty() const override { return _dirty; }
	// Note: As a convention, SetDirty() should be called where the user does the action to trigger the event.
	// As an example, if the user adds a new chip using the editor, SetDirty() should be called from the
	// editor where the action took place - NOT in our AddChip()-method!
	// SetDirty() is to be called as a direct result of a user action in the editor - Not when something is
	// done programerically! (AddChip() is called several times during loading for example.)
	virtual void SetDirty(bool dirty = true) override;

	// Set related classes dirty. See DirtyRelationsMask.
	void SetRelationsDirty(unsigned mask);

	ClassDiagramChip *GetClassDiagramChip() const { return _rgChip; }
	void SetClassDiagramChip(ClassDiagramChip *rgChip) { _rgChip = rgChip; }

	virtual Chip *AddChip(ChipTypeIndex chipGuid, bool initChip = true) override;
	virtual bool RemoveChip(Chip *ch) override;
	virtual Chip *AddChip(ChipTypeIndex chipGuid, FolderID folder, Vector2 pos);
	virtual Shortcut *CreateShortcut(Chip *ch, FolderID folder, Vector2 pos);

	virtual bool CanAddFoldersAndBackgroundItems() const { return true; }

	FolderID CreateFolder(FolderID parent, Vector2 pos, Vector2 inPos, String name);
	bool RemoveFolder(FolderID folderID);
	const FolderList &GetFolderList() const { return _folders; }
	bool IsFolderValid(FolderID folder) const { return _folders.size() > folder && _folders[folder].parentFolder != InvalidFolderID; }
	bool SetFolderParent(FolderID folder, FolderID parent);
	bool SetFolderPos(FolderID folder, Vector2 pos);
	bool SetFolderInPos(FolderID folder, Vector2 inPos);
	bool SetFolderName(FolderID folder, String name);
	void MoveChipsToFolder(FolderID folder, const Set<ChipID> &chips);
	void MoveFoldersToFolder(FolderID folder, const Set<FolderID> &folders);
	void MoveBackgroundItemsToFolder(FolderID folder, const Set<CGBackgroundItemID> &items);
	FolderID GetClosestFolder(FolderID target, FolderID base) const;
	bool IsParentFolder(FolderID parent, FolderID folderToCheck) const;

	const CGBackgroundItemMap &GetBackgroundItems() const { return _backgroundItems; }
	CGBackgroundItemID AddRectangle(FolderID folder, Vector2 pos, Vector2 size, Vector4 color);
	CGBackgroundItemID AddImage(FolderID folder, Vector2 pos, Vector2 size, const DataBuffer &imageData, ID3D11ShaderResourceView *srv = nullptr);
	CGBackgroundItemID AddText(FolderID folder, Vector2 pos, Vector2 size, Vector4 color, String text, float32 textSize);
	bool UpdateBackgroundItem(CGBackgroundItemID id, Vector2 pos, Vector2 size);
	bool UpdateRectangle(CGBackgroundItemID id, Vector4 color);
	bool UpdateText(CGBackgroundItemID id, Vector4 color, String text, float32 textSize);
	bool RemoveBackgroundItem(CGBackgroundItemID id);
	String GetDescription() const { return _description; }
	void SetDescription(String desc);

	bool CreateLink(Chip *ch, Chip *child, ConnectionID cid, bool insert);

	virtual void OnAddedToClassManager() override;
	virtual void OnRemovedFromClassManager() override;

	virtual bool AddBaseClass(Class *base) override;
	virtual bool RemoveBaseClass(Class *base) override;

	virtual bool SaveEditorData(DocumentSaver &saver) const override;
	virtual bool LoadEditorData(DocumentLoader &loader) override;

	enum ConvertType { PROXY, INSTANCE_DATA, FUNCTION_DATA, FUNCTION_CALL, PARAMETER, STANDARD };

	bool CanConvertChip(Chip *chip, ConvertType ct);
	bool ConvertChip(Chip *&chip, ConvertType ct);

	virtual ClassView *CreateView();
	virtual void DestroyView(ClassView *view);

	void GetVisualExtent(FolderID folder, Vector2 &a, Vector2 &b) const;

	void ArrangeChildren(Chip *chip);

	const Map<String, PublishSettings> &GetPublishProfiles() const { return _publishSettings; }
	void SetPublishProfiles(const Map<String, PublishSettings> &ps) { _publishSettings = ps; }

	const PublishSettings *GetPublishProfile(String profile) const;
	void SetPublishProfile(const PublishSettings &s);
	void RemovePublishProfile(String profile);
	void RemovePublishProfiles();

	virtual void OnShortcutSet(Shortcut *sc) override;

protected:
	// List of folders in the class. Index 0 is "main" folder.
	FolderList _folders;
	// true if class contains changes that can be saved.
	bool _dirty;
	// Class diagram chip which represent us in the Class Diagram.
	ClassDiagramChip *_rgChip;
	// Settings for publishing. One for each named profile.
	Map<String, PublishSettings> _publishSettings;
	// Background items (rects, images and texts)
	CGBackgroundItemMap _backgroundItems;
	// Class description
	String _description;

	virtual ClassLoadInfo *CreateLoadInfo() const override;
	virtual void DeleteLoadInfo(ClassLoadInfo *nfo) const override;
};

}



