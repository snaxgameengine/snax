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
#include "Command.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "Value.h"
#include "ClassChip.h"
#include "M3DEngine/Class.h"
#include "Envelope.h"
#include "VectorChip.h"
#include "Text.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/RenderWindowManager.h"
#include "M3DEngine/Window.h"
#include "GraphicsChips/Graphics.h"
#include "InstanceData.h"
#include "ValueArray.h"
#include "SetChip.h"
#include "M3DEngine/Application.h"
#pragma warning(disable:4251 4661)
#include "ClassInstanceRefChip.h"
#include "ClassInstanceRefArrayChip.h"

using namespace m3d;


CHIPDESCV1_DEF(Command, MTEXT("Command"), COMMAND_GUID, CHIP_GUID);


// checklist
// 1. override keyword
// 2. errors
// 3. Refresh
// 4. No stack error
// 5. copy/load ok
// 6. update stamp

Command::Command()
{
	_ot = OperatorType::NONE;
}

Command::~Command()
{
}

bool Command::CopyChip(Chip *chip)
{
	Command *c = dynamic_cast<Command*>(chip);
	B_RETURN(Chip::CopyChip(c));
	SetOperatorType(c->_ot);
	return true;
}

bool Command::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	OperatorType ot;
	LOAD(MTEXT("operatorType|ot"), ot);
	SetOperatorType(ot);
	return true;
}

bool Command::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE(MTEXT("operatorType"), _ot);
	return true;
}



