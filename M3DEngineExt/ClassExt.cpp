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

#include "pch.h"
#include "ClassExt.h"
#include "M3DEngine/Engine.h"
#include "DocumentExtSaver.h"
#include "DocumentExtLoader.h"
#include "M3DEngine/ChipManager.h"
#include "ClassView.h"
#include "ClassDiagram.h"
#include "M3DEngine/ClassInstance.h"
#include "StdChips/Parameter.h"
#include "StdChips/FunctionCall.h"
#include "StdChips/FunctionData.h"
#include "StdChips/InstanceData.h"
#include "StdChips/ClassDiagramChip.h"
#include "StdChips/ThisChip.h"
#include "StdChips/Shortcut.h"
#include "StdChips/BreakPoint.h"
#include "M3DEngine/Function.h"
#include "Publisher.h"
#include "M3DEngine/Document.h"

using namespace m3d;

static const Guid SHADERIOVECTOR_GUID = { 0xbba8dce6, 0xc97a, 0x4992,{ 0x9e, 0x35, 0x1b, 0x19, 0x9e, 0x3e, 0xac, 0x5d } };
static const Guid SHADERUNIFORMMATRIX_GUID = { 0x5c165cba, 0x8017, 0x43da,{ 0x83, 0x2, 0xca, 0x3e, 0x9, 0x1b, 0x1d, 0xa1 } };


static List<ColorScheme::ColorIndex> colorList; // The fill color for all chip types!


unsigned CGBackgroundItem::Counter = 0;


unsigned ChipEditorData::GetNumberOfChildConnections() const
{
	const ChildConnectionList &ccl = chip->GetChildren();
	size_t c = ccl.size();
	for (size_t i = 0; i < ccl.size(); i++) {
		ChildConnection *cc = ccl[i];
		if (cc && cc->desc.connType == ChildConnectionDesc::GROWING)
			c += cc->connections.size();
	}
	return (unsigned)c;
}

ConnectionID ChipEditorData::GetConnectionID(unsigned connectionIdx) const
{
	for (unsigned i = 0, j = 0; i < chip->GetChildren().size(); i++, j++) {
		ChildConnection *cc = chip->GetChildren()[i];
		if (cc && cc->desc.connType == ChildConnectionDesc::GROWING) {
			if (connectionIdx <= (j + cc->connections.size()))
				return std::make_pair(i, connectionIdx - j);
			j += (unsigned)cc->connections.size();
		}
		if (j == connectionIdx)
			return std::make_pair(i, 0);
	}
	return InvalidConnectionID;
}

void ChipEditorData::FindAllChildren(Set<Chip*> &s) const
{
	for (unsigned i = 0; i < chip->GetChildren().size(); i++) {
		ChildConnection *cc = chip->GetChildren()[i];
		if (cc) {
			for (unsigned j = 0; j < cc->connections.size(); j++) {
				Chip *c = cc->connections[j].chip;
				if (c && c->GetClass() == chip->GetClass() && s.insert(c).second)
					c->GetChipEditorData()->FindAllChildren(s);
			}
		}
	}
}

ChipID ChipEditorData::GetChild(const ConnectionID &cid) const
{
	const ChildConnectionList &cl = chip->GetChildren();
	if (cl.size() > cid.first && cl[cid.first]->connections.size() > cid.second) {
		Chip *ch = cl[cid.first]->connections[cid.second].chip;
		if (ch)
			return ch->GetID();
	}
	return InvalidChipID;
}

Class *ClassExtFactory::Create()
{
	return mmnew ClassExt();
}

