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
#include "ValueOperator.h"
#include "VectorChip.h"
#include "Text.h"
#include "ClassInstanceRefChip.h"
#include "ClassChip.h"
#include "Envelope.h"
#include "SetChip.h"
#include "M3DCore/HighPrecisionTimer.h"
#include "M3DEngine/Application.h"
#include "M3DEngine/ClassInstance.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/DocumentSaver.h"
#include "M3DEngine/DocumentLoader.h"
#include "M3DEngine/Engine.h"
#include <ctime>


using namespace m3d;


CHIPDESCV1_DEF(ValueOperator, MTEXT("Value Operator"), VALUEOPERATOR_GUID, VALUE_GUID);


ValueOperator::ValueOperator()
{
	ClearConnections();
	_ot = OperatorType::NONE;
}

ValueOperator::~ValueOperator()
{
}

bool ValueOperator::CopyChip(Chip *chip)
{
	ValueOperator *c = dynamic_cast<ValueOperator*>(chip);
	B_RETURN(Value::CopyChip(c));
	SetOperatorType(c->_ot);
	return true;
}

bool ValueOperator::LoadChip(DocumentLoader &loader) 
{ 
	B_RETURN(Value::LoadChip(loader));
	OperatorType ot;
	LOAD(MTEXT("operatorType|ot"), ot);
	SetOperatorType(ot);
	return true;
}

bool ValueOperator::SaveChip(DocumentSaver &saver) const 
{
	B_RETURN(Value::SaveChip(saver));
	SAVE(MTEXT("operatorType"), _ot);
	return true;
}

