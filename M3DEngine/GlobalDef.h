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
#include "M3DCore/MTypes.h"
#include "M3DCore/PlatformDef.h"
#include "M3DCore/Containers.h"
#include "M3DCore/Path.h"
#include "M3DCore/MString.h"
#include "Err.h"
#include "M3DCore/MemoryManager.h"
#include "M3DCore/GuidUtil.h"


#define M3D_TITLE "SnaX"

namespace m3d
{


class Application;
class Engine;
class Graphics;
class Document;
class DocumentManager;
class ClassManager;
class Class;
class DocumentLoader;
class DocumentSaver;
class ChipManager;
class Chip;
class FunctionSignatureManager;
class Function;
class DataBuffer;
class FunctionCall;


using ChipTypeIndex = uint32;
const ChipTypeIndex InvalidChipTypeIndex = -1;

typedef uint32 ChipID;
const ChipID InvalidChipID = 0;

typedef uint32 DocumentID;
const DocumentID InvalidDocumentID = 0;

typedef uint32 ClassID;
const ClassID InvalidClassID = 0;

typedef uint32 FunctionSignatureID;
const FunctionSignatureID InvalidFunctionSignatureID = 0;

typedef uint32 ClassInstanceID;
const ClassInstanceID InvalidClassInstanceID = 0;


enum MessageSeverity { DINFO, INFO, NOTICE, WARN, FATAL, ALWAYS };

// Utility function for easily setting debug messages.
extern void M3DENGINE_API msg(MessageSeverity severity, String message);
extern void M3DENGINE_API msg(MessageSeverity severity, String message, const Class *clazz);
extern void M3DENGINE_API msg(MessageSeverity severity, String message, const Chip *chip);

struct M3DENGINE_API Version
{
	union
	{
		uint32 version;
		uint8 elements[4]; // Major,Minor,Build,Revision.
	};
	Version(uint32 v = 0x01000000u) : version(v) {}
	Version(uint8 ma, uint8 mi, uint8 bu, uint8 re) : version((uint32(ma) << 24u | uint32(mi) << 16u | uint32(bu) << 8u | uint32(re))) {}

	bool operator==(Version b) const { return version == b.version; }
	bool operator!=(Version b) const { return version != b.version; }
	bool operator<(Version b) const { return version < b.version; }
	bool operator<=(Version b) const { return version <= b.version; }

	operator uint32() const { return version; }
};

#define VERSION1 m3d::Version(0x01000000u)


}

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif    
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=NULL; } }
#endif    
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif



