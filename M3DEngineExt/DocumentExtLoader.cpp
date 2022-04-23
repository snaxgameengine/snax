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
#include "DocumentExtLoader.h"
#include "M3DEngine/DocumentLoader.h"
#include "ClassDiagram.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngine/Chip.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "Publisher.h"
#include "StdChips/ClassDiagramChip.h"
#include "StdChips/Shortcut.h"
#include "M3DEngine/ClassInstance.h"


using namespace m3d;

struct LoadedItems
{
	Set<ChipID> &sChips;
	Set<CGBackgroundItemID> &sBackgroundItems;
	Set<FolderID> &sFolders;

	LoadedItems(Set<ChipID> &sChips, Set<CGBackgroundItemID> &sBackgroundItems, Set<FolderID> &sFolders) : sChips(sChips), sBackgroundItems(sBackgroundItems), sFolders(sFolders) {}
};

#define FROMINTPOS(x) (x * 0.01f)

bool __readFolderContent(ClassExt *cg, DocumentLoader &loader, FolderID folder, Vector2 offset, LoadedItems *li)
{
	bool ok = true;

	if (loader.GetDocumentVersion() > Version(1, 2, 5, 0))
		ok = ok && loader.EnterGroup(DocumentTags::Content);

	while (ok && loader.EnterGroup(DocumentTags::Chip)) {
		ChipID cid;
		Guid globalid;
		if (loader.GetAttribute(DocumentTags::chipid, globalid)) { // No prob is this fails!
			cid = loader.TranslateChipID(globalid);
			Chip *ch = cg->GetChip(cid);
			if (ch) {
				ch->GetChipEditorData()->folder = folder;
				Vector2 p(0.0f, 0.0f);
				if (loader.GetDocumentVersion() > Version(1, 2, 5, 0)) {
					int32 x = 0, y = 0;
					loader.GetAttribute(DocumentTags::x, x);
					loader.GetAttribute(DocumentTags::y, y);
					p = FROMINTPOS(Vector2(x, y));
				}
				else {
					// No prob if these fails!
					loader.GetAttribute(DocumentTags::x, p.x);
					loader.GetAttribute(DocumentTags::y, p.y);
				}
				ch->GetChipEditorData()->pos = p + offset;

				if (loader.GetDocumentVersion() > Version(1, 2, 5, 0)) {
					if (ok && loader.EnterGroup(DocumentTags::Comment)) {
						ok = ok && loader.ReadData(ch->GetChipEditorData()->comment);
						ok = ok && loader.LeaveGroup(DocumentTags::Comment);
					}
				}

				if (li)
					li->sChips.insert(cid);
			}
		}
		ok = ok && loader.LeaveGroup(DocumentTags::Chip);
	}
	while (ok && loader.EnterGroup(DocumentTags::Rect)) {
		Vector2 pos(0.0f, 0.0f), size(2.0f, 2.0f);
		Vector4 color(0.0f, 0.0f, 0.0f, 1.0f);
		if (loader.GetDocumentVersion() > Version(1, 2, 5, 0)) {
			int32 x = 0, y = 0, sx = 0, sy = 0;
			loader.GetAttribute(DocumentTags::x, x);
			loader.GetAttribute(DocumentTags::y, y);
			loader.GetAttribute(DocumentTags::sx, sx);
			loader.GetAttribute(DocumentTags::sy, sy);
			pos = FROMINTPOS(Vector2(x, y));
			size = FROMINTPOS(Vector2(sx, sy));
		}
		else {
			loader.GetAttribute(DocumentTags::x, pos.x);
			loader.GetAttribute(DocumentTags::y, pos.y);
			loader.GetAttribute(DocumentTags::sx, size.x);
			loader.GetAttribute(DocumentTags::sy, size.y);
		}
		loader.GetAttribute(DocumentTags::color, (float32*)&color, 4);
		cg->AddRectangle(folder, pos + offset, size, color);
		ok = ok && loader.LeaveGroup(DocumentTags::Rect);
	}
	while (ok && loader.EnterGroup(DocumentTags::Image)) {
		Vector2 pos(0.0f, 0.0f), size(2.0f, 2.0f);
		DataBuffer db;
		if (loader.GetDocumentVersion() > Version(1, 2, 5, 0)) {
			int32 x = 0, y = 0, sx = 0, sy = 0;
			loader.GetAttribute(DocumentTags::x, x);
			loader.GetAttribute(DocumentTags::y, y);
			loader.GetAttribute(DocumentTags::sx, sx);
			loader.GetAttribute(DocumentTags::sy, sy);
			pos = FROMINTPOS(Vector2(x, y));
			size = FROMINTPOS(Vector2(sx, sy));
		}
		else {
			loader.GetAttribute(DocumentTags::x, pos.x);
			loader.GetAttribute(DocumentTags::y, pos.y);
			loader.GetAttribute(DocumentTags::sx, size.x);
			loader.GetAttribute(DocumentTags::sy, size.y);
		}
		if (loader.ReadData(db))
			cg->AddImage(folder, pos + offset, size, db);
		ok = ok && loader.LeaveGroup(DocumentTags::Image);
	}
	while (ok && loader.EnterGroup(DocumentTags::Text)) {
		Vector2 pos(0.0f, 0.0f), size(2.0f, 2.0f);
		Vector4 color(0.0f, 0.0f, 0.0f, 1.0f);
		float32 textSize = 0.5f;
		String text;
		if (loader.GetDocumentVersion() > Version(1, 2, 5, 0)) {
			int32 x = 0, y = 0, sx = 0, sy = 0;
			loader.GetAttribute(DocumentTags::x, x);
			loader.GetAttribute(DocumentTags::y, y);
			loader.GetAttribute(DocumentTags::sx, sx);
			loader.GetAttribute(DocumentTags::sy, sy);
			pos = FROMINTPOS(Vector2(x, y));
			size = FROMINTPOS(Vector2(sx, sy));
		}
		else {
			loader.GetAttribute(DocumentTags::x, pos.x);
			loader.GetAttribute(DocumentTags::y, pos.y);
			loader.GetAttribute(DocumentTags::sx, size.x);
			loader.GetAttribute(DocumentTags::sy, size.y);
		}
		loader.GetAttribute(DocumentTags::color, (float32*)&color, 4);
		loader.GetAttribute(DocumentTags::textSize, textSize);
		if (loader.ReadData(text))
			cg->AddText(folder, pos + offset, size, color, text, textSize);
		ok = ok && loader.LeaveGroup(DocumentTags::Text);
	}
	while (ok && loader.EnterGroup(DocumentTags::Folder)) {
		String n;
		Vector2 p(0.0f, 0.0f), inp(0.0f, 0.0f);
		// Ignore return value for the next!
		loader.GetAttribute(DocumentTags::name, n);
		if (loader.GetDocumentVersion() > Version(1, 2, 5, 0)) {
			int32 x = 0, y = 0, ix = 0, iy = 0;
			loader.GetAttribute(DocumentTags::x, x);
			loader.GetAttribute(DocumentTags::y, y);
			loader.GetAttribute(DocumentTags::inx, ix);
			loader.GetAttribute(DocumentTags::iny, iy);
			p = FROMINTPOS(Vector2(x, y));
			inp = FROMINTPOS(Vector2(ix, iy));
		}
		else {
			loader.GetAttribute(DocumentTags::x, p.x);
			loader.GetAttribute(DocumentTags::y, p.y);
			loader.GetAttribute(DocumentTags::sx, inp.x);
			loader.GetAttribute(DocumentTags::sy, inp.y);
		}
		FolderID f = cg->CreateFolder(folder, p + offset, inp, n);
		ok = ok && __readFolderContent(cg, loader, f, Vector2(0.0f, 0.0f), 0);
		if (ok && li)
			li->sFolders.insert(f);
		ok = ok && loader.LeaveGroup(DocumentTags::Folder);
	}

	if (loader.GetDocumentVersion() > Version(1, 2, 5, 0))
		ok = ok && loader.LeaveGroup(DocumentTags::Content);

	return ok;
}

