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
#include "DocumentSaver.h"
#include "Class.h"
#include "DocumentSaveLoadUtil.h"
#include "StdChips/Shortcut.h"
#include "Document.h"
#include "Engine.h"
#include "ChipManager.h"
#include "Environment.h"

using namespace m3d;

bool DocumentSaver::_finalize()
{
	GUIDByVersionMap chipTypes;

	for (const auto &n : _chipTypes) {
		const ChipInfo *nfo = engine->GetChipManager()->GetChipInfo(n);
		if (!nfo)
			return false; // should not happen!
		if (chipTypes.insert(std::make_pair(nfo->chipDesc.type, nfo->chipDesc.version)).second) {
			for (size_t i = 0; i < nfo->baseguids.size(); i++) {
				const ChipInfo *nfo2 = engine->GetChipManager()->GetChipInfo(nfo->baseguids[i]);
				if (!nfo2)
					return false; // should not happen!
				if (!chipTypes.insert(std::make_pair(nfo2->chipDesc.type, nfo2->chipDesc.version)).second)
					break; // all base types already added!
			}
		}
	}

	bool ok = true;

	// Write document version.
	ok = ok && SetAttribute(DocumentTags::version, DocumentVersion);

	// Write the version for ALL chips encountered (also base types) when saving.
	ok = ok && PushGroup(DocumentTags::Chips);
	for (const auto &n : chipTypes) {
		ok = ok && PushGroup(DocumentTags::Chip);
		ok = ok && SetAttribute(DocumentTags::type, n.first);
		ok = ok && SetAttribute(DocumentTags::version, n.second);
		ok = ok && PopGroup(DocumentTags::Chip);
	}
	ok = ok && PopGroup(DocumentTags::Chips);

	return true;
}

bool DocumentSaver::SaveClasss(Document *doc)
{
	assert(doc);

	bool ok = true;

	Guid startCG = NullGUID;

	ok = ok && PushGroup(DocumentTags::Classes);

	for (size_t i = 0; i < doc->GetClasss().size(); i++) {
		Class *cg = doc->GetClasss()[i];
		assert(cg);
		ok = SaveClass(cg);
		if (!ok)
			break;
		if (doc->GetStartClass() == cg)
			startCG = cg->GetGuid();
	}

	ok = ok && PopGroup(DocumentTags::Classes);

	ok = ok && SetAttribute(DocumentTags::startclassid, startCG);

	return ok;
}

bool DocumentSaver::SaveClass(Class *cg)
{
	assert(cg);
	bool ok = true;
	Version version = VERSION1;
//	_currentCG = cg;
	ok = ok && PushGroup(DocumentTags::Class);
	ok = ok && SetAttribute(DocumentTags::name, cg->GetName());
	ok = ok && SetAttribute(DocumentTags::id, cg->GetGuid());
	ok = ok && SetAttribute(DocumentTags::startchipid, cg->GetStartChip() ? cg->GetStartChip()->GetGlobalID() : NullGUID);
	ok = ok && SetAttribute(DocumentTags::version, version); // This is a placeholder for future functionality only!

	// Inheritance
	{
		ok = ok && PushGroup(DocumentTags::Inheritance);
		for (const auto &n : cg->GetBaseClasses()) {
			ok = ok && PushGroup(DocumentTags::Class);
			ok = ok && SetAttribute(DocumentTags::id, n->GetGuid());
			ok = ok && SetAttribute(DocumentTags::filename, GetEnvironment()->CreateDocumentPath(n->GetDocument()->GetFileName(), cg->GetDocument()->GetFileName()));
			ok = ok && PopGroup(DocumentTags::Class);
		}
		if (cg->GetLoadInfo(false)) { // <= false: do not create the load info if it does not exist!
			for (const auto &n : cg->GetLoadInfo()->loadedBaseClasses) {
				ok = ok && PushGroup(DocumentTags::Class);
				ok = ok && SetAttribute(DocumentTags::id, n.first);
				ok = ok && SetAttribute(DocumentTags::filename, GetEnvironment()->CreateDocumentPath(n.second, cg->GetDocument()->GetFileName()));
				ok = ok && PopGroup(DocumentTags::Class);
			}
		}
		ok = ok && PopGroup(DocumentTags::Inheritance);
	}

	ok = ok && SaveChips(cg->GetChips());
	ok = ok && cg->SaveEditorData(*this);
	ok = ok && PopGroup(DocumentTags::Class);
//	_currentCG = nullptr;
	return ok;
}

bool DocumentSaver::SaveChips(const ChipPtrByChipIDMap&chips)
{
	bool ok = true;
	ok = ok && PushGroup(DocumentTags::Chips);
	for (const auto &n : chips) {
		ok = ok && SaveChip(n.second, &chips);
	}
	ok = ok && PopGroup(DocumentTags::Chips);

	return ok;
}

bool DocumentSaver::SaveChip(Chip *chip, const ChipPtrByChipIDMap*chips)
{
	assert(chip);
	bool ok = true;
	if (!IsSaveEditorData() && chip->AsShortcut())
		return true; // 1. Optimize away shortcuts! (Used together with 2. below)
	ok = ok && PushGroup(DocumentTags::Chip);
	ok = ok && SetAttribute(DocumentTags::chipid, chip->GetGlobalID());
	ok = ok && SetAttribute(DocumentTags::type, chip->GetChipDesc().type);
	ok = ok && SetAttribute(DocumentTags::name, chip->GetName());
	if (chip->GetRefreshManager().GetRefreshMode() != RefreshManager::RefreshMode::OncePerFunctionCall)
		ok = ok && SetAttribute(DocumentTags::refreshmode, chip->GetRefreshManager().GetRefreshMode()); // No need to write refresh mode if default!

	if (chips) {
		bool bHasPushedConnections = false; // No need to write connections if empty!
		for (uint32 i = 0; i < chip->GetChildren().size(); i++) {
			const ChildConnection* cc = chip->GetChildren()[i];
			if (cc) {
				bool isMulticonnection = cc->desc.connType == ChildConnectionDesc::MULTI;
				if (cc->connections.empty() && isMulticonnection) {
					if (!bHasPushedConnections)
						ok = ok && PushGroup(DocumentTags::Connections);
					bHasPushedConnections = true;
					ok = ok && PushGroup(DocumentTags::Connection);
					ok = ok && SetAttribute(DocumentTags::conn, i); // no need to write subconn (0) here!
					ok = ok && SetAttribute(DocumentTags::chipid, NullGUID);
					ok = ok && SetAttribute(DocumentTags::multiconnection, true);
					ok = ok && PopGroup(DocumentTags::Connection);
				}
				else {
					for (uint32 j = 0; j < cc->connections.size(); j++) {
						Chip* c = cc->connections[j].chip;
						if (!IsSaveEditorData() && c && c->AsShortcut())
							c = c->AsShortcut()->GetOriginal(); // 2. Optimize away shortcuts!
						if (c && chips->find(c->GetID()) == chips->end())
							c = nullptr; // The given child is not included among the chips to save. Then we skip it!
						if (!c && !isMulticonnection)
							continue;
						if (!bHasPushedConnections)
							ok = ok && PushGroup(DocumentTags::Connections);
						bHasPushedConnections = true;
						ok = ok && PushGroup(DocumentTags::Connection);
						ok = ok && SetAttribute(DocumentTags::conn, i);
						if (j > 0 || cc->desc.connType == ChildConnectionDesc::ConnectionType::GROWING)
							ok = ok && SetAttribute(DocumentTags::subconn, j); // Write subconnection only for growing!
						ok = ok && SetAttribute(DocumentTags::chipid, c ? c->GetGlobalID() : NullGUID);
						if (isMulticonnection)
							ok = ok && SetAttribute(DocumentTags::multiconnection, true);
						ok = ok && PopGroup(DocumentTags::Connection);
					}
				}
			}
		}
		if (bHasPushedConnections)
			ok = ok && PopGroup(DocumentTags::Connections);
	}

	if (chip->GetFunction()) {
		ok = ok && PushGroup(DocumentTags::Function);
		ok = ok && SetAttribute(DocumentTags::type, chip->GetFunction()->GetType());
		if (chip->GetFunction()->GetAccess() != Function::Access::Public)
			ok = ok && SetAttribute(DocumentTags::access, chip->GetFunction()->GetAccess()); // No need to write default access (Public)!
		ChipList cl;
		for (const auto &n : chip->GetFunction()->GetParameters()) {
			if (n.c >= cl.size())
				cl.resize(n.c + 1);
			cl[n.c] = (Chip*)n.p;
		}
		ok = ok && PushGroup(DocumentTags::Parameters);
		for (uint32 i = 0; i < cl.size(); i++) {
			ok = ok && PushGroup(DocumentTags::Parameter);
			ok = ok && SetAttribute(DocumentTags::chipid, cl[i] ? cl[i]->GetGlobalID() : NullGUID);
			ok = ok && PopGroup(DocumentTags::Parameter);
		}
		ok = ok && PopGroup(DocumentTags::Parameters);
		ok = ok && PopGroup(DocumentTags::Function);
	}

	ok = ok && PushGroup(DocumentTags::ChipData);
	Chip *oldCurrentChip = _currentChip;
	_currentChip = chip;
	ok = ok && chip->SaveChip(*this);
	_currentChip = oldCurrentChip;
	ok = ok && PopGroup(DocumentTags::ChipData);

	ok = ok && PopGroup(DocumentTags::Chip);

	// Add the type of the chip, so that we can write the global <Chips> section with versions later.
	_chipTypes.insert(engine->GetChipManager()->GetChipTypeIndex(chip->GetChipDesc().type));

	return ok;
}

Environment* DocumentSaver::GetEnvironment() const
{
	return engine->GetEnvironment();
}

