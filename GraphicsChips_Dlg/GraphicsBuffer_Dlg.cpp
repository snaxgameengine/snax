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
#include "GraphicsBuffer_Dlg.h"
#include "M3DCore/MagicEnum.h"

using namespace m3d;


DIALOGDESC_DEF(GraphicsBuffer_Dlg, GRAPHICSBUFFER_GUID);


GraphicsBuffer_Dlg::GraphicsBuffer_Dlg() : SimpleFormDialogPage(false) 
{
}

GraphicsBuffer_Dlg::~GraphicsBuffer_Dlg() 
{
}

void GraphicsBuffer_Dlg::Init()
{
	AddLineEdit(MTEXT("Size in bytes:"), strUtils::fromNum(GetChip()->GetSize()), false, [this](Id id, RVariant v) 
		{ 
			SetDirty();
			String s = v.ToString();
			UINT64 size;
			if (strUtils::toNum(s, size))
				GetChip()->SetSize(size);
		});
	AddCheckBox(MTEXT("Shader Resource"), (GetChip()->GetFlags() & M3D_RESOURCE_FLAG_DENY_SHADER_RESOURCE) == 0 ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) 
		{ 
			SetDirty(); 
			GetChip()->SetFlags((GetChip()->GetFlags() & ~M3D_RESOURCE_FLAG_DENY_SHADER_RESOURCE) | (v.ToBool() ? M3D_RESOURCE_FLAG_NONE : M3D_RESOURCE_FLAG_DENY_SHADER_RESOURCE));
		});
	AddCheckBox(MTEXT("Unordered Access View"), (GetChip()->GetFlags() & M3D_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) != 0 ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v) 
		{ 
			SetDirty();
			GetChip()->SetFlags((GetChip()->GetFlags() & ~M3D_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) | (v.ToBool() ? M3D_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : M3D_RESOURCE_FLAG_NONE));
		});
	AddCheckBox(MTEXT("GPU Only"), GetChip()->IsGPUOnly() ? RCheckState::Checked : RCheckState::Unchecked, [this](Id id, RVariant v)
		{
			SetDirty();
			GetChip()->SetGPUOnly(v.ToBool());
		});
}