bool ReadClassDiagram(ClassExt *cg, DocumentLoader &loader)
{
	bool ok = true;
	ClassExtLoadInfo *li = (ClassExtLoadInfo*)cg->GetLoadInfo();
	assert(li);

	if (ok && loader.EnterGroup(DocumentTags::ClassDiagram)) {
		if (ok && loader.EnterGroup(DocumentTags::Comment)) {
			loader.ReadData(li->comment);
			ok = ok && loader.LeaveGroup(DocumentTags::Comment);
		}
		if (ok && loader.EnterGroup(DocumentTags::Chips)) {
			while (ok && loader.EnterGroup(DocumentTags::Chip)) {
				std::pair<Guid, Vector2> p;
				int32 ip[2] = { 0, 0 };
				if (loader.GetAttribute(DocumentTags::chipid, p.first) && (loader.GetDocumentVersion() > Version(1, 2, 5, 0) ? (loader.GetAttribute(DocumentTags::x, ip[0]) && loader.GetAttribute(DocumentTags::y, ip[1])) : (loader.GetAttribute(DocumentTags::x, p.second.x) && loader.GetAttribute(DocumentTags::y, p.second.y)))) {
					p.second = FROMINTPOS(Vector2(ip[0], ip[1]));
					li->rgChips.push_back(p);
				}
				else if (li->rgChips.empty())
					li->rgChips.push_back(std::make_pair(NullGUID, Vector2(0.0f, 0.0f)));
				ok = ok && loader.LeaveGroup(DocumentTags::Chip);
			}
			ok = ok && loader.LeaveGroup(DocumentTags::Chips);
		}
		if (ok && loader.EnterGroup(DocumentTags::Inheritance)) {
			while (ok && loader.EnterGroup(DocumentTags::Class)) {
				Guid guid;
				unsigned subconn = 0;
				Guid chipid;
				if (loader.GetAttribute(DocumentTags::id, guid) && loader.GetAttribute(DocumentTags::subconn, subconn) && loader.GetAttribute(DocumentTags::chipid, chipid))
					li->baseClassMapping.insert(std::make_pair(guid, std::make_pair(subconn, chipid)));
				ok = ok && loader.LeaveGroup(DocumentTags::Class);
			}
			ok = ok && loader.LeaveGroup(DocumentTags::Inheritance);
		}
		ok = ok && loader.LeaveGroup(DocumentTags::ClassDiagram);
	}
	return ok;
}