ClassExt::ClassExt() : _dirty(false), _rgChip(nullptr)
{
	// Create the default folder
	Folder f = {0, Vector2(0.0f, 0.0f), Vector2(0.0f, 0.0f), MTEXT(""), false, false};
	_folders.push_back(f);

	if (colorList.size() == 0) { // only done once!
		colorList.resize(engine->GetChipManager()->GetChipInfoPtrByGUIDMap().size() + 1, ColorScheme::CI_CHIP);
		colorList[engine->GetChipManager()->GetChipTypeIndex(PARAMETER_GUID)] = ColorScheme::CI_PARAMETER;
		colorList[engine->GetChipManager()->GetChipTypeIndex(FUNCTIONCALL_GUID)] = ColorScheme::CI_FUNC_CALL;
		colorList[engine->GetChipManager()->GetChipTypeIndex(FUNCTIONDATA_GUID)] = ColorScheme::CI_FUNC_DATA;
		colorList[engine->GetChipManager()->GetChipTypeIndex(INSTANCEDATA_GUID)] = ColorScheme::CI_INSTANCE_DATA;
		colorList[engine->GetChipManager()->GetChipTypeIndex(THISCHIP_GUID)] = ColorScheme::CI_THIS_CHIP;
		colorList[engine->GetChipManager()->GetChipTypeIndex(CLASSDIAGRAMCHIP_GUID)] = ColorScheme::CI_RG_CHIP;
		colorList[engine->GetChipManager()->GetChipTypeIndex(BREAKPOINT_GUID)] = ColorScheme::CI_BREAKPOINT;
//		colorList[engine->GetChipManager()->GetChipTypeIndex(SHADERIOVECTOR_GUID)] = ColorScheme::CI_SHADERIO;
//		colorList[engine->GetChipManager()->GetChipTypeIndex(SHADERUNIFORMMATRIX_GUID)] = ColorScheme::CI_SHADERUNIFORM;
		
	}
}

ClassExt::~ClassExt()
{
}

void ClassExt::SetDirty(bool dirty)
{
	if (dirty && GetDocument())
		GetDocument()->SetDirty(true); // Mark document only if we are dirty!
	if (_dirty == dirty)
		return; // No change
	_dirty = dirty;
	if (GetEventListener())
		((ClassExtEventListener*)GetEventListener())->DirtyFlagChanged(this);
}

void ClassExt::SetRelationsDirty(unsigned mask)
{
	if (mask & DR_DERIVED) {
		for (const auto &n : GetSubClasses())
			((ClassExt*)n)->SetDirty();
	}

	if (mask & DR_FUNCTION_CALLS) {
		for (const auto &n : GetFunctions()) { // Iterate functions...
			for (const auto &m : n->GetFunctionCalls()) { // Iterate the function calls...
				ClassExt *cg = dynamic_cast<ClassExt*>(m->GetClass());
				if (cg)
					cg->SetDirty(); // Mark the function call's class dirty!
			}
		}
	}

	if (mask & DR_INSTANCES) {
		for (const auto &n : GetInstances()) {
			ClassExt *cg = dynamic_cast<ClassExt*>(n->GetOwner()->GetClass());
			if (cg)
				cg->SetDirty();
		}
	}

	if (mask & DR_ALL_INSTANCES) {
		for (const auto &n : GetSubClasses())
			((ClassExt*)n)->SetRelationsDirty(DR_ALL_INSTANCES);
	}

	if (mask & DR_CG) {
		for (const auto &n : GetClassChips())
			((Chip*)n)->GetClass()->SetDirty();
	}
}

Chip *ClassExt::AddChip(ChipTypeIndex chipGuid, bool initChip)
{
	Chip *ch = Class::AddChip(chipGuid, initChip);
	if (ch) {
		ch->SetChipEditorData(mmnew ChipEditorData(ch));
		ch->GetChipEditorData()->colorIndex = colorList[chipGuid];
	}
	return ch;
}

bool ClassExt::RemoveChip(Chip *ch)
{
	ChipEditorData *ced = (ChipEditorData*)ch->GetChipEditorData();
	if (!Class::RemoveChip(ch))
		return false;
	if (ced)
		mmdelete(ced);
	return true;
}

Chip *ClassExt::AddChip(ChipTypeIndex chipGuid, FolderID folder, Vector2 pos)
{
	Chip *chip = AddChip(chipGuid);

	if (!chip)
		return nullptr;

	// Make sure folder is valid
	if (!IsFolderValid(folder))
		folder = MainFolderID;

	ChipEditorData *ced = chip->GetChipEditorData();
	assert(ced);
	ced->folder = folder;
	ced->pos = pos;

	return chip;
}

