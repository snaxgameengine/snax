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
#include "DocumentLoader.h"
#include "DocumentManager.h"
#include "DocumentSaveLoadUtil.h"
#include "Document.h"
#include "Engine.h"
#include "ChipManager.h"
#include "ClassManager.h"
#include "Class.h"
#include "ClassInstance.h"
#include "StdChips/Shortcut.h"
#include "Environment.h"


using namespace m3d;


#define check(action, ifFailed, ...) if (!(action)) { lmsg(__VA_ARGS__); ifFailed; }


String _versionToString(Version v)
{
	return strUtils::ConstructString(MTEXT("%1.%2.%3.%4")).arg(strUtils::fromNum(v.elements[3])).arg(strUtils::fromNum(v.elements[2])).arg(strUtils::fromNum(v.elements[1])).arg(strUtils::fromNum(v.elements[0]));
}

bool DocumentLoader::_init()
{
	check(GetAttribute(DocumentTags::version, _version), _version = VERSION1, WARN, MTEXT("Failed reading document version. Assuming Version 1.0.0.0."));
	check(_version <= DocumentVersion, return false, FATAL, strUtils::ConstructString(MTEXT("This document is of a newer version (%1) then the current document format version (%2). Can not load it."))
		.arg(_versionToString(_version)).arg(_versionToString(DocumentVersion)));

	if (_version < Version(1,1,0,1))
		return true; // TODO: Remove this check!

	check(EnterGroup(DocumentTags::Chips), return false, FATAL, MTEXT("Could not find the list of chips used in the document (Document/Chips)."));

	while (EnterGroup(DocumentTags::Chip)) { // While we have chips in the list.
		Guid type;
		Version version;
		check(GetAttribute(DocumentTags::type, type), continue, WARN, MTEXT("Failed to read parameter \'type\' in data group Document/Chips/Chip."));
		check(GetAttribute(DocumentTags::version, version), version = VERSION1, WARN, MTEXT("Failed to read chip type version. Assuming version 1.0.0.0."));
		check(LeaveGroup(DocumentTags::Chip), goto lFail, FATAL, MTEXT("Failed to leave data group Document/Chips/Chip."));
		const ChipInfo *nfo = engine->GetChipManager()->GetChipInfo(type);
		check(nfo != nullptr, continue, WARN, strUtils::ConstructString(MTEXT("Found a chip type (%1) that is not available.")).arg(GuidToString(type)));
		check(nfo->chipDesc.version >= version, continue, WARN, strUtils::ConstructString(MTEXT("Found a chip type (%1) serialized as version %2 which is newer than the available version (%3)."))
			.arg(nfo->chipDesc.name).arg(_versionToString(version)).arg(_versionToString(nfo->chipDesc.version)));
		check(_chipVersions.insert(std::make_pair(type, version)).second,, WARN, strUtils::ConstructString(MTEXT("Found duplicate chip type (%1) in Document/Chips.")).arg(GuidToString(type)));
	}

	check(LeaveGroup(DocumentTags::Chips), goto lFail, FATAL, MTEXT("Failed to leave data group Document/Chips."));

	// TODO: Go through the list, and check if all base types are present in the list as well.

	return true;
lFail:
	return false;
}

void DocumentLoader::lmsg(MessageSeverity severity, String message, const Chip *chip)
{
	if (severity == WARN && !AllowingIrregularLoading())
		severity = FATAL; // Upgrade message.

	LMsg m = {severity, message};
	_messages.push_back(m);

	msg(severity, message, chip); // TODO: Remove!
}

Version DocumentLoader::GetChipTypeVersion(const Guid &type) const
{
	if (_version < Version(1,1,0,1))
		return VERSION1; // TODO: Remove!
	auto itr = _chipVersions.find(type);
	return itr != _chipVersions.end() ? itr->second : Version(0); 
}

