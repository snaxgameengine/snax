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
#include "ChipManager.h"
#include "Engine.h"
#include "M3DCore/GuidUtil.h"
#include "Chip.h"

using namespace m3d;


ChipManager::ChipManager() : _guidIndices(0), _guidSupportMap(nullptr), _eventListener(nullptr)
{
}

ChipManager::~ChipManager()
{
	msg(DINFO, MTEXT("ChipManager::~ChipManager() called."));
	_clear();
}

void ChipManager::_clear()
{
	ClearGlobalChips();

	if (_guidSupportMap)
		mmfree(_guidSupportMap);
	_guidSupportMap = nullptr;
	for (const auto &n : _chipsByGUIDMap)
		mmdelete(n.second);
	_chipsByGUIDMap.clear();
	_chipsByGUIDIndexMap.clear();
	for (const auto &n : _packetMap) {
		ONPACKETUNLOAD onPacketUnloadFunc = (ONPACKETUNLOAD)GetProcAddress(n.second->module, "OnPacketUnload");
		if (onPacketUnloadFunc)
			(*onPacketUnloadFunc)();
		FreeLibrary(n.second->module);
		mmdelete(n.second);
	}
	_packetMap.clear();
}

bool ChipManager::FindChips(Path folder)
{
	//Char buff[MAX_PATH];

	WIN32_FIND_DATAA FindFileData;
//	HANDLE hFind = FindFirstFile((folder.AsString() + MTEXT("*.dll")).c_str(), &FindFileData);
	HANDLE hFind = FindFirstFileExA((folder.AsString() + MTEXT("*.dll")).c_str(), FindExInfoStandard, &FindFileData, FindExSearchNameMatch, NULL, 0);

	_chipsDirectory = folder;

	// Process all DLLs in the specified directory
	if (hFind != INVALID_HANDLE_VALUE) {
//		if (::GetFullPathName((folder.AsString() + FindFileData.cFileName).c_str(), MAX_PATH, buff, 0)) {
			_processDLL(Path(FindFileData.cFileName, folder));
//		}
		while (FindNextFileA(hFind, &FindFileData) != 0) {
//			if (::GetFullPathName((folder.AsString() + FindFileData.cFileName).c_str(), MAX_PATH, buff, 0)) {
				_processDLL(Path(FindFileData.cFileName, folder));
//			}
		}
		FindClose(hFind);
	}

	// Allocate _guidSupportMap
	if (_guidSupportMap)
		mmfree(_guidSupportMap);
	_guidSupportMap = nullptr;
	_guidSupportMap = (bool*)mmalloc(sizeof(bool) * _guidIndices * _guidIndices);
	memset(_guidSupportMap, 0, sizeof(bool) * _guidIndices * _guidIndices);
	
	// build base guid list
	auto baseNode = _chipsByGUIDMap.find(CHIP_GUID);
	if (baseNode == _chipsByGUIDMap.end() || baseNode->second->chipDesc.basetype != CHIP_GUID) {
		// The base guid was not found or invalid!
		msg(FATAL, MTEXT("No chip with type = CHIP_GUID found. Unable to load chips."));
		_clear();
		return false;
	}
	for (auto n = _chipsByGUIDMap.begin(); n != _chipsByGUIDMap.end();) { // Iterate all chips
		Set<Guid> gs;
		auto m = n;
		gs.insert(m->first);
		while (m != baseNode) {
			m = _chipsByGUIDMap.find(m->second->chipDesc.basetype);
			if (m == _chipsByGUIDMap.end()) {
				msg(FATAL, String(MTEXT("Unable to use chip \'")) + n->second->chipDesc.name + MTEXT("\' in package \'") + n->second->packet->name + MTEXT("\'. No chip matching its base type \'") + GuidToString(n->second->chipDesc.basetype) + MTEXT("\' was found."));
				break; // base guid not found
			}
//			if (m == n)
//				break; // at base guid. look no further!
			if (!gs.insert(m->second->chipDesc.type).second) {
				msg(FATAL, String(MTEXT("Unable to use chip \'")) + n->second->chipDesc.name + MTEXT("\' in package \'") + n->second->packet->name + MTEXT("\'. Cyclic base guids found."));
				break; // Cyclic base guid found
			}
			n->second->baseguids.push_back(m->second->chipDesc.type); // Add base guid to chip
		}
		if (m == baseNode) { // Chip ok, Add to guid support map
			_isChipGuidSupported(GetChipTypeIndex(n->first), GetChipTypeIndex(n->first)) = true; // chip supports itself!
			for (uint32 i = 0; i < n->second->baseguids.size(); i++) // Iterate all base guids
				_isChipGuidSupported(GetChipTypeIndex(n->second->baseguids[i]), GetChipTypeIndex(n->first)) = true;
			n++;
		}
		else {
			// Remove chip (This should be no problem, because other processed chips having this as base have already been removed here)
			n->second->packet->chips.erase(n->second->chipDesc.type);
			_chipsByGUIDIndexMap.erase(n->second->chipTypeIndex);
			mmdelete(n->second);
			n = _chipsByGUIDMap.erase(n);
		}
	}

	if (_eventListener)
		_eventListener->ChipManagerInitiated();

	return true;
}

