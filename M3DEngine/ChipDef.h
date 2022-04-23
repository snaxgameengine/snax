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
#include "GlobalDef.h"
#include "M3DCore/Containers.h"
#include "M3DCore/MString.h"
#include "M3DCore/GuidUtil.h"

namespace m3d
{


struct ChipDesc
{
	enum Usage { STANDARD, HIDDEN };
	const Char *name;
	Guid type;
	Guid basetype;
	Usage usage;
	Version version;
	const Char *factoryFunc;
	const Char *filters; // For importers only. File types supported by Importer-chip, eg "dae;xml;3ds"
};


// Each packet implements this function in its exports.cpp file.
extern const ChipDesc &RegisterChipDesc(const Char *name, Guid type, Guid basetype, ChipDesc::Usage usage, uint32 version, const Char *factoryFunc, const Char *filters = MTEXT(""));

// CHIPDESC_DECL is placed in all new chips.
#define CHIPDESC_DECL public: static const m3d::ChipDesc DESC; virtual const m3d::ChipDesc &GetChipDesc() const override { return DESC; } 
// CHIPDESCV1_DEF, CHIPDESCV1_DEF_HIDDEN, CHIPDESCV1_DEF_VIRTUAL OR CHIPDESCV1_DEF_IMPORTER is placed in the cpp file of the chip, or in the exports.cpp of the packet.
// TODO: If placed in cpp file of chip, confirm that RegisterChipDesc(...) is always called on library loading, and that this technique is safe!
#define CHIPDESCV1_DEF(clazz, name, guid, baseGuid) const m3d::ChipDesc clazz::DESC = RegisterChipDesc(name, guid, baseGuid, m3d::ChipDesc::STANDARD, VERSION1, #clazz"_FACTORY"); extern "C" __declspec( dllexport ) m3d::Chip* __cdecl clazz##_FACTORY() throw(...) { return mmnew clazz(); }
#define CHIPDESCV1_DEF_HIDDEN(clazz, name, guid, baseGuid) const m3d::ChipDesc clazz::DESC = RegisterChipDesc(name, guid, baseGuid, m3d::ChipDesc::HIDDEN, VERSION1, #clazz"_FACTORY"); extern "C" __declspec( dllexport ) m3d::Chip* __cdecl clazz##_FACTORY() throw(...) { return mmnew clazz(); }
#define CHIPDESCV1_DEF_VIRTUAL(clazz, name, guid, baseGuid) const m3d::ChipDesc clazz::DESC = RegisterChipDesc(name, guid, baseGuid, m3d::ChipDesc::HIDDEN, VERSION1, #clazz"_FACTORY"); extern "C" __declspec( dllexport ) m3d::Chip* __cdecl clazz##_FACTORY() throw(...) { return nullptr; }
#define CHIPDESCV1_DEF_IMPORTER(clazz, name, guid, baseGuid, filters) const m3d::ChipDesc clazz::DESC = RegisterChipDesc(name, guid, baseGuid, m3d::ChipDesc::HIDDEN, VERSION1, #clazz"_FACTORY", filters); extern "C" __declspec( dllexport ) m3d::Chip* __cdecl clazz##_FACTORY() throw(...) { return mmnew clazz(); }
// These are for version other than 1.0.0.0:
#define CHIPDESC_DEF(clazz, name, guid, baseGuid, version) const m3d::ChipDesc clazz::DESC = RegisterChipDesc(name, guid, baseGuid, m3d::ChipDesc::STANDARD, version, #clazz"_FACTORY"); extern "C" __declspec( dllexport ) m3d::Chip* __cdecl clazz##_FACTORY() throw(...) { return mmnew clazz(); }
#define CHIPDESC_DEF_HIDDEN(clazz, name, guid, baseGuid, version) const m3d::ChipDesc clazz::DESC = RegisterChipDesc(name, guid, baseGuid, m3d::ChipDesc::HIDDEN, version, #clazz"_FACTORY"); extern "C" __declspec( dllexport ) m3d::Chip* __cdecl clazz##_FACTORY() throw(...) { return mmnew clazz(); }
#define CHIPDESC_DEF_VIRTUAL(clazz, name, guid, baseGuid, version) const m3d::ChipDesc clazz::DESC = RegisterChipDesc(name, guid, baseGuid, m3d::ChipDesc::HIDDEN, version, #clazz"_FACTORY"); extern "C" __declspec( dllexport ) m3d::Chip* __cdecl clazz##_FACTORY() throw(...) { return nullptr; }
#define CHIPDESC_DEF_IMPORTER(clazz, name, guid, baseGuid, filters, version) const m3d::ChipDesc clazz::DESC = RegisterChipDesc(name, guid, baseGuid, m3d::ChipDesc::HIDDEN, version, #clazz"_FACTORY", filters); extern "C" __declspec( dllexport ) m3d::Chip* __cdecl clazz##_FACTORY() throw(...) { return mmnew clazz(); }



struct ChildConnectionDesc
{
	enum DataDirection { UP=-1, BOTH, DOWN};
	enum ConnectionType { SINGLE, GROWING, MULTI };
	Guid type;
	ConnectionType connType;
	DataDirection dataDirection;
	String name;
};

class Chip;

struct SubConnection
{
	// The chip connected.
	Chip *chip;
	// Timestamp of the last time GetRawChild() was called. It is used to provide visual feedback of link activity in the editor.
	int32 lastHit;
	SubConnection(Chip *chip = nullptr) : chip(chip), lastHit(0) {}
};


typedef List<SubConnection> SubConnectionList;

typedef List<Chip*> ChipList;

struct ChildConnection
{
	const ChildConnectionDesc desc;
	const ChipTypeIndex chipTypeIndex;
	SubConnectionList connections;
	ChildConnection(const ChildConnectionDesc &desc, ChipTypeIndex chipTypeIndex) : desc(desc), chipTypeIndex(chipTypeIndex) {}
};

typedef List<ChildConnection*> ChildConnectionList;


#define CREATE_CHILD(index, type, growing, dataDirection, name) { m3d::ChildConnectionDesc desc = {type, (growing ? m3d::ChildConnectionDesc::GROWING : m3d::ChildConnectionDesc::SINGLE), m3d::ChildConnectionDesc::dataDirection, name}; SetConnection(index, desc, false); }
#define CREATE_CHILD_KEEP(index, type, growing, dataDirection, name) { m3d::ChildConnectionDesc desc = {type, (growing ? m3d::ChildConnectionDesc::GROWING : m3d::ChildConnectionDesc::SINGLE), m3d::ChildConnectionDesc::dataDirection, name}; SetConnection(index, desc, true); }

struct ChipExceptionScope
{
	static Chip M3DENGINE_API *currentChip;
	Chip *previousChip;
	ChipExceptionScope(Chip *chip) : previousChip(currentChip) { currentChip = chip; }
	~ChipExceptionScope() { currentChip = previousChip; }
};

struct ChipException
{
	String category;
	String msg;
	MessageSeverity severity;
	Chip *chip;
	ChipException(String category, String msg, MessageSeverity severity) : category(category), msg(msg), severity(severity), chip(ChipExceptionScope::currentChip) {}
	ChipException(String category, String msg, MessageSeverity severity, Chip *chip) : category(category), msg(msg), severity(severity), chip(chip) {}
};

struct ChipMessage
{
	String category;
	String msg;
	MessageSeverity severity;
	uint32 hitCount; // Initialize to 0.

	ChipMessage() : severity(DINFO), hitCount(0) {}
	ChipMessage(String category, String msg, MessageSeverity severity) : category(category), msg(msg), severity(severity), hitCount(0) {}
	ChipMessage(const ChipException &exc) : ChipMessage(exc.category, exc.msg, exc.severity) {}
};

#define __WIDEN(x) x
#define CHIPMSG(className, severity, msg) struct className : m3d::ChipException { className(m3d::String m = msg, m3d::MessageSeverity s = severity) : m3d::ChipException(__WIDEN(#className), m, s) {} className(m3d::Chip *chip, m3d::String m = msg, m3d::MessageSeverity s = severity) : m3d::ChipException(__WIDEN(#className), m, s, chip) {} };
#define CHIPMSGV(className, severity, msg, ...) struct className : m3d::ChipException { className(__VA_ARGS__, m3d::MessageSeverity s = severity) : m3d::ChipException(__WIDEN(#className), msg, s) {} className(m3d::Chip *chip, __VA_ARGS__, m3d::MessageSeverity s = severity) : m3d::ChipException(__WIDEN(#className), msg, s, chip) {} };


typedef List<ChipMessage> ChipMessageList;



class ShellChip;
class ProxyChip;
class Parameter;
class FunctionCall;
class FunctionData;
class InstanceData;
class ClassInstance;
class Shortcut;


class ProjectDependencies;
class RenderWindow;

class DocumentLoader;
class DocumentSaver;



class M3DENGINE_API RefreshManager
{
	friend struct RefreshT;
public:
	enum class RefreshMode { Always, OncePerFunctionCall, OncePerFrame, Once, Never };
private:
	RefreshMode _rm;

	uint32 _lastFrame; // The last frame nr we where hit. 0 if unhit.
	uint32 _lastStack; // The last stack nr we where hit.

public:
	RefreshManager() : _rm(RefreshMode::OncePerFunctionCall), _lastFrame(0), _lastStack(0) {}
	operator bool();

	RefreshMode GetRefreshMode() const { return _rm; }
	void SetRefreshMode(RefreshMode rm) { _rm = rm; }

	void Reset() { _lastFrame = 0; _lastStack = 0; }
};

struct RefreshT
{
	RefreshManager &rm;
	bool b;
	uint32 t1, t2;
	RefreshT(RefreshManager &rm) : rm(rm), b(rm), t1(rm._lastFrame), t2(rm._lastStack) {}
	~RefreshT() { rm._lastFrame = t1; rm._lastStack = t2; }
	inline operator bool() const { return b; }
};

struct ChipEditorData;

typedef uint32 UpdateStamp;

extern UpdateStamp M3DENGINE_API GenerateUpdateStamp();


}