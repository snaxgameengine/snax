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
#include "DocumentExtSaver.h"
#include "M3DEngine/DocumentSaver.h"
#include "M3DEngine/Chip.h"
#include "ClassDiagram.h"
#include "StdChips/ClassDiagramChip.h"
#include "StdChips/Shortcut.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"

#include "Publisher.h"

using namespace m3d;

#define TOINTPOS(x) ((int32)std::floorf((x) * 100.0f + 0.5f))

struct FolderContent
{
	bool includeContent;
	List<const ChipEditorData*> chips;
	List<FolderID> folders;
	List<CGBackgroundItemMap::const_iterator> backgroundItems;
	FolderContent() : includeContent(false) {}
};

void IncludeContentRecursive(List<FolderContent> &folders, FolderID folder)
{
	folders[folder].includeContent = true;
	for (unsigned i = 0; i < folders[folder].folders.size(); i++)
		IncludeContentRecursive(folders, folders[folder].folders[i]);
}

void CreateEmptyFolderContentList(const ClassExt *cg, List<FolderContent> &folders)
{
	folders.clear();
	folders.resize(cg->GetFolderList().size());
	for (FolderID i = 1; i < folders.size(); i++) {
		if (cg->IsFolderValid(i)) {
			folders[cg->GetFolderList()[i].parentFolder].folders.push_back(i);
		}
	}
}

void SearchForFolderContent(const ClassExt *cg, List<FolderContent> &folders)
{
	for (const auto &n : cg->GetChips()) {
		ChipEditorData *cgi = n.second->GetChipEditorData();
		assert(cgi);
		if (!folders[cgi->folder].includeContent)
			continue;
		folders[cgi->folder].chips.push_back(cgi);
	}
	for (auto n = cg->GetBackgroundItems().begin(); n != cg->GetBackgroundItems().end(); n++) {
		if (!folders[n->second.folder].includeContent)
			continue;
		folders[n->second.folder].backgroundItems.push_back(n);
	}
}

