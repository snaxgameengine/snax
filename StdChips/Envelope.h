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
#include "M3DEngine/Chip.h"
#include "Value.h"
#include "TCBSpline.h"
#include "M3DEngine/DocumentLoader.h"
#include "M3DEngine/DocumentSaver.h"

namespace m3d
{


// TODO: generic serialization functions should be moved....
template<typename T, typename S>
bool SerializeDocumentData(DocumentSaver &saver, const TCBSplineControlPoint<T, S> &data) 
{ 
	SAVE(MTEXT("type"), (uint32)data.type);
	SAVE(MTEXT("x"), data.x);
	SAVE(MTEXT("y"), data.y);
	SAVE(MTEXT("t"), data.t);
	SAVE(MTEXT("c"), data.c);
	SAVE(MTEXT("b"), data.b);
	return true; 
}

template<typename T, typename S>
bool DeserializeDocumentData(DocumentLoader &loader, TCBSplineControlPoint<T, S> &data) 
{ 
	LOAD(MTEXT("type"), (uint32&)data.type);
	LOAD(MTEXT("x"), data.x);
	LOAD(MTEXT("y"), data.y);
	LOAD(MTEXT("t"), data.t);
	LOAD(MTEXT("c"), data.c);
	LOAD(MTEXT("b"), data.b);
	return true; 
}

template<typename T, typename S>
bool SerializeDocumentData(DocumentSaver &saver, const TCBSpline<T, S> &data)
{
	SAVE(MTEXT("ControlPointList"), data.GetControlPoints());
	return true;
}

template<typename T, typename S>
bool DeserializeDocumentData(DocumentLoader &loader, TCBSpline<T, S> &data)
{
	typename TCBSpline<T, S>::ControlPointList cpl;
	LOAD(MTEXT("ControlPointList"), cpl);
	for (size_t i = 0; i < cpl.size(); i++) {
		B_RETURN(data.AddControlPoint(cpl[i]) != -1);
	}
	return true;
}



static const Guid ENVELOPE_GUID = { 0xc60ba548, 0x7017, 0x41e0, { 0x88, 0x92, 0xf2, 0xc3, 0xfb, 0x94, 0xb2, 0xb3 } };


typedef TCBSpline<value, value> ValueTCBSpline;
template class STDCHIPS_API TCBSpline<value, value>;


class STDCHIPS_API Envelope : public Value
{
	CHIPDESC_DECL;
public:
	Envelope();
	virtual ~Envelope();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual value GetValue() override;
	virtual void SetValue(value v) override;
	virtual value GetDerivative();

	virtual value GetValue(value x);
	virtual value GetDerivative(value x);

	// Sets the spline-curve
	virtual void SetSpline(const ValueTCBSpline &spline) { _spline = spline; }
	// Gets the spline-curve. non-const to enable edit without setting.
	virtual ValueTCBSpline &GetSpline() { return _spline; }

protected:
	ValueTCBSpline _spline;
};



}