Shortcut *ClassExt::CreateShortcut(Chip *ch, FolderID folder, Vector2 pos)
{
	assert(ch);

	Shortcut *shortcut = (Shortcut*)AddChip(engine->GetChipManager()->GetChipTypeIndex(SHORTCUT_GUID), folder, pos);
	if (shortcut) {
		shortcut->SetOriginal(ch);
	}
	return shortcut;
}

void ClassExt::OnShortcutSet(Shortcut *sc)
{
	assert(sc);
	assert(sc->GetOriginal());
	sc->GetChipEditorData()->colorIndex = sc->GetOriginal()->GetChipEditorData()->colorIndex; // Shortcut need to have same base color as original!
}

FolderID ClassExt::CreateFolder(FolderID parent, Vector2 pos, Vector2 inPos, String name)
{
	if (!CanAddFoldersAndBackgroundItems())
		return InvalidFolderID;
	// Make sure parent is valid
	if (!IsFolderValid(parent))
		parent = MainFolderID;
	Folder f = {parent, pos, inPos, name, false, false};
	for (FolderID i = 1; i < _folders.size(); i++) {
		if (_folders[i].parentFolder == InvalidFolderID) {
			_folders[i] = f;
			return i;
		}
	}
	_folders.push_back(f);
	return (unsigned)_folders.size() - 1;
}

bool ClassExt::RemoveFolder(FolderID folderID)
{
	if (folderID == MainFolderID || !IsFolderValid(folderID))
		return false;

	// Remove chips
	List<Chip*> chipsToRemove;
	const ChipPtrByChipIDMap &chipMap = GetChips();
	for (const auto &n : chipMap) {
		ChipEditorData *ced = n.second->GetChipEditorData();
		if (ced->folder == folderID) // chip in same folder?
			chipsToRemove.push_back(n.second);
	}
	for (size_t i = 0; i < chipsToRemove.size(); i++)
		RemoveChip(chipsToRemove[i]);

	// Remove background Items
	List<CGBackgroundItemID> itemsToRemove;
	for (const auto &n : _backgroundItems) {
		if (n.second.folder == folderID)
			itemsToRemove.push_back(n.first);
	}
	for (size_t i = 0; i < itemsToRemove.size(); i++)
		RemoveBackgroundItem(itemsToRemove[i]);

	// Remove folders
	for (size_t i = 1; i < _folders.size(); i++)
		if (_folders[i].parentFolder == folderID)
			RemoveFolder((FolderID)i);

	_folders[folderID].parentFolder = InvalidFolderID;
	if (folderID == _folders.size() - 1)
		_folders.pop_back();

	if (GetEventListener())
		((ClassExtEventListener*)GetEventListener())->OnFolderRemoved(this, folderID);

	return true;
}

bool ClassExt::SetFolderParent(FolderID folder, FolderID parent)
{
	if (folder == MainFolderID || !IsFolderValid(folder) || !IsFolderValid(parent))
		return false;

	for (FolderID f = parent; f != MainFolderID; f = _folders[f].parentFolder)
		if (f == folder)
			return false;

	_folders[folder].parentFolder = parent;

	return true;
}

bool ClassExt::SetFolderPos(FolderID folder, Vector2 pos)
{
	if (!IsFolderValid(folder))
		return false;
	_folders[folder].pos = pos;
	return true;
}

bool ClassExt::SetFolderInPos(FolderID folder, Vector2 inPos)
{
	if (!IsFolderValid(folder))
		return false;
	_folders[folder].inPos = inPos;
	return true;
}

bool ClassExt::SetFolderName(FolderID folder, String name)
{
	if (!IsFolderValid(folder))
		return false;
	_folders[folder].name = name;
	return true;
}

void ClassExt::MoveChipsToFolder(FolderID folder, const Set<ChipID> &chips)
{
	if (!IsFolderValid(folder))
		return;
	for (const auto &n : chips) {
		Chip *ch = GetChip(n);
		if (ch)
			ch->GetChipEditorData()->folder = folder;
	}
}

void ClassExt::MoveFoldersToFolder(FolderID folder, const Set<FolderID> &folders)
{
	for (const auto &n : folders)
		SetFolderParent(n, folder);
}

