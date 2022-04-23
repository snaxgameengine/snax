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
#include "M3DCore/Containers.h"
#include "M3DCore/Path.h"
#include "M3DCore/SlimRWLock.h"
#include <atomic>


namespace m3d
{

typedef const Char* (*GETPACKETNAME)();
typedef uint32 (*GETCHIPCOUNT)();
typedef const ChipDesc &(*GETCHIPTYPE)(uint32);
typedef bool (*ONPACKETQUERY)();
typedef bool (*ONPACKETLOAD)();
typedef void (*ONPACKETUNLOAD)();
typedef Chip *(*CHIPFACTORY)();
typedef void (*ADDDEPENDENCIES)(ProjectDependencies &deps);
typedef uint32 (*GETSUPPORTEDPLATFORMS)();


struct Packet;

struct ChipInfo
{
	Packet* packet;
	struct {
		String name;
		Guid type;
		Guid basetype;
		ChipDesc::Usage usage;
		Version version;
		String factoryFunc;
		String filters; // For importers only. File types supported by Importer-chip, eg "dae;xml;3ds"
	} chipDesc; // NOTE: Can not use ChipDesc here because of wchar_t* and char*.
	CHIPFACTORY chipFactoryFunc;
	ChipTypeIndex chipTypeIndex;

	List<Guid> baseguids;

	ChipInfo() : packet(nullptr), chipFactoryFunc(0), chipTypeIndex(0) {}
};

typedef Map<Guid, ChipInfo*> ChipInfoPtrByGUIDMap;
typedef Map<String, Packet*> PacketPtrByStringMap;
typedef Map<ChipTypeIndex, ChipInfo*> ChipInfoPtrByGUIDIndexMap;
typedef Map<std::pair<Guid, String>, Chip*> GlobalChipMap;

struct Packet
{
	Path filename;
	String name;
	uint32 chipCount;
	// Note: The supportedPlatforms-flag is mostly for use by the publisher! 
	// We don't do any testing on load! If the dll is present and loads for a given platform it is assumed to be supported!
	uint32 supportedPlatforms; 
	std::atomic<HMODULE> module; // The dll-module. Atomic because we load it on demand, possibly from any thread.
	ChipInfoPtrByGUIDMap chips; 
	ADDDEPENDENCIES addDependenciesFunc;

	Packet() : chipCount(0), supportedPlatforms(0), module(0), addDependenciesFunc(nullptr) {}
};



class M3DENGINE_API ChipManagerEventListener
{
public:
	virtual void ChipManagerInitiated() = 0;
	virtual void PacketLoaded(String packetName) = 0;
};

class M3DENGINE_API ChipManager
{
protected:
	ChipTypeIndex _guidIndices;
	ChipInfoPtrByGUIDMap _chipsByGUIDMap;
	ChipInfoPtrByGUIDIndexMap _chipsByGUIDIndexMap;
	PacketPtrByStringMap _packetMap;
	bool *_guidSupportMap;

	GlobalChipMap _globalChips;

	ChipManagerEventListener *_eventListener;

	Path _chipsDirectory;

	SlimRWLock _lock;
	SlimRWLock _globalChipsLock;

	void _clear();
	void _processDLL(Path fileName);

	bool &_isChipGuidSupported(ChipTypeIndex guidRequired, ChipTypeIndex guidToCheck) const { return _guidSupportMap[guidRequired * _guidIndices + guidToCheck]; }

public:
	ChipManager();
	virtual ~ChipManager();

	void SetEventListener(ChipManagerEventListener *eventListener) { _eventListener = eventListener; }

	bool FindChips(Path folder);

	Path GetChipsDirectory() const { return _chipsDirectory; }

	// THREAD SAFE!
	Chip *CreateChip(const Guid &chipType);
	// THREAD SAFE!
	Chip *CreateChip(ChipTypeIndex chipTypeIndex);

	const PacketPtrByStringMap &GetPacketMap() const { return _packetMap; }
	const ChipInfoPtrByGUIDMap &GetChipInfoPtrByGUIDMap() const { return _chipsByGUIDMap; }

	ChipTypeIndex GetChipTypeIndex(const Guid &chipType) const;
	const ChipInfo *GetChipInfo(ChipTypeIndex chipTypeIndex) const;
	const ChipInfo *GetChipInfo(const Guid &chipType) const { return GetChipInfo(GetChipTypeIndex(chipType)); }

	bool IsChipTypeSupported(const Guid &typeRequired, const Guid &typeToCheck) const;
	bool IsChipTypeSupported(ChipTypeIndex typeRequired, ChipTypeIndex typeToCheck) const;

	Chip *GetGlobalChip(const Guid &type, String name = MTEXT(""), bool create = true);
	bool ReleaseGlobalChip(const Guid &type, String name = MTEXT(""));
	void ClearGlobalChips();

	void DestroyDeviceObjects();

	void OnNewFrame();

	List<Guid> GetDerivedChips(const Guid &baseType) const;
};


}