void Command::CallChip()
{
	if (!Refresh)
		return;

	switch (_ot) 
	{
	case OperatorType::SLEEP:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			if (ch0) {
				float64 d = ch0->GetValue();
#ifdef WINDESKTOP
				if (d >= 0.0)
					Sleep(DWORD(d * 1000));
#endif
			}
		}
		break;
	case OperatorType::SET_WINDOW_PROPS:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			ChildPtr<Value> ch3 = GetChild(3);
			ChildPtr<Value> ch4 = GetChild(4);
			ChildPtr<Value> ch5 = GetChild(5);
			ChildPtr<Value> ch6 = GetChild(6);
			Window *w = engine->GetGraphics()->GetRenderWindowManager()->GetWindow();
			if (w->GetParentWindow() != NULL)
				return; // Not supported for non top level windows.
			if (ch0) {
				XMFLOAT4 v = ch0->GetVector();
				uint32 flags = SWP_NOZORDER;
				if (!w->CanMove())
					flags |= SWP_NOMOVE;
				if (!w->CanResize())
					flags |= SWP_NOSIZE;
				engine->GetGraphics()->GetRenderWindowManager()->GetWindow()->SetWindowPos((INT)v.x, (INT)v.y, (INT)v.z, (INT)v.w, flags);
			}
			String title;
			if (ch1)
				title = ch1->GetText();
			w->SetText(title);
			long style = 0;
			if (title.size() > 0)
				style |= WS_CAPTION;
			if (ch2 && ch3->GetValueAsBool())
				style |= WS_SYSMENU | WS_CAPTION;
			if (ch3 && ch3->GetValueAsBool())
				style |= WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION;
			if (ch4 && ch4->GetValueAsBool())
				style |= WS_MAXIMIZEBOX | WS_SYSMENU | WS_CAPTION;
			if (ch5 && ch5->GetValueAsBool())
				style |= WS_THICKFRAME;
			if (style != 0)
				style |= WS_OVERLAPPED;
			else 
				style |= WS_POPUP;
			w->SetStyle(style);
			if (ch6) {
				long exStyle = 0;//WS_EX_APPWINDOW;
				if (ch6->GetValueAsBool())
					exStyle |= WS_EX_TOPMOST;
				w->SetExtendedStyle(exStyle);
			}
		}
		break;
	case OperatorType::POST_MESSAGE:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch1) {
				int lvl = ch0 ? (int)ch0->GetValue() : 1;
				lvl = std::min(std::max(lvl, 0), 4);
				String txt = ch1->GetText();
				Chip *ch2 = GetRawChild(2);
				msg((MessageSeverity)lvl, txt, ch2 ? ch2 : this); // <=== A real hack!
			}
		}
		break;
	case OperatorType::SET_APP_NAME:
		{
		}
		break;
	case OperatorType::QUIT:
		{
			engine->GetApplication()->Quit();
		}
		break;
	case OperatorType::CREATE_INSTANCE:
		{
			ChildPtr<ClassChip> ch0 = GetChild(0);
			if (ch0) {
				Class *cg = ch0->GetCG();
				if (cg) {
					for (uint32 i = 0, j = GetSubConnectionCount(1); i < j; i++) {
						ChildPtr<ClassInstanceRefChip> ch1 = GetChild(1, i);
						if (ch1)
							ch1->CreateInstance(cg);
					}
				}
			}
		}
		break;
	case OperatorType::CLEAR_INSTANCE_REF:
		{
			for (uint32 i = 0, j = GetSubConnectionCount(0); i < j; i++) {
				ChildPtr<ClassInstanceRefChip> ch0 = GetChild(0, i);
				if (ch0)
					ch0->ClearInstance();
			}
		}
		break;
	case OperatorType::COPY_INSTANCE_REF:
		{
			ChildPtr<ClassInstanceRefChip> ch0 = GetChild(0);
			if (ch0) {
				ClassInstanceRef ref = ch0->GetInstance();
				if (ref) {
					for (uint32 i = 0, j = GetSubConnectionCount(1); i < j; i++) {
						ChildPtr<ClassInstanceRefChip> ch1 = GetChild(1, i);
						if (ch1)
							ch1->SetInstance(ref, ClassInstanceRefChip::AT_COPY);
					}
				}
			}
		}
		break;
	case OperatorType::MAKE_OWNER:
		{
			for (uint32 i = 0, j = GetSubConnectionCount(0); i < j; i++) {
				ChildPtr<ClassInstanceRefChip> ch0 = GetChild(0, i);
				if (ch0)
					ch0->MakeOwner();
			}
		}
		break;
	case OperatorType::VALUEARRAY_SORT:
		{
			ChildPtr<ValueArray> ch0 = GetChild(0);
			if (ch0) {
				ChildPtr<Value> ch1 = GetChild(1);
				ChildPtr<Value> ch2 = GetChild(2);
				ChildPtr<Value> ch3 = GetChild(3);
				int dir = ch1 ? (int)ch1->GetValue() : 1;
				uint32 index = ch2 ? (uint32)ch2->GetValue() : 0;
				uint32 count = ch3 ? (uint32)ch3->GetValue() : -1;
				ch0->Sort(dir, index, count);
			}
		}
		break;
	case OperatorType::INSTANCEARRAY_SORT:
		{
			ChildPtr<ClassInstanceRefArrayChip> ch0 = GetChild(0);
			if (ch0) {
				ChildPtr<Value> ch1 = GetChild(1);
				ChildPtr<Value> ch2 = GetChild(2);
				ChildPtr<Value> ch3 = GetChild(3);
				int dir = ch1 ? (int)ch1->GetValue() : 1;
				uint32 index = ch2 ? (uint32)ch2->GetValue() : 0;
				uint32 count = ch3 ? (uint32)ch3->GetValue() : -1;
				List<InstanceData*> keys;
				for (uint32 i = 0; i < GetSubConnectionCount(4); i++) {
					InstanceData *d = dynamic_cast<InstanceData*>(GetRawChild(3, i));
					if (d)
						keys.push_back(d);
				}
				if (keys.size()) {
					ch0->Sort(keys, dir, index, count);
				}
			}
		}
		break;
	case OperatorType::RESET_REFRESH_MANAGER:
		{
			for (uint32 i = 0, j = GetSubConnectionCount(0); i < j; i++) {
				ChipChildPtr ch0 = GetChild(0, i);
				if (ch0)
					ch0->GetRefreshManager().Reset();
			}
		}
		break;
	case OperatorType::COPY_CHIP:
		{
			ChipChildPtr ch0 = GetChild(0);
			ChipChildPtr ch1 = GetChild(1);
			if (ch0 && ch1 && ch0 != ch1) {
				if (!ch1->CopyChip(*(Chip**)&ch0)) { // <= Pointer hack!
					msg(WARN, MTEXT("Failed to copy chip. Type mismatch?"), this);
				}
			}
		}
		break;
	case OperatorType::ENVELOPE_ADD_STEP_CP:
		{
			ChildPtr<Envelope> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			if (ch0 && ch1 && ch2) {
				value x = ch1->GetValue();
				value y = ch2->GetValue();
				ch0->GetSpline().AddControlPoint(ValueTCBSpline::ControlPoint(x, y, ValueTCBSpline::ControlPoint::STEP));
			}
		}
		break;
	case OperatorType::ENVELOPE_ADD_LINEAR_CP:
		{
			ChildPtr<Envelope> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			if (ch0 && ch1 && ch2) {
				value x = ch1->GetValue();
				value y = ch2->GetValue();
				ch0->GetSpline().AddControlPoint(ValueTCBSpline::ControlPoint(x, y, ValueTCBSpline::ControlPoint::LINEAR));
			}
		}
		break;
	case OperatorType::ENVELOPE_ADD_TCB_CP:
		{
			ChildPtr<Envelope> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			ChildPtr<Value> ch3 = GetChild(3);
			ChildPtr<Value> ch4 = GetChild(4);
			ChildPtr<Value> ch5 = GetChild(5);
			if (ch0 && ch1 && ch2) {
				value x = ch1->GetValue();
				value y = ch2->GetValue();
				value t = ch3 ? ch3->GetValue() : 0.0;
				value c = ch4 ? ch4->GetValue() : 0.0;
				value b = ch5 ? ch5->GetValue() : 0.0;
				ch0->GetSpline().AddControlPoint(ValueTCBSpline::ControlPoint(x, y, ValueTCBSpline::ControlPoint::TCB, t, c, b));
			}
		}
		break;
	case OperatorType::ENVELOPE_REMOVE_CP:
		{
			ChildPtr<Envelope> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				int i = (int)ch1->GetValue();
				if (i >= 0 && i < (int)ch0->GetSpline().GetControlPoints().size())
					ch0->GetSpline().RemoveControlPoint(i);
			}
		}
		break;
	case OperatorType::ENVELOPE_CLEAR:
		{
			ChildPtr<Envelope> ch0 = GetChild(0);
			if (ch0) {
				ch0->GetSpline().Clear();
			}
		}
		break;
	case OperatorType::CONTAINER_CLEAR:
		{
			for (uint32 i = 0, j = GetSubConnectionCount(0); i < j; i++) {
				ChildPtr<ContainerChip> ch0 = GetChild(0, i);
				if (ch0)
					ch0->ClearContainer();
			}
		}
		break;
	case OperatorType::VALUESET_REMOVE_KEY:
		{
			ChildPtr<SetChipInterface<value>> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				value key = ch1->GetValue();
				ch0->RemoveElement(key);
			}
		}
		break;
	case OperatorType::TEXTSET_REMOVE_KEY:
		{
			ChildPtr<SetChipInterface<String>> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String key = ch1->GetText();
				ch0->RemoveElement(key);
			}
		}
		break;
	default:
		AddMessage(UninitializedException());
		break;
	}
}

