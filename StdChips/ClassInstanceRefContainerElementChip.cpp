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
#pragma warning(disable:4251 4661)
#include "ClassInstanceRefContainerElementChip.h"
#include "ClassInstanceRefArrayChip.h"
#include "ClassInstanceRefMapChip.h"
#include "Value.h"
#include "Text.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"

using namespace m3d;


CHIPDESCV1_DEF(ClassInstanceRefContainerElementChip, MTEXT("Instance Ref Container Element"), CLASSINSTANCEREFCONTAINERELEMENTCHIP_GUID, CLASSINSTANCEREFCHIP_GUID);



ClassInstanceRefContainerElementChip::ClassInstanceRefContainerElementChip()
{
	_ot = OperatorType::NONE;
}

ClassInstanceRefContainerElementChip::~ClassInstanceRefContainerElementChip()
{
}

bool ClassInstanceRefContainerElementChip::CopyChip(Chip *chip)
{
	ClassInstanceRefContainerElementChip *c = dynamic_cast<ClassInstanceRefContainerElementChip*>(chip);
	(Chip::CopyChip(c)); // skip ClassInstanceRefChip
	SetOperatorType(c->_ot);
	return true;
}

bool ClassInstanceRefContainerElementChip::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader)); // skip ClassInstanceRefChip
	OperatorType ot = OperatorType::ARRAY;
	LOAD(MTEXT("operatorType|ot"), ot);
	SetOperatorType(ot);
	return true;
}

bool ClassInstanceRefContainerElementChip::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver)); // skip ClassInstanceRefChip
	SAVE(MTEXT("operatorType"), _ot);
	return true;
}

