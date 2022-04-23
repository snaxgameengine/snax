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
#include "ExpressionValue.h"
#include "M3DEngine/DocumentSaver.h"
#include "M3DEngine/DocumentLoader.h"

using namespace m3d;

//abs() acos() asin() atan() ceil() clamp() cos() cosh() deg() exp() floor() fmod() frac() isinf() isnan() lerp() ln() log() max() min() pow() rad() rand() round() saturate() sign() sin() sinh() sqrt() tan() tanh()

CHIPDESCV1_DEF(ExpressionValue, MTEXT("Expression Value"), EXPRESSIONVALUE_GUID, VALUE_GUID);



ExpressionValue::ExpressionValue() : ExpressionChip(this)
{
	CREATE_CHILD(0, VALUE_GUID, true, UP, MTEXT("Operands"));
}

ExpressionValue::~ExpressionValue()
{
}

bool ExpressionValue::CopyChip(Chip *chip)
{
	ExpressionValue *c = dynamic_cast<ExpressionValue*>(chip);
	B_RETURN(Value::CopyChip(c));
	*_parser = *c->_parser;
	_parser->setCallback(this);
	_expression = c->_expression;
	return true;
}

bool ExpressionValue::LoadChip(DocumentLoader &loader) 
{ 
	B_RETURN(Value::LoadChip(loader));
	LOAD(MTEXT("expression|exp"), _expression);
	SetExpression(_expression);
	return true;
}

bool ExpressionValue::SaveChip(DocumentSaver &saver) const 
{
	B_RETURN(Value::SaveChip(saver));
	SAVE(MTEXT("expression"), _expression);
	return true;
}

value ExpressionValue::GetValue()
{
	RefreshT refresh(Refresh);
	if (refresh) {
		if (!_parser->valid()) {
			AddMessage(InvalidExpressionException());
			return _value = 0.0f;
		}

		Reset();

		try {
			expr::ExprValue v = _parser->calculate();
			if (v.type == expr::ExprValue::VALUE)
				_value = v.val;
			else
				throw expr::InvalidDimensionException();
			ClearMessages();
		}
		catch (expr::InvalidDimensionException)
		{
			AddMessage(UnexpectedDimensionException());
			_value = 0.0f;
		}
		catch (expr::ExprNotImplException)
		{
			AddMessage(UnexpectedDimensionException());
			_value = 0.0f;
		}
		catch (MissingChildException e)
		{
			AddMessage(e);
			_value = 0.0f;
		}
	}
	return _value;
}

