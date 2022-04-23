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

class Sphere;


class M3DCORE_API AxisAlignedBox
{
	XMFLOAT3 _minP;
	XMFLOAT3 _maxP;
	bool _isInfinite;

public:
	AxisAlignedBox();
	AxisAlignedBox(const XMFLOAT3 &minP, const XMFLOAT3 &maxP);
	AxisAlignedBox(const AxisAlignedBox &rhs);
	AxisAlignedBox(const Sphere &rhs);
	~AxisAlignedBox();

	AxisAlignedBox &operator=(const AxisAlignedBox &rhs);

	inline bool IsInfinite() const { return _isInfinite; }
	inline const XMFLOAT3 &GetMin() const { return _minP; }
	inline const XMFLOAT3 &GetMax() const { return _maxP; }

	void SetInfinite();
	void SetNull();
	void Set(const XMFLOAT3 &minP, const XMFLOAT3 &maxP);
	void Set(uint32 count, const XMFLOAT3* vertices, uint32 vStride);
	void Set(uint32 count, const XMFLOAT3* vertices, uint32 vStride, const uint16 *indices);
	void Set(uint32 count, const XMFLOAT3* vertices, uint32 vStride, const uint32 *indices);

	AxisAlignedBox &Merge(const XMFLOAT3 &p);
	AxisAlignedBox &Merge(const AxisAlignedBox &bb);
	AxisAlignedBox &Merge(const Sphere &s);

	AxisAlignedBox &Transform(CXMMATRIX m);

	AxisAlignedBox Intersect(const AxisAlignedBox &aab) const;

	inline AxisAlignedBox &operator+=(const AxisAlignedBox &rhs) { return Merge(rhs); }
	inline AxisAlignedBox &operator+=(const Sphere &rhs) { return Merge(rhs); }
	inline AxisAlignedBox &operator*=(CXMMATRIX m) { return Transform(m); }
	inline AxisAlignedBox operator+(const AxisAlignedBox &rhs) const  { return AxisAlignedBox(*this).Merge(rhs); }
	inline AxisAlignedBox operator+(const Sphere &rhs) const  { return AxisAlignedBox(*this).Merge(rhs); }
	inline AxisAlignedBox operator*(CXMMATRIX m) const { return AxisAlignedBox(*this).Transform(m); }

};




}