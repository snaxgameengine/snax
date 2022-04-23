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


#include <math.h>
#include <limits>
#include <stdlib.h>
#include <DirectXMath.h>
#include "ValueDef.h"
#include "M3DCore/MString.h"

#define _PI 3.14159265358979323846
#define _TORADFACT 0.01745329252
#define _TODEGFACT 57.29577951

#define DUPL(x) n->x ? x->duplicate() : nullptr

namespace m3d {

static XMVECTOR XMMask_(FXMVECTOR v, uint32 r, uint32 c)
{
	static const XMVECTOR MM[] = { XMVectorSelectControl(1, 1, 1, 1), XMVectorSelectControl(0, 1, 1, 1), XMVectorSelectControl(0, 0, 1, 1), XMVectorSelectControl(0, 0, 0, 1), XMVectorSelectControl(0, 0, 0, 0) };

	return XMVectorSelect(v, XMVectorZero(), MM[r]);
}

static XMMATRIX XMMask_(FXMMATRIX m, uint32 r, uint32 c)
{
	static const XMVECTOR MM[] = { XMVectorSelectControl(1, 1, 1, 1), XMVectorSelectControl(0, 1, 1, 1), XMVectorSelectControl(0, 0, 1, 1), XMVectorSelectControl(0, 0, 0, 1), XMVectorSelectControl(0, 0, 0, 0) };
	static const XMMATRIX ID = XMMatrixIdentity();

	XMMATRIX e;

	e.r[0] = XMVectorSelect(m.r[0], ID.r[0], MM[c > 0 ? r : 0]);
	e.r[1] = XMVectorSelect(m.r[1], ID.r[1], MM[c > 1 ? r : 0]);
	e.r[2] = XMVectorSelect(m.r[2], ID.r[2], MM[c > 2 ? r : 0]);
	e.r[3] = XMVectorSelect(m.r[3], ID.r[3], MM[c > 3 ? r : 0]);

	return e;
}

struct ExprValue;

class ExpressionVariableCallback
{
public:
	virtual uint32 Register(const String &s) = 0;
	virtual ExprValue GetValue(uint32 index) = 0;
};

struct InvalidDimensionException {};

struct __declspec(align(16)) ExprValue
{
	union 
	{
		XMVECTOR vec;
		XMMATRIX mat;
		value scalar;
		float32 arr[16];
		XMFLOAT4 vector;
		XMFLOAT4X4 matrix;
	};
	union 
	{
		struct {
			uint16 r;
			uint16 c;
		};
		uint32 rc;
	};
	uint32 n;

	static const uint32 RC_SCALAR = 0x10001;
	static const uint32 RC_4X1 = 0x10004;
	static const uint32 RC_2X2 = 0x20002;
	static const uint32 RC_3X3 = 0x30003;
	static const uint32 RC_4X4 = 0x40004;

	ExprValue(uint16 r = 0, uint16 c = 0) : r(r), c(c), n(r * c) { memset(arr, 0, sizeof(arr)); }
	ExprValue(value s) : scalar(s), rc(RC_SCALAR), n(1) { memset((uint8*)arr + sizeof(value), 0, sizeof(arr) - sizeof(value)); }
	ExprValue(const XMFLOAT4 &vector) : vec(XMLoadFloat4(&vector)), rc(RC_4X1), n(4) { memset((uint8*)arr + sizeof(XMFLOAT4), 0, sizeof(arr) - sizeof(XMFLOAT4)) ; }
	ExprValue(const XMFLOAT4X4 &matrix) : matrix(matrix), rc(RC_4X4), n(16) { }
	ExprValue(const ExprValue &rhs) : rc(rhs.rc), n(rhs.n) { std::memcpy(arr, rhs.arr, sizeof(arr)); }
	ExprValue(FXMVECTOR v) : vec(v), rc(RC_4X1), n(4) {}

	const ExprValue &operator=(const ExprValue &rhs)
	{
		if (this != &rhs) {
			rc = rhs.rc;
			n = rhs.n;
			std::memcpy(arr, rhs.arr, sizeof(arr));
		}
		return *this;
	}

