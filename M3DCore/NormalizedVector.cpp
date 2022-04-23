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
#include "NormalizedVector.h"

using namespace m3d;


#define __NormalizedVector32_abc(x,y,z,w) pDestination->a=int32(x*511);pDestination->b=int32(y*255);pDestination->c=int32(z*255);pDestination->s=w<0.0f?-1:(w>0.0f?1:0);
#define __NormalizedVector32_cba(x,y,z,w) x=float32(pSource->a)/511;y=float32(pSource->b)/255.0f;z=float32(pSource->c)/255.0f;w=sqrt(1.0f-x*x-y*y-z*z)*pSource->s;


void m3d::StoreNormalizedVector32(NormalizedVector32 *pDestination, FXMVECTOR V)
{
	XMFLOAT4 qq;
	XMStoreFloat4(&qq, V*V);
	float32 sum = qq.x+qq.y+qq.z+qq.w;
	if (sum == 0.0f)
		pDestination->v = 0;
	else {
		XMFLOAT4 q;
		if (sum != 1.0f)
			XMStoreFloat4(&q, XMVectorScale(V, 1.0f / sqrtf(sum)));
		else
			XMStoreFloat4(&q, V);
		pDestination->p = 3;
		if (qq.x > qq.y) {
			if (qq.x > qq.z) {
				if (qq.x > qq.w)
					pDestination->p = 0;
			}
			else if (qq.z > qq.w)
				pDestination->p = 2;
		}
		else if (qq.y > qq.z) {
			if (qq.y > qq.w)
				pDestination->p = 1;
		}
		else if (qq.z > qq.w)
			pDestination->p = 2;
		switch (pDestination->p)
		{
		case 0: __NormalizedVector32_abc(q.y, q.z, q.w, q.x); break;
		case 1: __NormalizedVector32_abc(q.x, q.z, q.w, q.y); break;
		case 2: __NormalizedVector32_abc(q.x, q.y, q.w, q.z); break;
		case 3: __NormalizedVector32_abc(q.x, q.y, q.z, q.w); break;
		}
	}
}

XMVECTOR m3d::LoadNormalizedVector32(const NormalizedVector32 *pSource)
{
	XMFLOAT4 q;
	switch (pSource->p)
	{
	case 0: __NormalizedVector32_cba(q.y, q.z, q.w, q.x); break;
	case 1: __NormalizedVector32_cba(q.x, q.z, q.w, q.y); break;
	case 2: __NormalizedVector32_cba(q.x, q.y, q.w, q.z); break;
	case 3: __NormalizedVector32_cba(q.x, q.y, q.z, q.w); break;
	}
	return XMLoadFloat4(&q);
}

#define __NormalizedVector64_abc(x,y,z,w) pDestination->a=int32(x*524287);pDestination->b=int32(y*524287);pDestination->c=int32(z*524287);pDestination->s=w<0.0f?-1:(w>0.0f?1:0);
#define __NormalizedVector64_cba(x,y,z,w) x=float32(pSource->a)/524287.0f;y=float32(pSource->b)/524287.0f;z=float32(pSource->c)/524287.0f;w=sqrt(1.0f-x*x-y*y-z*z)*pSource->s;


void m3d::StoreNormalizedVector64(NormalizedVector64 *pDestination, FXMVECTOR V)
{
	XMFLOAT4 qq;
	XMStoreFloat4(&qq, V*V);
	float32 sum = qq.x+qq.y+qq.z+qq.w;
	if (sum == 0.0f)
		pDestination->v = 0;
	else {
		XMFLOAT4 q;
		if (sum != 1.0f)
			XMStoreFloat4(&q, XMVectorScale(V, 1.0f / sqrtf(sum)));
		else
			XMStoreFloat4(&q, V);
		pDestination->p = 3;
		if (qq.x > qq.y) {
			if (qq.x > qq.z) {
				if (qq.x > qq.w)
					pDestination->p = 0;
			}
			else if (qq.z > qq.w)
				pDestination->p = 2;
		}
		else if (qq.y > qq.z) {
			if (qq.y > qq.w)
				pDestination->p = 1;
		}
		else if (qq.z > qq.w)
			pDestination->p = 2;
		switch (pDestination->p)
		{
		case 0: __NormalizedVector64_abc(q.y, q.z, q.w, q.x); break;
		case 1: __NormalizedVector64_abc(q.x, q.z, q.w, q.y); break;
		case 2: __NormalizedVector64_abc(q.x, q.y, q.w, q.z); break;
		case 3: __NormalizedVector64_abc(q.x, q.y, q.z, q.w); break;
		}
	}
}

XMVECTOR m3d::LoadNormalizedVector64(const NormalizedVector64 *pSource)
{
	XMFLOAT4 q;
	switch (pSource->p)
	{
	case 0: __NormalizedVector64_cba(q.y, q.z, q.w, q.x); break;
	case 1: __NormalizedVector64_cba(q.x, q.z, q.w, q.y); break;
	case 2: __NormalizedVector64_cba(q.x, q.y, q.w, q.z); break;
	case 3: __NormalizedVector64_cba(q.x, q.y, q.z, q.w); break;
	}
	return XMLoadFloat4(&q);
}

