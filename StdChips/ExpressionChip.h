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

#include "ExpressionParser2.h"
#include "ExpressionParserTree.h"
#include "Value.h"
#include "VectorChip.h"
#include "MatrixChip.h"
#include "M3DEngine/Engine.h"


namespace m3d
{


class STDCHIPS_API ExpressionChip : public expr::ExpressionCallback
{
public:
	ExpressionChip(Numeric *chip) : numeric(chip)
	{
		_parser = mmnew expr::ExpressionParser();
		_parser->setCallback(this);
	}

	~ExpressionChip()
	{
		mmdelete(_parser);
	}

	String GetExpression() const { return _expression; }
	bool SetExpression(String expression, String *msg = nullptr) 
	{
//		expr::ExpressionParser pp;
//		pp.parse(expression);
		_expression = expression;
		if (_parser->parse(_expression, msg)) {
			numeric->RemoveMessage(Numeric::InvalidExpressionException());
			return true;
		}
		return false;
	}

	expr::ExpressionParser *GetParser() { return _parser; }

	Numeric *GetChip() const { return numeric; }

protected:
	expr::ExpressionParser *_parser;

	String _expression;

	void Reset()
	{
		for (uint32 i = 0; i < 28; i++)
			_v[i].valid = false;
	}

private:
	Numeric *numeric;
	struct Cache
	{
		bool valid;
		uint32 type;
		union
		{
			value val;
			XMFLOAT4 vec;
			XMFLOAT4X4 mat;
		};
	};
	Cache _v[28];

	uint32 Register(const String &s) override
	{
		static const Char *a[] = { MTEXT("a"), MTEXT("b"), MTEXT("c"), MTEXT("d"), MTEXT("e"), MTEXT("f"), MTEXT("g"), MTEXT("h"), MTEXT("i"), MTEXT("j"), MTEXT("k"), MTEXT("l"), MTEXT("m"), MTEXT("n"), MTEXT("o"), MTEXT("p"), MTEXT("q"), MTEXT("r"), MTEXT("s"), MTEXT("t"), MTEXT("u"), MTEXT("v"), MTEXT("w"), MTEXT("x"), MTEXT("y"), MTEXT("z"), MTEXT("old"), MTEXT("dt") };
		for (uint32 index = 0; index < 28; index++)
			if (s == a[index])
				return index;
		return -1;
	}

	expr::OperandNode *GetOperand(uint32 index) override
	{
		Cache &c = _v[index];
		if (!c.valid) {
			c.valid = true;
			c.type = 0;
			if (index < 26) {
				ChildPtr<Numeric> ch = numeric->GetChild(0, index);
				if (ch) {
					if (ch->AsValue()) {
						c.type = 1;
						c.val = ch->AsValue()->GetValue();
					}
					else if (ch->AsVector()) {
						c.type = 2;
						c.vec = ch->AsVector()->GetVector();
					}
					else if (ch->AsMatrix()) {
						c.type = 3;
						c.mat = ch->AsMatrix()->GetMatrix();
					}
					else {} // throw not supported...
				}
				else
					throw Chip::MissingChildException(numeric, index);
			}
			else if (index == 26) {
				if (numeric->AsValue()) {
					c.type = 1;
					c.val = numeric->AsValue()->GetChipValue();
				}
				else if (numeric->AsVector()) {
					c.type = 2;
					c.vec = numeric->AsVector()->GetChipVector();
				}
				else if (numeric->AsMatrix()) {
					c.type = 3;
					c.mat = numeric->AsMatrix()->GetChipMatrix();
				}
				else {} // should not happend!
			}
			else if (index == 27) {
				c.type = 1;
				c.val = (value)engine->GetDt() / 1000000.0;
			}
			else {} // should not happend!
		}
			
		switch (c.type)
		{
		case 0: throw Chip::MissingChildException(numeric, index); // noe annet...
		case 1: return (expr::OperandNode*)expr::ValueNode::create(c.val);
		case 2: return (expr::OperandNode*)expr::Vector4Node::create(c.vec);
		case 3: return (expr::OperandNode*)expr::Matrix44Node::create(c.mat);
		}

		return nullptr;
	}
};


}