bool DocumentLoader::SearchForClasses(ClassMetaList& classList, bool includeFunctions)
{
	classList.clear();

	bool ok = true;

	if (_version > Version(1, 2, 5, 0))
		ok = ok && EnterGroup(DocumentTags::Classes);

	while (ok = ok && (_version > Version(1, 2, 5, 0) ? VerifyGroup(DocumentTags::Classes) : VerifyGroup(DocumentTags::Document))) {
		ClassMeta cm;
		String name;
		Guid startchipid;
		Map<Guid, size_t> functionIndexByChipGuid;
		if (!EnterGroup(DocumentTags::Class))
			break;
		ok = ok && GetAttribute(DocumentTags::name, cm.name);
		if (includeFunctions) {
			ok = ok && EnterGroup(DocumentTags::Chips);
			while (ok && EnterGroup(DocumentTags::Chip)) {
				Guid globalId;
				Guid type;
				String name;
				ok = ok && GetAttribute(DocumentTags::chipid, globalId);
				ok = ok && GetAttribute(DocumentTags::type, type);
				ok = ok && GetAttribute(DocumentTags::name, name);
				if (ok && EnterGroup(DocumentTags::Function)) {
					Function::Type ft;
					Function::Access fa;
					ok = ok && GetAttribute(DocumentTags::type, ft);
					if (ok && !GetAttribute(DocumentTags::access, fa))
						fa = Function::Access::Public; // default
					ok = LeaveGroup(DocumentTags::Function) && ok;
					if (ok) {
						FunctionMeta fm;
						fm.fa = fa;
						fm.ft = ft;
						fm.type = type;
						fm.name = name;
						cm.functions.push_back(fm);
						functionIndexByChipGuid.insert(std::make_pair(globalId, cm.functions.size() - 1));
					}
				}
				ok = LeaveGroup(DocumentTags::Chip) && ok;
			}
			ok = LeaveGroup(DocumentTags::Chips) && ok;
		}
		if (EnterGroup(DocumentTags::Editor)) // If editor data is included...
		{
			if (GetDocumentVersion() > Version(1, 2, 4, 0)) {
				if (ok && EnterGroup(DocumentTags::Description)) {
					ok = ok && ReadData(cm.description);
					ok = LeaveGroup(DocumentTags::Description) && ok;
				}
			}
			else if (GetDocumentVersion() > Version(1, 2, 2, 0)) {
				ok = ok && EnterGroup(DocumentTags::Description_old);
				ok = ok && ReadData(cm.description);
				ok = LeaveGroup(DocumentTags::Description_old) && ok;
			}
			if (includeFunctions) {
				if (ok && EnterGroup(DocumentTags::Comments)) {
					while (ok && EnterGroup(DocumentTags::Comment)) {
						Guid globalid;
						if (GetAttribute(DocumentTags::chipid, globalid)) {
							auto n = functionIndexByChipGuid.find(globalid);
							if (n != functionIndexByChipGuid.end()) {
								String comment;
								if (ReadData(comment)) {
									cm.functions[n->second].comment = comment;
								}
							}
						}
						ok = LeaveGroup(DocumentTags::Comment) && ok;
					}
					ok = LeaveGroup(DocumentTags::Comments) && ok;
				}
			}
			ok = LeaveGroup(DocumentTags::Editor) && ok;
		}

		ok = ok && GetAttribute(DocumentTags::startchipid, startchipid);
		ok = LeaveGroup(DocumentTags::Class) && ok;

		if (ok) {
			cm.hasStartchip = startchipid != NullGUID;
			classList.push_back(cm);
		}
	}

	if (_version > Version(1, 2, 5, 0))
		ok = LeaveGroup(DocumentTags::Classes) && ok;

	return ok;
}

bool DocumentLoader::LoadDocument(Document *doc)
{
	assert(doc);

	Guid startCG;
	bool startFound = false;

	lmsg(INFO, MTEXT("Loading document \'") + doc->GetFileName().AsString() + MTEXT("\'."));

	check(GetAttribute(DocumentTags::startclassid, startCG), startCG = NullGUID, WARN, MTEXT("Failed reading \'startcgid\' in data group Document."));

	if (_version > Version(1, 2, 5, 0)) {
		check(EnterGroup(DocumentTags::Classes), return false, FATAL, MTEXT("Document parsing error."));
	}

	while (true) {
		check((_version > Version(1, 2, 5, 0) ? VerifyGroup(DocumentTags::Classes) : VerifyGroup(DocumentTags::Document)), break, FATAL, MTEXT("Document parsing error."));
		Class *cg = nullptr;

		check(LoadClass(&cg, doc), continue, WARN, MTEXT("Failed loading class."));
		if (cg == nullptr)
			break; // No more classes...
		if (cg->GetGuid() == startCG) {
			startFound = true;
			doc->SetStartClass(cg);
		}
	}

	// TODO: Check warnings...
//	doc->SetReadOnly(true); // Indicate that we should not save this document because it would overwrite stuff we don't want to overwrite.

	if (_version > Version(1, 2, 5, 0)) {
		check(LeaveGroup(DocumentTags::Classes), return false, FATAL, MTEXT("Document parsing error."));
	}

	return true;
}


