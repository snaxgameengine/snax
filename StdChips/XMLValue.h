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


static const Guid XMLVALUE_GUID = { 0x03def144, 0x4645, 0x4a1b, { 0xa6, 0xa6, 0xdf, 0x9d, 0xd3, 0x0b, 0xf6, 0xe7 } };


class STDCHIPS_API XMLValue : public Value
{
	CHIPDESC_DECL;
public:
	enum class OperatorType { 
		NONE, 
		IS_ELEMENT,
		IS_ROOT,
		CHILD_COUNT,
		GET_ATTRIBUTE_COUNT,
		HAS_ATTRIBUTE,
		CONTENT,
		ATTRIBUTE,
		QUERY
	};

	XMLValue();
	virtual ~XMLValue();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual value GetValue() override;
	virtual void SetValue(value v) override;

	virtual OperatorType GetOperatorType() const { return _ot; }
	virtual void SetOperatorType(OperatorType ot);

protected:
	OperatorType _ot;
};


}