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


#include "M3DCore/MString.h"
#include "M3DCore/Containers.h"
#include "ValueDef.h"
#include <memory>

namespace m3d
{
namespace expr
{

struct ExprNode;
struct OperandNode;
class ExpressionCallback;

struct ExprValue
{
	enum Type { INVALID, VALUE, VECTOR, MATRIX };
	Type type;
	union
	{
		float64 val;
		XMFLOAT4 vec;
		XMFLOAT4X4 mat;
	};
	ExprValue() : type(INVALID) {}
	ExprValue(float64 val) : type(VALUE), val(val) {}
	ExprValue(XMFLOAT4 vec) : type(VECTOR), vec(vec) {}
	ExprValue(XMFLOAT4X4 mat) : type(MATRIX), mat(mat) {}

};

class ExpressionParser
{
public:
	ExpressionParser();
	~ExpressionParser();

	ExpressionParser &operator=(const ExpressionParser &rhs);

	void setCallback(ExpressionCallback *cb) { _callback = cb; }

	bool parse(const String &expression, String *msg = nullptr);

	ExprValue calculate() const;

	const ExprNode *rootNode() const { return _exp.get(); }

	bool valid() const { return _exp != nullptr; }

protected:
	std::unique_ptr<ExprNode> _exp;

	ExpressionCallback *_callback;
};

}
}