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
#include "XMLText.h"
#include "XMLElement.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"


using namespace m3d;


CHIPDESCV1_DEF(XMLText, MTEXT("XML Text"), XMLTEXT_GUID, TEXT_GUID);


XMLText::XMLText()
{
	_ot = OperatorType::NONE;
}

XMLText::~XMLText()
{
}

bool XMLText::CopyChip(Chip *chip)
{
	XMLText *c = dynamic_cast<XMLText*>(chip);
	B_RETURN(Chip::CopyChip(c));
	SetOperatorType(c->_ot);
	return true;
}

bool XMLText::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	OperatorType ot;
	LOAD(MTEXT("operatorType|ot"), ot);
	SetOperatorType(ot);
	return true;
}

bool XMLText::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE(MTEXT("operatorType"), _ot);
	return true;
}

String XMLText::GetText()
{
	RefreshT refresh(Refresh);
	if (!refresh)
		return Text::GetText();
	String str;

	switch (_ot) 
	{
	case OperatorType::CONTENT:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
			if (n) {
				xmlChar *r = xmlNodeGetContent(n);
				if (r) {
					str = (const Char*)r;
					xmlFree(r);
				}
			}
		}
		break;
	case OperatorType::ELEMENT_NAME:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
			if (n)
				str = (const Char*)n->name;
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
					str = (Char*)r;
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
						if (result->nodesetval) {
							for (int i = 0; i < result->nodesetval->nodeNr; i++) {
								xmlNodePtr n = result->nodesetval->nodeTab[i];
								xmlChar *t = xmlXPathCastNodeToString(n);
								if (t) { // Note: White-space handling for mixed nodes is an issue....
									if (i > 0)
										str += MTEXT("\n");
									str += (Char*)t;
								}
								xmlFree(t);
							}
						}
						break;
					case XPATH_BOOLEAN:
						str = result->boolval ? MTEXT("true") : MTEXT("false");
						break;
					case XPATH_NUMBER:
						str = strUtils::fromNum(result->floatval);
						break;
					case XPATH_STRING:
						str = (Char*)result->stringval;
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

	Text::SetText(str);
	return Text::GetText();
}

void XMLText::SetText(String text)
{
	Text::SetText(text);

	switch (_ot) 
	{
	case OperatorType::CONTENT:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
			if (n)
				xmlNodeSetContent(n, BAD_CAST Text::GetText().c_str());
		}
		break;
	case OperatorType::ELEMENT_NAME:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
			if (n)
				xmlNodeSetName(n, BAD_CAST Text::GetText().c_str());
		}
		break;
	case OperatorType::ATTRIBUTE:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
			ChildPtr<Text> ch1 = GetChild(1);
			if (n && ch1) {
				String s = ch1->GetText();
				xmlSetProp(n, BAD_CAST s.c_str(), BAD_CAST Text::GetText().c_str());
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

void XMLText::SetOperatorType(OperatorType ot)
{
	if (ot == _ot)
		return;
	_ot = ot;
	switch (_ot) 
	{
	case OperatorType::CONTENT:
	case OperatorType::ELEMENT_NAME:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, BOTH, MTEXT("XML Element"));
		ClearConnections(1);
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