bool DocumentLoader::LoadClass(Class **cg, Document *doc)
{
	assert(cg);
	assert(doc);

	*cg = nullptr;

	if (!EnterGroup(DocumentTags::Class))
		return true; // No more graphs found!

	String name;
	Guid id;
	Guid startid;

	check(GetAttribute(DocumentTags::id, id), GenerateGuid(id), WARN, MTEXT("Failed reading class id. Generating new id."));
	check(GetAttribute(DocumentTags::name, name), name = GuidToString(id), WARN, MTEXT("Failed reading class name. Using ID as name."));

	*cg = doc->CreateClass();
	check(*cg != nullptr, goto lFail, FATAL, MTEXT("Failed to create class."));

	(*cg)->SetName(name);
	(*cg)->SetGuid(id);

	// Inheritance
	check(EnterGroup(DocumentTags::Inheritance), goto lSkipInheritance, WARN, MTEXT("Failed to enter data group Document/Class/Inheritance. Skipping it."));

	while (EnterGroup(DocumentTags::Class)) { // While we have classes we inherit..
		Guid baseid;
		String fn;
		check(GetAttribute(DocumentTags::id, baseid), continue, WARN, MTEXT("Failed to read parameter \'baseid\' in data group Document/Class/Inheritance/Class."));
		check(GetAttribute(DocumentTags::filename, fn), fn.clear(), WARN, MTEXT("Failed to read parameter \'filename\' in data group Document/Class/Inheritance/Class."));
		Path filename;
		if (!fn.empty())
			filename = GetEnvironment()->ResolveDocumentPath(fn, doc->GetFileName());
		check((*cg)->GetLoadInfo()->loadedBaseClasses.insert(std::make_pair(baseid, filename)).second, continue, WARN, MTEXT("Failed adding base class."));
		check(LeaveGroup(DocumentTags::Class), goto lFail, FATAL, MTEXT("Failed to leave data group Document/Class/Inheritance/Class."));

		// Should we start the loading process of the base graphs? This is done async.
		if (_loadRelatedDocumentsAsync && filename.IsFile()) {
			Path fn = engine->GetDocumentManager()->PreloadDocument(filename, &baseid);
			if (fn.IsFile() && fn != filename) { // The file we are actually loading is different from the one we expected?
				msg(WARN, strUtils::ConstructString(MTEXT("Class \'%1\' (%4) tried to load base class \'%3\' from \'%2\', but found it in \'%5\'. Please resave!")).arg((*cg)->GetName()).arg(filename.AsString()).arg(GuidToString(baseid)).arg(doc->GetFileName().GetName()).arg(fn.AsString()));
				doc->SetDirty(); // Mark document as dirty so we can encourage user to save it with the updated filename!
			}
		}
	}
	
	check(LeaveGroup(DocumentTags::Inheritance), goto lFail, FATAL, MTEXT("Failed to leave data group Document/Class/Inheritance."));

lSkipInheritance:
	check(LoadChips(*cg, &(*cg)->GetLoadInfo()->instances),, WARN, MTEXT("Failed to load chips."));
	check((*cg)->LoadEditorData(*this),, WARN, MTEXT("Failed loading editor data."));
	
	check(GetAttribute(DocumentTags::startchipid, startid), startid = NullGUID, WARN, MTEXT("Failed reading start chip id."));

	if (startid != NullGUID) {
		Chip *ch = (*cg)->GetChip(TranslateChipID(startid));
		check(ch != nullptr,, WARN, MTEXT("Failed to find start chip."));
		(*cg)->SetStartChip(ch);
	}

	check(LeaveGroup(DocumentTags::Class), goto lFail, FATAL, MTEXT("Failed to leave data group Document/Class."));

	_idTranslationMap.clear();

	return true;

lFail:
	_idTranslationMap.clear();
	if (*cg)
		doc->RemoveClass(*cg);
	*cg = nullptr;
	check(LeaveGroup(DocumentTags::Class),, FATAL, MTEXT("Failed to leave data group Document/Class."));
	return false;
}