value ValueOperator::GetValue()
{
	RefreshT refresh(Refresh);
	if (!refresh)
		return _value;

	switch (_ot) 
	{
	case OperatorType::DT:
		_value = (value)engine->GetDt() / 1000000.0;
		break;
	case OperatorType::FPS:
		_value = (value)engine->GetFPS().GetFPS();
		break;
	case OperatorType::GET_UPDATE_STAMP:
		{
			ChipChildPtr ch0 = GetChild(0);
			_value = ch0 ? (value)ch0->GetUpdateStamp() : 0;
		}
		break;
	case OperatorType::GET_CMD_LINE_ARGUMENT_COUNT:
		_value = (value)engine->GetCmdLineArguments().size();
		break;
	case OperatorType::GET_RUNTIME_ENV:
		_value = (value)engine->GetApplication()->GetExeEnvironment();
		break;
	case OperatorType::VECTOR_X:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			_value = ch0 ? ch0->GetVector().x : 0.0f;
		}
		break;
	case OperatorType::VECTOR_Y:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			_value = ch0 ? ch0->GetVector().y : 0.0f;
		}
		break;
	case OperatorType::VECTOR_Z:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			_value = ch0 ? ch0->GetVector().z : 0.0f;
		}
		break;
	case OperatorType::VECTOR_W:
		{
			ChildPtr<VectorChip> ch0 = GetChild(0);
			_value = ch0 ? ch0->GetVector().w : 0.0f;
		}
		break;
	case OperatorType::VECTOR_LENGTH2:
		{
			_value = 0.0f;
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				_value = XMVectorGetX(XMVector2Length(XMLoadFloat4(&p0)));
			}
		}
		break;
	case OperatorType::VECTOR_LENGTH3:
		{
			_value = 0.0f;
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				_value = XMVectorGetX(XMVector3Length(XMLoadFloat4(&p0)));
			}
		}
		break;
	case OperatorType::VECTOR_LENGTH4:
		{
			_value = 0.0f;
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				_value = XMVectorGetX(XMVector4Length(XMLoadFloat4(&p0)));
			}
		}
		break;
	case OperatorType::VECTOR_LENGTHSQ2:
		{
			_value = 0.0f;
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				_value = XMVectorGetX(XMVector2LengthSq(XMLoadFloat4(&p0)));
			}
		}
		break;
	case OperatorType::VECTOR_LENGTHSQ3:
		{
			_value = 0.0f;
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				_value = XMVectorGetX(XMVector3LengthSq(XMLoadFloat4(&p0)));
			}
		}
		break;
	case OperatorType::VECTOR_LENGTHSQ4:
		{
			_value = 0.0f;
			ChildPtr<VectorChip> ch0 = GetChild(0);
			if (ch0) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				_value = XMVectorGetX(XMVector4LengthSq(XMLoadFloat4(&p0)));
			}
		}
		break;
	case OperatorType::VECTOR_DOT2:
		{
			_value = 0.0f;
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<VectorChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				const XMFLOAT4 &p1 = ch1->GetVector();
				_value = XMVectorGetX(XMVector2Dot(XMLoadFloat4(&p0), XMLoadFloat4(&p1)));
			}
		}
		break;
	case OperatorType::VECTOR_DOT3:
		{
			_value = 0.0f;
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<VectorChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				const XMFLOAT4 &p1 = ch1->GetVector();
				_value = XMVectorGetX(XMVector3Dot(XMLoadFloat4(&p0), XMLoadFloat4(&p1)));
			}
		}
		break;
	case OperatorType::VECTOR_DOT4:
		{
			_value = 0.0f;
			ChildPtr<VectorChip> ch0 = GetChild(0);
			ChildPtr<VectorChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				const XMFLOAT4 &p0 = ch0->GetVector();
				const XMFLOAT4 &p1 = ch1->GetVector();
				_value = XMVectorGetX(XMVector4Dot(XMLoadFloat4(&p0), XMLoadFloat4(&p1)));
			}
		}
		break;
	case OperatorType::LIMIT_RELATIVE: // fall through
	case OperatorType::LIMIT_ABS:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			if (ch0) {
				if (_ot == OperatorType::LIMIT_RELATIVE)
					_value += ch0->GetValue();
				else
					_value = ch0->GetValue();
				if (ch1) {
					value f =  ch1->GetValue();
					_value = XMMax(_value, f);
				}
				if (ch2) {
					value f = ch2->GetValue();
					_value = XMMin(_value, f);
				}
			}
			AddMessage(DeprecatedFeatureException());
	}
		break;
	case OperatorType::LOOP_RELATIVE:
		{
			ChildPtr<Value> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			ChildPtr<Value> ch2 = GetChild(2);
			if (ch0) {
				_value += ch0->GetValue();
				if (ch1 && ch2) {
					value f1 =  ch1->GetValue();
					value f2 = ch2->GetValue();
					if (_value < f1)
						_value = f2 + (_value - f1);
					else if (_value > f2)
						_value = f1 + (_value - f2);
				}
			}
			AddMessage(DeprecatedFeatureException());
	}
		break;
	case OperatorType::ENVELOPE_GET_DERIVATIVE:
		{
			ChildPtr<Envelope> ch0 = GetChild(0);
			_value = ch0 ? ch0->GetDerivative() : 0.0;
		}
		break;
	case OperatorType::TEXT_LENGTH:
		{
			_value = 0.0f;
			ChildPtr<Text> ch0 = GetChild(0);
			if (ch0)
				_value = (value)ch0->GetText().length();
		}
		break;
	case OperatorType::TEXT_COMPARE:
		{
			_value = 0.0f;
			ChildPtr<Text> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String s0 = ch0->GetText();
				String s1 = ch1->GetText();
				_value = (value)s0.compare(s1);
			}
		}
		break;
	case OperatorType::TEXT_VALUE:
		{
			_value = 0.0f;
			ChildPtr<Text> ch0 = GetChild(0);
			if (ch0) {
				String s = ch0->GetText();
				strUtils::toNum(s, _value); // Should we be setting to nan if fail?
			}
		}
		break;
	case OperatorType::INSTANCEREF_EXIST:
		{
			ChildPtr<ClassInstanceRefChip> ch0 = GetChild(0);
			_value = ch0 && ch0->GetInstance() ? 1.0 : 0.0;
		}
		break;
	case OperatorType::INSTANCEREF_EQUALS:
		{
			_value = 0.0;
			ChildPtr<ClassInstanceRefChip> ch0 = GetChild(0);
			ChildPtr<ClassInstanceRefChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				ClassInstanceRef r0 = ch0->GetInstance();
				ClassInstanceRef r1 = ch1->GetInstance();
				_value = r0 == r1 ? 1.0 : 0.0;
			}
		}
		break;
	case OperatorType::INSTANCEREF_TYPE_OF:
		{
			_value = 0.0;
			ChildPtr<ClassInstanceRefChip> ch0 = GetChild(0);
			ChildPtr<ClassChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				ClassInstanceRef ref = ch0->GetInstance();
				_value = ref && ref->GetClass() == ch1->GetCG() ? 1.0 : 0.0;
			}
		}
		break;
	case OperatorType::INSTANCEREF_TYPE_COMPATIBLE:
		{
			_value = 0.0;
			ChildPtr<ClassInstanceRefChip> ch0 = GetChild(0);
			ChildPtr<ClassChip> ch1 = GetChild(1);
			if (ch0 && ch1) {
				ClassInstanceRef ref = ch0->GetInstance();
				Class *cg = ch1->GetCG();
				_value = ref && cg && ref->GetClass()->IsBaseClass(cg) ? 1.0 : 0.0;
			}
		}
		break;
	case OperatorType::INSTANCEREF_IS_OWNER:
		{
			ChildPtr<ClassInstanceRefChip> ch0 = GetChild(0);
			_value = ch0 && ch0->GetInstance().IsOwner() ? 1.0 : 0.0;
		}
		break;
	case OperatorType::INSTANCEREF_GET_ID:
		{
			_value = 0.0;
			ChildPtr<ClassInstanceRefChip> ch0 = GetChild(0);
			if (ch0) {
				ClassInstanceRef ref = ch0->GetInstance();
				_value = ref ? (value)ref->GetRuntimeID() : 0.0;
			}
		}
		break;
	case OperatorType::VALUESET_KEY_EXIST:
		{
			ChildPtr<SetChipInterface<value>> ch0 = GetChild(0);
			ChildPtr<Value> ch1 = GetChild(1);
			if (ch0 && ch1) {
				value key = ch1->GetValue();
				_value = ch0->HasElement(key) ? 1.0 : 0.0;
			}
		}
		break;
	case OperatorType::TEXTSET_KEY_EXIST:
		{
			ChildPtr<SetChipInterface<String>> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String key = ch1->GetText();
				_value = ch0->HasElement(key) ? 1.0 : 0.0;
			}
		}
		break;
	case OperatorType::UTC_TIME:
		{
			time_t t = time(nullptr);
			_value = (value)t;
		}
		break;
	case OperatorType::UTC_TO_SECONDS_SINCE_EPOCH:
	case OperatorType::LOCAL_TO_SECONDS_SINCE_EPOCH:
	{
			tm TM = { 0, 0, 0, 1, 0, 0, 0, 0 };
			ChildPtr<Value> ch = GetChild(0);
			if (ch) TM.tm_year = (int)ch->GetValue() - 1900;
			ch = GetChild(1);
			if (ch) TM.tm_mon = (int)ch->GetValue() - 1;
			ch = GetChild(2);
			if (ch) TM.tm_mday = (int)ch->GetValue();
			ch = GetChild(3);
			if (ch) TM.tm_hour = (int)ch->GetValue();
			ch = GetChild(4);
			if (ch) TM.tm_min = (int)ch->GetValue();
			ch = GetChild(5);
			if (ch) TM.tm_sec = (int)ch->GetValue();
			ch = GetChild(6);
			if (ch) TM.tm_wday = int((uint32)ch->GetValue() % 7);
			ch = GetChild(8);
			if (ch) {
				value dst = ch->GetValue();
				if (dst < 0)
					TM.tm_isdst = -1;
				else if (dst > 0)
					TM.tm_isdst = 1;
				else
					TM.tm_isdst = 0;
			}
			time_t t = 0;
			if (_ot == OperatorType::UTC_TO_SECONDS_SINCE_EPOCH)
				t = _mkgmtime64(&TM);
			else
				t = mktime(&TM);
			ch = GetChild(7);
			if (ch) ch->SetValue(TM.tm_yday);
			_value = (value)t;
		}
		break;
	default:
		AddMessage(UninitializedException());
		break;
	}

	return _value;
}

