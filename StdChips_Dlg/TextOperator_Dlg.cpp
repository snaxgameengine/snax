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

#include "StdAfx.h"
#include "TextOperator_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(TextOperator_Dlg, TEXTOPERATOR_GUID);

void TextOperator_Dlg::Init()
{

	RPtr vectoro = AddItem(MTEXT("Text Operations"), (uint32)TextOperator::OperatorType::NONE);
	AddItem(vectoro, MTEXT("Subtext"), (uint32)TextOperator::OperatorType::SUBTEXT);
	AddItem(vectoro, MTEXT("Merge Texts"), (uint32)TextOperator::OperatorType::MERGE_TEXT);
	AddItem(vectoro, MTEXT("Replace Text"), (uint32)TextOperator::OperatorType::REPLACE_TEXT);
	AddItem(vectoro, MTEXT("To Uppercase"), (uint32)TextOperator::OperatorType::UPPERCASE);
	AddItem(vectoro, MTEXT("To Lowercase"), (uint32)TextOperator::OperatorType::LOWERCASE);
	AddItem(vectoro, MTEXT("From Value"), (uint32)TextOperator::OperatorType::VALUE_TO_STRING);

	RPtr sysop = AddItem(MTEXT("System Operations"), (uint32)TextOperator::OperatorType::NONE);
	AddItem(sysop, MTEXT("Get Computer Name"), (uint32)TextOperator::OperatorType::GET_COMPUTER_NAME);
	AddItem(sysop, MTEXT("Get/Set Environment Variable"), (uint32)TextOperator::OperatorType::XET_ENVIRONMENT_VARIABLE);
	AddItem(sysop, MTEXT("Get Command Line Argument"), (uint32)TextOperator::OperatorType::GET_CMD_LINE_ARGUMENT);
	AddItem(sysop, MTEXT("Get/Set Registry Value"), (uint32)TextOperator::OperatorType::XET_REGISTRY_VALUE);
	AddItem(sysop, MTEXT("Get/Set Instance Name"), (uint32)TextOperator::OperatorType::XET_INSTANCE_NAME);

	RPtr files = AddItem(MTEXT("File System"), (uint32)TextOperator::OperatorType::NONE);
	AddItem(files, MTEXT("Get Project Start File"), (uint32)TextOperator::OperatorType::FILESYSTEM_GET_PROJECT_START_FILE);
	AddItem(files, MTEXT("Get Executable File Name"), (uint32)TextOperator::OperatorType::FILESYSTEM_GET_EXE_FILE);
	AddItem(files, MTEXT("Get Directory From File Path"), (uint32)TextOperator::OperatorType::FILESYSTEM_GET_DIR_FROM_FILE_PATH);
	AddItem(files, MTEXT("Get File Name"), (uint32)TextOperator::OperatorType::FILESYSTEM_GET_FILE_NAME);
	AddItem(files, MTEXT("Get File Name Without Extension"), (uint32)TextOperator::OperatorType::FILESYSTEM_GET_FILE_NAME_EXCL_EXT);
	AddItem(files, MTEXT("Get File Name Extension"), (uint32)TextOperator::OperatorType::FILESYSTEM_GET_FILE_EXT);
	AddItem(files, MTEXT("Create Absolute File Path"), (uint32)TextOperator::OperatorType::FILESYSTEM_GET_ABSOLUTE_FILE_PATH);

	sort();

	SetSelectionChangedCallback([this](RData data) -> bool {
		TextOperator::OperatorType cmd = (TextOperator::OperatorType)data;
		if (cmd == GetChip()->GetOperatorType())
			return false;
		GetChip()->SetOperatorType(cmd);
		return true;
		});

	SetInit((uint32)GetChip()->GetOperatorType(), (uint32)TextOperator::OperatorType::NONE);
}
