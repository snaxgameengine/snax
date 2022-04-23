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
#include "NormalizedVector.h"

namespace m3d
{


struct M3DCORE_API Transformation
{
	XMFLOAT3 translation;
	NormalizedVector64 rotation;
	XMFLOAT3 scaling;

	Transformation() {}
	Transformation(const XMFLOAT3 &translation, const NormalizedVector64 &rotation, const XMFLOAT3 &scaling) : translation(translation), rotation(rotation), scaling(scaling) {}
	Transformation(const Transformation &rhs): translation(rhs.translation), rotation(rhs.rotation), scaling(rhs.scaling) {}
	~Transformation() {}

	static bool fromMatrix(const XMFLOAT4X4 &m, Transformation &tr)
	{
		XMVECTOR t, r, s;
		if (!XMMatrixDecompose(&s, &r, &t, XMLoadFloat4x4(&m)))
			return false;
		XMStoreFloat3(&tr.translation, t);
		StoreNormalizedVector64(&tr.rotation, r);
		XMStoreFloat3(&tr.scaling, s);
		return true;
	}

	static Transformation lerp(const Transformation &a, const Transformation &b, float64 f)
	{
		Transformation t;
		XMStoreFloat3(&t.translation,  XMVectorAdd(XMLoadFloat3(&a.translation), (XMLoadFloat3(&b.translation) -  XMLoadFloat3(&a.translation)) * float32(f)));
		StoreNormalizedVector64(&t.rotation, XMQuaternionSlerp(LoadNormalizedVector64(&a.rotation), LoadNormalizedVector64(&b.rotation), float32(f)));
		XMStoreFloat3(&t.scaling,  XMVectorAdd(XMLoadFloat3(&a.scaling), (XMLoadFloat3(&b.scaling) -  XMLoadFloat3(&a.scaling)) * float32(f)));
		return t;
	}

	operator XMFLOAT4X4() const
	{
		XMFLOAT4X4 m;
		XMStoreFloat4x4(&m, XMMatrixAffineTransformation(XMLoadFloat3(&scaling), XMVectorZero(), LoadNormalizedVector64(&rotation), XMLoadFloat3(&translation)));
		return m;
	}

	bool operator==(const Transformation &rhs) const
	{
		return XMVector3Equal(XMLoadFloat3(&translation), XMLoadFloat3(&rhs.translation)) && XMVector3Equal(LoadNormalizedVector64(&rotation), LoadNormalizedVector64(&rhs.rotation)) && XMVector3Equal(XMLoadFloat3(&scaling), XMLoadFloat3(&rhs.scaling));
	}
	bool operator!=(const Transformation &rhs) const { return !(*this == rhs); }
};

struct M3DCORE_API TransformationNoScale
{
	XMFLOAT3 translation;
	NormalizedVector64 rotation;

	TransformationNoScale() {}
	TransformationNoScale(const XMFLOAT3 &translation, const NormalizedVector64 &rotation) : translation(translation), rotation(rotation) {}
	TransformationNoScale(const TransformationNoScale &rhs): translation(rhs.translation), rotation(rhs.rotation) {}
	~TransformationNoScale() {}

	static bool fromMatrix(const XMFLOAT4X4 &m, TransformationNoScale &tr)
	{
		XMVECTOR t, r, s;
		if (!XMMatrixDecompose(&s, &r, &t, XMLoadFloat4x4(&m)))
			return false;
		XMStoreFloat3(&tr.translation, t);
		StoreNormalizedVector64(&tr.rotation, r);
		return true;
	}

	static TransformationNoScale lerp(const TransformationNoScale &a, const TransformationNoScale &b, float64 f)
	{
		TransformationNoScale t;
		XMStoreFloat3(&t.translation,  XMVectorAdd(XMLoadFloat3(&a.translation), (XMLoadFloat3(&b.translation) -  XMLoadFloat3(&a.translation)) * float32(f)));
		StoreNormalizedVector64(&t.rotation, XMQuaternionSlerp(LoadNormalizedVector64(&a.rotation), LoadNormalizedVector64(&b.rotation), float32(f)));
		return t;
	}

	operator XMFLOAT4X4() const
	{
		XMFLOAT4X4 m;
		XMStoreFloat4x4(&m, XMMatrixAffineTransformation(XMVectorSplatOne(), XMVectorZero(), LoadNormalizedVector64(&rotation), XMLoadFloat3(&translation)));
		return m;
	}

	bool operator==(const TransformationNoScale &rhs) const
	{
		return XMVector3Equal(XMLoadFloat3(&translation), XMLoadFloat3(&rhs.translation)) && XMVector3Equal(LoadNormalizedVector64(&rotation), LoadNormalizedVector64(&rhs.rotation));
	}
	bool operator!=(const TransformationNoScale &rhs) const { return !(*this == rhs); }
};


}


