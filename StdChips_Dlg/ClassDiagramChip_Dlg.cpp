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
#include "ClassDiagramChip_Dlg.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngine/Document.h"


using namespace m3d;


DIALOGDESC_DEF_ADV(ClassDiagramChip_Dlg, CLASSDIAGRAMCHIP_GUID, NullGUID);


void ClassDiagramChip_Dlg::Init()
{
	Class *cg = GetChip()->GetCG();
	Path p = cg->GetDocument()->GetFileName();
	AddLineEdit(MTEXT("Name:"), cg->GetName(), false, [this](Id id, RVariant v) 
		{ 
			String newName = v.ToString();
			if (newName.empty())
				return;
			Class* cg = engine->GetClassManager()->GetClass(newName);
			if (cg != nullptr)
				return;
			engine->GetClassManager()->SetClassName(GetChip()->GetCG(), newName);
			GetChip()->GetCG()->SetDirty();
			// TODO: Set relations dirty!!
		});
	
	
	AddReadOnlyLineEdit(MTEXT("ID:"), GuidToString(cg->GetGuid()));
	AddReadOnlyLineEdit(MTEXT("Document:"), p.IsFile() ? p.AsString() : String(MTEXT("Not Saved")));
	AddReadOnlyLineEdit(MTEXT("Chips:"), strUtils::fromNum(cg->GetChips().size()));
	AddReadOnlyLineEdit(MTEXT("Instances:"), strUtils::fromNum(cg->GetInstances().size()));
}
