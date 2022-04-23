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
#include "GuidUtil.h"
#include <Objbase.h> // Windows


using namespace m3d;


bool m3d::GenerateGuid(Guid &guid)
{
	return CoCreateGuid((GUID*)&guid) == S_OK;
}

String m3d::GuidToString(const Guid&guid)
{
	wchar_t buff[64];
	StringFromGUID2((GUID&)guid, buff, 64);

	char buff2[64];
	WideCharToMultiByte(CP_ACP, 0, buff, -1, buff2, 64, NULL, NULL);

	return String(buff2);
}

bool m3d::StringToGUID(const String &str, Guid&guid)
{
	wchar_t buff2[64];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buff2, 64);

	HRESULT hr = CLSIDFromString(buff2, (LPCLSID)&guid);

	return hr == S_OK;
}