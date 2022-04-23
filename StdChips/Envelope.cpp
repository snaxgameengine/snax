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
#include "Envelope.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"

using namespace m3d;


CHIPDESCV1_DEF(Envelope, MTEXT("Envelope"), ENVELOPE_GUID, VALUE_GUID);



Envelope::Envelope()
{
}

Envelope::~Envelope()
{
}

bool Envelope::CopyChip(Chip *chip)
{
	Envelope *c = dynamic_cast<Envelope*>(chip);
	B_RETURN(Value::CopyChip(c));
	_spline = c->_spline;
	return true;
}

bool Envelope::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Value::LoadChip(loader));
	LOAD(MTEXT("spline"), _spline);
	return true;
}

bool Envelope::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Value::SaveChip(saver));
	SAVE(MTEXT("spline"), _spline);
	return true;
}

value Envelope::GetValue()
{
	ChildPtr<Value> ch0 = GetChild(0);
	value v = ch0 ? ch0->GetValue() : 0.0;
	return _value = GetValue(v);
}

value Envelope::GetDerivative()
{
	ChildPtr<Value> ch0 = GetChild(0);
	value v = ch0 ? ch0->GetValue() : 0.0;
	return _value = GetDerivative(v);
}

void Envelope::SetValue(value v)
{
	// TODO: Improve!
	ChildPtr<Value> ch0 = GetChild(0);
	if (ch0) {
		value w = ch0->GetValue();
		_spline.AddControlPoint(ValueTCBSpline::ControlPoint(w, v));
	}
}

value Envelope::GetValue(value x)
{
	return _spline.Evaluate(x);
}

value Envelope::GetDerivative(value x)
{
	return _spline.EvaluateDerivative(x);
}