bool DocumentLoader::LoadChips(Class *cg, ClassInstancePtrByGUIDMap *instances)
{
	assert(cg);
	assert(instances);
	assert(_instances == nullptr);
	assert(_connections.empty());
	assert(_functions.empty());
	assert(_loadedShortcuts.empty());
	assert(_currentChip == nullptr);

	check(EnterGroup(DocumentTags::Chips), goto lFail, WARN, MTEXT("Failed to enter data group Document/Class/Chips."));

	// Set this so instances in chips can update it.
	_instances = instances;

	// Load chips
	while (true) {
		check(VerifyGroup(DocumentTags::Chips), goto lFail, FATAL, MTEXT("Document parsing error in data group Document/Class/Chips."));
		Chip *chip = nullptr;
		check(LoadChip(&chip, cg), continue, WARN, MTEXT("Failed to load chip."));
		if (chip == nullptr)
			break; // ok, no more chips found!
	}

	_instances = nullptr; // No need for _instances no more!

	check(LeaveGroup(DocumentTags::Chips), goto lFail, FATAL, MTEXT("Failed to leave data group Document/Class/Chips."));


	// Link all shortcuts to their original ones. (Only if we do not optimize them away below)
	if (!_optimize) {
		for (const auto &p : _loadedShortcuts) {
			Chip *original = cg->GetChip(TranslateChipID(p.second));
			if (!original) {
				// It could happen, during copy/paste, that we do not include the original chip.
				// We can now search the class for the original chip.
				for (const auto &n : cg->GetChips()) {
					if (n.second->GetGlobalID() == p.second) { // Note: there is no need to translate p->second because we are searching for something already in the cg.
						original = n.second;
						break;
					}
				}
			}
			
			check(original != nullptr,
				_connections.erase(p.first->GetID()); // Not really neccessary, shortcuts don't have connectors...
				_functions.erase(p.first->GetID()); // Not really neccessary, shortcuts don't have functions...
				cg->RemoveChip(p.first); // Remove the shortcut...
				continue, 
				WARN,
				strUtils::ConstructString(MTEXT("Could not find the original chip with id %1 for shortcut \'%2\' (id: %3)."))
				.arg(GuidToString(p.second))
				.arg(p.first->GetName())
				.arg(GuidToString(p.first->GetGlobalID()))
				);

			p.first->SetOriginal(original);
		}
	}

	// Do connections. At this stage, ALL chips are loaded and added to cg.
	for (const auto& n : _connections) { // Iterate chips with child connections
		Chip *ch = cg->GetChip(n.first);
		assert(ch);
		for (const auto& m : n.second) { // Iterate child connectors.
			if (m.second.isMulticonnection) {
				check(m.first.second == 0, ;, WARN, String(MTEXT("Unexpected multiconnection tag found...")));
				if (m.first.second == 0) {
					const ChildConnection* cc = ch->GetChildren()[m.first.first];
					if (cc && cc->desc.connType == ChildConnectionDesc::GROWING) {
						ChildConnectionDesc ccd = cc->desc;
						ccd.connType = ChildConnectionDesc::MULTI;
						ch->SetConnection(m.first.first, ccd);
					}
				}
			}

			if (m.second.guid == NullGUID)
				continue;

			Chip* child = cg->GetChip(TranslateChipID(m.second.guid));
			if (_optimize && child && child->AsShortcut()) { // If we're optimizing away shortcuts.
				auto p = _loadedShortcuts.find(child->AsShortcut());
				if (p != _loadedShortcuts.end()) {
					child = cg->GetChip(TranslateChipID(p->second)); // link to original...
				}
				else {
					child = nullptr; // something is wrong. We could not find the shortcut in the shortcut map!?
				}
			}

			check(
				child != nullptr,
				continue,
				WARN,
				strUtils::ConstructString(MTEXT("Could not find the chip with id %1 when searching for child linked to connector (%2, %3) at chip \'%4\' (id: %5) of type \'%6\'."))
				.arg(GuidToString(m.second.guid))
				.arg(strUtils::fromNum(m.first.first))
				.arg(strUtils::fromNum(m.first.second))
				.arg(ch->GetName())
				.arg(GuidToString(ch->GetGlobalID()))
				.arg(ch->GetChipDesc().name));

			check(
				ch->SetChild(child, m.first.first, m.first.second), 
				continue, 
				WARN,
				strUtils::ConstructString(MTEXT("Could not link chip \'%1\' (id: %2) of type \'%3\' as a child to connector (%4, %5) at chip \'%6\' (id: %7) of type \'%8\'."))
				.arg(child->GetName())
				.arg(GuidToString(child->GetGlobalID()))
				.arg(child->GetChipDesc().name)
				.arg(strUtils::fromNum(m.first.first))
				.arg(strUtils::fromNum(m.first.second))
				.arg(ch->GetName())
				.arg(GuidToString(ch->GetGlobalID()))
				.arg(ch->GetChipDesc().name)
				);
		}
	}
	_connections.clear();

	// Do functions
	for (const auto &n : _functions) { // Iterate all chips with functions
		Chip *ch = cg->GetChip(n.first);
		check(ch != nullptr, continue, WARN, strUtils::ConstructString(MTEXT("Chip not found when creating functions. All I know is that it should have been loaded with runtime id %1.")).arg(strUtils::fromNum(n.first)));
		ParameterConnectionSet parameters;
		for (uint32 i = 0, j = 0; i < n.second.parameters.size(); i++) { // Iterate parameters
			Chip *param = cg->GetChip(TranslateChipID(n.second.parameters[i])); // Find the parameter (green chip)
			check(
				param != nullptr && param->AsParameter(), 
				continue, 
				WARN, 
				strUtils::ConstructString(MTEXT("Parameter %1 (id: %2) not found for function \'%3\' (id: %4)."))
				.arg(strUtils::fromNum(i))
				.arg(GuidToString(n.second.parameters[i]))
				.arg(ch->GetName())
				.arg(GuidToString(ch->GetGlobalID()))
				);
			parameters.insert(ParameterConnection(param->AsParameter(), j++)); // TODO: check return type
		}
		Function *function = ch->CreateFunction();
		check(function != nullptr, continue, WARN, strUtils::ConstructString(MTEXT("Failed to create function for chip \'%1\' (id: %2).")).arg(ch->GetName()).arg(GuidToString(ch->GetGlobalID())));
		function->Set(ch->GetName(), n.second.ft, n.second.fa, parameters);
	}
	_functions.clear();

	if (_optimize) { 
		// Remove all shortcuts. Chips are linked directly to original.
		for (const auto &p : _loadedShortcuts)
			cg->RemoveChip(p.first); // TODO: check return value.
	}
	_loadedShortcuts.clear();

	return true;

	// Do some cleanup before leaving in shame...
lFail:
	_loadedShortcuts.clear();
	_functions.clear();
	_connections.clear();
	_instances = nullptr;

	return false;
}



