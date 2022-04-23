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

// NB: DO NOT INCLUDE (DIRECTLY OR INDIRECTLY)  ANY QT FILES HERE!
#include "Exports.h"
#include "M3DCore/Path.h"
#include "M3DCore/Containers.h"
#include "M3DCore/GuidUtil.h"
#include "M3DCore/DestructionObserver.h"
#include "M3DEngine/GlobalDef.h"
#include "DialogPage.h"
#include "ChipDialog.h"

class QWidget;

namespace m3d
{

class Chip;
class ChipDialog;
class ChipDialogContainer;
class ClassInstance;

typedef uint32 (*GETDIALOGCOUNT)();
typedef const DialogType &(*GETDIALOGTYPE)(uint32);
typedef DialogPage *(*DIALOGFACTORY)();
typedef bool (*ONDLGPACKETQUERY)();
typedef bool (*ONDLGPACKETLOAD)();
typedef void (*ONDLGPACKETUNLOAD)();


struct DlgPacket;

struct ChipDlgInfo
{
	DlgPacket *packet;
	struct {
		Guid chipType;
		Guid chipTypeNP;
		String factoryFunc;
	} dialogDesc; // Can not use DialogType here because of const char*.
	DIALOGFACTORY dialogFactoryFunc;
	bool failedToLoad;
	uint32 index;

	ChipDlgInfo() : packet(0), dialogFactoryFunc(0), failedToLoad(false), index(0) {}
};

typedef Map<Guid, ChipDlgInfo*> ChipDlgInfoByGUIDMap;
typedef List<ChipDlgInfo*> ChipDlgInfoList;

struct DlgPacket
{
	Path filename;
	uint32 dlgCount;
	HMODULE module;
	RELEASEMEMORY deleteMemFunc;
	bool failedToLoad;

	ChipDlgInfoList dialogs;

	DlgPacket() : dlgCount(0), module(0), deleteMemFunc(nullptr), failedToLoad(false) {}
};

typedef List<DlgPacket*> DlgPacketPtrList;

struct ChipPageSettings
{
	int32 w;
	int32 h;
	PageData::MaximizedState maximized;
};

typedef Map<ChipTypeIndex, ChipPageSettings> ChipPageSettingsByChipTypeIndexMap;

struct ChipDialogSettings
{
	int32 x;
	int32 y;
	int32 commentW;
	int32 commentH;
	PageData::MaximizedState commentMaximized;
	ChipPageSettingsByChipTypeIndexMap pageSizes;
	bool embedded;
	uint32 embeddedID; 
	ChipDialogSettings() : x(-1), y(-1), commentW(-1), commentH(-1), commentMaximized(PageData::MAX_UNSET), embedded(false), embeddedID(0) {}
};

typedef Map<Chip*, ChipDialogSettings> ChipDialogSettingsByChipPtrMap;
typedef Map<Chip*, ChipDialogContainer*> ChipDialogPtrByChipPtrMap;

class CHIPDIALOGS_EXPORT ChipDialogManagerCallback
{
public:
	virtual QWidget *GetParentWidget() = 0;
	virtual void SetDirty(Class *clazz) = 0;
	virtual bool HasComments(Chip *ch) = 0;
	virtual String GetComment(Chip *ch) = 0;
	virtual void SetComment(Chip *ch, String comment) = 0;
	virtual void DisableFrameTimer(bool disable) = 0;
	virtual void OpenInstanceDialog(ClassInstance *instance) = 0;
	virtual Path GetCurrentPath() const = 0;
	virtual void SetCurrentPath(Path p) = 0;
	virtual ChipDialogContainer *GetEmbeddedContainer(uint32 id) = 0;
};

class CHIPDIALOGS_EXPORT ChipDialogManager : public DestructionObserver
{
	friend class ChipDialogDestructionObserver;
protected:
	ChipDlgInfoByGUIDMap _dialogInfos;
	DlgPacketPtrList _dlgPackets;
	// Map of all open dialogs.
	ChipDialogPtrByChipPtrMap _dialogs;
	// Map of dialog settings (pos, size) for all chips that has been shown.
	ChipDialogSettingsByChipPtrMap _settings;
	// Map of dialog settings (pos, size) for chip types. This is default sizes pr chip type.
	ChipPageSettingsByChipTypeIndexMap _defaultPageSizes;
	// Maps if dialogs of a gived type should be opened embedded and in which editor.
	Map<ChipTypeIndex, uint32> _embedTypeByDefault;
	// Maps if dialogs in general should be embedded and in which editor.
	uint32 _embedByDefault;
	// Default comment page width.
	int32 _commentW;
	// Default comment page height.
	int32 _commentH;
	// Callback. Should be editor main window.
	ChipDialogManagerCallback *_callback;

	void _processDLL(Path fileName);
	void _keepDialogSettings(ChipDialogContainer *container);

	virtual void OnDestruction(DestructionObservable *observable);
	DialogPage *CreateDialogPage(const Guid &guid);

public:
	ChipDialogManager(ChipDialogManagerCallback *callback);
	virtual ~ChipDialogManager();

	// Initiate dialog manager by searching for dialogs;
	bool FindDialogs(Path folder);
	// Open dialog for the given chip. Inits with the comment page if wanted. embeddedID is the ID of the editor to be used; 0 if nonembedded. If modalResult!=nullptr, the dialog will be blocking/modal.
	void OpenChipDialog(Chip *chip, bool showComment = false, uint32 embeddedID = 0, int32 *modalResult = nullptr);
	// Closes the dialog for the specified chip.
	void CloseChipDialog(Chip *chip);
	// Closes all open dialogs.
	void CloseAllChipDialogs();
	// Calls Update() on all dialog pages. This is done each frame.
	void UpdateChipDialogs();
	// Dialogs will call this method to mark a class dirty.
	void SetDirty(Class *clazz) { return _callback->SetDirty(clazz); }
	// Dialogs will call this method to temporary disable the frame timer. This is neccessary eg. when showing file-dialogs, because the don't render correctly if not.
	void DisableFrameTimer(bool disable) { return _callback->DisableFrameTimer(disable); }
	// Open instance dialog
	void OpenInstanceDialog(ClassInstance *instance) { return _callback->OpenInstanceDialog(instance); }
	// Getting chip comment
	String GetComment(Chip *ch) { return _callback->GetComment(ch); }
	// Setting chip comment
	void SetComment(Chip *ch, String comment) { return _callback->SetComment(ch, comment); }
	// Toggle embedding for the given dialog.
	void ToggleEmbedding(ChipDialog *cd);
	// Set if dialogs of the given type should be opened embedded by default. 0 means no embeddning.
	void SetDefaultEmbedding(Guid chipType, uint32 embeddedID);

	void DestroyDeviceObject();

	Path GetCurrentPath() const { return _callback->GetCurrentPath(); }
	void SetCurrentPath(Path p) { return _callback->SetCurrentPath(p); }
	
	// Dialogs can't create QMimeData for use by clipboard because when their heap is destroyed, the application crash on exit.
	class QMimeData* CreateMimeData() const;
};


}