void ClassExt::MoveBackgroundItemsToFolder(FolderID folder, const Set<CGBackgroundItemID> &items)
{
	if (!IsFolderValid(folder))
		return;
	for (const auto &n : items) {
		auto m = _backgroundItems.find(n);
		if (m != _backgroundItems.end())
			m->second.folder = folder;
	}
}

FolderID ClassExt::GetClosestFolder(FolderID target, FolderID base) const
{
	if (!(IsFolderValid(target) && IsFolderValid(base)))
		return MainFolderID; // TODO: May return InvalidFolderID instead?
	if (target == base)
		return base; // same folder
	for (FolderID f = target; f != 0; f = _folders[f].parentFolder)
		if (_folders[f].parentFolder == base)
			return f; // Got to go to folder f (lays in base) to get to target from base.
	return _folders[base].parentFolder; // If nothing is wrong with the folders, the target MUST be on the way from the base's parent.
}

bool ClassExt::IsParentFolder(FolderID parent, FolderID folderToCheck) const
{
	if (!(IsFolderValid(parent) && IsFolderValid(folderToCheck)))
		return false;
	if (parent == MainFolderID)
		return true;
	for (FolderID f = folderToCheck; f != 0; f = _folders[f].parentFolder)
		if (_folders[f].parentFolder == parent)
			return true;
	return false;
}

CGBackgroundItemID ClassExt::AddRectangle(FolderID folder, Vector2 pos, Vector2 size, Vector4 color)
{
	if (!CanAddFoldersAndBackgroundItems())
		return InvalidCGBackgroundItemID;

	// Make sure folder is valid
	if (!IsFolderValid(folder))
		folder = MainFolderID;

	if (size.x < BACKGROUNDITEM_MIN_SIZE)
		size.x = BACKGROUNDITEM_MIN_SIZE;
	if (size.y < BACKGROUNDITEM_MIN_SIZE)
		size.y = BACKGROUNDITEM_MIN_SIZE;

	return _backgroundItems.insert(std::make_pair(CGBackgroundItemID(CGBackgroundItem::RECT, ++CGBackgroundItem::Counter), CGBackgroundItem(pos, size, folder, color))).first->first;
}

CGBackgroundItemID ClassExt::AddImage(FolderID folder, Vector2 pos, Vector2 size, const DataBuffer &imageData, ID3D11ShaderResourceView *srv)
{
	if (!CanAddFoldersAndBackgroundItems())
		return InvalidCGBackgroundItemID;

	// Make sure folder is valid
	if (!IsFolderValid(folder))
		folder = MainFolderID;

	if (size.x < BACKGROUNDITEM_MIN_SIZE)
		size.x = BACKGROUNDITEM_MIN_SIZE;
	if (size.y < BACKGROUNDITEM_MIN_SIZE)
		size.y = BACKGROUNDITEM_MIN_SIZE;

	return _backgroundItems.insert(std::make_pair(CGBackgroundItemID(CGBackgroundItem::IMAGE, ++CGBackgroundItem::Counter), CGBackgroundItem(pos, size, folder, imageData, srv))).first->first;
}

CGBackgroundItemID ClassExt::AddText(FolderID folder, Vector2 pos, Vector2 size, Vector4 color, String text, float32 textSize)
{
	if (!CanAddFoldersAndBackgroundItems())
		return InvalidCGBackgroundItemID;

	// Make sure folder is valid
	if (!IsFolderValid(folder))
		folder = MainFolderID;

	if (size.x < BACKGROUNDITEM_MIN_SIZE)
		size.x = BACKGROUNDITEM_MIN_SIZE;
	if (size.y < BACKGROUNDITEM_MIN_SIZE)
		size.y = BACKGROUNDITEM_MIN_SIZE;

	return _backgroundItems.insert(std::make_pair(CGBackgroundItemID(CGBackgroundItem::TEXT, ++CGBackgroundItem::Counter), CGBackgroundItem(pos, size, folder, color, text, textSize))).first->first;
}

