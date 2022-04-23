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
#include "GraphicsMatrix_Dlg.h"

using namespace m3d;


DIALOGDESC_DEF(GraphicsMatrix_Dlg, GRAPHICSMATRIX_GUID);


void GraphicsMatrix_Dlg::Init()
{
	AddItem(MTEXT("Get/Set World Matrix"), (int32)GraphicsMatrix::OperatorType::WORLD_MATRIX);
	AddItem(MTEXT("Get/Set View Matrix"), (int32)GraphicsMatrix::OperatorType::VIEW_MATRIX);
	AddItem(MTEXT("Get/Set Projection Matrix"), (int32)GraphicsMatrix::OperatorType::PROJECTION_MATRIX);
	AddItem(MTEXT("Get/Set Culling Matrix"), (int32)GraphicsMatrix::OperatorType::CULLING_MATRIX);
	AddItem(MTEXT("Get/Set Shadow Matrix"), (int32)GraphicsMatrix::OperatorType::SHADOW_MATRIX);

	SetSelectionChangedCallback([this](RData data) -> bool {
		GraphicsMatrix::OperatorType cmd = (GraphicsMatrix::OperatorType)data;
		if (cmd == GetChip()->GetOperatorType())
			return false;
		GetChip()->SetOperatorType(cmd);
		return true;
		});

	SetInit((int32)GetChip()->GetOperatorType(), (int32)GraphicsMatrix::OperatorType::NONE);
}
