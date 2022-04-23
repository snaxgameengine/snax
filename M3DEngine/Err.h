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

namespace m3d
{

	// mode 0: Only message
	// mode 1: Message box is shown
	// mode 2: DebugBreak() is always called.
	HRESULT M3DENGINE_API Trace(const Char *file, uint32 line, HRESULT hr, const Char *msg, int32 mode = 0); // Returns hr
	bool M3DENGINE_API Trace(const Char *file, uint32 line, bool b, const Char *msg, int32 mode = 0); // returns b

}




#if defined(DEBUG) || defined(_DEBUG)
#ifndef V
#define V(x)           { hr = x; if( FAILED(hr) ) { m3d::Trace( __FILE__, (uint32)__LINE__, hr, #x, 1 ); } }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = x; if( FAILED(hr) ) { return m3d::Trace( __FILE__, (uint32)__LINE__, hr, #x, 1 ); } }
#ifndef B_RETURN
#define B_RETURN(x)	   { if ( !(x) ) { return m3d::Trace( __FILE__, (uint32)__LINE__, false, #x, 1 ); } }
#endif
#endif
#else
#ifndef V
#define V(x)           { hr = x; if( FAILED(hr) ) { m3d::Trace( __FILE__, (uint32)__LINE__, hr, nullptr, 0 ); } }
#endif
#ifndef V_RETURN
#define V_RETURN(x)    { hr = x; if( FAILED(hr) ) { return m3d::Trace( __FILE__, (uint32)__LINE__, hr, nullptr, 0 ); } }
#ifndef B_RETURN
#define B_RETURN(x)	   { if ( !(x) ) { return m3d::Trace( __FILE__, (uint32)__LINE__, false, nullptr, 0 ); } }
#endif
#endif
#endif