bool ReadEditorData(ClassExt *cg, DocumentLoader &loader, FolderID startFolder, Vector2 offset, LoadedItems *li, bool readClassVisualOnly)
{
	if (!loader.EnterGroup(DocumentTags::Editor))
		return true; // No editor data included!

	bool ok = true;

	if (ok && !readClassVisualOnly)
	{
		ok = ok && ReadClassDiagram(cg, loader);
		if (loader.GetDocumentVersion() > Version(1, 2, 4, 0)) {
			String classDesc;
			if (ok && loader.EnterGroup(DocumentTags::Description)) {
				ok = ok && loader.ReadData(classDesc);
				if (ok)
					cg->SetDescription(classDesc);
				ok = ok && loader.LeaveGroup(DocumentTags::Description);
			}
		}
		else if (loader.GetDocumentVersion() > Version(1, 2, 2, 0)) {
			String classDesc;
			ok = ok && loader.EnterGroup(DocumentTags::Description_old);
			ok = ok && loader.ReadData(classDesc);
			if (ok)
				cg->SetDescription(classDesc);
			ok = ok && loader.LeaveGroup(DocumentTags::Description_old);
		}
		if (loader.GetDocumentVersion() < Version(1, 2, 6, 0)) {
			if (ok && loader.EnterGroup(DocumentTags::Comments)) {
				while (ok && loader.EnterGroup(DocumentTags::Comment)) {
					Guid globalid;
					if (loader.GetAttribute(DocumentTags::chipid, globalid)) {
						Chip* cv = cg->GetChip(loader.TranslateChipID(globalid));
						if (cv && loader.ReadData(cv->GetChipEditorData()->comment)) {}
					}
					ok = ok && loader.LeaveGroup(DocumentTags::Comment);
				}
				ok = ok && loader.LeaveGroup(DocumentTags::Comments);
			}
		}
	}
	ok = ok && loader.EnterGroup(DocumentTags::ClassVisual);
	ok = ok && __readFolderContent(cg, loader, startFolder, offset, li);
	ok = ok && loader.LeaveGroup(DocumentTags::ClassVisual);
	if (ok && !readClassVisualOnly) {
		if (loader.GetDocumentVersion() < Version(1,2,0,0)) { // TODO: Remove this!
			assert(false);
		}
		else { // From version 1.2.0.0:
			if (ok && loader.EnterGroup(DocumentTags::PublishProfiles)) {
				while (ok && loader.EnterGroup(DocumentTags::Profile)) {
					PublishSettings ps;
					ok = ok && loader.ReadData(ps);
					ok = ok && loader.LeaveGroup(DocumentTags::Profile);
					if (ok)
						cg->SetPublishProfile(ps);
				}
				ok = ok && loader.LeaveGroup(DocumentTags::PublishProfiles);
			}
		}
	}

	ok = ok && loader.LeaveGroup(DocumentTags::Editor);
	return ok;
}


