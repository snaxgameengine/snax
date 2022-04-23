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

#include <winapifamily.h>

namespace m3d
{


#define PLATFORM_undefined			0x0000
#define PLATFORM_WINDESKTOP_X64		0x0001 
#define PLATFORM_WINDESKTOP_X86		0x0002 
#define PLATFORM_WINDESKTOP			0x0003 // FAMILY
#define PLATFORM_WINSTOREAPP_X64	0x0004 
#define PLATFORM_WINSTOREAPP_X86	0x0008 
#define PLATFORM_WINSTOREAPP_ARM	0x0010
#define PLATFORM_WINSTOREAPP		0x001C // FAMILY
#define PLATFORM_WINPHONE8_WIN32	0x0020
#define PLATFORM_WINPHONE8_ARM		0x0040
#define PLATFORM_WINPHONE8			0x0060 // FAMILY
#define PLATFORM_all_platforms		0x007F

/*
enum AppPlatform 
{ 
	PLATFORM_undefined =		0x0000,
	PLATFORM_WINDESKTOP_X64 =	0x0001, 
	PLATFORM_WINDESKTOP_X86 =	0x0002, 
	PLATFORM_WINDESKTOP =		0x0003, // FAMILY
	PLATFORM_WINSTOREAPP_X64 =	0x0004, 
	PLATFORM_WINSTOREAPP_X86 =	0x0008, 
	PLATFORM_WINSTOREAPP_ARM =	0x0010,
	PLATFORM_WINSTOREAPP =		0x001C, // FAMILY
	PLATFORM_WINPHONE8_WIN32 =	0x0020,
	PLATFORM_WINPHONE8_ARM =	0x0040,
	PLATFORM_WINPHONE8 =		0x0060, // FAMILY
	PLATFORM_all_platforms =	0x007F
};
*/
/*
enum _PlatformFamily
{
	PLATFORM_FAMILY_undefined =		0x0000,
	PLATFORM_FAMILY_WINDESKTOP =	PLATFORM_WINDESKTOP_X64|PLATFORM_WINDESKTOP_X86,
	PLATFORM_FAMILY_WINSTOREAPP =	PLATFORM_WINSTOREAPP_X64|PLATFORM_WINSTOREAPP_X86|PLATFORM_WINSTOREAPP_ARM,
	PLATFORM_FAMILY_WINPHONE8 =		PLATFORM_WINPHONE8_WIN32|PLATFORM_WINPHONE8_ARM
};
*/
}



// PLATFORM_FAMILY will always be defined for current platform family.
// PLATFORM will be defined for the current platform.
// WINDESKTOP will be defined for desktop applications.
// WINPHONE8 will be defined for WP8 apps.
// WINSTOREAPP will be defined for store apps.


#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	#define PLATFORM_FAMILY PLATFORM_WINDESKTOP
	#define WINDESKTOP
	#ifdef _WIN64
		#define PLATFORM PLATFORM_WINDESKTOP_X64
	#else
		#define PLATFORM PLATFORM_WINDESKTOP_X86
	#endif
#elif defined(WINAPI_FAMILY) && WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP
	#define PLATFORM_FAMILY PLATFORM_WINPHONE8
	#define WINPHONE8
	#ifdef _M_ARM
		#define PLATFORM PLATFORM_WINPHONE8_ARM
	#else 
		#define PLATFORM PLATFORM_WINPHONE8_WIN32
	#endif
#else
	#define PLATFORM_FAMILY PLATFORM_WINSTOREAPP
	#define WINSTOREAPP
	#if _M_ARM
		#define PLATFORM PLATFORM_WINSTOREAPP_ARM
	#elif _M_X64
		#define PLATFORM PLATFORM_WINSTOREAPP_X64
	#else
		#define PLATFORM PLATFORM_WINSTOREAPP_X86
	#endif
#endif