void Command::SetOperatorType(OperatorType ot)
{
	if (ot == _ot)
		return;
	_ot = ot;
	switch (_ot) 
	{
	case OperatorType::SLEEP:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("Seconds to sleep"));
		ClearConnections(1);
		break;
	case OperatorType::SET_WINDOW_PROPS:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Position and Size"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Window Title"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("Has System Menu (0/1)"));
		CREATE_CHILD_KEEP(3, VALUE_GUID, false, UP, MTEXT("Has Minimize Button (0/1)"));
		CREATE_CHILD_KEEP(4, VALUE_GUID, false, UP, MTEXT("Has Maximize Button (0/1)"));
		CREATE_CHILD_KEEP(5, VALUE_GUID, false, UP, MTEXT("Has Sizeable Border (0/1)"));
		CREATE_CHILD_KEEP(6, VALUE_GUID, false, UP, MTEXT("Always on Top (0/1)"));
		ClearConnections(7);
		break;
	case OperatorType::POST_MESSAGE:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("Level (0-4)"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Message"));
		CREATE_CHILD_KEEP(2, CHIP_GUID, false, UP, MTEXT("Chip to Report (Optional)"));
		ClearConnections(3);
		break;
	case OperatorType::SET_APP_NAME:
		CREATE_CHILD_KEEP(0, TEXT_GUID, false, UP, MTEXT("Organization Name"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Application Name"));
		ClearConnections(2);
		break;
	case OperatorType::QUIT:
		ClearConnections();
		break;
	case OperatorType::CREATE_INSTANCE:
		CREATE_CHILD_KEEP(0, CLASSCHIP_GUID, false, UP, MTEXT("Class"));
		CREATE_CHILD_KEEP(1, CLASSINSTANCEREFCHIP_GUID, true, DOWN, MTEXT("Instance Refs"));
		ClearConnections(2);
		break;
	case OperatorType::CLEAR_INSTANCE_REF:
		CREATE_CHILD_KEEP(0, CLASSINSTANCEREFCHIP_GUID, true, DOWN, MTEXT("Instance Refs"));
		ClearConnections(1);
		break;
	case OperatorType::COPY_INSTANCE_REF:
		CREATE_CHILD_KEEP(0, CLASSINSTANCEREFCHIP_GUID, false, UP, MTEXT("Instance to Copy"));
		CREATE_CHILD_KEEP(1, CLASSINSTANCEREFCHIP_GUID, true, DOWN, MTEXT("Instance Refs"));
		ClearConnections(2);
		break;
	case OperatorType::MAKE_OWNER:
		CREATE_CHILD_KEEP(0, CLASSINSTANCEREFCHIP_GUID, true, DOWN, MTEXT("Instance Refs"));
		ClearConnections(1);
		break;
	case OperatorType::VALUEARRAY_SORT:
		CREATE_CHILD_KEEP(0, VALUEARRAY_GUID, false, DOWN, MTEXT("Array"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Direction (-1/1 - Desc/Asc)"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("First Element"));
		CREATE_CHILD_KEEP(3, VALUE_GUID, false, UP, MTEXT("Element Count"));
		ClearConnections(4);
		break;
	case OperatorType::INSTANCEARRAY_SORT:
		CREATE_CHILD_KEEP(0, CLASSINSTANCEREFARRAYCHIP_GUID, false, DOWN, MTEXT("Array"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Direction (-1/1 - Desc/Asc)"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("First Element"));
		CREATE_CHILD_KEEP(3, VALUE_GUID, false, UP, MTEXT("Element Count"));
		CREATE_CHILD_KEEP(4, CHIP_GUID, true, UP, MTEXT("Instance Data Sorting Keys"));
		ClearConnections(5);
		break;
	case OperatorType::RESET_REFRESH_MANAGER:
		CREATE_CHILD_KEEP(0, CHIP_GUID, true, DOWN, MTEXT("Chips"));
		ClearConnections(1);
		break;
	case OperatorType::COPY_CHIP:
		CREATE_CHILD_KEEP(0, CHIP_GUID, false, UP, MTEXT("Copy From"));
		CREATE_CHILD_KEEP(1, CHIP_GUID, false, DOWN, MTEXT("Copy To"));
		ClearConnections(2);
		break;
	case OperatorType::ENVELOPE_ADD_STEP_CP:
	case OperatorType::ENVELOPE_ADD_LINEAR_CP:
		CREATE_CHILD_KEEP(0, ENVELOPE_GUID, false, UP, MTEXT("Envelope"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("x"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("y"));
		ClearConnections(3);
		break;
	case OperatorType::ENVELOPE_ADD_TCB_CP:
		CREATE_CHILD_KEEP(0, ENVELOPE_GUID, false, UP, MTEXT("Envelope"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("x"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("y"));
		CREATE_CHILD_KEEP(3, VALUE_GUID, false, UP, MTEXT("Tension"));
		CREATE_CHILD_KEEP(4, VALUE_GUID, false, UP, MTEXT("Continuity"));
		CREATE_CHILD_KEEP(5, VALUE_GUID, false, UP, MTEXT("Bias"));
		ClearConnections(6);
		break;
	case OperatorType::ENVELOPE_REMOVE_CP:
		CREATE_CHILD_KEEP(0, ENVELOPE_GUID, false, UP, MTEXT("Envelope"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Control Point Index"));
		ClearConnections(2);
		break;
	case OperatorType::ENVELOPE_CLEAR:
		CREATE_CHILD_KEEP(0, ENVELOPE_GUID, false, UP, MTEXT("Envelope"));
		ClearConnections(1);
		break;
	case OperatorType::CONTAINER_CLEAR:
		CREATE_CHILD_KEEP(0, CONTAINERCHIP_GUID, true, DOWN, MTEXT("Containers"));
		ClearConnections(1);
		break;
	case OperatorType::VALUESET_REMOVE_KEY:
		CREATE_CHILD_KEEP(0, VALUESET_GUID, false, DOWN, MTEXT("Value Set"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Key"));
		ClearConnections(2);
		break;
	case OperatorType::TEXTSET_REMOVE_KEY:
		CREATE_CHILD_KEEP(0, TEXTSET_GUID, false, DOWN, MTEXT("Text Set"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Key"));
		ClearConnections(2);
		break;
	default:
		ClearConnections();
		break;
	}
	RemoveMessage(UninitializedException());
}