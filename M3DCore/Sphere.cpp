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
#include "Sphere.h"
#include "AxisAlignedBox.h"

using namespace m3d;


Sphere::Sphere() : _position(XMFLOAT3(0.0f, 0.0f, 0.0f)), _radius(0.0f) {}
Sphere::Sphere(const XMFLOAT3 &position, FLOAT radius) : _position(position), _radius(radius) {}
Sphere::Sphere(const AxisAlignedBox &rhs)
{
	if (rhs.IsInfinite()) {
		_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		_radius = std::numeric_limits<float32>::max();
	} 
	else {
		XMVECTOR a = XMLoadFloat3(&rhs.GetMin());
		XMVECTOR b = XMLoadFloat3(&rhs.GetMax()) - a;
		XMStoreFloat3(&_position, a + b * 0.5f);
		_radius = XMVectorGetX(XMVector3Length(b)) * 0.5f;
	}
}
Sphere::Sphere(const Sphere &rhs) : _position(rhs._position), _radius(rhs._radius) {}
Sphere::~Sphere() {}

Sphere &Sphere::operator=(const Sphere &rhs)
{	
	_position = rhs._position;
	_radius = rhs._radius;
	return *this;
}

void Sphere::SetNull()
{
	_position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	_radius = 0.0f;
//	_isInfinite = false;
}

void Sphere::Set(const XMFLOAT3 &position, FLOAT radius)
{
	_position = position;
	_radius = radius;
//	_isInfinite = false;
}

void Sphere::Set(uint32 count, const XMFLOAT3* vertices, uint32 vStride)
{
	SetNull();
	for (uint32 i = 0; i < count; i++)
		Merge(*((XMFLOAT3*)((int8*)vertices + vStride * i)));
//	_isInfinite = false;
}

void Sphere::Set(uint32 count, const XMFLOAT3* vertices, uint32 vStride, const uint16 *indices)
{
	SetNull();
	for (uint32 i = 0; i < count; i++) {
		if (indices[i] != -1) { // Could be -1 to indicate restart strip
			Merge(*((XMFLOAT3*)((int8*)vertices + vStride * indices[i])));
		}
	}
//	_isInfinite = false;
}

void Sphere::Set(uint32 count, const XMFLOAT3* vertices, uint32 vStride, const uint32 *indices)
{
	SetNull();
	for (uint32 i = 0; i < count; i++) {
		if (indices[i] != -1) { // Could be -1 to indicate restart strip
			Merge(*((XMFLOAT3*)((int8*)vertices + vStride * indices[i])));
		}
	}
//	_isInfinite = false;
}

Sphere &Sphere::Merge(const XMFLOAT3 &p)
{
	XMVECTOR pos = XMLoadFloat3(&_position);
	XMVECTOR cVec = XMLoadFloat3(&p) - pos;
	FLOAT d = XMVectorGetX(XMVector3LengthSq(cVec));
	if (d > _radius * _radius) {
		d = sqrtf(d);
		FLOAT r = 0.5f * (d + _radius);
		FLOAT scale = (r - _radius) / d;
		pos += cVec * scale;
		XMStoreFloat3(&_position, pos);
		_radius = r;
	}
	return *this;
}

Sphere &Sphere::Merge(const Sphere &s)
{
	XMVECTOR p0 = XMLoadFloat3(&_position);
	XMVECTOR p1 = XMLoadFloat3(&s._position);
	FLOAT d = XMVectorGetX(XMVector3Length(p1 - p0));
	if (_radius > s._radius + d)
		return *this; // we enclose s!
	if (s._radius > _radius + d)
		return *this = s; // s enclose us!
	FLOAT r = (_radius + s._radius + d) * 0.5f;
	XMStoreFloat3(&_position, p0 + (p1 - p0) * ((r - _radius) / d));
	_radius = r;
	return *this;
}

Sphere &Sphere::Merge(const AxisAlignedBox &aab)
{
	const XMFLOAT3 &a = aab.GetMin();
	const XMFLOAT3 &b = aab.GetMax();
	Merge(XMFLOAT3(a.x, a.y, a.z));
	Merge(XMFLOAT3(a.x, a.y, b.z));
	Merge(XMFLOAT3(a.x, b.y, a.z));
	Merge(XMFLOAT3(a.x, b.y, b.z));
	Merge(XMFLOAT3(b.x, a.y, a.z));
	Merge(XMFLOAT3(b.x, a.y, b.z));
	Merge(XMFLOAT3(b.x, b.y, a.z));
	Merge(XMFLOAT3(b.x, b.y, b.z));
	return *this;
}

Sphere &Sphere::Transform(CXMMATRIX m)
{
	XMStoreFloat3(&_position, XMVector3TransformCoord(XMLoadFloat3(&_position), m));
//	_radius *= XMVectorGetX(XMVector3Length(m.r[0]));
	return *this;
}