bool WriteFolderContentRec(const ClassExt *cg, DocumentSaver &saver, FolderID folderID, const List<FolderContent> &folderContents, const Vector2 &posOffset)
{
	bool ok = true;
	ok = ok && saver.PushGroup(DocumentTags::Content);
	for (unsigned i = 0; i < folderContents[folderID].chips.size(); i++) {
		const ChipEditorData *ced = folderContents[folderID].chips[i];
		ok = ok && saver.PushGroup(DocumentTags::Chip);
		ok = ok && saver.SetAttribute(DocumentTags::chipid, ced->chip->GetGlobalID());
		ok = ok && saver.SetAttribute(DocumentTags::x, TOINTPOS(ced->pos.x + posOffset.x));
		ok = ok && saver.SetAttribute(DocumentTags::y, TOINTPOS(ced->pos.y + posOffset.y));
		if (!ced->comment.empty()) {
			ok = ok && saver.PushGroup(DocumentTags::Comment);
			ok = ok && saver.WriteData(ced->comment);
			ok = ok && saver.PopGroup(DocumentTags::Comment);
		}
		ok = ok && saver.PopGroup(DocumentTags::Chip);
	}
	for (unsigned i = 0; i < folderContents[folderID].backgroundItems.size(); i++) {
		const auto &n = folderContents[folderID].backgroundItems[i];
		switch (n->first.first) 
		{
		case CGBackgroundItem::RECT:
			ok = ok && saver.PushGroup(DocumentTags::Rect);
			ok = ok && saver.SetAttribute(DocumentTags::x, TOINTPOS(n->second.pos.x + posOffset.x));
			ok = ok && saver.SetAttribute(DocumentTags::y, TOINTPOS(n->second.pos.y + posOffset.y));
			ok = ok && saver.SetAttribute(DocumentTags::sx, TOINTPOS(n->second.size.x));
			ok = ok && saver.SetAttribute(DocumentTags::sy, TOINTPOS(n->second.size.y));
			ok = ok && saver.SetAttribute(DocumentTags::color, (const float32*)&n->second.color, 4);
			ok = ok && saver.PopGroup(DocumentTags::Rect);
			break;
		case CGBackgroundItem::IMAGE:
			ok = ok && saver.PushGroup(DocumentTags::Image);
			ok = ok && saver.SetAttribute(DocumentTags::x, TOINTPOS(n->second.pos.x + posOffset.x));
			ok = ok && saver.SetAttribute(DocumentTags::y, TOINTPOS(n->second.pos.y + posOffset.y));
			ok = ok && saver.SetAttribute(DocumentTags::sx, TOINTPOS(n->second.size.x));
			ok = ok && saver.SetAttribute(DocumentTags::sy, TOINTPOS(n->second.size.y));
			ok = ok && saver.WriteData(n->second.imageData);
			ok = ok && saver.PopGroup(DocumentTags::Image);
			break;
		case CGBackgroundItem::TEXT:
			ok = ok && saver.PushGroup(DocumentTags::Text);
			ok = ok && saver.SetAttribute(DocumentTags::x, TOINTPOS(n->second.pos.x + posOffset.x));
			ok = ok && saver.SetAttribute(DocumentTags::y, TOINTPOS(n->second.pos.y + posOffset.y));
			ok = ok && saver.SetAttribute(DocumentTags::sx, TOINTPOS(n->second.size.x));
			ok = ok && saver.SetAttribute(DocumentTags::sy, TOINTPOS(n->second.size.y));
			ok = ok && saver.SetAttribute(DocumentTags::color, (const float32*)&n->second.color, 4);
			ok = ok && saver.SetAttribute(DocumentTags::textSize, n->second.textSize);
			ok = ok && saver.WriteData(n->second.text);
			ok = ok && saver.PopGroup(DocumentTags::Text);
			break;
		}
	}
	for (unsigned i = 0; i < folderContents[folderID].folders.size(); i++) {
		FolderID f = folderContents[folderID].folders[i];
		ok = ok && saver.PushGroup(DocumentTags::Folder);
		ok = ok && saver.SetAttribute(DocumentTags::name, cg->GetFolderList()[f].name);
		ok = ok && saver.SetAttribute(DocumentTags::x, TOINTPOS(cg->GetFolderList()[f].pos.x + posOffset.x));
		ok = ok && saver.SetAttribute(DocumentTags::y, TOINTPOS(cg->GetFolderList()[f].pos.y + posOffset.y));
		ok = ok && saver.SetAttribute(DocumentTags::inx, TOINTPOS(cg->GetFolderList()[f].inPos.x)); // no +posOffset.x!
		ok = ok && saver.SetAttribute(DocumentTags::iny, TOINTPOS(cg->GetFolderList()[f].inPos.y)); // no +posOffset.y!
		ok = ok && WriteFolderContentRec(cg, saver, f, folderContents, Vector2(0.0f, 0.0f));
		ok = ok && saver.PopGroup(DocumentTags::Folder);
	}
	ok = ok && saver.PopGroup(DocumentTags::Content);
	return ok;
}

bool WriteClassDiagram(const ClassExt *cg, DocumentSaver &saver) // This section should not be written when doing copy/paste!
{
	ClassDiagramChip *ch = cg->GetClassDiagramChip();
	if (!ch)
		return true;

	bool ok = true;

	ChipEditorData *ced = ch->GetChipEditorData();
	assert(ced);
	ok = ok && saver.PushGroup(DocumentTags::ClassDiagram);
	if (!ced->comment.empty()) {
		ok = ok && saver.PushGroup(DocumentTags::Comment);
		ok = ok && saver.WriteData(ced->comment);
		ok = ok && saver.PopGroup(DocumentTags::Comment);
	}
	ok = ok && saver.PushGroup(DocumentTags::Chips);
	List<Shortcut*> shortcuts = ch->GetShortcuts();
	Chip *tmp = ch; 
	for (unsigned i = 0; true; i++) {
		ok = ok && saver.PushGroup(DocumentTags::Chip); // The first chip is the main ClassDiagramChip. Any following chips are shortcuts!
		ok = ok && saver.SetAttribute(DocumentTags::chipid, tmp->GetGlobalID());
		ok = ok && saver.SetAttribute(DocumentTags::x, TOINTPOS(tmp->GetChipEditorData()->pos.x));
		ok = ok && saver.SetAttribute(DocumentTags::y, TOINTPOS(tmp->GetChipEditorData()->pos.y));
		ok = ok && saver.PopGroup(DocumentTags::Chip);
		if (i == shortcuts.size())
			break;
		tmp = shortcuts[i];
	} 
	ok = ok && saver.PopGroup(DocumentTags::Chips);
	ok = ok && saver.PushGroup(DocumentTags::Inheritance);
	for (unsigned i = 0; i < ch->GetChildren()[0]->connections.size(); i++) {
		Chip *sc = ch->GetChildren()[0]->connections[i].chip;
		if (sc) {
			ClassDiagramChip *c = (ClassDiagramChip*)(sc->AsShortcut() ? sc->AsShortcut()->GetOriginal() : sc);
			ok = ok && saver.PushGroup(DocumentTags::Class);
			ok = ok && saver.SetAttribute(DocumentTags::id, c->GetCG()->GetGuid());
			ok = ok && saver.SetAttribute(DocumentTags::subconn, i);
			ok = ok && saver.SetAttribute(DocumentTags::chipid, sc->GetGlobalID()); // <= GlobalID is used to figure out which chip (main or shortcut) to be linked!
			ok = ok && saver.PopGroup(DocumentTags::Class);
		}
	}
	ok = ok && saver.PopGroup(DocumentTags::Inheritance);
	ok = ok && saver.PopGroup(DocumentTags::ClassDiagram);

	return ok;
}

