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
#include "Frustum.h"
#include "AxisAlignedBox.h"
#include "Sphere.h"


using namespace m3d;


Frustum::Frustum() 
{
}

Frustum::Frustum(const XMFLOAT4X4 &m)
{
	Set(m);
}

Frustum::~Frustum() 
{
}

void Frustum::Set(const XMFLOAT4X4 &m)
{
	// Left clipping plane
	_planes[LEFTP].x = m._14 + m._11;
	_planes[LEFTP].y = m._24 + m._21;
	_planes[LEFTP].z = m._34 + m._31;
	_planes[LEFTP].w = m._44 + m._41;
	// Right clipping plane
	_planes[RIGHTP].x = m._14 - m._11;
	_planes[RIGHTP].y = m._24 - m._21;
	_planes[RIGHTP].z = m._34 - m._31;
	_planes[RIGHTP].w = m._44 - m._41;
	// Top clipping plane
	_planes[TOPP].x = m._14 - m._12;
	_planes[TOPP].y = m._24 - m._22;
	_planes[TOPP].z = m._34 - m._32;
	_planes[TOPP].w = m._44 - m._42;
	// Bottom clipping plane
	_planes[BOTTOMP].x = m._14 + m._12;
	_planes[BOTTOMP].y = m._24 + m._22;
	_planes[BOTTOMP].z = m._34 + m._32;
	_planes[BOTTOMP].w = m._44 + m._42;
	// Near clipping plane
	_planes[NEARP].x = m._13;
	_planes[NEARP].y = m._23;
	_planes[NEARP].z = m._33;
	_planes[NEARP].w = m._43;
	// Far clipping plane
	_planes[FARP].x = m._14 - m._13;
	_planes[FARP].y = m._24 - m._23;
	_planes[FARP].z = m._34 - m._33;
	_planes[FARP].w = m._44 - m._43;

	XMStoreFloat4(&_planes[0], XMPlaneNormalize(XMLoadFloat4(&_planes[0])));
	XMStoreFloat4(&_planes[1], XMPlaneNormalize(XMLoadFloat4(&_planes[1])));
	XMStoreFloat4(&_planes[2], XMPlaneNormalize(XMLoadFloat4(&_planes[2])));
	XMStoreFloat4(&_planes[3], XMPlaneNormalize(XMLoadFloat4(&_planes[3])));
	XMStoreFloat4(&_planes[4], XMPlaneNormalize(XMLoadFloat4(&_planes[4])));
	XMStoreFloat4(&_planes[5], XMPlaneNormalize(XMLoadFloat4(&_planes[5])));
}

Frustum::Intersection Frustum::Test(const AxisAlignedBox &aab, PlaneId *lastTime) const 
{
	static const PlaneId next[6] = {RIGHTP, TOPP, BOTTOMP, NEARP, FARP, LEFTP};

	if (aab.IsInfinite())
		return INSIDE;

	PlaneId pid = LEFTP;
	if (!lastTime)
		lastTime = &pid;

	XMVECTOR a = XMVectorSetW(XMLoadFloat3(&aab.GetMin()), 1.0f), b = XMVectorSetW(XMLoadFloat3(&aab.GetMax()), 1.0f);

	Intersection r = INSIDE;

	for (uint32 i = 0; i < 6; i++, *lastTime = next[*lastTime]) {
		XMVECTOR pl = XMLoadFloat4(&_planes[*lastTime]);
		XMVECTOR l = XMVectorLess(pl, XMVectorZero());

		// Find positive vertex
		if (XMVector4Less(XMVector4Dot(XMVectorSelect(b, a, l), pl), XMVectorZero()))
			return OUTSIDE; // No need do test the rest of the planes!

		// Find negative vertex
		if (r == INSIDE && XMVector4Less(XMVector4Dot(XMVectorSelect(a, b, l), pl), XMVectorZero()))
			r = INTERSECT; // The best we can do is INTERSECT, but we need to check if OUTSIDE any of the other planes!
	}
	return r;
}

Frustum::Intersection Frustum::Test(const Sphere &s, PlaneId *lastTime) const
{
	static const PlaneId next[6] = {RIGHTP, TOPP, BOTTOMP, NEARP, FARP, LEFTP};

	PlaneId pid = LEFTP;
	if (!lastTime)
		lastTime = &pid;

	XMVECTOR p = XMLoadFloat3(&s.GetPosition());
	FLOAT r = s.GetRadius();

	Intersection result = INSIDE;

	for (uint32 i = 0; i < 6; i++, *lastTime = next[*lastTime]) {
		XMVECTOR pl = XMLoadFloat4(&_planes[*lastTime]);

		FLOAT f = XMVectorGetX(XMVector3Dot(pl, p)) + _planes[*lastTime].w;
		if (f < -r)
			return OUTSIDE;

		if (f * f < r * r)
			result = INTERSECT; // The best we can do is INTERSECT, but we need to check if OUTSIDE any of the other planes!

	}

	return result;
}

Frustum::Intersection Frustum::Test(const Capsule &capsule, PlaneId *lastTime) const
{
	static const PlaneId next[6] = {RIGHTP, TOPP, BOTTOMP, NEARP, FARP, LEFTP};

	PlaneId pid = LEFTP;
	if (!lastTime)
		lastTime = &pid;

	FLOAT r2 = capsule.radius * capsule.radius;
    
	Intersection result = INSIDE;

	for (uint32 i = 0; i < 6; i++, *lastTime = next[*lastTime]) {
		XMVECTOR p = XMLoadFloat4(&_planes[*lastTime]);
		FLOAT signedDist0 = XMVectorGetX(XMPlaneDotCoord(p, XMLoadFloat3(&capsule.position)));
		FLOAT signedDist1 = signedDist0 + XMVectorGetX(XMVector3Dot(p, XMLoadFloat3(&capsule.axis)));
		if (signedDist0 * signedDist1 > r2) {
			// Endpoints of capsule are on same side of plane; test closest endpoint to see if it lies closer to the plane than radius.
			if (signedDist0 * signedDist0 <= signedDist1 * signedDist1) {
				if (signedDist0 < -capsule.radius)
					return OUTSIDE;
				if (signedDist0 < capsule.radius)
					result = INTERSECT;
			}
			else {
				if (signedDist1 < -capsule.radius)
					return OUTSIDE;
				if (signedDist1 < capsule.radius)
					result = INTERSECT;
			}
		}
		else
			result = INTERSECT; // Capsule endpoints are on different sides of the plane, so we have an intersection
#if 0
		// wass iss dass?
		Vec3f plNormal       = planes[i].normal;
		Vec3d plNormalDbl(plNormal.x, plNormal.y, plNormal.z); 
         
		float64 distanceToPlane = planes[i].distanceToSegment(capsule.origin, capsule.axis);
		if (distanceToPlane < -capsule.radius)
			return Outside;
		else if (distanceToPlane <= capsule.radius)
			intersections |= (1 << i);
#endif
	}
	return result;
}


