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
#include "XMLValue.h"
#include "XMLElement.h"
#include "Text.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"


using namespace m3d;


CHIPDESCV1_DEF(XMLValue, MTEXT("XML Value"), XMLVALUE_GUID, VALUE_GUID);


XMLValue::XMLValue()
{
	_ot = OperatorType::NONE;

	ClearConnections();
}

XMLValue::~XMLValue()
{
}

bool XMLValue::CopyChip(Chip *chip)
{
	XMLValue *c = dynamic_cast<XMLValue*>(chip);
	B_RETURN(Chip::CopyChip(c));
	SetOperatorType(c->_ot);
	return true;
}

bool XMLValue::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	OperatorType ot;
	LOAD(MTEXT("operatorType|ot"), ot);
	SetOperatorType(ot);
	return true;
}

bool XMLValue::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE(MTEXT("operatorType"), _ot);
	return true;
}

value XMLValue::GetValue()
{
	RefreshT refresh(Refresh);
	if (!refresh)
		return _value;
	_value = 0.0f;

	switch (_ot) 
	{
	case OperatorType::IS_ELEMENT:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			_value = ch0 && ch0->GetNode() != nullptr ? 1.0 : 0.0;
		}
		break;
	case OperatorType::IS_ROOT:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			if (ch0) {
				xmlNodePtr n = ch0->GetNode();
				_value = n && xmlDocGetRootElement(n->doc) == n ? 1.0f : 0.0f;
			}
		}
		break;
	case OperatorType::CHILD_COUNT:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
			_value = n ? value(xmlChildElementCount(n)) : 0.0f;
		}
		break;
	case OperatorType::GET_ATTRIBUTE_COUNT:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
			_value = 0.0f;
			for (n = n ? n->children : nullptr; n != nullptr; n = n->next)
				if (n->type == XML_ATTRIBUTE_NODE)
					_value++;
		}
		break;
	case OperatorType::HAS_ATTRIBUTE:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
			ChildPtr<Text> ch1 = GetChild(1);
			if (n && ch1) {
				String s = ch1->GetText();
				xmlAttrPtr p = xmlHasProp(n, BAD_CAST s.c_str());
				_value = p != nullptr ? 1.0f : 0.0f;
			}
		}
		break;
	case OperatorType::CONTENT:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
			if (n) {
				xmlChar *r = xmlNodeGetContent(n);
				if (r) {
					strUtils::toNum(String((const Char*)r), _value);
					xmlFree(r);
				}
			}
		}
		break;
	case OperatorType::ATTRIBUTE:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
			ChildPtr<Text> ch1 = GetChild(1);
			if (n && ch1) {
				String s = ch1->GetText();
				xmlChar *r = xmlGetProp(n, BAD_CAST s.c_str());
				if (r) {
					strUtils::toNum(String((const Char*)r), _value);
					xmlFree(r);
				}
			}
		}
		break;
	case OperatorType::QUERY:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String s = ch1->GetText();
				xmlXPathObjectPtr result = ch0->DoXPathQuery(s);
				if (result) {
					switch (result->type) 
					{
					case XPATH_NODESET:
						if (result->nodesetval && result->nodesetval->nodeNr > 0) {
							xmlNodePtr n = result->nodesetval->nodeTab[0];
							xmlChar *t = xmlXPathCastNodeToString(n);
							if (t) {
								strUtils::toNum(String((Char*)t), _value);
							}
							xmlFree(t);
						}
						break;
					case XPATH_BOOLEAN:
						_value = result->boolval ? 1.0 : 0.0;
						break;
					case XPATH_NUMBER:
						_value = value(result->floatval);
						break;
					case XPATH_STRING:
						strUtils::toNum(String((Char*)result->stringval), _value);
						break;
					default:
						break;
					}
					xmlFree(result);
				}
			}
		}
		break;
	default:
		if (_ot == OperatorType::NONE)
			AddMessage(UninitializedException());
		else 
			AddMessage(UnsupportedOperationException());
		break;
	}
	return _value;
}

void XMLValue::SetValue(value v)
{
	_value = v;
	switch (_ot) 
	{
	case OperatorType::CONTENT:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
			if (n)
				xmlNodeSetContent(n, BAD_CAST strUtils::fromNum(_value).c_str());
		}
		break;
	case OperatorType::ATTRIBUTE:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
			ChildPtr<Text> ch1 = GetChild(1);
			if (n && ch1) {
				String s = ch1->GetText();
				xmlSetProp(n, BAD_CAST s.c_str(), BAD_CAST strUtils::fromNum(_value).c_str());
			}
		}
		break;
	default:
		if (_ot == OperatorType::NONE)
			AddMessage(UninitializedException());
		else
			AddMessage(UnsupportedOperationException());
		break;
	}
}

void XMLValue::SetOperatorType(OperatorType ot)
{
	if (ot == _ot)
		return;
	_ot = ot;
	switch (_ot) 
	{
	case OperatorType::IS_ELEMENT:
	case OperatorType::IS_ROOT:
	case OperatorType::CHILD_COUNT:
	case OperatorType::GET_ATTRIBUTE_COUNT:
	case OperatorType::CONTENT:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, UP, MTEXT("XML Element"));
		ClearConnections(1);
		break;
	case OperatorType::HAS_ATTRIBUTE:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, UP, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Attribute Name"));
		ClearConnections(2);
		break;
	case OperatorType::ATTRIBUTE:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, BOTH, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Attribute Name"));
		ClearConnections(2);
		break;
	case OperatorType::QUERY:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, UP, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("XPath Query"));
		ClearConnections(2);
		break;
	default:
		ClearConnections();
		break;
	}
	RemoveMessage(UninitializedException());
}