void ValueOperator::SetValue(value v)
{
	switch (_ot)
	{
	case OperatorType::UTC_TO_SECONDS_SINCE_EPOCH:
	case OperatorType::LOCAL_TO_SECONDS_SINCE_EPOCH:
	{
		float64 d = v;// std::max(v, 0.0);
		_value = v;
		time_t t = (time_t)d;
		tm TM = { 0 };
		if (_ot == OperatorType::UTC_TO_SECONDS_SINCE_EPOCH)
			gmtime_s(&TM, &t);
		else
			localtime_s(&TM, &t);
		ChildPtr<Value> ch = GetChild(0);
		if (ch) ch->SetValue(value(1900ll + TM.tm_year));
		ch = GetChild(1);
		if (ch) ch->SetValue(value(1ll + TM.tm_mon));
		ch = GetChild(2);
		if (ch) ch->SetValue(TM.tm_mday);
		ch = GetChild(3);
		if (ch) ch->SetValue(TM.tm_hour);
		ch = GetChild(4);
		if (ch) ch->SetValue(TM.tm_min);
		ch = GetChild(5);
		if (ch) ch->SetValue(TM.tm_sec);
		ch = GetChild(6);
		if (ch) ch->SetValue(TM.tm_wday == 0 ? 7 : TM.tm_wday);
		ch = GetChild(7);
		if (ch) ch->SetValue(TM.tm_yday);
		ch = GetChild(8);
		if (ch) ch->SetValue(TM.tm_isdst ? 1.0f : 0.0f);
		break;
	}
	default:
		break;
	}
}

