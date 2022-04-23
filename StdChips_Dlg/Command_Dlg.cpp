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

#include "stdafx.h"
#include "Command_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(Command_Dlg, COMMAND_GUID);


void Command_Dlg::Init()
{
	RPtr sysop = AddItem(MTEXT("System Commands"), (uint32)Command::OperatorType::NONE);
	AddItem(sysop, MTEXT("Sleep"), (uint32)Command::OperatorType::SLEEP);
	AddItem(sysop, MTEXT("Configure Application Window"), (uint32)Command::OperatorType::SET_WINDOW_PROPS);
	AddItem(sysop, MTEXT("Post Message"), (uint32)Command::OperatorType::POST_MESSAGE);
	AddItem(sysop, MTEXT("Quit the Application"), (uint32)Command::OperatorType::QUIT);

	RPtr op = AddItem(MTEXT("Instance Commands"), (uint32)Command::OperatorType::NONE);
	AddItem(op, MTEXT("Create Instance"), (uint32)Command::OperatorType::CREATE_INSTANCE);
	AddItem(op, MTEXT("Clear Instance Ref"), (uint32)Command::OperatorType::CLEAR_INSTANCE_REF);
	AddItem(op, MTEXT("Copy Instance"), (uint32)Command::OperatorType::COPY_INSTANCE_REF);
	AddItem(op, MTEXT("Make Owner"), (uint32)Command::OperatorType::MAKE_OWNER);

	RPtr miscOp = AddItem(MTEXT("Misc Commands"), (uint32)Command::OperatorType::NONE);
	AddItem(miscOp, MTEXT("Reset Refresh Manager"), (uint32)Command::OperatorType::RESET_REFRESH_MANAGER);
	AddItem(miscOp, MTEXT("Copy Chip"), (uint32)Command::OperatorType::COPY_CHIP);
	AddItem(miscOp, MTEXT("Sort Instance Array"), (uint32)Command::OperatorType::INSTANCEARRAY_SORT);
	AddItem(miscOp, MTEXT("Sort Value Array"), (uint32)Command::OperatorType::VALUEARRAY_SORT);

	RPtr envelopeOp = AddItem(MTEXT("Envelope Commands"), (uint32)Command::OperatorType::NONE);
	AddItem(envelopeOp, MTEXT("Add \'Step\' Control Point"), (uint32)Command::OperatorType::ENVELOPE_ADD_STEP_CP);
	AddItem(envelopeOp, MTEXT("Add \'Linear\' Control Point"), (uint32)Command::OperatorType::ENVELOPE_ADD_LINEAR_CP);
	AddItem(envelopeOp, MTEXT("Add \'TCB\' Control Point"), (uint32)Command::OperatorType::ENVELOPE_ADD_TCB_CP);
	AddItem(envelopeOp, MTEXT("Remove Control Point"), (uint32)Command::OperatorType::ENVELOPE_REMOVE_CP);
	AddItem(envelopeOp, MTEXT("Clear"), (uint32)Command::OperatorType::ENVELOPE_CLEAR);

	RPtr containerOp = AddItem(MTEXT("Container Commands"), (uint32)Command::OperatorType::NONE);
	AddItem(containerOp, MTEXT("Clear Containers"), (uint32)Command::OperatorType::CONTAINER_CLEAR);
	AddItem(containerOp, MTEXT("Value-Set Remove Key"), (uint32)Command::OperatorType::VALUESET_REMOVE_KEY);
	AddItem(containerOp, MTEXT("Text-Set Remove Key"), (uint32)Command::OperatorType::TEXTSET_REMOVE_KEY);

	sort();

	SetSelectionChangedCallback([this](RData data) -> bool {
		Command::OperatorType cmd = (Command::OperatorType)data;
		if (cmd == GetChip()->GetOperatorType())
			return false;
		GetChip()->SetOperatorType(cmd);
		return true;
		});

	SetInit((uint32)GetChip()->GetOperatorType(), (uint32)Command::OperatorType::NONE);
}
