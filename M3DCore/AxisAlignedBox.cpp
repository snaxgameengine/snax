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
#include "AxisAlignedBox.h"
#include "Sphere.h"

using namespace m3d;

AxisAlignedBox::AxisAlignedBox() : _isInfinite(true), _minP(-std::numeric_limits<float32>::max(), -std::numeric_limits<float32>::max(), -std::numeric_limits<float32>::max()), _maxP(std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max()) {}
AxisAlignedBox::AxisAlignedBox(const XMFLOAT3 &minP, const XMFLOAT3 &maxP) : _isInfinite(false), _minP(minP), _maxP(maxP) {}
AxisAlignedBox::AxisAlignedBox(const AxisAlignedBox &rhs) : _isInfinite(rhs._isInfinite), _minP(rhs._minP), _maxP(rhs._maxP) {}
AxisAlignedBox::AxisAlignedBox(const Sphere &rhs) : _isInfinite(false)
{
	const XMFLOAT3 p = rhs.GetPosition();
	FLOAT r = rhs.GetRadius();
	_minP = XMFLOAT3(p.x - r, p.y - r, p.z - r);
	_maxP = XMFLOAT3(p.x + r, p.y + r, p.z + r);
}
AxisAlignedBox::~AxisAlignedBox() {}

AxisAlignedBox &AxisAlignedBox::operator=(const AxisAlignedBox &rhs)
{
	_minP = rhs._minP;
	_maxP = rhs._maxP;
	_isInfinite = rhs._isInfinite;
	return *this;
}

void AxisAlignedBox::SetInfinite() 
{ 
	_isInfinite = true; 
	_minP = XMFLOAT3(-std::numeric_limits<float32>::max(), -std::numeric_limits<float32>::max(), -std::numeric_limits<float32>::max());
	_maxP = XMFLOAT3(std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max());
}

void AxisAlignedBox::SetNull()
{
	_isInfinite = false; 
	_minP = XMFLOAT3(std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max(), std::numeric_limits<float32>::max());
	_maxP = XMFLOAT3(-std::numeric_limits<float32>::max(), -std::numeric_limits<float32>::max(), -std::numeric_limits<float32>::max());
}

void AxisAlignedBox::Set(const XMFLOAT3 &minP, const XMFLOAT3 &maxP)
{
	_isInfinite = false;
	_minP = minP;
	_maxP = maxP;
}

void AxisAlignedBox::Set(uint32 count, const XMFLOAT3* vertices, uint32 vStride)
{
	XMVECTOR a = XMVectorReplicate(std::numeric_limits<float32>::max()), b = XMVectorReplicate(-std::numeric_limits<float32>::max());
	for (uint32 i = 0; i < count; i++) {
		XMVECTOR c = XMLoadFloat3((XMFLOAT3*)((int8*)vertices + vStride * i));
		a = XMVectorMin(a, c);
		a = XMVectorMax(a, c);
	}
	XMStoreFloat3(&_minP, a);
	XMStoreFloat3(&_maxP, b);
	_isInfinite = false;
}

void AxisAlignedBox::Set(uint32 count, const XMFLOAT3* vertices, uint32 vStride, const uint16 *indices)
{
	XMVECTOR a = XMVectorReplicate(std::numeric_limits<float32>::max()), b = XMVectorReplicate(-std::numeric_limits<float32>::max());
	for (uint32 i = 0; i < count; i++) {
		if (indices[i] != -1) { // Could be -1 to indicate restart strip
			XMVECTOR c = XMLoadFloat3((XMFLOAT3*)((int8*)vertices + vStride * indices[i]));
			a = XMVectorMin(a, c);
			b = XMVectorMax(b, c);
		}
	}
	XMStoreFloat3(&_minP, a);
	XMStoreFloat3(&_maxP, b);
	_isInfinite = false;
}

void AxisAlignedBox::Set(uint32 count, const XMFLOAT3* vertices, uint32 vStride, const uint32 *indices)
{
	XMVECTOR a = XMVectorReplicate(std::numeric_limits<float32>::max()), b = XMVectorReplicate(-std::numeric_limits<float32>::max());
	for (uint32 i = 0; i < count; i++) {
		if (indices[i] != -1) { // Could be -1 to indicate restart strip
			XMVECTOR c = XMLoadFloat3((XMFLOAT3*)((int8*)vertices + vStride * indices[i]));
			a = XMVectorMin(a, c);
			b = XMVectorMax(b, c);
		}
	}
	XMStoreFloat3(&_minP, a);
	XMStoreFloat3(&_maxP, b);
	_isInfinite = false;
}

AxisAlignedBox &AxisAlignedBox::Merge(const XMFLOAT3 &p) 
{
	if (_isInfinite)
		return *this;
	_minP.x = XMMin(_minP.x, p.x);
	_minP.y = XMMin(_minP.y, p.y);
	_minP.z = XMMin(_minP.z, p.z);
	_maxP.x = XMMax(_maxP.x, p.x);
	_maxP.y = XMMax(_maxP.y, p.y);
	_maxP.z = XMMax(_maxP.z, p.z);
	return *this;
}

AxisAlignedBox &AxisAlignedBox::Merge(const AxisAlignedBox &bb)
{
	if (_isInfinite)
		return *this;
	if (bb._isInfinite)
		SetInfinite();
	else {
		Merge(bb._minP);
		Merge(bb._maxP);
	}
	return *this;
}

AxisAlignedBox &AxisAlignedBox::Merge(const Sphere &s)
{
	if (_isInfinite)
		return *this;
	const XMFLOAT3 p = s.GetPosition();
	FLOAT r = s.GetRadius();
	Merge(XMFLOAT3(p.x + r, p.y + r, p.z + r));
	Merge(XMFLOAT3(p.x - r, p.y - r, p.z - r));
	return *this;
}

AxisAlignedBox &AxisAlignedBox::Transform(CXMMATRIX m)
{
	if (_isInfinite)
		return *this;

	XMVECTORF32 corners[8] = // Changed from XMVECTOR do to ARM
	{
		{ _minP.x, _minP.y, _minP.z, 0.0f },
		{ _minP.x, _minP.y, _maxP.z, 0.0f },
		{ _minP.x, _maxP.y, _minP.z, 0.0f },
		{ _minP.x, _maxP.y, _maxP.z, 0.0f },
		{ _maxP.x, _minP.y, _minP.z, 0.0f },
		{ _maxP.x, _minP.y, _maxP.z, 0.0f },
		{ _maxP.x, _maxP.y, _minP.z, 0.0f },
		{ _maxP.x, _maxP.y, _maxP.z, 0.0f }
	};

	XMVECTOR a = XMVectorReplicate(std::numeric_limits<float32>::max()), b = XMVectorReplicate(-std::numeric_limits<float32>::max()), c;

	for (uint32 i = 0; i < 8; i++) {
		c = XMVector3TransformCoord(corners[i], m);
		a = XMVectorMin(a, c);
		b = XMVectorMax(b, c);	
	}

	XMStoreFloat3(&_minP, a);
	XMStoreFloat3(&_maxP, b);

	return *this;
}


AxisAlignedBox AxisAlignedBox::Intersect(const AxisAlignedBox &aab) const
{
	AxisAlignedBox r;
	XMStoreFloat3(&r._minP, XMVectorMax(XMLoadFloat3(&_minP), XMLoadFloat3(&aab._minP)));
	XMStoreFloat3(&r._maxP, XMVectorMin(XMLoadFloat3(&_maxP), XMLoadFloat3(&aab._maxP)));
	return r;
}
