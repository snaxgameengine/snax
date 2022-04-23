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
#include "MMath.h"


namespace m3d
{

struct NormalizedVector32;
struct NormalizedVector64;

extern void M3DCORE_API StoreNormalizedVector32(NormalizedVector32 *pDestination, FXMVECTOR V);
extern XMVECTOR M3DCORE_API LoadNormalizedVector32(const NormalizedVector32 *pSource);

extern void M3DCORE_API StoreNormalizedVector64(NormalizedVector64 *pDestination, FXMVECTOR V);
extern XMVECTOR M3DCORE_API LoadNormalizedVector64(const NormalizedVector64 *pSource);

struct M3DCORE_API NormalizedVector32
{
	union {
		struct {
			// int64 testet to be neccessary to get a size of 8 bytes... (else its 12)
			int32 a : 10;
			int32 b : 9;
			int32 c : 9;
			uint32 p : 2;
			int32 s : 2; // TODO: s==-2 is available.. (can indicate nan or something?)
		};
		int32 v;
	};

    NormalizedVector32() {};
	NormalizedVector32(int32 packed) : v(packed) {}
	NormalizedVector32(float32 x, float32 y, float32 z, float32 w) { XMFLOAT4 tmp(x, y, z, w); StoreNormalizedVector32(this, XMLoadFloat4(&tmp)); }
	NormalizedVector32(const float32* pArray) { XMFLOAT4 tmp(pArray);  StoreNormalizedVector32(this, XMLoadFloat4(&tmp)); }
    NormalizedVector32& operator=(const NormalizedVector32 &nv) { v = nv.v; return *this; }
};

struct M3DCORE_API NormalizedVector64
{
	union {
		struct {
			// int64 testet to be neccessary to get a size of 8 bytes... (else its 12)
			int64 a : 20;
			int64 b : 20;
			int64 c : 20;
			uint64 p : 2;
			int64 s : 2; // TODO: s==-2 is available.. (can indicate nan or something?)
		};
		int64 v;
	};

    NormalizedVector64() {}
	NormalizedVector64(int64 packed) : v(packed) {}
	NormalizedVector64(float32 x, float32 y, float32 z, float32 w) { XMFLOAT4 tmp(x, y, z, w); StoreNormalizedVector64(this, XMLoadFloat4(&tmp)); }
	NormalizedVector64(const float32* pArray) { XMFLOAT4 tmp(pArray); StoreNormalizedVector64(this, XMLoadFloat4(&tmp)); }
    NormalizedVector64& operator=(const NormalizedVector64 &nv) { v = nv.v; return *this; }
};





}