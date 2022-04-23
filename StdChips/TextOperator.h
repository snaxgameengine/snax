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
#include "Text.h"


namespace m3d
{


static const Guid TEXTOPERATOR_GUID = { 0xac3e6b23, 0xc482, 0x4848, { 0xb4, 0x84, 0x5a, 0xb4, 0xc7, 0xbf, 0x77, 0x38 } };

class STDCHIPS_API TextOperator : public Text
{
	CHIPDESC_DECL;
public:
	enum class OperatorType 
	{ 
		NONE, 
		SUBTEXT,
		MERGE_TEXT,
		REPLACE_TEXT,
		UPPERCASE,
		LOWERCASE,
		VALUE_TO_STRING,
		GET_COMPUTER_NAME = 50,
		XET_ENVIRONMENT_VARIABLE,
		GET_CMD_LINE_ARGUMENT,
		XET_REGISTRY_VALUE,
		FILESYSTEM_GET_PROJECT_START_FILE = 100,
		FILESYSTEM_GET_EXE_FILE,
		FILESYSTEM_GET_DIR_FROM_FILE_PATH,
		FILESYSTEM_GET_FILE_NAME,
		FILESYSTEM_GET_FILE_NAME_EXCL_EXT,
		FILESYSTEM_GET_FILE_EXT,
		FILESYSTEM_GET_ABSOLUTE_FILE_PATH,
		XET_INSTANCE_NAME = 150
	};

	TextOperator();
	virtual ~TextOperator();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual String GetText() override;
	virtual void SetText(String text) override;

	virtual OperatorType GetOperatorType() const { return _ot; }
	virtual void SetOperatorType(OperatorType ot);

protected:
	OperatorType _ot;
};



}