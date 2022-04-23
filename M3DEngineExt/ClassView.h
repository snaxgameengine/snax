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

#include "M3DCore/Databuffer.h"
#include "ClassExt.h"

#define WHEEL_SENSITIVITY 12.0f
#define MIN_ZOOM 3.0f
#define MAX_ZOOM 400.0f
#define MAX_MOVE 800.0f
#define LINK_MOUSE_LIMIT_SQ 32.0f

// Number of vertices for cubic hermite interpolated links. Set to 2 for straight lines!
#define VERTICES_PR_LINK 64


namespace m3d
{

class InputManager;
class DocumentLoader;

struct Link
{
	ChipConnectionID id;
	Vector2 a;
	Vector2 b;
	ChildConnectionDesc::DataDirection dir;
	bool hover;
	bool selected;
	int32 lastHit;
};

// ILink: "Imaginary Link". Nonselectable link
struct ILink
{
	Vector2 a;
	Vector2 b;
	ChildConnectionDesc::DataDirection dir;
};

class ClassViewCallback
{
public:
	virtual void ShowChipDialog(Chip *chip, bool showComment = false) = 0;
	virtual void ShowContextMenu() = 0;
	virtual void FocusOnChip(ClassExt *clazz, Chip *chip) = 0;
	virtual void EnterFolder(ClassExt *clazz, FolderID folderID, bool inside) = 0;
	virtual void AddBackgroundItem(CGBackgroundItem::Type type) = 0;
	virtual void UpdateBackgroundItem(CGBackgroundItemID id) = 0;
};

class ClassView;

class ClassInfoBox
{
	friend class ClassRenderer;
	friend class ClassRenderer2;
protected:
	// Mouse position in pixels
	Vector2 mousePosVP;
	// Timestamp for last mouse movement
	int32 mouseMoveTime;
	// Mouse position when info box appeared
	Vector2 mousePosVPInfo;
	// show info box?
	bool show;
	// view's hm for infobox
	HoverMode hm;
	// view's hoverCh for infobox
	ChipID hoverCh;
	// view's hoverConn for infobox
	unsigned hoverConn;

public:
	ClassInfoBox() : show(false) {}
	~ClassInfoBox() {}

	void Update(InputManager *im, ClassView *view, float32 left, float32 top);
};

class ClassView
{
	friend class ClassRenderer;
	friend class ClassRenderer2;
protected:
	// The class
	ClassExt *clazz;
	// Width of view
	float32 w;
	// Height of view
	float32 h;
	// Aspect ratio w/h
	float32 ar;
	// Current cursor position in the view
	Vector2 mousePos;
	// Current zoom
	float32 zoom;
	// Current view pos
	Vector2 camPos;

	ClassInfoBox infoBox;

	// Current folder
	FolderID folder;
	// List of all chips currently visible and/or selected.
	List<Chip*> vChips; 
	// List of folders currently visible.
	List<FolderID> vFolders;
	// List of all links currently visible.
	List<Link> vLinks;
	// List of all imaginary links currently visible.
	List<ILink> vILinks;
	// List of all background items visible and/or selected.
	List<CGBackgroundItemMap::const_iterator> vBackgroundItems;

	// What type of element the mouse cursor is currently over.
	HoverMode hm;
	// ID of the chip hovered.
	ChipID hoverCh;
	// Index of a chip's child connection hovered.
	unsigned hoverConn;
	// ID of folder hovered.
	FolderID hoverFolder;
	// Index into vLinks of the link currenly hovered.
	unsigned hoverLink;
	// ID of background item currently hovered.
	CGBackgroundItemID hoverBackgroundItem;

	// Start of rubberband when selecting (end is mousepos)
	Vector2 selectionStart;
	// Selected chips
	Set<ChipID> sChips;
	// Chips inside "rubberband" when selecting
	Set<ChipID> rChips;
	// Selected links
	Set<ChipConnectionID> sLinks;
	// Links inside "rubberband" when selecting
	Set<ChipConnectionID> rLinks;
	// Selected folders
	Set<FolderID> sFolders;
	// Fodlers inside "rubberband" when selecting
	Set<FolderID> rFolders;
	// Selected background items
	Set<CGBackgroundItemID> sBackgroundItems;
	// Background items inside rubberband
	Set<CGBackgroundItemID> rBackgroundItems;

	// Current type of mouse dragging
	DragMode dm;
	// The Guid of the connection currently dragging from
	ChipTypeIndex dragGuid;
	// ID of the chip we are dragging connection from
	ChipID connDragChip;
	// Index of the child connection draggin from
	unsigned dragConn;
	//
	bool dragSelected;
	//
	bool connSwap;
	//
	ChipConnectionID connSwapConnection;
	// When dragging new links, this list contains all start positions of lines to be rendered. the end position is the cursor position.
	List<Vector2> dragStarts;
	// 
	Vector2 _dScaling;

	void _clearSelection();
	bool _isChipVisible(Chip *ch) const;
	void _selectChip(Chip *ch, bool keep, bool selectChildren);
	void _createLinkFromChildConnection();
	void _createLinkFromTopConnections(bool insert);
	bool _isCursorOutsideView();

public:
	ClassView(ClassExt *clazz);
	virtual ~ClassView();

	ClassExt *GetClass() const { return clazz; }

	float32 GetWidth() const { return w; }
	float32 GetHeight() const { return h; }
	float32 GetAspectRatio() const { return ar; }
	float32 GetZoom() const { return zoom; }
	Vector2 GetCameraPos() const { return camPos; }
	Vector2 GetMousePos() const { return mousePos; }

	FolderID GetFolder() const { return folder; }
	void SetFolder(FolderID f, bool inside);

	void SetCameraPos(Vector2 p) { camPos = p; }
	void SetZoom(float32 z) { zoom = z; }

	HoverMode GetHoverMode() const { return hm; }
	ChipID GetHoverChip() const { return hoverCh; }
	unsigned GetHoverConn() const { return hoverConn; }
	FolderID GetHoverFolder() const { return hoverFolder; }
	CGBackgroundItemID GetHoverBackgroundItem() const { return hoverBackgroundItem; }
	DragMode GetDragMode() const { return dm; }
	ChipTypeIndex GetDragGUID() const { return dragGuid; }

	void FocusOnChip(ChipID cid, bool centerFocus = true);

	virtual void PreRenderProcssing(InputManager *im, float32 width, float32 height, float32 left, float32 top);
	virtual void PostRenderProcssing(InputManager *im, ClassViewCallback *callback);

	virtual bool CanCopy();
	virtual bool CanDelete();
	virtual bool CanPaste() { return true; }
	virtual bool CanInsertTemplate() { return true; }
	virtual bool CanInsertAnyChips() { return true; }

	void Copy(DataBuffer &db);
	void Paste(DataBuffer &&db);
	void Delete();
	void InsertTemplate(DocumentLoader &loader, String name);

	void AddChildToDraggedConnection(Chip* c, bool allowRename = true);

	void AddRectangle(Vector4 color, Vector2 pos);
	void AddText(String text, Vector2 s, float32 size, Vector4 color, Vector2 pos);
	void AddImage(Vector2 s, const DataBuffer &db, Vector2 pos, ID3D11ShaderResourceView *srv = nullptr);
	void AddFolder(String name, Vector2 pos);

	void UnpackFolder(FolderID f);
	void AddSelectionToNewFolder();

};

}