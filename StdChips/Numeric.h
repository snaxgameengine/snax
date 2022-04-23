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
#include "ValueDef.h"

namespace m3d
{

static const Guid NUMERIC_GUID = { 0x9478e85d, 0x274a, 0x4793,{ 0xb9, 0xcf, 0x9d, 0xd6, 0x8f, 0x40, 0x22, 0x8d } };

class Value;
class VectorChip;
class MatrixChip;

class STDCHIPS_API Numeric : public Chip
{
	CHIPDESC_DECL;
	CHIPMSG(InvalidExpressionException, FATAL, MTEXT("The given expression is not valid!"))
	CHIPMSG(UnexpectedDimensionException, FATAL, MTEXT("An operand is of an unexepted dimension!"))
public:
	Numeric();
	~Numeric();

	virtual Value *AsValue() { return nullptr; }
	virtual VectorChip *AsVector() { return nullptr; }
	virtual MatrixChip *AsMatrix() { return nullptr; }
};



}