Vector4 FindExtent(const ClassExt *cg, List<FolderContent> &folderContents, FolderID folderID, const Vector2 &posOffset)
{
	Vector4 v(std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max(), -std::numeric_limits<float32>::max(), -std::numeric_limits<float32>::max());
	Vector2 p;
	for (unsigned i = 0; i < folderContents[folderID].chips.size(); i++) {
		const ChipEditorData *cv = folderContents[folderID].chips[i];
		p = cv->pos + posOffset;
		float32 w = CHIP_HEIGHT;
		w = CHIP_HEIGHT + CONNECTION_WIDTH * (float32)cv->GetNumberOfChildConnections();
		v.x = std::min(p.x - w * 0.5f, v.x);
		v.y = std::min(p.y - CHIP_HEIGHT * 0.5f, v.y);
		v.z = std::max(p.x + w * 0.5f, v.z);
		v.w = std::max(p.y + CHIP_HEIGHT * 0.5f, v.w);
	}
	for (unsigned i = 0; i < folderContents[folderID].backgroundItems.size(); i++) {
		const auto &n = folderContents[folderID].backgroundItems[i];
		p = n->second.pos + posOffset;
		v.x = std::min(p.x - n->second.size.x * 0.5f, v.x);
		v.y = std::min(p.y - n->second.size.y * 0.5f, v.y);
		v.z = std::max(p.x + n->second.size.x * 0.5f, v.z);
		v.w = std::max(p.y + n->second.size.y * 0.5f, v.w);
	}
	for (unsigned i = 0; i < folderContents[folderID].folders.size(); i++) {
		FolderID f = folderContents[folderID].folders[i];
		p = cg->GetFolderList()[f].pos + posOffset;
		v.x = std::min(p.x - CHIP_HEIGHT * 0.5f, v.x);
		v.y = std::min(p.y - CHIP_HEIGHT * 0.5f, v.y);
		v.z = std::max(p.x + CHIP_HEIGHT * 0.5f, v.z);
		v.w = std::max(p.y + CHIP_HEIGHT * 0.5f, v.w);
	}
	if (v == Vector4(std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max(), -std::numeric_limits<float32>::max(), -std::numeric_limits<float32>::max()))
		v = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
	return v;
}