bool ClassExt::UpdateBackgroundItem(CGBackgroundItemID id, Vector2 pos, Vector2 size)
{
	auto n = _backgroundItems.find(id);
	if (n == _backgroundItems.end())
		return false;

	if (size.x < BACKGROUNDITEM_MIN_SIZE)
		size.x = BACKGROUNDITEM_MIN_SIZE;
	if (size.y < BACKGROUNDITEM_MIN_SIZE)
		size.y = BACKGROUNDITEM_MIN_SIZE;

	n->second.pos = pos;
	n->second.size = size;
	return true;
}

bool ClassExt::UpdateRectangle(CGBackgroundItemID id, Vector4 color)
{
	if (id.first != CGBackgroundItem::RECT)
		return false;
	auto n = _backgroundItems.find(id);
	if (n == _backgroundItems.end())
		return false;
	n->second.color = color;
	return true;
}

bool ClassExt::UpdateText(CGBackgroundItemID id, Vector4 color, String text, float32 textSize)
{
	if (id.first != CGBackgroundItem::TEXT)
		return false;
	auto n = _backgroundItems.find(id);
	if (n == _backgroundItems.end())
		return false;
	n->second.color = color;
	n->second.text = text;
	n->second.textSize = textSize;
	return true;
}

bool ClassExt::RemoveBackgroundItem(CGBackgroundItemID id)
{
	return _backgroundItems.erase(id) != 0;
}

void ClassExt::SetDescription(String desc) 
{ 
	if (_description == desc)
		return;
	_description = desc; 
	if (GetEventListener())
		((ClassExtEventListener*)GetEventListener())->OnDescriptionChanged(this);
}


bool ClassExt::CreateLink(Chip *chip, Chip *child, ConnectionID cid, bool insert)
{
	if (cid.first >= chip->GetConnectionCount() || !chip->GetChildren()[cid.first])
		return false;
	unsigned a = chip->GetChipEditorData()->GetNumberOfChildConnections();

	if (insert) {
		if (!chip->InsertChild(child, cid.first, cid.second))
			return false;
	}
	else {
		if (!chip->SetChild(child, cid.first, cid.second))
			return false;
	}

	unsigned b = chip->GetChipEditorData()->GetNumberOfChildConnections();
	chip->GetChipEditorData()->pos.x += CONNECTION_WIDTH * 0.5f * float32(b - a);

	return true;
}

bool ClassExt::SaveEditorData(DocumentSaver &saver) const
{
	return DocumentExtSaver::SaveExtData(this, saver);
}

bool ClassExt::LoadEditorData(DocumentLoader &loader)
{
	return DocumentExtLoader::LoadExtData(this, loader);
}

bool ClassExt::CanConvertChip(Chip *chip, ConvertType ct)
{
	assert(chip);
	if (chip->AsShortcut())
		chip = chip->AsShortcut()->GetOriginal();
	if (!chip)
		return false;
	if (chip->AsProxyChip() && (ct != STANDARD || !chip->AsProxyChip()->IsChipTypeSet()))
		return false;
	if (!chip->AsProxyChip() && ct == STANDARD)
		return false;
	if (chip->AsFunctionCall() && (ct == FUNCTION_CALL || ct == PARAMETER))
		return false;
	if (dynamic_cast<ClassDiagramChip*>(chip))
		return false;
	return true;
}

