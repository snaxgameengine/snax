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

#include "M3DCore/Containers.h"


namespace m3d
{

// how close two control points can be set...
#define MIN_DELTA 1.0e-4


// This can't be a subclass of TCBSpline because of problems with serialization (It can't find correct (De)SerializeDocumentData functions...)
template<typename T, typename S>
struct TCBSplineControlPoint
{
	enum Type { STEP, LINEAR, TCB };
	Type type;
	S x;
	T y;
	S t;
	S c;
	S b;
	T it;
	T ot;

	TCBSplineControlPoint() {} // Default contructor for serialization purposes...
	TCBSplineControlPoint(S x, T y, Type type = TCB, S t = 0.0, S c = 0.0, S b = 0.0) : type(type), x(x), y(y), t(t), c(c), b(b), it(T()), ot(T()) {}

	void CalculateTangents(const TCBSplineControlPoint &prev, const TCBSplineControlPoint &next)
	{
		ot = it = T();
		if (type == STEP)
			return;
		if (this != &prev) { // not first point
			ot += (y - prev.y) * ((1.0 - t) * (1.0 + b) * (1.0 + c) * 0.5 / (x - prev.x));
			it += (y - prev.y) * ((1.0 - t) * (1.0 + b) * (1.0 - c) * 0.5 / (x - prev.x));
		}
		if (this != &next) { // not last point
			ot += (next.y - y) * ((1.0 - t) * (1.0 - b) * (1.0 - c) * 0.5 / (next.x - x));
			it += (next.y - y) * ((1.0 - t) * (1.0 - b) * (1.0 + c) * 0.5 / (next.x - x));
			if (this == &prev) { // first point
				ot *= 2.0;
				it *= 0.0;
			}
		}
		else { // last point
			ot *= 0.0;
			it *= 2.0;
		}
	}

	T Evaluate(S xx, const TCBSplineControlPoint &next) const
	{
		switch (type) 
		{
		case STEP:
			return y;
		case LINEAR:
			{
				S t = (xx - x) / (next.x - x);
				return y * (1.0 - t) + next.y * t;
			}
		case TCB:
			{
				S d = next.x - x, t = (xx - x) / d, tt = t * t, ttt = tt * t;
				return y * (2.0 * ttt - 3.0 * tt + 1.0) + ot * ((ttt - 2.0 * tt + t) * d) + next.y * (3.0 * tt - 2.0 * ttt) + next.it * ((ttt - tt) * d);
			}
		}
		return T();
	}

	T EvaluateDerivative(S xx, const TCBSplineControlPoint &next) const
	{
		switch (type) 
		{
		case STEP:
			return T();
		case LINEAR:
			return (next.y - y) * (1.0 / (next.x - x));
		case TCB:
			{
				S d = 1.0 / (next.x - x),  t = (xx - x) * d, tt = t * t;
				return y * (6.0 * (tt - t) * d) + ot * (3.0 * tt - 4.0 * t + 1.0) + next.y * (6.0 * (t - tt) * d) + next.it * (3.0 * tt - 2.0 * t);
			}
		}
		return T();
	}
};


// TODO: Code faster methods for finding control points to interpolate (eg Divide&Conquer)
template<typename T, typename S>
class TCBSpline
{
public:
	typedef TCBSplineControlPoint<T,S> ControlPoint;
	typedef List<ControlPoint> ControlPointList;

private:
	ControlPointList _controlPoints;

	void _calculateTangents(size_t i)
	{
		_controlPoints[i].CalculateTangents(i > 0 ? _controlPoints[i - 1] : _controlPoints[i], i < _controlPoints.size() - 1 ? _controlPoints[i + 1] : _controlPoints[i]);
		if (i > 1) _controlPoints[i - 1].CalculateTangents(_controlPoints[i - 2], _controlPoints[i]);
		else if (i > 0) _controlPoints[i - 1].CalculateTangents(_controlPoints[i - 1], _controlPoints[i]);
		if (i + 2 < _controlPoints.size()) _controlPoints[i + 1].CalculateTangents(_controlPoints[i], _controlPoints[i + 2]);
		else if (i + 1 < _controlPoints.size()) _controlPoints[i + 1].CalculateTangents(_controlPoints[i], _controlPoints[i + 1]);
	}

public:
	TCBSpline() {}
	~TCBSpline() {}

	const ControlPointList &GetControlPoints() const { return _controlPoints; }

	size_t AddControlPoint(const ControlPoint &cp) 
	{
		size_t i = 0;
		for (; i < _controlPoints.size(); i++)
			if (cp.x < _controlPoints[i].x)
				break;
		if (i < _controlPoints.size() && _controlPoints[i].x - cp.x < MIN_DELTA || i > 0 && cp.x - _controlPoints[i - 1].x < MIN_DELTA)
			return -1; // duplicate x
		if (i == _controlPoints.size())
			_controlPoints.push_back(cp); 
		else
			_controlPoints.insert(_controlPoints.begin() + i, cp);
		_calculateTangents(i);
		return i; // returns index of new control point
	}
	
	void UpdateControlPoint(size_t i, const ControlPoint &cp) 
	{ 
		ControlPoint &c = _controlPoints[i];
		c = cp;
		if (i > 0)
			c.x = std::max(_controlPoints[i - 1].x + MIN_DELTA, c.x);
		if (i < _controlPoints.size() - 1)
			c.x = std::min(_controlPoints[i + 1].x - MIN_DELTA, c.x);
		_calculateTangents(i);
	}

	void RemoveControlPoint(size_t i) 
	{ 
		_controlPoints.erase(_controlPoints.begin() + i);
		_calculateTangents(i);
	}

	void Clear() 
	{
		_controlPoints.clear(); 
	}

	T Evaluate(S x) const 
	{ 
		if (_controlPoints.empty())
			return T();
		if (x <= _controlPoints.front().x)
			return _controlPoints.front().y;
		for (uint32 i = 0; i < _controlPoints.size() - 1; i++)
			if (_controlPoints[i + 1].x > x)
				return _controlPoints[i].Evaluate(x, _controlPoints[i + 1]);
		return _controlPoints.back().y;
	}

	T EvaluateDerivative(S x) const
	{ 
		if (_controlPoints.empty() || x < _controlPoints.front().x)
			return T();
		for (uint32 i = 0; i < _controlPoints.size() - 1; i++)
			if (_controlPoints[i + 1].x > x)
				return _controlPoints[i].EvaluateDerivative(x, _controlPoints[i + 1]);
		return T();
	}
};

}