bool WriteEditorData(const ClassExt *cg, DocumentSaver &saver, List<FolderContent> &folders, FolderID startFolder, const Vector2 &posOffset, bool writeClassVisualOnly)
{
	bool ok = true;
	ok = ok && saver.PushGroup(DocumentTags::Editor);

	if (ok && !writeClassVisualOnly) {
		ok = ok && WriteClassDiagram(cg, saver);
		if (!cg->GetDescription().empty()) {
			ok = ok && saver.PushGroup(DocumentTags::Description);
			ok = ok && saver.WriteData(cg->GetDescription());
			ok = ok && saver.PopGroup(DocumentTags::Description);
		}
	}

	Vector4 v;
	if (ok)
		v = FindExtent(cg, folders, startFolder, posOffset);
	int32 vi[4] = { TOINTPOS(v.x), TOINTPOS(v.y), TOINTPOS(v.z), TOINTPOS(v.w) };
	ok = ok && saver.PushGroup(DocumentTags::ClassVisual);
	ok = ok && saver.SetAttribute(DocumentTags::extent, vi, 4);
	ok = ok && WriteFolderContentRec(cg, saver, startFolder, folders, posOffset);
	ok = ok && saver.PopGroup(DocumentTags::ClassVisual);

	if (ok && !writeClassVisualOnly)
	{
		const Map<String, PublishSettings>& ps = cg->GetPublishProfiles();
		if (!ps.empty()) {
			ok = ok && saver.PushGroup(DocumentTags::PublishProfiles);
			for (const auto& n : ps) {
				ok = ok && saver.PushGroup(DocumentTags::Profile);
				ok = ok && saver.WriteData(n.second);
				ok = ok && saver.PopGroup(DocumentTags::Profile);
			}
			ok = ok && saver.PopGroup(DocumentTags::PublishProfiles);
		}
	}

	ok = ok && saver.PopGroup(DocumentTags::Editor);
	return ok;
}

bool DocumentExtSaver::SaveExtData(const ClassExt *cg, DocumentSaver &saver)
{
	assert(cg);

	if (!saver.IsSaveEditorData())
		return true;

	List<FolderContent> folders;

	CreateEmptyFolderContentList(cg, folders);
	IncludeContentRecursive(folders, MainFolderID);
	SearchForFolderContent(cg, folders);

	return WriteEditorData(cg, saver, folders, MainFolderID, Vector2(0.0f, 0.0f), false);
}

bool DocumentExtSaver::CopySelection(ClassExt *cg, DocumentSaver &saver, FolderID folder, const Set<ChipID> &sChips, const Set<CGBackgroundItemID> &sBackgroundItems, const Set<FolderID> &sFolders)
{
	assert(cg);

	List<FolderContent> folders;
	CreateEmptyFolderContentList(cg, folders);

	Vector2 offset(0.0f, 0.0f);

	folders[folder].folders.clear(); // Erase all subfolders in current folder.
	for (const auto &n : sFolders) // Iterate folders in selection
		if (cg->IsFolderValid(n) && cg->GetFolderList()[n].parentFolder == folder)  {// Confirm folder is child of our folder
			folders[folder].folders.push_back(n); // Add folder
			offset += cg->GetFolderList()[n].pos;
		}

	if (folders[folder].folders.size()) { // Any subfolders confirmed in selection?
		IncludeContentRecursive(folders, folder); // Include all content in subfolders
		folders[folder].includeContent = false; // Do not include content in current folder!
		SearchForFolderContent(cg, folders); // Search for content in subfolders
	}

	for (const auto &n : sChips) { // Iterate all chips in selection
		Chip *ch = cg->GetChip(n);
		if (ch && ch->GetChipEditorData()->folder == folder) { // Confirm same folder as current.
			folders[folder].chips.push_back(ch->GetChipEditorData()); // Add to list
			offset += ch->GetChipEditorData()->pos;
		}
	}

	for (const auto &n : sBackgroundItems) { // Iterate all background items in selection
		auto m = cg->GetBackgroundItems().find(n);
		if (m != cg->GetBackgroundItems().end() && m->second.folder == folder) {
			folders[folder].backgroundItems.push_back(m);
			offset += m->second.pos;
		}
	}

	size_t itemCount = (unsigned)folders[folder].chips.size() + folders[folder].backgroundItems.size() + folders[folder].folders.size();
	if (itemCount > 0)
		offset /= (float32)itemCount;

	ChipPtrByChipIDMap chipsToSave;
	for (unsigned i = 0; i < folders.size(); i++) // Iterate all folders
		for (unsigned j = 0; j < folders[i].chips.size(); j++) // Iterate all chips
			chipsToSave.insert(std::make_pair(folders[i].chips[j]->chip->GetID(), folders[i].chips[j]->chip)); // Add to list of real chips to save.

	bool ok = true;

	ok = ok && saver.PushGroup(DocumentTags::SelectionCopy);
	ok = ok && saver.SaveChips(chipsToSave);
	ok = ok && WriteEditorData(cg, saver, folders, folder, -offset, true);
	ok = ok && saver.PopGroup(DocumentTags::SelectionCopy);

	return ok;
}