bool DocumentExtLoader::LoadExtData(ClassExt *cg, DocumentLoader &loader)
{
	return ReadEditorData(cg, loader, MainFolderID, Vector2(0.0f, 0.0f), 0, false);
}

bool DocumentExtLoader::Paste(ClassExt *cg, DocumentLoader &loader, FolderID folder, Vector2 pos, Set<ChipID> &sChips, Set<CGBackgroundItemID> &sBackgroundItems, Set<FolderID> &sFolders)
{
	bool ok = true;

	ClassInstancePtrByGUIDMap instances;

	loader.SetKeepGlobalIDs(false);
	LoadedItems li(sChips, sBackgroundItems, sFolders);
	ok = ok && loader.EnterGroup(DocumentTags::SelectionCopy);
	ok = ok && loader.LoadChips(cg, &instances);
	ok = ok && ReadEditorData(cg, loader, folder, pos, &li, true);
	ok = ok && loader.LeaveGroup(DocumentTags::SelectionCopy);

	if (ok) {
		for (const auto &n : instances) {
			n.second->CompleteLoading(n.second->GetOwner() != nullptr); // Do change if we have an owner, ie we pasted an instance - not only a ref.
		}
	}

	return ok;
}

bool DocumentExtLoader::InsertTemplate(ClassExt *cg, DocumentLoader &loader, String name, FolderID folder, Vector2 pos, bool updateStartchip, Set<ChipID> &sChips, Set<CGBackgroundItemID> &sBackgroundItems, Set<FolderID> &sFolders)
{
	bool ok = true;

	ClassInstancePtrByGUIDMap instances;

	loader.SetKeepGlobalIDs(false);
	LoadedItems li(sChips, sBackgroundItems, sFolders);
	Guid startchipid = NullGUID;
	if (loader.GetDocumentVersion() > Version(1, 2, 5, 0))
		ok = ok && loader.EnterGroup(DocumentTags::Classes);
	ok = ok && loader.EnterGroup(DocumentTags::Class, DocumentTags::name, name);
	ok = ok && loader.GetAttribute(DocumentTags::startchipid, startchipid);
	ok = ok && loader.LoadChips(cg, &instances);
	ok = ok && ReadEditorData(cg, loader, folder, pos, &li, true);
	ok = ok && loader.LeaveGroup(DocumentTags::Class);
	if (loader.GetDocumentVersion() > Version(1, 2, 5, 0))
		ok = ok && loader.LeaveGroup(DocumentTags::Classes);

	if (updateStartchip && startchipid != NullGUID) {
		ChipID cid = loader.TranslateChipID(startchipid);
		if (cid != InvalidChipID) {
			Chip *c = cg->FindChip(cid);
			if (c)
				cg->SetStartChip(c);
		}
	}

	// TODO: What about instances in this case?!

	return ok;
}


