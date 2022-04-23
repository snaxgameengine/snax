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
#include "Err.h"
#include "Engine.h"


using namespace m3d;



HRESULT m3d::Trace(const Char *file, uint32 line, HRESULT hr, const Char *strMsg, int32 mode)
{
	CHAR szError[64] = {'\0'};
	if(HRESULT_FACILITY(hr) == FACILITY_WINDOWS)
		hr = HRESULT_CODE(hr);
	FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&szError, 64, NULL);
	for (CHAR*w = szError; *w; w++)
		if (*w < 0x20) *w = MCHAR('\0');

	String f = file;
	size_t p = f.find_last_of(MCHAR('\\'));
	if (p != String::npos)
		f = f.substr(p + 1);

	String a = f + MTEXT(" (Line ") + strUtils::fromNum((uint32)line) + (strMsg ? MTEXT("):") : MTEXT(").")), b = strMsg ? (strMsg + String(MTEXT("."))) : MTEXT(""), c = strUtils::format(MTEXT("hr=0x%0.8x ("), hr) + szError + MTEXT(")");

	msg(FATAL, a + MTEXT(" ") + b + MTEXT(" ") + c);

#ifdef WINDESKTOP
	if (mode == 1) {
		// TODO: Check for fullscreen!
		int32 nResult = MessageBoxA( GetForegroundWindow(), (a + MTEXT("\n\n") + b + MTEXT("\n\n") + c + MTEXT("\n\nDo you want to debug?")).c_str(), MTEXT("Unexpected error encountered"), MB_YESNO | MB_ICONERROR| MB_TASKMODAL );
		if( nResult == IDYES )
			DebugBreak();
	}
	else if (mode == 2)
		DebugBreak();
#endif
	return hr;
}

bool m3d::Trace(const Char *file, uint32 line, bool b, const Char *strMsg, int32 mode)
{
	String f = file;
	size_t p = f.find_last_of(MCHAR('\\'));
	if (p != String::npos)
		f = f.substr(p + 1);

	String a = f + MTEXT(" (Line ") + strUtils::fromNum((uint32)line) + (strMsg ? MTEXT("):") : MTEXT(").")), d = strMsg ? (strMsg + String(MTEXT("."))) : MTEXT(""), c = String(MTEXT("Value was ")) + (b ? MTEXT("true.") : MTEXT("false."));

	msg(FATAL, a + MTEXT(" ") + d + MTEXT(" ") + c);

#ifdef WINDESKTOP
	if (mode == 1) {
		// TODO: Check for fullscreen!
		int32 nResult = MessageBoxA( GetForegroundWindow(), (a + MTEXT("\n\n") + d + MTEXT("\n\n") + c + MTEXT("\n\nDo you want to debug?")).c_str(), MTEXT("Unexpected error encountered"), MB_YESNO | MB_ICONERROR| MB_TASKMODAL );
		if( nResult == IDYES )
			DebugBreak();
	}
	else if (mode == 2)
		DebugBreak();
#endif
	return b;
}

