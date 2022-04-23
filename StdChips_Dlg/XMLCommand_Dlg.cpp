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
#include "XMLCommand_Dlg.h"



using namespace m3d;

DIALOGDESC_DEF(XMLCommand_Dlg, XMLCOMMAND_GUID);



void XMLCommand_Dlg::Init()
{
	AddItem(MTEXT("New XML"), (uint32)XMLCommand::OperatorType::NEW_XML);
	AddItem(MTEXT("Load XML from File"), (uint32)XMLCommand::OperatorType::LOAD_XML_FROM_FILE);
	AddItem(MTEXT("Load XML from Text"), (uint32)XMLCommand::OperatorType::LOAD_XML_FROM_TEXT);
	AddItem(MTEXT("Save XML to File"), (uint32)XMLCommand::OperatorType::SAVE_XML_TO_FILE);
	AddItem(MTEXT("Save XML to Text"), (uint32)XMLCommand::OperatorType::SAVE_XML_TO_TEXT);
	AddItem(MTEXT("Add Child Element"), (uint32)XMLCommand::OperatorType::ADD_CHILD_ELEMENT);
	AddItem(MTEXT("Remove Element"), (uint32)XMLCommand::OperatorType::REMOVE_ELEMENT);
	AddItem(MTEXT("Remove Attribute"), (uint32)XMLCommand::OperatorType::REMOVE_ATTRIBUTE);
	AddItem(MTEXT("Get Parent"), (uint32)XMLCommand::OperatorType::GET_PARENT);
	AddItem(MTEXT("Get First Child"), (uint32)XMLCommand::OperatorType::GET_FIRST_CHILD);
	AddItem(MTEXT("Get Next Sibling"), (uint32)XMLCommand::OperatorType::GET_NEXT_SIBLING);
	AddItem(MTEXT("XPath Query (elements)"), (uint32)XMLCommand::OperatorType::QUERY_NODE);
	AddItem(MTEXT("XPath Query (text)"), (uint32)XMLCommand::OperatorType::QUERY_TEXT);
	AddItem(MTEXT("XPath Query (value)"), (uint32)XMLCommand::OperatorType::QUERY_VALUE);

	SetSelectionChangedCallback([this](RData data) -> bool {
		XMLCommand::OperatorType cmd = (XMLCommand::OperatorType)data;
		if (cmd == GetChip()->GetOperatorType())
			return false;
		GetChip()->SetOperatorType(cmd);
		return true;
		});

	SetInit((uint32)GetChip()->GetOperatorType(), (uint32)XMLCommand::OperatorType::NONE);
}
