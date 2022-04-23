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
#include "ValueOperator_dlg.h"

using namespace m3d;


DIALOGDESC_DEF(ValueOperator_Dlg, VALUEOPERATOR_GUID);



void ValueOperator_Dlg::Init()
{
	RPtr vectoro = AddItem(MTEXT("Vector Operations"), (uint32)ValueOperator::OperatorType::NONE);
	AddItem(vectoro, MTEXT("Get Vector X"), (uint32)ValueOperator::OperatorType::VECTOR_X);
	AddItem(vectoro, MTEXT("Get Vector Y"), (uint32)ValueOperator::OperatorType::VECTOR_Y);
	AddItem(vectoro, MTEXT("Get Vector Z"), (uint32)ValueOperator::OperatorType::VECTOR_Z);
	AddItem(vectoro, MTEXT("Get Vector W"), (uint32)ValueOperator::OperatorType::VECTOR_W);
	AddItem(vectoro, MTEXT("Get Vector Length 2"), (uint32)ValueOperator::OperatorType::VECTOR_LENGTH2);
	AddItem(vectoro, MTEXT("Get Vector Length 3"), (uint32)ValueOperator::OperatorType::VECTOR_LENGTH3);
	AddItem(vectoro, MTEXT("Get Vector Length 4"), (uint32)ValueOperator::OperatorType::VECTOR_LENGTH4);
	AddItem(vectoro, MTEXT("Get Vector Length 2 Square"), (uint32)ValueOperator::OperatorType::VECTOR_LENGTHSQ2);
	AddItem(vectoro, MTEXT("Get Vector Length 3 Square"), (uint32)ValueOperator::OperatorType::VECTOR_LENGTHSQ3);
	AddItem(vectoro, MTEXT("Get Vector Length 4 Square"), (uint32)ValueOperator::OperatorType::VECTOR_LENGTHSQ4);
	AddItem(vectoro, MTEXT("Get Vector Dot 2 Product"), (uint32)ValueOperator::OperatorType::VECTOR_DOT2);
	AddItem(vectoro, MTEXT("Get Vector Dot 3 Product"), (uint32)ValueOperator::OperatorType::VECTOR_DOT3);
	AddItem(vectoro, MTEXT("Get Vector Dot 4 Product"), (uint32)ValueOperator::OperatorType::VECTOR_DOT4);

	RPtr misc = AddItem(MTEXT("Misc"), (uint32)ValueOperator::OperatorType::NONE);
	AddItem(misc, MTEXT("Get Number of Command Line Arguments"), (uint32)ValueOperator::OperatorType::GET_CMD_LINE_ARGUMENT_COUNT);
	AddItem(misc, MTEXT("Get Run-Time Environment"), (uint32)ValueOperator::OperatorType::GET_RUNTIME_ENV);
	AddItem(misc, MTEXT("Get DT"), (uint32)ValueOperator::OperatorType::DT);
	AddItem(misc, MTEXT("Get Framerate"), (uint32)ValueOperator::OperatorType::FPS);
	AddItem(misc, MTEXT("Get Envelope Derivative"), (uint32)ValueOperator::OperatorType::ENVELOPE_GET_DERIVATIVE);
	AddItem(misc, MTEXT("Get Chip Timestamp"), (uint32)ValueOperator::OperatorType::GET_UPDATE_STAMP);
	
	RPtr textop = AddItem(MTEXT("Text"), (uint32)ValueOperator::OperatorType::NONE);
	AddItem(textop, MTEXT("Get Text Length"), (uint32)ValueOperator::OperatorType::TEXT_LENGTH);
	AddItem(textop, MTEXT("Text Compare"), (uint32)ValueOperator::OperatorType::TEXT_COMPARE);
	AddItem(textop, MTEXT("Get Value From Text"), (uint32)ValueOperator::OperatorType::TEXT_VALUE);

	RPtr instanceop = AddItem(MTEXT("Instances"), (uint32)ValueOperator::OperatorType::NONE);
	AddItem(instanceop, MTEXT("Instance Exist?"), (uint32)ValueOperator::OperatorType::INSTANCEREF_EXIST);
	AddItem(instanceop, MTEXT("Instances Equal?"), (uint32)ValueOperator::OperatorType::INSTANCEREF_EQUALS);
	AddItem(instanceop, MTEXT("Instance is Type Of?"), (uint32)ValueOperator::OperatorType::INSTANCEREF_TYPE_OF);
	AddItem(instanceop, MTEXT("Instance is Type Compatible?"), (uint32)ValueOperator::OperatorType::INSTANCEREF_TYPE_COMPATIBLE);
	AddItem(instanceop, MTEXT("Instance Ref is Owner?"), (uint32)ValueOperator::OperatorType::INSTANCEREF_IS_OWNER);
	AddItem(instanceop, MTEXT("Get Instance ID"), (uint32)ValueOperator::OperatorType::INSTANCEREF_GET_ID);

	RPtr containerop = AddItem(MTEXT("Container Operations"), (uint32)ValueOperator::OperatorType::NONE);
	AddItem(containerop, MTEXT("Value-Set Key Exist?"), (uint32)ValueOperator::OperatorType::VALUESET_KEY_EXIST);
	AddItem(containerop, MTEXT("Text-Set Key Exist?"), (uint32)ValueOperator::OperatorType::TEXTSET_KEY_EXIST);

	RPtr timeop = AddItem(MTEXT("Time Operations"), (uint32)ValueOperator::OperatorType::NONE);
	AddItem(timeop, MTEXT("Get current UTC time in seconds since epoch"), (uint32)ValueOperator::OperatorType::UTC_TIME);
	AddItem(timeop, MTEXT("Convert seconds since epoch to/from UTC time"), (uint32)ValueOperator::OperatorType::UTC_TO_SECONDS_SINCE_EPOCH);
	AddItem(timeop, MTEXT("Convert seconds since epoch to/from local time"), (uint32)ValueOperator::OperatorType::LOCAL_TO_SECONDS_SINCE_EPOCH);

	sort();

	SetSelectionChangedCallback([this](RData data) -> bool {
		ValueOperator::OperatorType cmd = (ValueOperator::OperatorType)data;
		if (cmd == GetChip()->GetOperatorType())
			return false;
		GetChip()->SetOperatorType(cmd);
		return true;
		});

	SetInit((uint32)GetChip()->GetOperatorType(), (uint32)ValueOperator::OperatorType::NONE);
}
