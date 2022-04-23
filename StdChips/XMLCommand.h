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

namespace m3d
{


static const Guid XMLCOMMAND_GUID = { 0x1da1fd4c, 0xe7e4, 0x4c3d, { 0x8d, 0x26, 0xe7, 0xae, 0xb1, 0xe2, 0x6f, 0x72 } };


class STDCHIPS_API XMLCommand : public Chip
{
	CHIPDESC_DECL;
public:
	enum OperatorType { 
		NONE, 
		NEW_XML,
		LOAD_XML_FROM_FILE, 
		LOAD_XML_FROM_TEXT, 
		SAVE_XML_TO_FILE, 
		SAVE_XML_TO_TEXT,
		ADD_CHILD_ELEMENT,
		REMOVE_ELEMENT,
		REMOVE_ATTRIBUTE,
		GET_PARENT,
		GET_FIRST_CHILD,
		GET_NEXT_SIBLING,
		QUERY_NODE, 
		QUERY_TEXT, 
		QUERY_VALUE
	};

	XMLCommand();
	virtual ~XMLCommand();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual void CallChip() override;

	virtual OperatorType GetOperatorType() const { return _ot; }
	virtual void SetOperatorType(OperatorType ot);

protected:
	OperatorType _ot;
};



}