bool ClassExt::ConvertChip(Chip *&chip, ConvertType ct)
{
	assert(chip);

	if (chip->AsShortcut())
		chip = chip->AsShortcut()->GetOriginal();
	if (!chip)
		return false;

	if (!CanConvertChip(chip, ct))
		return false;

	ChipEditorData *ced = chip->GetChipEditorData();
	assert(ced);

	// TODO: Need to call InitChip() for the new one?

	// Create chip to replace
	Chip *rChip = nullptr;
	if (ct == PROXY) {
		rChip = AddChip(engine->GetChipManager()->GetChipTypeIndex(PROXYCHIP_GUID), ced->folder, ced->pos);
		if (rChip)
			rChip->AsProxyChip()->SetChipType(chip->GetChipDesc().type);
	}
	else if (ct == INSTANCE_DATA) {
		rChip = AddChip(engine->GetChipManager()->GetChipTypeIndex(INSTANCEDATA_GUID), ced->folder, ced->pos);
		if (rChip)
			rChip->AsInstanceData()->SetChipTypeAndCreateTemplate(chip->GetChipDesc().type, chip);
	}
	else if (ct == FUNCTION_DATA) {
		rChip = AddChip(engine->GetChipManager()->GetChipTypeIndex(FUNCTIONDATA_GUID), ced->folder, ced->pos);
		if (rChip) {
			rChip->AsFunctionData()->SetChipType(chip->GetChipDesc().type);
			rChip->AsFunctionData()->CreateTemplate(chip);
		}
	}
	else if (ct == FUNCTION_CALL) {
		rChip = AddChip(engine->GetChipManager()->GetChipTypeIndex(FUNCTIONCALL_GUID), ced->folder, ced->pos);
		if (rChip)
			rChip->AsFunctionCall()->SetChipType(chip->GetChipDesc().type);
	}
	else if (ct == PARAMETER) {
		rChip = AddChip(engine->GetChipManager()->GetChipTypeIndex(PARAMETER_GUID), ced->folder, ced->pos);
		if (rChip)
			rChip->AsParameter()->SetChipType(chip->GetChipDesc().type);
	}
	else if (ct == STANDARD) {
		rChip = AddChip(engine->GetChipManager()->GetChipTypeIndex(chip->AsProxyChip()->GetChipType()), ced->folder, ced->pos);
		if (rChip) {
			TemplateChip *temp = dynamic_cast<TemplateChip*>(chip);
			if (temp && temp->GetTemplate()) {
				rChip->CopyChip(temp->GetTemplate()); // return type?
			}
			else
				rChip->InitChip(); // return type?
		}
	}

	if (!rChip)
		return false;

	rChip->SetName(chip->GetName());
	rChip->GetChipEditorData()->comment = ced->comment;

	// Copy function settings
	if (ct == PROXY || ct == INSTANCE_DATA || ct == FUNCTION_DATA || ct == STANDARD) {
		if (chip->GetFunction()) {
			Function *f = rChip->CreateFunction();
			f->Set(chip->GetName(), chip->GetFunction()->GetType(), chip->GetFunction()->GetAccess(), chip->GetFunction()->GetParameters());
		}
	}

	// Create shortcuts
	Map<Chip*, Chip*> shortcutMap;
	shortcutMap.insert(std::make_pair(chip, rChip));
	List<Shortcut*> shortcuts = chip->GetShortcuts();
	for (size_t i = 0; i < shortcuts.size(); i++) {
		ChipEditorData *e = (ChipEditorData*)shortcuts[i]->GetChipEditorData();
		assert(e);
		Shortcut *sc = CreateShortcut(rChip, e->folder, e->pos);
		shortcutMap.insert(std::make_pair(shortcuts[i], sc));
	}

	// Replace children for all chips having our chip as child
	for (const auto& n : GetChips()) {
		const ChildConnectionList &children = n.second->GetChildren();
		for (unsigned i = 0; i < children.size(); i++) {
			if (children[i]) {
				for (unsigned j = 0; j < children[i]->connections.size(); j++) {
					auto itr = shortcutMap.find(children[i]->connections[j].chip);
					Chip *c = itr != shortcutMap.end() ? itr->second : nullptr;
					if (c)
						n.second->SetChild(c, i, j);
				}
			}
		}
	}

	// Set children (only for instance/function data)
	if (ct == INSTANCE_DATA || ct == FUNCTION_DATA) {
		for (unsigned i = 0; i < chip->GetChildren().size(); i++) {
			if (!chip->GetChildren()[i])
				continue;
			for (unsigned j = 0; j < chip->GetChildren()[i]->connections.size(); j++) {
				Chip *child = chip->GetChildren()[i]->connections[j].chip;
				rChip->SetChild(child, i, j);
			}
		}
	}

	// Remove old chip
	RemoveChip(chip);

	chip = rChip;

	return true;
}