bool DocumentLoader::LoadChip(Chip **chip, Class *cg)
{
	assert(chip);
	// Note: cg could be null!

	*chip = nullptr;

	if (!EnterGroup(DocumentTags::Chip))
		return true; // No more chips found!

	Chip *oldCurrentChip = _currentChip;
	Chip *ch = nullptr;

	Guid globalID;
	Guid type;
	String name;
	RefreshManager::RefreshMode rm;
	
	const ChipInfo* nfo = nullptr;

	check(GetAttribute(DocumentTags::chipid, globalID), goto lFailChip, FATAL, MTEXT("Failed reading id of chip. Skipping it."));
	check(_idTranslationMap.find(globalID) == _idTranslationMap.end(), goto lFailChip, FATAL, strUtils::ConstructString(MTEXT("Chip with the id %1 is already loaded. Skipping second one.")).arg(GuidToString(globalID)));
	check(GetAttribute(DocumentTags::type, type), goto lFailChip, FATAL, strUtils::ConstructString(MTEXT("Failed reading type-id for chip with id %1. Skipping it.")).arg(GuidToString(globalID)));
	check(GetAttribute(DocumentTags::name, name), name = GuidToString(globalID), WARN, strUtils::ConstructString(MTEXT("Failed reading name for chip with id %1.")).arg(GuidToString(globalID)));
	if (!GetAttribute(DocumentTags::refreshmode, rm))
		rm = RefreshManager::RefreshMode::OncePerFunctionCall;

	nfo = engine->GetChipManager()->GetChipInfo(type);
	check(
		nfo != nullptr, 
		goto lFailChip, 
		FATAL, 
		strUtils::ConstructString(MTEXT("Could not find chip-type with id %1 when loading chip \'%2\' (id: %3). Skipping it."))
		.arg(GuidToString(type))
		.arg(name)
		.arg(GuidToString(globalID))
		);

	// TODO: Check if chip-type is in version list!
	if (_version >= Version(1,1,0,1)) { // TODO: Remove this test!
		auto n = _chipVersions.find(type);
		check(
			n != _chipVersions.end(),
			goto lFailChip, 
			FATAL, 
			strUtils::ConstructString(MTEXT("Chip of type \'%1\' is not included in the document's chip list. Skipping chip \'%2\' (id: %3)."))
			.arg(nfo->chipDesc.name)
			.arg(name)
			.arg(GuidToString(globalID))
			);
	}

	if (cg)
		ch = cg->AddChip(nfo->chipTypeIndex, false);
	else {
		ch = engine->GetChipManager()->CreateChip(type);
		if (ch && GetCurrentChip()) {
			ch->SetClass(GetCurrentChip()->GetClass());
			ch->SetOwner(GetCurrentChip());
		}
	}

	check(
		ch != nullptr, 
		goto lFailChip, 
		FATAL, 
		strUtils::ConstructString(MTEXT("Could not create chip of type \'%1\' when loading chip \'%2\' (id: %3). Skipping it."))
		.arg(nfo->chipDesc.name)
		.arg(name)
		.arg(GuidToString(globalID))
		);

	check(_idTranslationMap.insert(std::make_pair(globalID, ch->GetID())).second, goto lFailChip, FATAL, MTEXT("Could not insert chip into id translation map. This should NOT happen...! :-/"));

	if (_keepGlobalIDs)
		ch->SetGlobalID(globalID);

	ch->SetName(name);
	ch->GetRefreshManager().SetRefreshMode(rm);

	check(_readConnections(ch, globalID), goto lFail, FATAL, strUtils::ConstructString(MTEXT("Failed reading connections for chip \'%1\' (id: %2). Skipping it.")).arg(name).arg(GuidToString(globalID)));
	check(_readFunction(ch, globalID), goto lFail, FATAL, strUtils::ConstructString(MTEXT("Failed reading function for chip \'%1\' (id: %2). Skipping it.")).arg(name).arg(GuidToString(globalID)));

	check(EnterGroup(DocumentTags::ChipData), goto lFailChip, FATAL, MTEXT("Failed to enter data group ...Chip/ChipData. Skipping the chip."));
	_currentChip = ch;
	check(ch->LoadChip(*this), goto lFailChipData, FATAL, strUtils::ConstructString(MTEXT("Failed to load chip data for chip \'%1\' (id: %2) of type \'%3\'."))
		.arg(name).arg(GuidToString(globalID)).arg(nfo->chipDesc.name));
	_currentChip = oldCurrentChip;
	check(LeaveGroup(DocumentTags::ChipData), goto lFail, FATAL, MTEXT("Failed to leave data group ...Chip/ChipData."));

	check(LeaveGroup(DocumentTags::Chip), goto lFail, FATAL, MTEXT("Failed to leave data group ...Chip."));

	*chip = ch;

	return true;

lFailChipData:
	check(LeaveGroup(DocumentTags::ChipData), goto lFail, FATAL, MTEXT("Failed to leave data group ...Chip/ChipData."));
lFailChip:
	check(LeaveGroup(DocumentTags::Chip),, FATAL, MTEXT("Failed to leave data group ...Chip."));
lFail:
	if (ch) {
		_currentChip = oldCurrentChip;
		_connections.erase(ch->GetID());
		_functions.erase(ch->GetID());
		_idTranslationMap.erase(globalID);
		if (cg)
			cg->RemoveChip(ch);
		else
			ch->Release();
	}

	return false;
}

