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

#include "pch.h"
#include "SkeletonControllerCmd.h"
#include "SkeletonController.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "StdChips/Text.h"
#include "StdChips/Value.h"
#include "M3DEngine/Engine.h"


using namespace m3d;


CHIPDESCV1_DEF(SkeletonControllerCmd, MTEXT("Skeleton Controller Command"), SKELETONCONTROLLERCMD_GUID, CHIP_GUID);


SkeletonControllerCmd::SkeletonControllerCmd()
{
	_cmd = OperatorType::NONE;
}

SkeletonControllerCmd::~SkeletonControllerCmd()
{
}

bool SkeletonControllerCmd::CopyChip(Chip *chip)
{
	SkeletonControllerCmd *c = dynamic_cast<SkeletonControllerCmd*>(chip);
	B_RETURN(Chip::CopyChip(c));
	SetCommand(c->_cmd);
	return true;
}

bool SkeletonControllerCmd::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	OperatorType cmd;
	LOAD("operatorType|cmd", cmd);
	SetCommand(cmd);
	return true;
}

bool SkeletonControllerCmd::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE("operatorType", _cmd);
	return true;
}

void SkeletonControllerCmd::CallChip()
{
	switch (_cmd) {
		case OperatorType::ADD_ANIMATION:
			{
				ChildPtr<SkeletonController> ch0 = GetChild(0);
				if (!ch0)
					return;
				ChildPtr<Text> ch1 = GetChild(1);
				if (!ch1)
					return;
				ChildPtr<Value> ch2 = GetChild(2);
				ChildPtr<Value> ch3 = GetChild(3);
				ChildPtr<Value> ch4 = GetChild(4);

				value vm = 1.0, vi = 0.0, vo = 0.0;

				if (ch2) vm = ch2->GetValue();
				if (ch3) vi = ch3->GetValue();
				if (ch4) vo = ch4->GetValue();
				
				vi = std::max(vi, 0.0);
				vo = std::max(vo, 0.0);

				String name = ch1->GetText();

				ch0->AddCommand(name, (float32)vm, (float32)vi, (float32)vo);
			}
			break;
		case OperatorType::ADD_LOOPED_ANIMATION:
			{
				ChildPtr<SkeletonController> ch0 = GetChild(0);
				if (!ch0)
					return;
				ChildPtr<Text> ch1 = GetChild(1);
				if (!ch1)
					return;
				ChildPtr<Value> ch2 = GetChild(2);
				ChildPtr<Value> ch3 = GetChild(3);

				value vm = 1.0, vi = 0.0;

				if (ch2) vm = ch2->GetValue();
				if (ch3) vi = ch3->GetValue();
				
				vi = std::max(vi, 0.0);

				String name = ch1->GetText();
				
				ch0->AddCommand(name, (float32)vm, (float32)vi);
			}
			break;
		case OperatorType::REMOVE_ANIMATION:
			{
				ChildPtr<SkeletonController> ch0 = GetChild(0);
				if (!ch0)
					return;
				ChildPtr<Text> ch1 = GetChild(1);
				if (!ch1)
					return;
				ChildPtr<Value> ch2 = GetChild(2);

				value vo = 0.0;

				if (ch2) vo = ch2->GetValue();
				
				vo = std::max(vo, 0.0);

				String name = ch1->GetText();
				
				ch0->RemoveCommand(name, (float32)vo);
			}
			break;
		default:
			break;
	}
}

void SkeletonControllerCmd::SetCommand(OperatorType cmd)
{
	if (_cmd == cmd)
		return;
	_cmd = cmd;
	switch (_cmd) {
		case OperatorType::ADD_ANIMATION:
			{
				CREATE_CHILD_KEEP(0, SKELETONCONTROLLER_GUID, false, DOWN, MTEXT("Skeleton Controller"));
				CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Name"));
				CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("Multiplier"));
				CREATE_CHILD_KEEP(3, VALUE_GUID, false, UP, MTEXT("Fadein Time"));
				CREATE_CHILD_KEEP(4, VALUE_GUID, false, UP, MTEXT("Fadeout Time"));
				ClearConnections(5);
			}
			break;
		case OperatorType::ADD_LOOPED_ANIMATION:
			{
				CREATE_CHILD_KEEP(0, SKELETONCONTROLLER_GUID, false, DOWN, MTEXT("Skeleton Controller"));
				CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Name"));
				CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("Multiplier"));
				CREATE_CHILD_KEEP(3, VALUE_GUID, false, UP, MTEXT("Fadein Time"));
				ClearConnections(4);
			}
			break;
		case OperatorType::REMOVE_ANIMATION:
			{
				CREATE_CHILD_KEEP(0, SKELETONCONTROLLER_GUID, false, DOWN, MTEXT("Skeleton Controller"));
				CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Name"));
				CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("Fadeout Time"));
				ClearConnections(3);
			}
			break;
		default:
			break;
	}
}