void ClassExt::OnAddedToClassManager()
{
	ClassDiagram::Singleton()->OnClassAdded(this);

	String comment;
	List<std::pair<Guid, Vector2>> rgChips;
	Map<Guid, std::pair<unsigned, Guid>> baseGraphMapping;

	{
		ClassExtLoadInfo *nfo = (ClassExtLoadInfo*)GetLoadInfo(false);
		if (nfo) {
			comment = nfo->comment;
			rgChips = std::move(nfo->rgChips);
			baseGraphMapping = std::move(nfo->baseClassMapping);
		}
	}

	Class::OnAddedToClassManager(); // Note: LoadInfo is deleted during this call!

	assert(_rgChip);
	assert(_rgChip->GetChipEditorData());

	// TODO: We got to update RGs cg-editor-data as well, because here is a bug causing all cgs to be marked dirty when sometinhg is updated in rg..... (N/A?!?)
	
	_rgChip->GetChipEditorData()->comment = comment;
	if (!rgChips.empty()) {
		if (rgChips.front().first != NullGUID) {
			_rgChip->SetGlobalID(rgChips.front().first);
			_rgChip->GetChipEditorData()->pos = rgChips.front().second;
		}
		for (unsigned i = 1; i < rgChips.size(); i++) {
			Shortcut *sc = ClassDiagram::Singleton()->CreateShortcut(_rgChip, 0, rgChips[i].second);
			if (sc)
				sc->SetGlobalID(rgChips[i].first);
		}
	}

	ClassDiagram::Singleton()->SetConnectionMapping(this, baseGraphMapping);

	ClassDiagram::Singleton()->UpdateRgChipData(this);
}

void ClassExt::OnRemovedFromClassManager()
{
	Class::OnRemovedFromClassManager();

	ClassDiagram::Singleton()->OnClassRemoved(this);
}

bool ClassExt::AddBaseClass(Class *base)
{
	if (Class::AddBaseClass(base)) {
		ClassDiagram::Singleton()->OnBaseClassAdded(this, base);
		return true;
	}
	return false;
}

bool ClassExt::RemoveBaseClass(Class *base)
{
	if (Class::RemoveBaseClass(base)) {
		ClassDiagram::Singleton()->OnBaseClassRemoved(this, base);
		return true;
	}
	return false;
}

ClassView *ClassExt::CreateView()
{
	return mmnew ClassView(this); 
}

void ClassExt::DestroyView(ClassView *view)
{
	mmdelete(view);
}

void ClassExt::GetVisualExtent(FolderID folder, Vector2 &a, Vector2 &b) const
{
	a = Vector2(std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max());
	b = Vector2(-std::numeric_limits<float32>::max(), -std::numeric_limits<float32>::max());

	// Include chips in folder
	const ChipPtrByChipIDMap &chips = GetChips();
	for (const auto &n : chips) {
		ChipEditorData *ced = n.second->GetChipEditorData();
		assert(ced);
		if (ced->folder == folder) { // chip in same folder?
			float32 w = CHIP_HEIGHT + CONNECTION_WIDTH * (float32)ced->GetNumberOfChildConnections();
			a = Vector2(std::min(ced->pos.x - w * 0.5f, a.x), std::min(ced->pos.y - CHIP_HEIGHT * 0.5f, a.y));
			b = Vector2(std::max(ced->pos.x + w * 0.5f, b.x), std::max(ced->pos.y + CHIP_HEIGHT * 0.5f, b.y));
		}
	}

	// Include direct sub folders
	for (unsigned i = 0; i < _folders.size(); i++) {
		const Folder &f = _folders[i];
		if (f.parentFolder == folder) {
			a = Vector2(std::min(f.pos.x - CHIP_HEIGHT * 0.5f, a.x), std::min(f.pos.y - CHIP_HEIGHT * 0.5f, a.y));
			b = Vector2(std::max(f.pos.x + CHIP_HEIGHT * 0.5f, b.x), std::max(f.pos.y + CHIP_HEIGHT * 0.5f, b.y));
		}
	}

	// Include rect&text

	if (a == Vector2(std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max()) && b == Vector2(-std::numeric_limits<float32>::max(), -std::numeric_limits<float32>::max()))
		a = b = Vector2(0.0f, 0.0f);
}

#include "NodePositioning.h"