void ValueOperator::SetOperatorType(OperatorType ot)
{ 
	if (_ot == ot)
		return;
	_ot = ot; 

	switch (_ot) 
	{
	case OperatorType::DT:
	case OperatorType::FPS:
	case OperatorType::GET_CMD_LINE_ARGUMENT_COUNT:
	case OperatorType::GET_RUNTIME_ENV:
		ClearConnections();
		break;
	case OperatorType::GET_UPDATE_STAMP:
		CREATE_CHILD_KEEP(0, CHIP_GUID, false, UP, MTEXT("Chip"));
		ClearConnections(1);
		break;
	case OperatorType::VECTOR_X:
	case OperatorType::VECTOR_Y:
	case OperatorType::VECTOR_Z:
	case OperatorType::VECTOR_W:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Vector"));
		ClearConnections(1);
		break;
	case OperatorType::VECTOR_LENGTH2:
	case OperatorType::VECTOR_LENGTH3:
	case OperatorType::VECTOR_LENGTH4:
	case OperatorType::VECTOR_LENGTHSQ2:
	case OperatorType::VECTOR_LENGTHSQ3:
	case OperatorType::VECTOR_LENGTHSQ4:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Vector"));
		ClearConnections(1);
		break;
	case OperatorType::VECTOR_DOT2:
	case OperatorType::VECTOR_DOT3:
	case OperatorType::VECTOR_DOT4:
		CREATE_CHILD_KEEP(0, VECTORCHIP_GUID, false, UP, MTEXT("Vector 1"));
		CREATE_CHILD_KEEP(1, VECTORCHIP_GUID, false, UP, MTEXT("Vector 2"));
		ClearConnections(2);
		break;
	case OperatorType::LIMIT_RELATIVE:
	case OperatorType::LIMIT_ABS:
	case OperatorType::LOOP_RELATIVE:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, UP, MTEXT("Value"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Min"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, UP, MTEXT("Max"));
		ClearConnections(3);
		break;
	case OperatorType::ENVELOPE_GET_DERIVATIVE:
		CREATE_CHILD_KEEP(0, ENVELOPE_GUID, false, UP, MTEXT("Envelope"));
		ClearConnections(1);
		break;
	case OperatorType::TEXT_LENGTH:
	case OperatorType::TEXT_VALUE:
		CREATE_CHILD_KEEP(0, TEXT_GUID, false, UP, MTEXT("Text"));
		ClearConnections(1);
		break;
	case OperatorType::TEXT_COMPARE:
		CREATE_CHILD_KEEP(0, TEXT_GUID, false, UP, MTEXT("Text 1"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Text 2"));
		ClearConnections(2);
		break;
	case OperatorType::INSTANCEREF_EXIST:
	case OperatorType::INSTANCEREF_IS_OWNER:
		CREATE_CHILD_KEEP(0, CLASSINSTANCEREFCHIP_GUID, false, UP, MTEXT("InstanceRef"));
		ClearConnections(1);
		break;
	case OperatorType::INSTANCEREF_EQUALS:
		CREATE_CHILD_KEEP(0, CLASSINSTANCEREFCHIP_GUID, false, UP, MTEXT("InstanceRef 1"));
		CREATE_CHILD_KEEP(1, CLASSINSTANCEREFCHIP_GUID, false, UP, MTEXT("InstanceRef 2"));
		ClearConnections(2);
		break;
	case OperatorType::INSTANCEREF_TYPE_OF:
	case OperatorType::INSTANCEREF_TYPE_COMPATIBLE:
		CREATE_CHILD_KEEP(0, CLASSINSTANCEREFCHIP_GUID, false, UP, MTEXT("InstanceRef"));
		CREATE_CHILD_KEEP(1, CLASSCHIP_GUID, false, UP, MTEXT("Class"));
		ClearConnections(2);
		break;
	case OperatorType::VALUESET_KEY_EXIST:
		CREATE_CHILD_KEEP(0, VALUESET_GUID, false, UP, MTEXT("Value Set"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, UP, MTEXT("Key"));
		ClearConnections(2);
		break;
	case OperatorType::TEXTSET_KEY_EXIST:
		CREATE_CHILD_KEEP(0, TEXTSET_GUID, false, UP, MTEXT("Text Set"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Key"));
		ClearConnections(2);
		break;
	case OperatorType::UTC_TIME:
		ClearConnections(0);
		break;
	case OperatorType::UTC_TO_SECONDS_SINCE_EPOCH:
	case OperatorType::LOCAL_TO_SECONDS_SINCE_EPOCH:
		CREATE_CHILD_KEEP(0, VALUE_GUID, false, BOTH, MTEXT("Year (1900-)"));
		CREATE_CHILD_KEEP(1, VALUE_GUID, false, BOTH, MTEXT("Month (1-12)"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, BOTH, MTEXT("Day of Month (1-31)"));
		CREATE_CHILD_KEEP(3, VALUE_GUID, false, BOTH, MTEXT("Hour of Day (0-23)"));
		CREATE_CHILD_KEEP(4, VALUE_GUID, false, BOTH, MTEXT("Minute of Hour (0-59)"));
		CREATE_CHILD_KEEP(5, VALUE_GUID, false, BOTH, MTEXT("Seconds of Minute (0-59)"));
		CREATE_CHILD_KEEP(6, VALUE_GUID, false, BOTH, MTEXT("Weekday (1-7: Mon-Sun)"));
		CREATE_CHILD_KEEP(7, VALUE_GUID, false, DOWN, MTEXT("Day of Year (0-365)"));
		CREATE_CHILD_KEEP(8, VALUE_GUID, false, BOTH, MTEXT("Is daylight savings (0/1)"));
		ClearConnections(9);
	break;
	default:
		ClearConnections();
		break;
	}
	RemoveMessage(UninitializedException());
}