ClassInstanceRef ClassInstanceRefContainerElementChip::GetInstance()
{
	ClassInstanceRef ref;
	switch (_ot) 
	{
	case OperatorType::ARRAY:
		{
			ChildPtr<ClassInstanceRefArrayChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize())
					ref = ch0->GetInstance(index);
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::VALUEMAP_BY_KEY:
		{
			ChildPtr<ClassInstanceRefByValueMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				value key = ch1->GetValue();
				const ClassInstanceRef *r = ch0->GetValueByKey(key); // Can be null if not found
				if (r)
					ref = *r;
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::VALUEMAP_BY_INDEX:
		{
			ChildPtr<ClassInstanceRefByValueMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize())
					ref = *ch0->GetValueByIndex(index);
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::TEXTMAP_BY_KEY:
		{
			ChildPtr<ClassInstanceRefByTextMapChip> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String key = ch1->GetText();
				const ClassInstanceRef *r = ch0->GetValueByKey(key); // Can be null if not found
				if (r)
					ref = *r;
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::TEXTMAP_BY_INDEX:
		{
			ChildPtr<ClassInstanceRefByTextMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize())
					ref = *ch0->GetValueByIndex(index);
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	default:
		AddMessage(UninitializedException());
		break;
	}
	return ref;
}

void ClassInstanceRefContainerElementChip::CreateInstance(Class *instanceOf)
{
	switch (_ot) 
	{
	case OperatorType::ARRAY:
		{
			ChildPtr<ClassInstanceRefArrayChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize())
					ch0->CreateInstance(index, instanceOf);
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::VALUEMAP_BY_KEY:
		{
			ChildPtr<ClassInstanceRefByValueMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				value key = ch1->GetValue();
				ch0->CreateInstance(key, instanceOf);
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::VALUEMAP_BY_INDEX:
		{
			ChildPtr<ClassInstanceRefByValueMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize()) {
					const value *key = ch0->GetElementByIndex(index);
					assert(key);
					if (key)
						ch0->CreateInstance(*key, instanceOf);
				}
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::TEXTMAP_BY_KEY:
		{
			ChildPtr<ClassInstanceRefByTextMapChip> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String key = ch1->GetText();
				ch0->CreateInstance(key, instanceOf);
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::TEXTMAP_BY_INDEX:
		{
			ChildPtr<ClassInstanceRefByTextMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize()) {
					const String *key = ch0->GetElementByIndex(index);
					assert(key);
					ch0->CreateInstance(*key, instanceOf);
				}
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	default:
		AddMessage(UninitializedException());
		break;
	}
}

void ClassInstanceRefContainerElementChip::SetInstance(ClassInstanceRef ref, AssignType at)
{
	switch (_ot) 
	{
	case OperatorType::ARRAY:
		{
			ChildPtr<ClassInstanceRefArrayChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize())
					ch0->SetInstance(index, ref, (ClassInstanceRefArrayChip::AssignType)at);
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::VALUEMAP_BY_KEY:
		{
			ChildPtr<ClassInstanceRefByValueMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				value key = ch1->GetValue();
				ch0->SetInstance(key, ref, (ClassInstanceRefByValueMapChip::AssignType)at);
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::VALUEMAP_BY_INDEX:
		{
			ChildPtr<ClassInstanceRefByValueMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize()) {
					const value *key = ch0->GetElementByIndex(index);
					assert(key);
					ch0->SetInstance(*key, ref, (ClassInstanceRefByValueMapChip::AssignType)at);
				}
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::TEXTMAP_BY_KEY:
		{
			ChildPtr<ClassInstanceRefByTextMapChip> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String key = ch1->GetText();
				ch0->SetInstance(key, ref, (ClassInstanceRefByTextMapChip::AssignType)at);
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::TEXTMAP_BY_INDEX:
		{
			ChildPtr<ClassInstanceRefByTextMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize()) {
					const String *key = ch0->GetElementByIndex(index);
					assert(key);
					ch0->SetInstance(*key, ref, (ClassInstanceRefByTextMapChip::AssignType)at);
				}
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	default:
		AddMessage(UninitializedException());
		break;
	}
}

void ClassInstanceRefContainerElementChip::ClearInstance()
{
	switch (_ot) 
	{
	case OperatorType::ARRAY:
		{
			ChildPtr<ClassInstanceRefArrayChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize())
					ch0->ClearInstance(index);
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::VALUEMAP_BY_KEY:
		{
			ChildPtr<ClassInstanceRefByValueMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				value key = ch1->GetValue();
				ch0->SetInstance(key, ClassInstanceRef());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::VALUEMAP_BY_INDEX:
		{
			ChildPtr<ClassInstanceRefByValueMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize()) {
					const value *key = ch0->GetElementByIndex(index);
					assert(key);
					ch0->SetInstance(*key, ClassInstanceRef());
				}
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::TEXTMAP_BY_KEY:
		{
			ChildPtr<ClassInstanceRefByTextMapChip> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String key = ch1->GetText();
				ch0->SetInstance(key, ClassInstanceRef());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::TEXTMAP_BY_INDEX:
		{
			ChildPtr<ClassInstanceRefByTextMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize()) {
					const String *key = ch0->GetElementByIndex(index);
					assert(key);
					ch0->SetInstance(*key, ClassInstanceRef());
				}
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	default:
		AddMessage(UninitializedException());
		break;
	}
}

void ClassInstanceRefContainerElementChip::MakeOwner()
{
	switch (_ot) 
	{
	case OperatorType::ARRAY:
		{
			ChildPtr<ClassInstanceRefArrayChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize())
					ch0->MakeOwner(index);
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::VALUEMAP_BY_KEY:
		{
			ChildPtr<ClassInstanceRefByValueMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				value key = ch1->GetValue();
				ch0->MakeOwner(key);
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::VALUEMAP_BY_INDEX:
		{
			ChildPtr<ClassInstanceRefByValueMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize()) {
					const value *key = ch0->GetElementByIndex(index);
					assert(key);
					ch0->MakeOwner(*key);
				}
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::TEXTMAP_BY_KEY:
		{
			ChildPtr<ClassInstanceRefByTextMapChip> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String key = ch1->GetText();
				ch0->MakeOwner(key);
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	case OperatorType::TEXTMAP_BY_INDEX:
		{
			ChildPtr<ClassInstanceRefByTextMapChip> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				uint32 index = uint32(ch1->GetValue());
				if (index < ch0->GetContainerSize()) {
					const String *key = ch0->GetElementByIndex(index);
					assert(key);
					ch0->MakeOwner(*key);
				}
				else
					AddMessage(ContainerChip::IndexOutOfBoundsException());
			}
			else
				AddMessage(MissingChildException(0));
		}
		break;
	default:
		AddMessage(UninitializedException());
		break;
	}
}

void ClassInstanceRefContainerElementChip::SetOperatorType(OperatorType ot)
{
	if (_ot == ot)
		return;
	_ot = ot;
	switch (_ot) 
	{
	case OperatorType::ARRAY:
		CREATE_CHILD_KEEP(0, CLASSINSTANCEREFARRAYCHIP_GUID, false, BOTH, MTEXT("Array"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Array Index"));
		ClearConnections(2);
		break;
	case OperatorType::VALUEMAP_BY_KEY:
		CREATE_CHILD_KEEP(0, CLASSINSTANCEREFBYVALUEMAPCHIP_GUID, false, BOTH, MTEXT("Map"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Map Key"));
		ClearConnections(2);
		break;
	case OperatorType::VALUEMAP_BY_INDEX:
		CREATE_CHILD_KEEP(0, CLASSINSTANCEREFBYVALUEMAPCHIP_GUID, false, BOTH, MTEXT("Map"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Element Index"));
		ClearConnections(2);
		break;
	case OperatorType::TEXTMAP_BY_KEY:
		CREATE_CHILD_KEEP(0, CLASSINSTANCEREFBYTEXTMAPCHIP_GUID, false, BOTH, MTEXT("Map"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Map Key"));
		ClearConnections(2);
		break;
	case OperatorType::TEXTMAP_BY_INDEX:
		CREATE_CHILD_KEEP(0, CLASSINSTANCEREFBYTEXTMAPCHIP_GUID, false, BOTH, MTEXT("Map"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Element Index"));
		ClearConnections(2);
		break;
	default:
		break;
	}
	RemoveMessage(UninitializedException());
}