PNODE _arrange(PNODE parent, Chip *c, ClassExt *cg, FolderID f, Set<Chip*> &chips, Set<FolderID> &folders)
{
	assert(c);

	if (!chips.insert(c).second)
		return nullptr; // cyclic!

	PNODE n = parent;

	ChipEditorData *ced = c->GetChipEditorData();

	if (ced->folder == f) { // same folder?
		n = CreateNode(parent);
		if (!n)
			return nullptr;
		n->xCoordinate = ced->pos.x;
		n->yCoordinate = ced->pos.y;
		// Note: We can't rely on c->width because it might not be up to date!
		n->width = CHIP_HEIGHT + CONNECTION_WIDTH * (float32)ced->GetNumberOfChildConnections();
		n->chip = c;
		n->folder = 0;
	}
	else {
		FolderID closest = cg->GetClosestFolder(ced->folder, f);
		if (cg->IsFolderValid(closest)) {
			bool upFolder = cg->GetFolderList()[f].parentFolder == closest;
			const Folder &folder = cg->GetFolderList()[upFolder ? f : closest];
			if (folders.insert(upFolder ? f : closest).second) {
				n = CreateNode(parent);
				if (!n)
					return nullptr;
				n->xCoordinate = upFolder ? folder.inPos.x : folder.pos.x;
				n->yCoordinate = upFolder ? folder.inPos.y : folder.pos.y;
				n->width = CHIP_HEIGHT; // this is also width of a folder!
				n->chip = nullptr;
				n->folder = upFolder ? f : closest;
			}
		}
	}

	const ChildConnectionList &children = c->GetChildren();
	for (unsigned i = 0, h = 0; i < children.size(); i++, h++) { // Iterate child connections
		ChildConnection *cc = children[i];
		if (!cc) // Empty connection?
			continue;
		for (unsigned j = 0; j < cc->connections.size(); j++, h += (cc->desc.connType == ChildConnectionDesc::GROWING ? 1 : 0)) { // Iterate children
			Chip *child = cc->connections[j].chip;
			if (!child || child->GetClass() != cg) 
				continue; // Has the chip been linked to a chip in another class? That's not good!
			_arrange(n, child, cg, f, chips, folders); // ignore return value
		}
	}
	return n;
}

void _repos(ClassExt *cg, FolderID f, PNODE n, float32 extraY)
{
	if (!n)
		return;
	if (n->chip)
		n->chip->GetChipEditorData()->pos = Vector2(n->xCoordinate, n->yCoordinate - extraY);
	else {
		assert(n->folder);
		if (n->folder == f)
			cg->SetFolderInPos(n->folder, Vector2(n->xCoordinate, n->yCoordinate - extraY));
		else 
			cg->SetFolderPos(n->folder, Vector2(n->xCoordinate, n->yCoordinate - extraY));
	}
	_repos(cg, f, n->rightsibling, extraY);
	if (n->offspring) {
		float32 a, b;
		a = n->offspring->xCoordinate;
		for (PNODE m = n->offspring; m != nullptr; m = m->rightsibling)
			b = m->xCoordinate;
		_repos(cg, f, n->offspring, extraY + (b - a) * 0.15f);
	}
}

void ClassExt::ArrangeChildren(Chip *ch)
{
	if (!ch || ch->GetClass() != this)
		return;

	Set<Chip*> chips;
	Set<FolderID> folders;

	PNODE n = _arrange(nullptr, ch, this, ch->GetChipEditorData()->folder, chips, folders);
	if (TreePosition(n))
		_repos(this, ch->GetChipEditorData()->folder, n, 0);
	DestroyNodes(n);
}

const PublishSettings *ClassExt::GetPublishProfile(String profile) const
{
	auto n = _publishSettings.find(profile);
	return n != _publishSettings.end() ? &n->second : nullptr;
}

void ClassExt::SetPublishProfile(const PublishSettings &s)
{
	_publishSettings[s.profile] = s;
}

void ClassExt::RemovePublishProfile(String profile)
{
	_publishSettings.erase(profile);
}

void ClassExt::RemovePublishProfiles()
{
	_publishSettings.clear();
}

ClassLoadInfo *ClassExt::CreateLoadInfo() const
{
	return mmnew ClassExtLoadInfo();
}

void ClassExt::DeleteLoadInfo(ClassLoadInfo *nfo) const
{
	return mmdelete((ClassExtLoadInfo*)nfo);
}
