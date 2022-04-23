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
#include "SkeletonControllerCmd_Dlg.h"


using namespace m3d;


DIALOGDESC_DEF(SkeletonControllerCmd_Dlg, SKELETONCONTROLLERCMD_GUID);


void SkeletonControllerCmd_Dlg::Init()
{
	AddItem(MTEXT("Add Animation"), (uint32)SkeletonControllerCmd::OperatorType::ADD_ANIMATION);
	AddItem(MTEXT("Add Looped Animation"), (uint32)SkeletonControllerCmd::OperatorType::ADD_LOOPED_ANIMATION);
	AddItem(MTEXT("Remove Animation"), (uint32)SkeletonControllerCmd::OperatorType::REMOVE_ANIMATION);

	SetSelectionChangedCallback([this](RData data) -> bool {
		SkeletonControllerCmd::OperatorType cmd = (SkeletonControllerCmd::OperatorType)data;
		if (cmd == GetChip()->GetCommand())
			return false;
		GetChip()->SetCommand(cmd);
		return true;
		});

	SetInit((uint32)GetChip()->GetCommand(), (uint32)SkeletonControllerCmd::OperatorType::NONE);
}