bool DocumentLoader::_readConnections(Chip *ch, Guid globalID)
{
	if (EnterGroup(DocumentTags::Connections)) {
		ConnectionMap &connections = _connections.insert(std::make_pair(ch->GetID(), ConnectionMap())).first->second;
		while (EnterGroup(DocumentTags::Connection)) {
			uint32 c = 0, sc = 0;
			LoadedConnection lc = { NullGUID, false };
			check(GetAttribute(DocumentTags::conn, c), goto lSkipConnection, WARN, strUtils::ConstructString(MTEXT("Failed reading connection id for chip \'%1\' (id: %2). Skipping the connection.")).arg(ch->GetName()).arg(GuidToString(globalID)));
			if (!GetAttribute(DocumentTags::subconn, sc))
				sc = 0; // default to 0 if missing!
			check(GetAttribute(DocumentTags::chipid, lc.guid), goto lSkipConnection, WARN, strUtils::ConstructString(MTEXT("Failed reading connected chip id for connection %1/%2 on chip \'%3\' (id: %4). Skipping the connection.")).arg(strUtils::fromNum(c)).arg(strUtils::fromNum(sc)).arg(ch->GetName()).arg(GuidToString(globalID)));
			if (sc == 0) {
				if (!GetAttribute(DocumentTags::multiconnection, lc.isMulticonnection)) // The return value will be false if the attribute is not found, but that is ok as we only save the attribute when it is true!
					lc.isMulticonnection = false;
			}
			check(connections.insert(std::make_pair(std::make_pair(c, sc), lc )).second, goto lSkipConnection, WARN, strUtils::ConstructString(MTEXT("Failed preparing connection %1/%2 on chip \'%3\' (id: %4) because it already exist.")).arg(strUtils::fromNum(c)).arg(strUtils::fromNum(sc)).arg(ch->GetName()).arg(GuidToString(globalID)));
lSkipConnection:
			check(LeaveGroup(DocumentTags::Connection), return false, FATAL, MTEXT("Failed to leave data group ...Chip/Connections/Connection."));
		}
		check(LeaveGroup(DocumentTags::Connections), return false, FATAL, MTEXT("Failed to leave data group ...Chip/Connections."));
	}
	return true;
}