	ExprValue operator+(const ExprValue &rhs) const
	{
		if (rc != rhs.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ExprValue(scalar + rhs.scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] + rhs.arr[i];
		return e;
	}

	ExprValue operator-(const ExprValue &rhs) const
	{
		if (rc != rhs.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ExprValue(scalar - rhs.scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] - rhs.arr[i];
		return e;
	}

	ExprValue operator*(const ExprValue &rhs) const
	{
		return mul(rhs);
		/*if (n != rhs.n)
			throw InvalidDimensionException();
		ExprValue r;
		if (n == 1)
			return ExprValue(scalar * rhs.scalar);
		for (uint32 i = 0; i < n; i++)
			r.arr[i] = arr[i] * rhs.arr[i];
		return r;*/
	}

	ExprValue operator/(const ExprValue &rhs) const
	{
		if (rc != rhs.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ExprValue(scalar / rhs.scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] / rhs.arr[i];
		return e;
	}

	ExprValue operator^(const ExprValue &rhs) const 
	{
		if (rc != rhs.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ::pow(scalar, rhs.scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::pow(arr[i], rhs.arr[i]);
		return e;
	}

	ExprValue operator%(const ExprValue &rhs) const
	{
		if (rc != rhs.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ::fmod(scalar, rhs.scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::fmod(arr[i], rhs.arr[i]);
		return e;
	}

	ExprValue operator==(const ExprValue &rhs) const
	{
		if (rc != rhs.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ExprValue(scalar == rhs.scalar ? 1.0 : 0.0);
		if (c == 1)
			return ExprValue(XMVector4Equal(vec, rhs.vec) ? 1.0f : 0.0f);
		
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] == rhs.arr[i] ? 1.0f : 0.0f;
		return e;
	}

	ExprValue operator!=(const ExprValue &rhs) const
	{
		if (rc != rhs.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ExprValue(scalar != rhs.scalar ? 1.0 : 0.0);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] != rhs.arr[i] ? 1.0f : 0.0f;
		return e;
	}

	ExprValue operator<(const ExprValue &rhs) const
	{
		if (rc != rhs.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ExprValue(scalar < rhs.scalar ? 1.0 : 0.0);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] < rhs.arr[i] ? 1.0f : 0.0f;
		return e;
	}

	ExprValue operator<=(const ExprValue &rhs) const
	{
		if (rc != rhs.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ExprValue(scalar <= rhs.scalar ? 1.0 : 0.0);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] <= rhs.arr[i] ? 1.0f : 0.0f;
		return e;
	}

	ExprValue operator>(const ExprValue &rhs) const
	{
		if (rc != rhs.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ExprValue(scalar > rhs.scalar ? 1.0 : 0.0);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] > rhs.arr[i] ? 1.0f : 0.0f;
		return e;
	}

	ExprValue operator>=(const ExprValue &rhs) const
	{
		if (rc != rhs.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ExprValue(scalar >= rhs.scalar ? 1.0 : 0.0);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] >= rhs.arr[i] ? 1.0f : 0.0f;
		return e;
	}

	ExprValue operator&&(const ExprValue &rhs) const
	{
		if (rc != rhs.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ExprValue(::abs(scalar) >= 0.5 && ::abs(rhs.scalar) >= 0.5 ? 1.0 : 0.0);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::abs(arr[i]) >= 0.5f && ::abs(rhs.arr[i]) >= 0.5f ? 1.0f : 0.0f;
		return e;
	}

	ExprValue operator||(const ExprValue &rhs) const
	{
		if (rc != rhs.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ExprValue(::abs(scalar) >= 0.5 || ::abs(rhs.scalar) >= 0.5 ? 1.0 : 0.0);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::abs(arr[i]) >= 0.5f || ::abs(rhs.arr[i]) >= 0.5f ? 1.0f : 0.0f;
		return e;
	}

	ExprValue operator<<(const ExprValue &rhs) const
	{
		return ExprValue();
	}

	ExprValue operator>>(const ExprValue &rhs) const
	{
		return ExprValue();
	}

	ExprValue operator&(const ExprValue &rhs) const
	{
		return ExprValue();
	}

	ExprValue operator|(const ExprValue &rhs) const
	{
		return ExprValue();
	}

	ExprValue operator+() const
	{
		return *this;
	}

	ExprValue operator-() const
	{
		if (rc == RC_SCALAR)
			return ExprValue(-scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = -arr[i];
		return e;
	}

	ExprValue operator!() const
	{
		if (rc == RC_SCALAR)
			return ExprValue(::abs(scalar) >= 0.5f ? 0.0f : 1.0f);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::abs(arr[i]) >= 0.5f ? 0.0f : 1.0f;
		return e;
	}

	template<typename T>
	static T _min(T a, T b) { return a < b ? a : b; }
	template<typename T>
	static T _max(T a, T b) { return a > b ? a : b; }

	static ExprValue min(const ExprValue &lhs, const ExprValue &rhs)
	{
		if (lhs.rc != rhs.rc)
			throw InvalidDimensionException();
		if (lhs.rc == RC_SCALAR)
			return ExprValue(_min(lhs.scalar, rhs.scalar));
		ExprValue e(lhs.r, lhs.c);
		for (uint32 i = 0; i < lhs.n; i++)
			e.arr[i] = _min(lhs.arr[i], rhs.arr[i]);
		return e;
	}

	static ExprValue max(const ExprValue &lhs, const ExprValue &rhs)
	{
		if (lhs.rc != rhs.rc)
			throw InvalidDimensionException();
		if (lhs.rc == RC_SCALAR)
			return ExprValue(_max(lhs.scalar, rhs.scalar));
		ExprValue e(lhs.r, lhs.c);
		for (uint32 i = 0; i < lhs.n; i++)
			e.arr[i] = _max(lhs.arr[i], rhs.arr[i]);
		return e;
	}

	static ExprValue lerp(const ExprValue &l, const ExprValue &h, const ExprValue &v)
	{
		if (l.rc != h.rc || l.rc != v.rc)
			throw InvalidDimensionException();
		if (l.rc == RC_SCALAR)
			return ExprValue(l.scalar * (1.0 - v.scalar) + h.scalar * v.scalar);
		ExprValue e(l.r, l.c);
		for (uint32 i = 0; i < l.n; i++)
			e.arr[i] = l.arr[i] * (1.0f - v.arr[i]) + h.arr[i] * v.arr[i];
		return e;
	}

	ExprValue clamp(const ExprValue &l, const ExprValue &h) const
	{
		if (rc != l.rc || rc != h.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ExprValue(_min(_max(scalar, l.scalar), h.scalar));
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = _min(_max(arr[i], l.arr[i]), h.arr[i]);
		return e;
	}

	ExprValue fmod(const ExprValue &rhs) const
	{
		if (rc != rhs.rc)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ::fmod(scalar, rhs.scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::fmod(arr[i], rhs.arr[i]);
		return e;
	}

	ExprValue frac() const
	{
		if (rc == RC_SCALAR)
			return scalar - ::floor(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] - ::floor(arr[i]);
		return e;
	}

	ExprValue saturate() const
	{
		if (rc == RC_SCALAR)
			return _min(_max(scalar, 0.0), 1.0);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = _min(_max(arr[i], 0.0f), 1.0f);
		return e;
	}

	ExprValue sin() const
	{
		if (rc == RC_SCALAR)
			return ::sin(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::sin(arr[i]);
		return e;
	}

	ExprValue cos() const
	{
		if (rc == RC_SCALAR)
			return ::cos(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::cos(arr[i]);
		return e;
	}

	ExprValue tan() const
	{
		if (rc == RC_SCALAR)
			return ::tan(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::tan(arr[i]);
		return e;
	}

	ExprValue asin() const
	{
		if (rc == RC_SCALAR)
			return ::asin(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::asin(arr[i]);
		return e;
	}

	ExprValue acos() const
	{
		if (rc == RC_SCALAR)
			return ::acos(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::acos(arr[i]);
		return e;
	}

	ExprValue atan() const
	{
		if (rc == RC_SCALAR)
			return ::atan(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::atan(arr[i]);
		return e;
	}

	ExprValue abs() const
	{
		if (rc == RC_SCALAR)
			return ::abs(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::abs(arr[i]);
		return e;
	}

	ExprValue sqrt() const
	{
		if (rc == RC_SCALAR)
			return ::sqrt(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::sqrt(arr[i]);
		return e;
	}

	ExprValue floor() const
	{
		if (rc == RC_SCALAR)
			return ::floor(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::floor(arr[i]);
		return e;
	}

	ExprValue ceil() const
	{
		if (rc == RC_SCALAR)
			return ::ceil(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::ceil(arr[i]);
		return e;
	}

	ExprValue round() const
	{
		if (rc == RC_SCALAR)
			return ::floor(scalar + 0.5);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::floor(arr[i] + 0.5f);
		return e;
	}

	ExprValue sinh() const
	{
		if (rc == RC_SCALAR)
			return ::sinh(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::sinh(arr[i]);
		return e;
	}

	ExprValue cosh() const
	{
		if (rc == RC_SCALAR)
			return ::cosh(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::cosh(arr[i]);
		return e;
	}

	ExprValue tanh() const
	{
		if (rc == RC_SCALAR)
			return ::tanh(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::tanh(arr[i]);
		return e;
	}

	ExprValue log() const
	{
		if (rc == RC_SCALAR)
			return ::log10(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::log10(arr[i]);
		return e;
	}

	ExprValue ln() const
	{
		if (rc == RC_SCALAR)
			return ::log(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::log(arr[i]);
		return e;
	}

	ExprValue exp() const
	{
		if (rc == RC_SCALAR)
			return ::exp(scalar);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = ::exp(arr[i]);
		return e;
	}

	ExprValue sign() const
	{
		if (rc == RC_SCALAR)
			return scalar < 0.0 ? -1.0 : (scalar > 0.0 ? 1.0 : 0.0);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] < 0.0f ? -1.0f : (arr[i] > 0.0f ? 1.0f : 0.0f);
		return e;
	}

	ExprValue radians() const
	{
		if (rc == RC_SCALAR)
			return scalar * _TORADFACT;
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] * (float32)_TORADFACT;
		return e;
	}

	ExprValue degrees() const
	{
		if (rc == RC_SCALAR)
			return scalar * _TODEGFACT;
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] * (float32)_TODEGFACT;
		return e;
	}

	ExprValue isnan() const
	{
		if (rc == RC_SCALAR)
			return scalar != scalar ? 1.0 : 0.0;
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] != arr[i] ? 1.0f : 0.0f;
		return e;
	}

	ExprValue isinf() const
	{
		if (rc == RC_SCALAR)
			return scalar == std::numeric_limits<value>::infinity() ? 1.0f : (scalar == -std::numeric_limits<value>::infinity() ? -1.0f : 0.0f);
		ExprValue e(r, c);
		for (uint32 i = 0; i < n; i++)
			e.arr[i] = arr[i] == std::numeric_limits<float32>::infinity() ? 1.0f : (arr[i] == -std::numeric_limits<float32>::infinity() ? -1.0f : 0.0f);
		return e;
	}

	ExprValue all() const
	{
		if (rc == RC_SCALAR)
			return ::abs(scalar) >= 0.5f ? ExprValue(1.0f) : ExprValue(0.0f);
		for (uint32 i = 0; i < n; i++)
			if (::abs(arr[i]) < 0.5f)
				return ExprValue(0.0f);
		return ExprValue(1.0f);
	}

	ExprValue any() const
	{
		if (rc == RC_SCALAR)
			return ::abs(scalar) >= 0.5f ? ExprValue(1.0f) : ExprValue(0.0f);
		for (uint32 i = 0; i < n; i++)
			if (::abs(arr[i]) >= 0.5f)
				return ExprValue(1.0f);
		return ExprValue(0.0f);
	}

	ExprValue determinant() const
	{
	}

	ExprValue cross(const ExprValue &rhs) const
	{
	}

	ExprValue dst(const ExprValue &rhs) const
	{
	}

	ExprValue faceforward(const ExprValue &i, const ExprValue &ng) const
	{
	}

	ExprValue length() const 
	{
		if (rc == RC_SCALAR)
			return *this;
		if (c > 1)
			throw InvalidDimensionException();
		float32 f = 0.0f;
		XMStoreFloat(&f, XMVector4Length(vec));
		return ExprValue(f);
	}

	ExprValue normalize() const
	{
		if (rc == RC_SCALAR)
			return ExprValue(1.0f);
		if (c > 1)
			throw InvalidDimensionException();
		return ExprValue(XMVector4Normalize(vec));
	}

	ExprValue rsqrt() const
	{
	}

	ExprValue smoothstep(const ExprValue &min, const ExprValue &max) const
	{
	}

	ExprValue step(const ExprValue &rhs) const
	{
	}

	ExprValue reflect(const ExprValue &n) const
	{
	}

	ExprValue refract(const ExprValue &n, const ExprValue &index) const
	{
	}

	ExprValue dot(const ExprValue &rhs) const
	{
		if (rc != rhs.rc || c > 1)
			throw InvalidDimensionException();
		if (rc == RC_SCALAR)
			return ExprValue(scalar * rhs.scalar);
		float32 f = 0.0f;
		XMStoreFloat(&f, XMVector4Dot(vec, rhs.vec));
		return ExprValue(f);
	}

	ExprValue mul(const ExprValue &rhs) const
	{
		if (rc == RC_SCALAR) { // We are scalar
			if (rhs.rc == RC_SCALAR) // Both are scalars
				return ExprValue(scalar * rhs.scalar);
			ExprValue e(rhs.r, rhs.c); // They are vector or matrix.

			for (uint32 i = 0; i < rhs.n; i++)
				e.arr[i] = rhs.arr[i] * (float32)scalar;
			return e;
		}
		if (rhs.rc == RC_SCALAR) { // The are scalar, we are vector or matrix
			ExprValue e(r, c);
			for (uint32 i = 0; i < n; i++)
				e.arr[i] = arr[i] * (float32)rhs.scalar;
			return e;
		}
		if (c == 1 && rhs.c == r) { // We are vector, they are matrix
			ExprValue e(r, c);
			if (rhs.rc == RC_4X4)
				XMStoreFloat4((XMFLOAT4*)e.arr, XMVector4Transform(XMLoadFloat4((const XMFLOAT4*)arr), XMMatrixTranspose(XMLoadFloat4x4((const XMFLOAT4X4*)rhs.arr))));
			else if (rhs.rc == RC_3X3)
				XMStoreFloat3((XMFLOAT3*)e.arr, XMVector3Transform(XMLoadFloat3((const XMFLOAT3*)arr), XMMatrixTranspose(XMLoadFloat3x3((const XMFLOAT3X3*)rhs.arr))));
			else if (rhs.rc == RC_2X2) {
				e.arr[0] = arr[0] * rhs.arr[0] + arr[1] * rhs.arr[2];
				e.arr[1] = arr[1] * rhs.arr[1] + arr[1] * rhs.arr[3];
			}
			else
				throw InvalidDimensionException();
			return e;
		}
		if (rhs.c == 1 && c == rhs.r) { // We are matrix, they are vector
			ExprValue e(rhs.r, rhs.c);
			if (rc == RC_4X4)
				XMStoreFloat4((XMFLOAT4*)e.arr, XMVector4Transform(XMLoadFloat4((const XMFLOAT4*)rhs.arr), XMLoadFloat4x4((const XMFLOAT4X4*)arr)));
			else if (rc == RC_3X3)
				XMStoreFloat3((XMFLOAT3*)e.arr, XMVector3Transform(XMLoadFloat3((const XMFLOAT3*)rhs.arr), XMLoadFloat3x3((const XMFLOAT3X3*)arr)));
			else if (rc == RC_2X2) {
				e.arr[0] = rhs.arr[0] * arr[0] + rhs.arr[1] * arr[1];
				e.arr[1] = rhs.arr[1] * arr[2] + rhs.arr[1] * arr[3];
			}
			else 
				throw InvalidDimensionException();
			return e;
		}
		if (r == rhs.c) { // Both are matrices.
			if (rc == RC_4X4) {
				ExprValue e(4, 4);
				XMStoreFloat4x4((XMFLOAT4X4*)e.arr, XMMatrixMultiply(XMLoadFloat4x4((const XMFLOAT4X4*)arr), XMLoadFloat4x4((const XMFLOAT4X4*)rhs.arr)));
				return e;
			}
			if (rc == RC_3X3) {
				ExprValue e(3, 3);
				XMStoreFloat3x3((XMFLOAT3X3*)e.arr, XMMatrixMultiply(XMLoadFloat3x3((const XMFLOAT3X3*)arr), XMLoadFloat3x3((const XMFLOAT3X3*)rhs.arr)));
				return e;
			}
			if (rc == RC_2X2) {
			}
			// All other matrices eg 4x3, 2x4 etc are illigal!
			throw InvalidDimensionException();
		}
		if (n == rhs.n) { // Both are vectors.
			ExprValue e(r, c);
			for (uint32 i = 0; i < n; i++)
				e.arr[i] = arr[i] * rhs.arr[i];
			return e;
		}
		throw InvalidDimensionException();
	}

	bool toBool() const
	{
		if (rc != RC_SCALAR)
			throw InvalidDimensionException();
		return ::abs(scalar) >= 0.5;
	}
};

struct ExprNode
{
	enum Type { MEMBER, NEGOPR, POSOPR, NOTOPR, ADDOPR, SUBOPR, MULOPR, DIVOPR, POWOPR, INTMOD, LESSOPR, LESSEQOPR, GREATEROPR, GREATEREQOPR, EQUALOPR, NOTEQUALOPR, ANDOPR, OROPR, MIN, MAX,
				CLAMP, SATURATE, SIN, COS, TAN, ASIN, ACOS, ATAN, ABS, SQRT, FLOOR, CEIL, ROUND, SINH, COSH, TANH, LOG, LN, EXP, POW, FMOD, FRAC, LERP, SIGN, RAD, DEG, ISNAN, ISINF, RAND, PI, 
				IFTHENELSE, SCALAR, CHILD, DOT, MUL, SHFTLFT, SHFTRHT, BTWAND, BTWOR, ALL, ANY, DETERMINANT, CROSS, DISTANCE, DST, FACEFWD, LENGTH, NORMALIZE, RSQRT, SMOOTHSTEP, STEP, REFLECT, REFRACT };

	const Type type;

	ExprNode(Type type) : type(type) {}
	virtual ~ExprNode() {}
	virtual ExprValue operator()() const = 0;
	virtual int precedence() const = 0;
	virtual bool isOperator() const = 0;
	virtual ExprNode *duplicate() const = 0;
	virtual String hlsl() const = 0;
	virtual bool build(List<ExprNode*> &v) { v.pop_back(); return true; }
};

typedef ExprNode *(*ExprNodeFactFunc)();

struct OperandExprNode : public ExprNode
{
	OperandExprNode(Type type) : ExprNode(type) {}
	int precedence() const override { return 0; }
	bool isOperator() const override { return false; }
};

struct OperatorExprNode : public ExprNode
{
	OperatorExprNode(Type type) : ExprNode(type) {}
	bool isOperator() const override { return true; }
};

struct UnaryExprNode : public OperatorExprNode
{
	ExprNode *a;
	UnaryExprNode(Type type) : OperatorExprNode(type), a(nullptr) {}
	~UnaryExprNode() { if (a) delete a; }

	bool build(List<ExprNode*> &v) 
	{ 
		v.pop_back();
		if (!v.size())
			return false;
		a = v.back();
		return a->build(v);
	}
};

struct BinaryExprNode : public OperatorExprNode
{
	ExprNode *a, *b;
	BinaryExprNode(Type type) : OperatorExprNode(type), a(nullptr), b(nullptr) {}
	~BinaryExprNode() { if (a) delete a; if (b) delete b; }

	bool build(List<ExprNode*> &v) 
	{ 
		v.pop_back();
		if (!v.size())
			return false;
		b = v.back();
		if (!b->build(v))
			return false;
		if (!v.size())
			return false;
		a = v.back();
		return a->build(v);
	}
};

struct TernaryExprNode : public OperatorExprNode
{
	ExprNode *a, *b, *c;
	TernaryExprNode(Type type) : OperatorExprNode(type), a(nullptr), b(nullptr), c(nullptr) {}
	~TernaryExprNode() { if (a) delete a; if (b) delete b; if(c) delete c; }

	bool build(List<ExprNode*> &v) 
	{ 
		v.pop_back();
		if (!v.size())
			return false;
		c = v.back();
		if (!c->build(v))
			return false;
		if (!v.size())
			return false;
		b = v.back();
		if (!b->build(v))
			return false;
		if (!v.size())
			return false;
		a = v.back();
		return a->build(v);
	}
};

struct MemberExprNode : public UnaryExprNode
{
	String member;
	uint32 m[4];
	uint32 n0, n1;
	MemberExprNode(String member) : UnaryExprNode(MEMBER), member(member), m{0u, 0u, 0u, 0u}, n0(0), n1(0) {}
	ExprValue operator()() const override 
	{ 
		ExprValue v = (*a)();
		if (v.n == 1 || v.n > 4 || v.n < n1)
			throw InvalidDimensionException();
		if (n0 == 1)
			return ExprValue((value)v.arr[m[0]]);
		ExprValue e((uint16)n0, (uint16)1);
		for (uint32 i = 0; i < n0; i++)
			e.arr[i] = v.arr[m[i]];
		return e;
	}
	int precedence() const override { return 3; }
	ExprNode *duplicate() const override { return 0; }//MemberExprNode *n = (MemberExprNode*)create(); DUPL(a); return n; }
	static ExprNode *create(String m) { return new MemberExprNode(m); }

	bool build(List<ExprNode*> &v)
	{
		v.pop_back();
		if (!v.size())
			return false;
		if (member.size() < 1 || member.size() > 4)
			return false;
		n0 = (uint32)member.size();
		for (uint32 i = 0; i < n0; i++) {
			Char c = member[i];
			uint32 n = -1;
			if (c == 'x') n = 0;
			else if (c == 'y') n = 1;
			else if (c == 'z') n = 2;
			else if (c == 'w') n = 3;
			if (n == -1)
				return false;
			m[i] = n;
			n1 = std::max(n1, n + 1);
		}
		a = v.back();
		if (!a->build(v))
			return false;
		return true;
	}
	String hlsl() const override 
	{ 
		static const String s[] = {MTEXT("x"), MTEXT("y"), MTEXT("z"), MTEXT("w")};
		return strUtils::ConstructString("(%1).%2%3%4%5").arg(a->hlsl()).arg(s[m[0]]).arg(n0 > 1 ? s[m[1]] : String()).arg(n0 > 2 ? s[m[2]] : String()).arg(n0 > 3 ? s[m[3]] : String());
	}
};

struct NegExprNode : public UnaryExprNode
{
	NegExprNode() : UnaryExprNode(NEGOPR) {}
	ExprValue operator()() const override { return -(*a)(); }
	int precedence() const override { return 3; }
	ExprNode *duplicate() const override { NegExprNode *n = (NegExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new NegExprNode(); }
	String hlsl() const override { return strUtils::ConstructString(MTEXT("-%1")).arg(a->hlsl()); }
};

struct PosExprNode : public UnaryExprNode
{
	PosExprNode() : UnaryExprNode(POSOPR) {}
	ExprValue operator()() const override { return (*a)(); }
	int precedence() const override { return 3; }
	ExprNode *duplicate() const override { PosExprNode *n = (PosExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new PosExprNode(); }
	String hlsl() const override { return a->hlsl(); }
};

struct NotExprNode : public UnaryExprNode
{
	NotExprNode() : UnaryExprNode(NOTOPR) {}
	ExprValue operator()() const override { return !(*a)(); }
	int precedence() const override { return 3; }
	ExprNode *duplicate() const override { NotExprNode *n = (NotExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new NotExprNode(); }
	String hlsl() const override { return strUtils::ConstructString(MTEXT("!%1")).arg(a->hlsl()); }
};

struct AddExprNode : public BinaryExprNode
{
	AddExprNode() : BinaryExprNode(ADDOPR) {}
	ExprValue operator()() const override { return (*a)() + (*b)(); }
	int precedence() const override { return 6; }
	ExprNode *duplicate() const override { AddExprNode *n = (AddExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new AddExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("(%1 + %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct SubExprNode : public BinaryExprNode
{
	SubExprNode() : BinaryExprNode(SUBOPR) {}
	ExprValue operator()() const override { return (*a)() - (*b)(); }
	int precedence() const override { return 6; }
	ExprNode *duplicate() const override { SubExprNode *n = (SubExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new SubExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("(%1 - %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct MulOprExprNode : public BinaryExprNode
{
	MulOprExprNode() : BinaryExprNode(MULOPR) {}
	ExprValue operator()() const override { return (*a)() * (*b)(); }
	int precedence() const override { return 5; }
	ExprNode *duplicate() const override { MulOprExprNode *n = (MulOprExprNode*)create(); DUPL(a); DUPL(b); return n; }
	static ExprNode *create() { return new MulOprExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("(%1 * %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct DivExprNode : public BinaryExprNode
{
	DivExprNode() : BinaryExprNode(DIVOPR) {}
	ExprValue operator()() const override { return (*a)() / (*b)(); }
	int precedence() const override { return 5; }
	ExprNode *duplicate() const override { DivExprNode *n = (DivExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new DivExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("(%1 / %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct PowOprExprNode : public BinaryExprNode
{
	PowOprExprNode() : BinaryExprNode(POWOPR) {}
	ExprValue operator()() const override { return (*a)() ^ (*b)(); }
	int precedence() const override { return 4; }
	ExprNode *duplicate() const override { PowOprExprNode *n = (PowOprExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new PowOprExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("pow(%1, %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct IntModExprNode : public BinaryExprNode
{
	IntModExprNode() : BinaryExprNode(INTMOD) {}
	ExprValue operator()() const override { return (*a)() % (*b)(); }// (value)(((int)(*a)()) % ((int)(*b)())); } // NB: casting to int can lead to problems with large floats!
	int precedence() const override { return 5; }
	ExprNode *duplicate() const override { IntModExprNode *n = (IntModExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new IntModExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("(%1 % %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct LessExprNode : public BinaryExprNode
{
	LessExprNode() : BinaryExprNode(LESSOPR) {}
	ExprValue operator()() const override { return (*a)() < (*b)(); }
	int precedence() const override { return 7; }
	ExprNode *duplicate() const override { LessExprNode *n = (LessExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new LessExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("(%1 < %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct LessEqExprNode : public BinaryExprNode
{
	LessEqExprNode() : BinaryExprNode(LESSEQOPR) {}
	ExprValue operator()() const override { return (*a)() <= (*b)(); }
	int precedence() const override { return 7; }
	ExprNode *duplicate() const override { LessEqExprNode *n = (LessEqExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new LessEqExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("(%1 <= %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct GreaterExprNode : public BinaryExprNode
{
	GreaterExprNode() : BinaryExprNode(GREATEROPR) {}
	ExprValue operator()() const override { return (*a)() > (*b)(); }
	int precedence() const override { return 7; }
	ExprNode *duplicate() const override { GreaterExprNode *n = (GreaterExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new GreaterExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("(%1 > %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct GreaterEqExprNode : public BinaryExprNode
{
	GreaterEqExprNode() : BinaryExprNode(GREATEREQOPR) {}
	ExprValue operator()() const override { return (*a)() >= (*b)(); }
	int precedence() const override { return 7; }
	ExprNode *duplicate() const override { GreaterEqExprNode *n = (GreaterEqExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new GreaterEqExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("(%1 >= %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct EqExprNode : public BinaryExprNode
{
	EqExprNode() : BinaryExprNode(EQUALOPR) {}
	ExprValue operator()() const override { return (*a)() == (*b)(); }
	int precedence() const override { return 8; }
	ExprNode *duplicate() const override { EqExprNode *n = (EqExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new EqExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("(%1 == %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct NotEqExprNode : public BinaryExprNode
{
	NotEqExprNode() : BinaryExprNode(NOTEQUALOPR) {}
	ExprValue operator()() const override { return (*a)() != (*b)(); }
	int precedence() const override { return 8; }
	ExprNode *duplicate() const override { NotEqExprNode *n = (NotEqExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new NotEqExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("(%1 != %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct AndExprNode : public BinaryExprNode
{
	AndExprNode() : BinaryExprNode(ANDOPR) {}
	ExprValue operator()() const override { return (*a)() && (*b)(); }
	int precedence() const override { return 9; }
	ExprNode *duplicate() const override { AndExprNode *n = (AndExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new AndExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("(%1 && %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct OrExprNode : public BinaryExprNode
{
	OrExprNode() : BinaryExprNode(OROPR) {}
	ExprValue operator()() const override { return (*a)() || (*b)(); }
	int precedence() const override { return 10; }
	ExprNode *duplicate() const override { OrExprNode *n = (OrExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new OrExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("(%1 || %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct MinExprNode : public BinaryExprNode
{
	MinExprNode() : BinaryExprNode(MIN) {}
	ExprValue operator()() const override { return ExprValue::min((*a)(), (*b)()); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { MinExprNode *n = (MinExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new MinExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("min(%1, %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct MaxExprNode : public BinaryExprNode
{
	MaxExprNode() : BinaryExprNode(MAX) {}
	ExprValue operator()() const override { return ExprValue::min((*a)(), (*b)()); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { MaxExprNode *n = (MaxExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new MaxExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("max(%1, %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct ClampExprNode : public TernaryExprNode
{
	ClampExprNode() : TernaryExprNode(CLAMP) {}
	ExprValue operator()() const override { return (*a)().clamp((*b)(), (*c)()); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { ClampExprNode *n = (ClampExprNode*)create(); DUPL(a); DUPL(b); DUPL(c); return n; } 
	static ExprNode *create() { return new ClampExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("clamp(%1, %2, %3)").arg(a->hlsl()).arg(b->hlsl()).arg(c->hlsl()); }
};

struct SaturateExprNode : public UnaryExprNode
{
	SaturateExprNode() : UnaryExprNode(SATURATE) {}
	ExprValue operator()() const override { return (*a)().saturate(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { SaturateExprNode *n = (SaturateExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new SaturateExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("saturate(%1)").arg(a->hlsl()); }
};

struct SinExprNode : public UnaryExprNode
{
	SinExprNode() : UnaryExprNode(SIN) {}
	ExprValue operator()() const override { return (*a)().sin(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { SinExprNode *n = (SinExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new SinExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("sin(%1)").arg(a->hlsl()); }
};

struct CosExprNode : public UnaryExprNode
{
	CosExprNode() : UnaryExprNode(COS) {}
	ExprValue operator()() const override { return (*a)().cos(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { CosExprNode *n = (CosExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new CosExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("cos(%1)").arg(a->hlsl()); }
};

struct TanExprNode : public UnaryExprNode
{
	TanExprNode() : UnaryExprNode(TAN) {}
	ExprValue operator()() const override { return (*a)().tan(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { TanExprNode *n = (TanExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new TanExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("tan(%1)").arg(a->hlsl()); }
};

struct aSinExprNode : public UnaryExprNode
{
	aSinExprNode() : UnaryExprNode(ASIN) {}
	ExprValue operator()() const override { return (*a)().asin(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { aSinExprNode *n = (aSinExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new aSinExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("asin(%1)").arg(a->hlsl()); }
};

struct aCosExprNode : public UnaryExprNode
{
	aCosExprNode() : UnaryExprNode(ACOS) {}
	ExprValue operator()() const override { return (*a)().acos(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { aCosExprNode *n = (aCosExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new aCosExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("acos(%1)").arg(a->hlsl()); }
};

struct aTanExprNode : public UnaryExprNode
{
	aTanExprNode() : UnaryExprNode(ATAN) {}
	ExprValue operator()() const override { return (*a)().atan(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { aTanExprNode *n = (aTanExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new aTanExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("atan(%1)").arg(a->hlsl()); }
};

struct AbsExprNode : public UnaryExprNode
{
	AbsExprNode() : UnaryExprNode(ABS) {}
	ExprValue operator()() const override { return (*a)().abs(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { AbsExprNode *n = (AbsExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new AbsExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("abs(%1)").arg(a->hlsl()); }
};

struct SqrtExprNode : public UnaryExprNode
{
	SqrtExprNode() : UnaryExprNode(SQRT) {}
	ExprValue operator()() const override { return (*a)().sqrt(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { SqrtExprNode *n = (SqrtExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new SqrtExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("sqrt(%1)").arg(a->hlsl()); }
};

struct FloorExprNode : public UnaryExprNode
{
	FloorExprNode() : UnaryExprNode(FLOOR) {}
	ExprValue operator()() const override { return (*a)().floor(); }  // NB: floor(-2.x) => -3  (int)-2.x => -2
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { FloorExprNode *n = (FloorExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new FloorExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("floor(%1)").arg(a->hlsl()); }
};

struct CeilExprNode : public UnaryExprNode
{
	CeilExprNode() : UnaryExprNode(CEIL) {}
	ExprValue operator()() const override { return (*a)().ceil(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { CeilExprNode *n = (CeilExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new CeilExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("ceil(%1)").arg(a->hlsl()); }
};

struct RoundExprNode : public UnaryExprNode
{
	RoundExprNode() : UnaryExprNode(ROUND) {}
	ExprValue operator()() const override { return (*a)().round(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { RoundExprNode *n = (RoundExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new RoundExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("round(%1)").arg(a->hlsl()); }
};

struct SinhExprNode : public UnaryExprNode
{
	SinhExprNode() : UnaryExprNode(SINH) {}
	ExprValue operator()() const override { return (*a)().sinh(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { SinhExprNode *n = (SinhExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new SinhExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("sinh(%1)").arg(a->hlsl()); }
};

struct CoshExprNode : public UnaryExprNode
{
	CoshExprNode() : UnaryExprNode(COSH) {}
	ExprValue operator()() const override { return (*a)().cosh(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { CoshExprNode *n = (CoshExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new CoshExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("cosh(%1)").arg(a->hlsl()); }
};

struct TanhExprNode : public UnaryExprNode
{
	TanhExprNode() : UnaryExprNode(TANH) {}
	ExprValue operator()() const override { return (*a)().tanh(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { TanhExprNode *n = (TanhExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new TanhExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("tanh(%1)").arg(a->hlsl()); }
};

struct LogExprNode : public UnaryExprNode
{
	LogExprNode() : UnaryExprNode(LOG) {}
	ExprValue operator()() const override { return (*a)().log(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { LogExprNode *n = (LogExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new LogExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("log10(%1)").arg(a->hlsl()); }
};

struct LnExprNode : public UnaryExprNode
{
	LnExprNode() : UnaryExprNode(LN) {}
	ExprValue operator()() const override { return (*a)().ln(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { LnExprNode *n = (LnExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new LnExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("log(%1)").arg(a->hlsl()); }
};

struct ExpExprNode : public UnaryExprNode
{
	ExpExprNode() : UnaryExprNode(EXP) {}
	ExprValue operator()() const override { return (*a)().exp(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { ExpExprNode *n = (ExpExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new ExpExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("exp(%1)").arg(a->hlsl()); }
};

struct PowExprNode : public BinaryExprNode
{
	PowExprNode() : BinaryExprNode(POW) {}
	ExprValue operator()() const override { return (*a)() ^ (*b)(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { PowExprNode *n = (PowExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new PowExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("pow(%1, %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct FmodExprNode : public BinaryExprNode
{
	FmodExprNode() : BinaryExprNode(FMOD) {}
	ExprValue operator()() const override { return (*a)().fmod((*b)()); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { FmodExprNode *n = (FmodExprNode*)create(); DUPL(a); DUPL(b); return n; } 
	static ExprNode *create() { return new FmodExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("fmod(%1, %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct FracExprNode : public UnaryExprNode
{
	FracExprNode() : UnaryExprNode(FRAC) {}
	ExprValue operator()() const override { return (*a)().frac(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { FracExprNode *n = (FracExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new FracExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("frac(%1)").arg(a->hlsl()); }
};

struct LerpExprNode : public TernaryExprNode
{
	LerpExprNode() : TernaryExprNode(LERP) {}
	ExprValue operator()() const override { return ExprValue::lerp((*a)(), (*b)(), (*c)()); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { LerpExprNode *n = (LerpExprNode*)create(); DUPL(a); DUPL(b); DUPL(c); return n; } 
	static ExprNode *create() { return new LerpExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("lerp(%1, %2, %3)").arg(a->hlsl()).arg(b->hlsl()).arg(c->hlsl()); }
};

struct SignExprNode : public UnaryExprNode
{
	SignExprNode() : UnaryExprNode(SIGN) {}
	ExprValue operator()() const override { return (*a)().sign(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { SignExprNode *n = (SignExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new SignExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("sign(%1)").arg(a->hlsl()); }
};

struct RadExprNode : public UnaryExprNode
{
	RadExprNode() : UnaryExprNode(RAD) {}
	ExprValue operator()() const override { return (*a)().radians(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { RadExprNode *n = (RadExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new RadExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("radians(%1)").arg(a->hlsl()); }
};

struct DegExprNode : public UnaryExprNode
{
	DegExprNode() : UnaryExprNode(DEG) {}
	ExprValue operator()() const override { return (*a)().degrees(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { DegExprNode *n = (DegExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new DegExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("degrees(%1)").arg(a->hlsl()); }
};

struct IsNaNExprNode : public UnaryExprNode
{
	IsNaNExprNode() : UnaryExprNode(ISNAN) {}
	ExprValue operator()() const override { return (*a)().isnan(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { IsNaNExprNode *n = (IsNaNExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new IsNaNExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("isnan(%1)").arg(a->hlsl()); }
};

struct IsInfExprNode : public UnaryExprNode
{
	IsInfExprNode() : UnaryExprNode(ISINF) {}
	ExprValue operator()() const override { return (*a)().isinf(); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { IsInfExprNode *n = (IsInfExprNode*)create(); DUPL(a); return n; } 
	static ExprNode *create() { return new IsInfExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("isinf(%1)").arg(a->hlsl()); }
};

struct RandExprNode : public OperatorExprNode
{
	RandExprNode() : OperatorExprNode(RAND) {}
	ExprValue operator()() const override { return ExprValue((value)rand() / (value)RAND_MAX); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { RandExprNode *n = (RandExprNode*)create(); return n; } 
	static ExprNode *create() { return new RandExprNode(); }
	String hlsl() const override { return MTEXT("0.0"); }
};

struct PiExprNode : public OperandExprNode
{
	PiExprNode() : OperandExprNode(PI) {}
	ExprValue operator()() const override { return ExprValue(_PI); }
	ExprNode *duplicate() const override { PiExprNode *n = (PiExprNode*)create(); return n; } 
	static ExprNode *create() { return new PiExprNode(); }
	String hlsl() const override { return strUtils::fromNum(_PI); }
};

struct IfThenElseExprNode : public TernaryExprNode
{
	IfThenElseExprNode() : TernaryExprNode(IFTHENELSE) {}
	ExprValue operator()() const override { return (*a)().toBool() ? (*b)() : (*c)(); }
	int precedence() const override { return 11; }
	ExprNode *duplicate() const override { IfThenElseExprNode *n = (IfThenElseExprNode*)create(); DUPL(a); DUPL(b); DUPL(c); return n; } 
	static ExprNode *create() { return new IfThenElseExprNode(); }
	String hlsl() const override { return strUtils::ConstructString(MTEXT("%1 ? %2 : %3")).arg(a->hlsl()).arg(b->hlsl()).arg(c->hlsl()); }
};

struct ConstExprNode : public OperandExprNode
{
	value f;
	ConstExprNode(value f) : OperandExprNode(SCALAR), f(f) {}
	ExprValue operator()() const override { return ExprValue(f); }
	ExprNode *duplicate() const override { ConstExprNode *n = (ConstExprNode*)create(f); return n; }
	static ExprNode *create(value f) { return new ConstExprNode(f); }
	String hlsl() const override { return strUtils::fromNum(f); }
};

struct CallbackExprNode : public OperandExprNode
{
	uint32 index;
	ExpressionVariableCallback *cb;
	CallbackExprNode(uint32 index, ExpressionVariableCallback *cb) : OperandExprNode(CHILD), index(index), cb(cb) {}
	ExprValue operator()() const override { return cb->GetValue(index); }
	ExprNode *duplicate() const override { CallbackExprNode *n = (CallbackExprNode*)create(index, cb); return n; } 
	static ExprNode *create(uint32 index, ExpressionVariableCallback *cb) { return new CallbackExprNode(index, cb); }
	String hlsl() const override { return strUtils::ConstructString(MTEXT("§%1")).arg(String(1, 'a' + index)); }
};

struct DotExprNode : public BinaryExprNode
{
	DotExprNode() : BinaryExprNode(DOT) {}
	ExprValue operator()() const override { return (*a)().dot((*b)()); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { DotExprNode *n = (DotExprNode*)create(); DUPL(a); DUPL(b); return n; }
	static ExprNode *create() { return new DotExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("dot(%1, %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

struct MulExprNode : public BinaryExprNode
{
	MulExprNode() : BinaryExprNode(MUL) {}
	ExprValue operator()() const override { return (*a)().mul((*b)()); }
	int precedence() const override { return 2; }
	ExprNode *duplicate() const override { MulExprNode *n = (MulExprNode*)create(); DUPL(a); DUPL(b); return n; }
	static ExprNode *create() { return new MulExprNode(); }
	String hlsl() const override { return strUtils::ConstructString("mul(%1, %2)").arg(a->hlsl()).arg(b->hlsl()); }
};

}