// See MSDN for LoadLibraryEx()
//#define LOAD_LIBRARY_SEARCH_DEFAULT_DIRS 0x00001000
//#define LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR 0x00000100

String GetLastErrorAsString()
{
	//Get the error message, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0)
		return String();

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	String message(messageBuffer, size);

	//Free the buffer.
	LocalFree(messageBuffer);

	return message;
}

void ChipManager::_processDLL(Path fileName) 
{
#ifdef WINDESKTOP
	assert(fileName.IsAbsolute());
//	HMODULE hDLL = LoadLibraryA(fileName.AsString().c_str());
	HMODULE hDLL = LoadLibraryExA(fileName.AsString().c_str(), 0, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
#else
	HMODULE hDLL = LoadPackagedLibrary(fileName.AsString().c_str(), 0);
#endif

	msg(DINFO, String(MTEXT("Searching for chips in \'")) + fileName.AsString() + MTEXT("\'..."));
	if (hDLL) {
		GETCHIPCOUNT getChipCountFunc = (GETCHIPCOUNT)GetProcAddress(hDLL, "GetChipCount");
		GETCHIPTYPE getChipTypeFunc = (GETCHIPTYPE)GetProcAddress(hDLL, "GetChipDesc");
		GETPACKETNAME getPacketNameFunc = (GETPACKETNAME)GetProcAddress(hDLL, "GetPacketName");
		GETSUPPORTEDPLATFORMS getSupportedPlatformsFunc = (GETSUPPORTEDPLATFORMS)GetProcAddress(hDLL, "GetSupportedPlatforms");
		if (getChipCountFunc && getChipTypeFunc && getPacketNameFunc && getSupportedPlatformsFunc) {
			uint32 supportedPlatforms = (*getSupportedPlatformsFunc)();
			uint32 cCount = (*getChipCountFunc)();
			String packetName = (*getPacketNameFunc)();
			auto n = _packetMap.find(packetName);
			if (n == _packetMap.end()) {
				Packet *p = mmnew Packet();
				p->filename = fileName.AsString();
				p->name = packetName;
				p->chipCount = cCount;
				p->supportedPlatforms = supportedPlatforms; 
				_packetMap.insert(std::make_pair(p->name, p)); // always true
				for (uint32 i = 0; i < cCount; i++) {
					const ChipDesc &cd = (*getChipTypeFunc)(i);
					ChipInfo *ci = mmnew ChipInfo();
					ci->packet = p;
					ci->chipDesc.name = cd.name;
					ci->chipDesc.type = cd.type;
					ci->chipDesc.basetype = cd.basetype;
					ci->chipDesc.usage = cd.usage;
					ci->chipDesc.version = cd.version;
					ci->chipDesc.factoryFunc = cd.factoryFunc;
					ci->chipDesc.filters = cd.filters;
					ci->chipTypeIndex = _guidIndices++;
					CHIPFACTORY cf = (CHIPFACTORY)GetProcAddress(hDLL, ci->chipDesc.factoryFunc.c_str());
					if (!cf) {
						// Corrupt chip?
						msg(FATAL, String(MTEXT("Unable to use chip \'")) + ci->chipDesc.name + MTEXT("\' in packet \'") + packetName + MTEXT("\' because of missing factory function."));
						_guidIndices--;
						mmdelete(ci);
						continue; 
					}
					if (!_chipsByGUIDMap.insert(std::make_pair(ci->chipDesc.type, ci)).second) {
						// Found another chip with this guid!
						msg(FATAL, String(MTEXT("Unable to use chip \'")) + ci->chipDesc.name + MTEXT("\' in packet \'") + packetName + MTEXT("\'. Its type \'") + GuidToString(ci->chipDesc.type) + MTEXT("\' already exist."));
						_guidIndices--;
						mmdelete(ci);
						continue; 
					}
					_chipsByGUIDIndexMap.insert(std::make_pair(ci->chipTypeIndex, ci)); // always true
					p->chips.insert(std::make_pair(ci->chipDesc.type, ci)); // always true
				}
				msg(INFO, String(MTEXT("Found ")) + strUtils::fromNum(p->chips.size()) + MTEXT(" chips in packet \'") + packetName +  MTEXT("\' (") + fileName.AsString() + MTEXT(")."));
			}
			else {
				// Packet already exist => conflict!
				msg(FATAL, String(MTEXT("Packet \'")) + packetName + MTEXT("\' already exist. Skipping packet in \'") + fileName.AsString() + MTEXT("\'."));
			}
		}
		else {
			msg(WARN, String(MTEXT("Skipping search for chips in \'")) + fileName.AsString() + MTEXT("\'. It does not appear to be a valid chip dll."));
		}
		msg(DINFO, String(MTEXT("Unloading \'")) + fileName.AsString() + MTEXT("\'..."));
		FreeLibrary(hDLL);
	}
	else {
		msg(WARN, String(MTEXT("Unable to load \'")) + fileName.AsString() + MTEXT("\' to search for chips. Reason: ") + GetLastErrorAsString() + MTEXT("."));
	}
}

Chip *ChipManager::CreateChip(const Guid &chipType)
{
	auto n = _chipsByGUIDMap.find(chipType);
	if (n == _chipsByGUIDMap.end()) {
		msg(FATAL, String(MTEXT("Unable to create chip with guid \'")) + GuidToString(chipType) + MTEXT("\'. No such guid exist."));
		return nullptr; // no such chip!
	}

	HMODULE hm = n->second->packet->module.load();

	if (hm == 0) { // not loaded? (Double-checked locking...)
		SlimWLockBlock lock(_lock);

		hm = n->second->packet->module.load();

		if (hm == 0) { // still not loaded?
			// load dll
			msg(DINFO, String(MTEXT("Trying to load chip dll \'")) + n->second->packet->filename.AsString() + MTEXT("\'..."));
#ifdef WINDESKTOP
			HMODULE hDLL = LoadLibraryExA(n->second->packet->filename.AsString().c_str(), 0, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR);
#else
			HMODULE hDLL = LoadPackagedLibrary(n->second->packet->filename.AsString().c_str(), 0);
#endif
			if (hDLL) {
				GETCHIPCOUNT getChipCountFunc = (GETCHIPCOUNT)GetProcAddress(hDLL, "GetChipCount");
				GETPACKETNAME getPacketNameFunc = (GETPACKETNAME)GetProcAddress(hDLL, "GetPacketName");
				ADDDEPENDENCIES addDependenciesFunc = (ADDDEPENDENCIES)GetProcAddress(hDLL, "AddDependencies");
				// confirm packet name & count
				if (getChipCountFunc && getPacketNameFunc) {
					uint32 cCount = (*getChipCountFunc)();
					String packetName = (*getPacketNameFunc)();
					if (cCount != n->second->packet->chipCount || packetName != n->second->packet->name) {
						// chip count or packet name do not match!
						msg(FATAL, String(MTEXT("Packet \'")) + n->second->packet->name + MTEXT("\' appears to be different from when first loaded."));
					}
					else {
						ONPACKETLOAD onPacketLoadFunc = (ONPACKETLOAD)GetProcAddress(hDLL, "OnPacketLoad");
						bool loadOK = onPacketLoadFunc ? (*onPacketLoadFunc)() : true;
						if (loadOK) { // Packet loaded!!
							// Load chip factory functions...
							for (const auto &m : n->second->packet->chips) {
								m.second->chipFactoryFunc = (CHIPFACTORY)GetProcAddress(hDLL, m.second->chipDesc.factoryFunc.c_str());
								if (m.second->chipFactoryFunc == 0) {  // could not find factory func!
									msg(FATAL, String(MTEXT("Unable to find factory function for chip \'")) + m.second->chipDesc.name + MTEXT("\'."));
								}
							}

							n->second->packet->addDependenciesFunc = addDependenciesFunc;
							n->second->packet->module.store(hDLL); // ok
							msg(INFO, String(MTEXT("Successfully loaded chip dll \'")) + n->second->packet->filename.AsString() + MTEXT("\'."));
							if (_eventListener)
								_eventListener->PacketLoaded(n->second->packet->name);
						}
						else {
							msg(FATAL, String(MTEXT("Packet \'")) + n->second->packet->name + MTEXT("\' refused to load."));

						}
					}
				}
				if (n->second->packet->module.load() == 0) {
					msg(FATAL, String(MTEXT("Unable to use packet \'")) + n->second->packet->name + MTEXT("\'. It appears to be invalid."));
					FreeLibrary(hDLL);
					return nullptr; // Invalid dll?
				}
			}
			else {
				msg(FATAL, String(MTEXT("Unable to load \'")) + n->second->packet->filename.AsString() + MTEXT("\' to create chip with guid \'") + GuidToString(chipType) + MTEXT("\'."));
				return nullptr; // packet not found anymore?
			}
		}
	}


	if (n->second->chipFactoryFunc == 0) { // factory func not found?
		return nullptr;
	}

	Chip *ch = nullptr;
	try {
		ch = (*n->second->chipFactoryFunc)();
		if (!ch) {
			msg(FATAL, String(MTEXT("Factory function returned null for chip \'")) + n->second->chipDesc.name + MTEXT("\'."));
			// could not create the chip for some reason!?
		}
	}
	catch(...) {
		ch = nullptr;
		msg(FATAL, String(MTEXT("Factory function for channnel \'")) + n->second->chipDesc.name + MTEXT("\' throwed an exception."));
	}

	return ch;
}

Chip *ChipManager::CreateChip(ChipTypeIndex chipTypeIndex)
{
	const ChipInfo *i = GetChipInfo(chipTypeIndex);
	if (i)
		return CreateChip(i->chipDesc.type);
	return nullptr;
}

ChipTypeIndex ChipManager::GetChipTypeIndex(const Guid &chipType) const
{
	auto n = _chipsByGUIDMap.find(chipType);
	if (n != _chipsByGUIDMap.cend())
		return n->second->chipTypeIndex;
	return InvalidChipTypeIndex;
}

const ChipInfo *ChipManager::GetChipInfo(ChipTypeIndex chipTypeIndex) const
{
	auto n = _chipsByGUIDIndexMap.find(chipTypeIndex);
	if (n != _chipsByGUIDIndexMap.cend())
		return n->second;
	return nullptr;
}

bool ChipManager::IsChipTypeSupported(const Guid &typeRequired, const Guid &typeToCheck) const
{
	return IsChipTypeSupported(GetChipTypeIndex(typeRequired), GetChipTypeIndex(typeToCheck));
}

bool ChipManager::IsChipTypeSupported(ChipTypeIndex typeRequired, ChipTypeIndex typeToCheck) const
{
	if (typeRequired < _guidIndices && typeToCheck < _guidIndices)
		return _isChipGuidSupported(typeRequired, typeToCheck); 
	return false;
}

Chip *ChipManager::GetGlobalChip(const Guid &type, String name, bool create)
{
	SlimWLockBlock lock(_globalChipsLock); // Just use the lock in exclusive mode... This function is not to be called very often...

	auto n = _globalChips.find(std::make_pair(type, name));
	if (n != _globalChips.end())
		return n->second;
	if (!create)
		return nullptr;
	Chip *chip = CreateChip(type);
	if (!chip)
		return nullptr;
	if (!chip->InitChip()) {
		chip->Release();
		return nullptr;
	}
	_globalChips.insert(std::make_pair(std::make_pair(type, name), chip));
	return chip;
}

bool ChipManager::ReleaseGlobalChip(const Guid &type, String name)
{
	SlimWLockBlock lock(_globalChipsLock); // Just use the lock in exclusive mode... This function is not to be called very often...

	const auto n = _globalChips.find(std::make_pair(type, name));
	if (n == _globalChips.end())
		return false;
	if (n->second)
		n->second->Release();
	_globalChips.erase(n);
	return true;
}

void ChipManager::ClearGlobalChips()
{
	for (const auto &n : _globalChips)
		n.second->Release();
	_globalChips.clear();
}

void ChipManager::OnNewFrame()
{
	for (const auto &n : _globalChips)
		n.second->OnNewFrame();	
}

List<Guid> ChipManager::GetDerivedChips(const Guid &baseType) const
{
	List<Guid> g;
	for (const auto &n : _chipsByGUIDMap) {
		for (size_t i = 0; i < n.second->baseguids.size(); i++) {
			if (n.second->baseguids[i] == baseType) {
				g.push_back(n.first);
				break;
			}
		}
	}
	return g;
}

void ChipManager::DestroyDeviceObjects()
{
	for (const auto &n : _globalChips)
		n.second->OnDestroyDevice();
}