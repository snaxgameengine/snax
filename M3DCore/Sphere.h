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

class AxisAlignedBox;


class M3DCORE_API Sphere
{
	XMFLOAT3 _position;
	FLOAT _radius;

public:
	Sphere();
	Sphere(const XMFLOAT3 &position, FLOAT radius);
	Sphere(const AxisAlignedBox &rhs);
	Sphere(const Sphere &rhs);
	~Sphere();

	Sphere &operator=(const Sphere &rhs);

//	inline bool IsInfinite() const { return _isInfinite; }
	inline const XMFLOAT3 &GetPosition() const { return _position; }
	inline FLOAT GetRadius() const { return _radius; }

//	void SetInfinite();
	void SetNull();
	void Set(const XMFLOAT3 &position, FLOAT radius);
	void Set(uint32 count, const XMFLOAT3* vertices, uint32 vStride);
	void Set(uint32 count, const XMFLOAT3* vertices, uint32 vStride, const uint16 *indices);
	void Set(uint32 count, const XMFLOAT3* vertices, uint32 vStride, const uint32 *indices);

	Sphere &Merge(const XMFLOAT3 &p);
	Sphere &Merge(const AxisAlignedBox &aab);
	Sphere &Merge(const Sphere &s);

	Sphere &Transform(CXMMATRIX m);

	inline Sphere &operator+=(const AxisAlignedBox &rhs) { return Merge(rhs); }
	inline Sphere &operator+=(const Sphere &rhs) { return Merge(rhs); }
	inline Sphere &operator*=(CXMMATRIX m) { return Transform(m); }
	inline Sphere operator+(const AxisAlignedBox &rhs) const  { return Sphere(*this).Merge(rhs); }
	inline Sphere operator+(const Sphere &rhs) const  { return Sphere(*this).Merge(rhs); }
	inline Sphere operator*(CXMMATRIX m) const { return Sphere(*this).Transform(m); }

};


// Utility class. Nice for swepth sphere-frustum testing.
struct Capsule
{
	XMFLOAT3 position;
	XMFLOAT3 axis;
	FLOAT radius;

	Capsule() {}
	Capsule(const XMFLOAT3 &position, const XMFLOAT3 &axis, FLOAT radius) : position(position), axis(axis), radius(radius) {}
	Capsule(const Sphere &s, const XMFLOAT3 &axis) : position(s.GetPosition()), axis(axis), radius(s.GetRadius()) {}
};


}