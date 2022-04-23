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
#include "TextOperator.h"
#include "Value.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"
#include "ClassInstanceRefChip.h"
#include "M3DEngine/ClassInstance.h"
#include "M3DEngine/Application.h"
#include "M3DEngine/Document.h"

using namespace m3d;


CHIPDESCV1_DEF(TextOperator, MTEXT("Text Operator"), TEXTOPERATOR_GUID, TEXT_GUID);


TextOperator::TextOperator()
{
	_ot = OperatorType::NONE;
}

TextOperator::~TextOperator()
{
}

bool TextOperator::CopyChip(Chip *chip)
{
	TextOperator *c = dynamic_cast<TextOperator*>(chip);
	B_RETURN(Text::CopyChip(c));
	SetOperatorType(c->_ot);
	return true;
}

bool TextOperator::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Text::LoadChip(loader));
	OperatorType ot;
	LOAD(MTEXT("operatorType|ot"), ot);
	SetOperatorType(ot);
	return true;
}

bool TextOperator::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Text::SaveChip(saver));
	SAVE(MTEXT("operatorType"), _ot);
	return true;
}

String TextOperator::GetText()
{
	RefreshT refresh(Refresh);
	if (!refresh)
		return Text::GetText();

	String result;

	switch (_ot) 
	{
	case OperatorType::SUBTEXT:
		{
			ChildPtr<Text> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			String s = ch0 ? ch0->GetText() : MTEXT("");
			uint32 a = ch1 ? (uint32)ch1->GetValue() : 0;
			uint32 b = ch2 ? (uint32)ch2->GetValue() : ((uint32)s.length() - a);
			result = s.substr(a, b);
		}
		break;
	case OperatorType::MERGE_TEXT:
		{
			String s;
			for (uint32 i = 0, j = GetSubConnectionCount(0); i < j; i++) {
				ChildPtr<Text> ch0 = GetChild(0, i);
				if (ch0) {
					String s0 = ch0->GetText();
					s += s0;
				}
			}
			result = s;
		}
		break;
	case OperatorType::REPLACE_TEXT:
		AddMessage(NotImplementedException(MTEXT("REPLACE_TEXT")));
		break;
	case OperatorType::UPPERCASE:
		{
			ChildPtr<Text> ch0 = GetChild(0);
			if (ch0) {
				String s0 = ch0->GetText();
				result = strUtils::toUpper(s0);
			}
		}
		break;
	case OperatorType::LOWERCASE:
		{
			ChildPtr<Text> ch0 = GetChild(0);
			if (ch0) {
				String s0 = ch0->GetText();
				result = strUtils::toLower(s0);
			}
		}
		break;
	case OperatorType::VALUE_TO_STRING:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			if (ch0) {
				result = strUtils::fromNum(ch0->GetValue());
			}
		}
		break;
	case OperatorType::GET_COMPUTER_NAME:
		{
#ifdef WINDESKTOP
			Char chrComputerName[MAX_COMPUTERNAME_LENGTH + 1];  
			DWORD dwBufferSize = MAX_COMPUTERNAME_LENGTH + 1;  
			if(GetComputerNameA(chrComputerName,&dwBufferSize))
				result = chrComputerName;
#endif
		}
		break;
	case OperatorType::XET_ENVIRONMENT_VARIABLE:
		{
#ifdef WINDESKTOP
			ChildPtr<Text> ch0 = GetChild(0);
			if (ch0) {
				String s = ch0->GetText();
				Char buff[32767];
				if (GetEnvironmentVariableA(s.c_str(), buff, 32767) != 0) {
					result = buff;
				}
			}
#endif
		}
		break;
	case OperatorType::GET_CMD_LINE_ARGUMENT:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			if (ch0) {
				uint32 i = (uint32)ch0->GetValue();
				if (i < engine->GetCmdLineArguments().size())
					result = engine->GetCmdLineArguments()[i];
			}
		}
		break;
	case OperatorType::XET_REGISTRY_VALUE:
		{
#ifdef WINDESKTOP
			ChildPtr<Text> ch0 = GetChild(0);
			if (ch0) { // TODO: Implement!
				/*
				String keyString = MTEXT("Software\\FVM\\ProjectXEditor");
				String s = ch0->GetText().trimmed(L'\\');
				size_t i = s.find_last_of(MTEXT("\\"));
				String valueString;
				if (i == -1)
					valueString = s;
				else {
					valueString = s.substr(i + 1);
					keyString += MTEXT("\\") + s.substr(0, i);
				}
				HKEY key;
				if (RegOpenKeyEx(HKEY_CURRENT_USER, keyString.c_str(), 0, KEY_READ, &key) == ERROR_SUCCESS) {
					DWORD size = 256;
					DWORD type = 0;
					if (RegGetValue(key, nullptr, valueString.c_str(), RRF_RT_REG_SZ, &type, nullptr, &size) == ERROR_SUCCESS) {
					//	String str;
					//	str.append(
					//RegGetValue(key, nullptr, valueString.c_str(),  RRF_RT_REG_SZ, nullptr, &ty, &si);
					}
				}
				*/
			}
#endif
		}
		break;
	case OperatorType::FILESYSTEM_GET_PROJECT_START_FILE:
		{
			Class *start = engine->GetClassManager()->GetStartClass();
			if (start)
				result = start->GetDocument()->GetFileName().AsString();
		}
		break;
	case OperatorType::FILESYSTEM_GET_EXE_FILE:
		result = engine->GetApplication()->GetExeFile().AsString();
		break;
	case OperatorType::FILESYSTEM_GET_DIR_FROM_FILE_PATH:
		{
			ChildPtr<Text> ch0 = GetChild(0);
			if (ch0) {
				Path p(ch0->GetText());
				if (p.IsValid())
					result = p.GetDirectory().AsString();
			}
		}
		break;
	case OperatorType::FILESYSTEM_GET_FILE_NAME:
		{
			ChildPtr<Text> ch0 = GetChild(0);
			if (ch0) {
				Path p(ch0->GetText());
				if (p.IsValid())
					result = p.GetName();
			}
		}
		break;
	case OperatorType::FILESYSTEM_GET_FILE_NAME_EXCL_EXT:
		{
			ChildPtr<Text> ch0 = GetChild(0);
			if (ch0) {
				Path p(ch0->GetText());
				if (p.IsValid())
					result = p.GetFileNameWithoutExtention();
			}
		}
		break;
	case OperatorType::FILESYSTEM_GET_FILE_EXT:
		{
			ChildPtr<Text> ch0 = GetChild(0);
			if (ch0) {
				Path p(ch0->GetText());
				if (p.IsValid())
					result = p.GetFileExtention();
			}
		}
		break;
	case OperatorType::FILESYSTEM_GET_ABSOLUTE_FILE_PATH:
		{
			ChildPtr<Text> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				Path q(ch1->GetText());
				if (q.IsValid()) {
					Path p(ch0->GetText(), q);
					if (p.IsValid())
						result = p.AsString();
				}
			}
		}
		break;
	case OperatorType::XET_INSTANCE_NAME:
		{
			ChildPtr<ClassInstanceRefChip> ch0 = GetChild(0);
			if (ch0) {
				ClassInstanceRef ref = ch0->GetInstance();
				if (ref)
					result = ref->GetName();
			}
		}
		break;
	default:
		AddMessage(_ot == OperatorType::NONE ? (ChipMessage)UninitializedException() : (ChipMessage)UnsupportedOperationException());
		break;
	};

	Text::SetText(result);

	return Text::GetText();
}

