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
#include "Value.h"


namespace m3d
{


static const Guid VALUEOPERATOR_GUID = { 0x9c9fe924, 0xe3db, 0x40f3, { 0xa1, 0x1c, 0xc7, 0x5a, 0x3b, 0x14, 0xd2, 0x61 } };

class STDCHIPS_API ValueOperator : public Value
{
	CHIPDESC_DECL;
public:
	enum class OperatorType { 
		NONE, 
		DT, 
		FPS,
		GET_UPDATE_STAMP,
		GET_CMD_LINE_ARGUMENT_COUNT = 50,
		GET_RUNTIME_ENV,
		VECTOR_X = 100, 
		VECTOR_Y, 
		VECTOR_Z, 
		VECTOR_W, 
		VECTOR_LENGTH2 = 110, 
		VECTOR_LENGTH3, 
		VECTOR_LENGTH4,
		VECTOR_LENGTHSQ2, 
		VECTOR_LENGTHSQ3, 
		VECTOR_LENGTHSQ4, 
		VECTOR_DOT2 = 120, 
		VECTOR_DOT3, 
		VECTOR_DOT4, 
//		VECTOR_DIST2, VECTOR_DIST3, VECTOR_DIST4, 
//		VECTOR_DISTSQ2, VECTOR_DISTSQ3, VECTOR_DISTSQ4,
		LIMIT_RELATIVE = 200, 
		LIMIT_ABS, 
		LOOP_RELATIVE,
		ENVELOPE_GET_DERIVATIVE,
		TEXT_LENGTH = 400, 
		TEXT_COMPARE, 
		TEXT_VALUE,
		INSTANCEREF_EXIST = 500,
		INSTANCEREF_EQUALS,
		INSTANCEREF_TYPE_OF,
		INSTANCEREF_TYPE_COMPATIBLE,
		INSTANCEREF_IS_OWNER,
		INSTANCEREF_GET_ID,
		VALUESET_KEY_EXIST = 600,
		TEXTSET_KEY_EXIST,
		UTC_TIME = 650,
		UTC_TO_SECONDS_SINCE_EPOCH,
		LOCAL_TO_SECONDS_SINCE_EPOCH
	};

	ValueOperator();
	virtual ~ValueOperator();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	value GetValue() override;
	void SetValue(value v) override;

	virtual OperatorType GetOperatorType() const { return _ot; }
	virtual void SetOperatorType(OperatorType ot);

protected:
	OperatorType _ot;
};



}