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
#include "ChipDialogs/DialogPage.h"

using namespace m3d;

namespace 
{
	const uint32 MAX_DIALOGS = 128;
	DialogType dialogDescs[MAX_DIALOGS];
	uint32 dialogCount = 0;
}

// Defined in DialogPage.h. 
const DialogType &m3d::RegisterDialogDesc(Guid chipType, Guid chipTypeNP, const Char *factoryFunc)
{
	DialogType dd = { chipType, chipTypeNP, factoryFunc };
	assert(dialogCount < MAX_DIALOGS);
	return dialogDescs[dialogCount++] = dd;
}

extern "C" 
{
	__declspec( dllexport ) const DialogType& __cdecl GetDialogType(uint32 index) { return dialogDescs[index]; }
	__declspec( dllexport ) uint32 __cdecl GetDialogCount() { return dialogCount; }
	// Memory allocated in this dll (eg chips created with the corresponding factory functions) should be freed in this dll as well.
	__declspec( dllexport ) void __cdecl ReleaseMemory(void *v) { delete v; }
	__declspec( dllexport ) bool __cdecl OnDlgPacketQuery() { return true; }
	__declspec( dllexport ) bool __cdecl OnDlgPacketLoad() { return true; }
	__declspec( dllexport ) void __cdecl OnDlgPacketUnload() { }
}

