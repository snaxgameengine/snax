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
#include "Exports.h"
#include "M3DEngine/ChipDef.h"
#include "M3DEngine/ProjectDependencies.h"

using namespace m3d;


namespace 
{
	const uint32 MAX_CHIPS = 128;
	ChipDesc chipDescs[128];
	uint32 chipCount = 0;
}

// Defined in ChipDesc.h. 
const ChipDesc &m3d::RegisterChipDesc(const Char *name, Guid type, Guid basetype, ChipDesc::Usage usage, uint32 version, const Char *factoryFunc, const Char *filters)
{
	ChipDesc cd = { name, type, basetype, usage, version, factoryFunc, filters };
	assert(chipCount < MAX_CHIPS);
	return chipDescs[chipCount++] = cd;
}


extern "C" 
{
	__declspec( dllexport ) const ChipDesc& __cdecl GetChipDesc(uint32 index) { return chipDescs[index]; }
	__declspec( dllexport ) uint32 __cdecl GetChipCount() { return chipCount; } 
	__declspec( dllexport ) const Char* __cdecl GetPacketName() { return MTEXT("PhysX"); }
	__declspec( dllexport ) bool __cdecl OnPacketQuery() { return true; }
	__declspec( dllexport ) bool __cdecl OnPacketLoad() { return true; }
	__declspec( dllexport ) void __cdecl OnPacketUnload() { }
	__declspec( dllexport ) void __cdecl AddDependencies(ProjectDependencies &deps) 
	{
		if (deps.GetPlatform() == ProjectDependencies::WD_X64) 
		{
			deps.AddDependency(MTEXT("PhysX_64.dll"));
			deps.AddDependency(MTEXT("PhysXCommon_64.dll"));
			deps.AddDependency(MTEXT("PhysXCooking_64.dll"));	
			deps.AddDependency(MTEXT("PhysXFoundation_64.dll"));
			deps.AddDependency(MTEXT("PhysXGPU_64.dll"));
		}
	}
	__declspec( dllexport ) uint32 __cdecl GetSupportedPlatforms() { return PLATFORM_WINDESKTOP_X64; }
}