bool DocumentLoader::_readFunction(Chip *ch, Guid globalID)
{
	if (EnterGroup(DocumentTags::Function)) {
		FunctionDesc &fd = _functions.insert(std::make_pair(ch->GetID(), FunctionDesc())).first->second;
		check(GetAttribute(DocumentTags::type, fd.ft), goto lSkipFunction, WARN, strUtils::ConstructString(MTEXT("Failed reading \'type\' for function \'%1\' (id: %2). Skipping the function.")).arg(ch->GetName()).arg(GuidToString(globalID)));
		if (!GetAttribute(DocumentTags::access, fd.fa))
			fd.fa = Function::Access::Public; // Default to public access!
		if (_version > Version(1, 2, 5, 0)) {
			check(EnterGroup(DocumentTags::Parameters), goto lSkipFunction, WARN, MTEXT("Failed to enter group \'Parameters\'."));
		}
		while (EnterGroup(DocumentTags::Parameter)) {
			Guid cid = NullGUID;
			check(GetAttribute(DocumentTags::chipid, cid), goto lSkipParameter, WARN, strUtils::ConstructString(MTEXT("Failed reading parameter for function \'%1\' (id: %2). Skipping the parameter.")).arg(ch->GetName()).arg(GuidToString(globalID)));
			fd.parameters.push_back(cid);
lSkipParameter:
			check(LeaveGroup(DocumentTags::Parameter), return false, FATAL, MTEXT("Failed to leave data group ...Chip/Function/Parameter."));
		}
		if (_version > Version(1, 2, 5, 0)) {
			check(LeaveGroup(DocumentTags::Parameters), goto lSkipFunction, WARN, MTEXT("Failed leaving group \'Parameters\'."));
		}
	lSkipFunction:
		check(LeaveGroup(DocumentTags::Function), return false, FATAL, MTEXT("Failed to leave data group ...Chip/Function."));
	}
	return true;
}

ChipID DocumentLoader::TranslateChipID(const Guid &globalChipID)
{
	auto itr = _idTranslationMap.find(globalChipID);
	return itr != _idTranslationMap.end() ? itr->second : (ChipID)0; 
}

void DocumentLoader::AddInstance(ClassInstance *instance)
{
	assert(_instances);
	bool b = _instances->insert(std::make_pair(instance->GetID(), instance)).second;
	assert(b);
}

ClassInstance *DocumentLoader::GetInstance(const Guid &originalID) const
{
	assert(_instances);
	auto itr = _instances->find(originalID);
	return itr != _instances->end() ? itr->second : nullptr; 
}

bool DocumentLoader::Reset()
{
	_idTranslationMap.clear();
	_connections.clear();
	_functions.clear();
	_readAttr = false;
	_currentChip = nullptr;
	_instances = nullptr;
	_version = Version(0);
	_chipVersions.clear();
	return true;
}

Environment* DocumentLoader::GetEnvironment() const
{
	return engine->GetEnvironment();
}
