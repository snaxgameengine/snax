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
#include "XMLValue_Dlg.h"



using namespace m3d;


DIALOGDESC_DEF(XMLValue_Dlg, XMLVALUE_GUID);


void XMLValue_Dlg::Init()
{
	AddItem(MTEXT("Is Element not Null?"), (uint32)XMLValue::OperatorType::IS_ELEMENT);
	AddItem(MTEXT("Is Element Root?"), (uint32)XMLValue::OperatorType::IS_ROOT);
	AddItem(MTEXT("Get Child Count"), (uint32)XMLValue::OperatorType::CHILD_COUNT);
	AddItem(MTEXT("Get Attribute Count"), (uint32)XMLValue::OperatorType::GET_ATTRIBUTE_COUNT);
	AddItem(MTEXT("Has Attribute"), (uint32)XMLValue::OperatorType::HAS_ATTRIBUTE);
	AddItem(MTEXT("Get/Set Content"), (uint32)XMLValue::OperatorType::CONTENT);
	AddItem(MTEXT("Get/Set Attribute"), (uint32)XMLValue::OperatorType::ATTRIBUTE);
	AddItem(MTEXT("XPath Query"), (uint32)XMLValue::OperatorType::QUERY);

	SetSelectionChangedCallback([this](RData data) -> bool {
		XMLValue::OperatorType cmd = (XMLValue::OperatorType)data;
		if (cmd == GetChip()->GetOperatorType())
			return false;
		GetChip()->SetOperatorType(cmd);
		return true;
		});

	SetInit((uint32)GetChip()->GetOperatorType(), (uint32)XMLValue::OperatorType::NONE);
}