void TextOperator::SetText(String text) 
{
	switch (_ot) 
	{
	case OperatorType::XET_ENVIRONMENT_VARIABLE:
		{
#ifdef WINDESKTOP
			ChildPtr<Text> ch0 = GetChild(0);
			if (ch0) {
				String s = ch0->GetText();
				if (text.size() < 32767 && SetEnvironmentVariableA(s.c_str(), text.c_str()))
					Text::SetText(text); // success!
			}
#endif
		}
		break;
	case OperatorType::XET_INSTANCE_NAME:
		{
			ChildPtr<ClassInstanceRefChip> ch0 = GetChild(0);
			if (ch0) {
				ClassInstanceRef ref = ch0->GetInstance();
				if (ref) {
					ref->SetName(text);
					Text::SetText(text);
				}
			}
		}
		break;
	default:
		AddMessage(_ot == OperatorType::NONE ? (ChipMessage)UninitializedException() : (ChipMessage)UnsupportedOperationException());
		break;
	}
}

void TextOperator::SetOperatorType(OperatorType ot)
{
	if (_ot == ot)
		return;
	_ot = ot;
	switch (_ot) 
	{
	case OperatorType::SUBTEXT:
		CREATE_CHILD_KEEP(0, TEXT_GUID, false, UP, MTEXT("Text"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Offset"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("Count"));
		ClearConnections(3);
		break;
	case OperatorType::MERGE_TEXT:
		CREATE_CHILD_KEEP(0, TEXT_GUID, true, UP, MTEXT("Text"));
		ClearConnections(1);
		break;
	case OperatorType::REPLACE_TEXT:
		CREATE_CHILD_KEEP(0, TEXT_GUID, false, UP, MTEXT("Text"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("To be Replaced"));
		CREATE_CHILD_KEEP(2, TEXT_GUID, false, UP, MTEXT("Replace with"));
		ClearConnections(3);
		break;
	case OperatorType::UPPERCASE:
	case OperatorType::LOWERCASE:
		CREATE_CHILD_KEEP(0, TEXT_GUID, false, UP, MTEXT("Text"));
		ClearConnections(1);
		break;
	case OperatorType::VALUE_TO_STRING:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("value"));
		ClearConnections(1);
		break;
	case OperatorType::GET_COMPUTER_NAME:
		ClearConnections(0);
		break;
	case OperatorType::XET_ENVIRONMENT_VARIABLE:
		CREATE_CHILD_KEEP(0, TEXT_GUID, false, UP, MTEXT("Name"));
		ClearConnections(1);
		break;
	case OperatorType::GET_CMD_LINE_ARGUMENT:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("Index"));
		ClearConnections(1);
		break;
	case OperatorType::XET_REGISTRY_VALUE:
		CREATE_CHILD_KEEP(0, TEXT_GUID, false, UP, MTEXT("Name"));
		ClearConnections(1);
		break;
	case OperatorType::FILESYSTEM_GET_PROJECT_START_FILE:
	case OperatorType::FILESYSTEM_GET_EXE_FILE:
		ClearConnections(0);
		break;
	case OperatorType::FILESYSTEM_GET_DIR_FROM_FILE_PATH:
	case OperatorType::FILESYSTEM_GET_FILE_NAME:
	case OperatorType::FILESYSTEM_GET_FILE_NAME_EXCL_EXT:
	case OperatorType::FILESYSTEM_GET_FILE_EXT:
		CREATE_CHILD_KEEP(0, TEXT_GUID, false, UP, MTEXT("File Path"));
		ClearConnections(1);
		break;
	case OperatorType::FILESYSTEM_GET_ABSOLUTE_FILE_PATH:
		CREATE_CHILD_KEEP(0, TEXT_GUID, false, UP, MTEXT("Relative File Path"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Absolute File Path"));
		ClearConnections(2);
		break;
	case OperatorType::XET_INSTANCE_NAME:
		CREATE_CHILD_KEEP(0, CLASSINSTANCEREFCHIP_GUID, false, BOTH, MTEXT("Instance"));
		ClearConnections(1);
		break;
	default:
		ClearConnections();
		break;
	};
	RemoveMessage(UninitializedException());
}