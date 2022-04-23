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
#include "XMLCommand.h"
#include "XMLElement.h"
#include "Text.h"
#include "Value.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"


using namespace m3d;


CHIPDESCV1_DEF(XMLCommand, MTEXT("XML Command"), XMLCOMMAND_GUID, CHIP_GUID);


XMLCommand::XMLCommand()
{
	_ot = OperatorType::NONE;
}

XMLCommand::~XMLCommand()
{
}

bool XMLCommand::CopyChip(Chip *chip)
{
	XMLCommand *c = dynamic_cast<XMLCommand*>(chip);
	B_RETURN(Chip::CopyChip(c));
	SetOperatorType(c->_ot);
	return true;
}

bool XMLCommand::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	OperatorType ot;
	LOAD(MTEXT("operatorType|ot"), ot);
	SetOperatorType(ot);
	return true;
}

bool XMLCommand::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE(MTEXT("operatorType"), _ot);
	return true;
}

#define CALL3RD for (uint32 i = 0; i < GetSubConnectionCount(3); i++) { ChipChildPtr ch3 = GetChild(3, i); if (ch3) ch3->CallChip(); }

void XMLCommand::CallChip()
{
	switch (_ot) 
	{
	case OperatorType::NEW_XML:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String s = ch1->GetText();
				ch0->NewXML(s);
			}
		}
		break;
	case OperatorType::LOAD_XML_FROM_FILE:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String s = ch1->GetText();
				ch0->LoadXMLFromFile(s);
			}
		}
		break;
	case OperatorType::LOAD_XML_FROM_TEXT:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String s = ch1->GetText();
				ch0->LoadXMLFromText(s);
			}
		}
		break;
	case OperatorType::SAVE_XML_TO_FILE:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String s = ch1->GetText();
				ch0->SaveXMLToFile(s);
			}
		}
		break;
	case OperatorType::SAVE_XML_TO_TEXT:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String s;
				if (ch0->SaveXMLToText(s))
					ch1->SetText(s);
			}
		}
		break;
	case OperatorType::ADD_CHILD_ELEMENT:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
			ChildPtr<Text> ch1 = GetChild(1);
			if (n && ch1) {
				String s = ch1->GetText();
				xmlNodePtr m = xmlNewChild(n, nullptr, BAD_CAST s.c_str(), nullptr);
				ChildPtr<XMLElement> ch2 = GetChild(2);
				if (ch2)
					ch2->SetNode(m);
			}
		}
		break;
	case OperatorType::REMOVE_ELEMENT:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			if (ch0)
				ch0->RemoveElement();
		}
		break;
	case OperatorType::REMOVE_ATTRIBUTE:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
			ChildPtr<Text> ch1 = GetChild(1);
			if (n && ch1) {
				String s = ch1->GetText();
				xmlAttrPtr p = xmlHasProp(n, BAD_CAST s.c_str());
				if (p) {
					xmlRemoveProp(p);
				}
			}
		}
		break;
	case OperatorType::GET_PARENT:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			ChildPtr<XMLElement> ch1 = GetChild(1);
			if (ch1) { 
				xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
				ch1->SetNode(n ? n->parent : nullptr);
			}
		}
		break;
	case OperatorType::GET_FIRST_CHILD:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			ChildPtr<XMLElement> ch1 = GetChild(1);
			if (ch1) {
				xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
				for (n = n ? n->children : nullptr; n != nullptr; n = n->next)
					if (n->type == XML_ELEMENT_NODE)
						break;
				ch1->SetNode(n); // n may be nullptr!
			}
		}
		break;
	case OperatorType::GET_NEXT_SIBLING:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			ChildPtr<XMLElement> ch1 = GetChild(1);
			if (ch1) { 
				xmlNodePtr n = ch0 ? ch0->GetNode() : nullptr;
				for (n = n ? n->next : nullptr; n != nullptr; n = n->next)
					if (n->type == XML_ELEMENT_NODE)
						break;
				ch1->SetNode(n); // n may be nullptr!
			}
		}
		break;
	case OperatorType::QUERY_NODE:
		{
			ChildPtr<XMLElement> ch0 = GetChild(0);
			ChildPtr<Text> ch1 = GetChild(1);
			if (ch0 && ch1) {
				String s = ch1->GetText();
				xmlXPathObjectPtr result = ch0->DoXPathQuery(s);
				if (result) {
					if (result->type == XPATH_NODESET) {
						for (int i = 0; i < result->nodesetval->nodeNr; i++) { // NOTE: If the user does something stupid, the doc may be freed and all nodes invalid!
							xmlNodePtr n = result->nodesetval->nodeTab[i];
							if (n->type == XML_ELEMENT_NODE) {
								ChildPtr<XMLElement> ch2 = GetChild(2);
								if (ch2) {
									ch2->SetNode(n);
									CALL3RD;
								}
							}
						}
					}
					xmlFree(result);
				}
			}
		}
		break;
	case OperatorType::QUERY_TEXT:
	case OperatorType::QUERY_VALUE:
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
						for (int i = 0; i < result->nodesetval->nodeNr; i++) { // NOTE: If the user does something stupid, the doc may be freed and all nodes invalid!
							xmlNodePtr n = result->nodesetval->nodeTab[i];
							xmlChar *t = xmlXPathCastNodeToString(n);
							if (t) { // Note: White-space handling for mixed nodes is an issue....
								if (_ot == OperatorType::QUERY_TEXT) {
									ChildPtr<Text> ch2 = GetChild(2);
									if (ch2) {
										ch2->SetText((Char*)t);
										CALL3RD;
									}
								}
								else {
									ChildPtr<Value> ch2 = GetChild(2);
									value v;
									if (ch2 && strUtils::toNum(String((Char*)t), v)) {
										ch2->SetValue(v);
										CALL3RD;
									}
								}
							}
							xmlFree(t);
						}
						break;
					case XPATH_BOOLEAN:
						if (_ot == OperatorType::QUERY_TEXT) {
							ChildPtr<Text> ch2 = GetChild(2);
							if (ch2) {
								ch2->SetText(result->boolval ? MTEXT("true") : MTEXT("false"));
								CALL3RD;
							}
						}
						else {
							ChildPtr<Value> ch2 = GetChild(2);
							if (ch2) {
								ch2->SetValue(result->boolval ? 1.0 : 0.0);
								CALL3RD;
							}
						}
						break;
					case XPATH_NUMBER:
						if (_ot == OperatorType::QUERY_TEXT) {
							ChildPtr<Text> ch2 = GetChild(2);
							if (ch2) {
								ch2->SetText(strUtils::fromNum(result->floatval));
								CALL3RD;
							}
						}
						else {
							ChildPtr<Value> ch2 = GetChild(2);
							if (ch2) {
								ch2->SetValue(value(result->floatval));
								CALL3RD;
							}
						}
						break;
					case XPATH_STRING:
						if (result->stringval) {
							if (_ot == OperatorType::QUERY_TEXT) {
								ChildPtr<Text> ch2 = GetChild(2);
								if (ch2) {
									ch2->SetText((Char*)result->stringval);
									CALL3RD;
								}
							}
							else {
								ChildPtr<Value> ch2 = GetChild(2);
								value v;
								if (ch2 && strUtils::toNum(String((Char*)result->stringval), v)) {
									ch2->SetValue(v);
									CALL3RD;
								}
							}
						}
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
		AddMessage(UninitializedException());
		break;
	}
}

void XMLCommand::SetOperatorType(OperatorType ot)
{
	if (ot == _ot)
		return;
	_ot = ot;
	switch (_ot) 
	{
	case OperatorType::NEW_XML:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, DOWN, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Root Element Name"));
		ClearConnections(2);
		break;
	case OperatorType::LOAD_XML_FROM_FILE:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, DOWN, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Filename"));
		ClearConnections(2);
		break;
	case OperatorType::LOAD_XML_FROM_TEXT:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, DOWN, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("XML Text"));
		ClearConnections(2);
		break;
	case OperatorType::SAVE_XML_TO_FILE:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, UP, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Filename"));
		ClearConnections(2);
		break;
	case OperatorType::SAVE_XML_TO_TEXT:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, UP, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, DOWN, MTEXT("XML Text"));
		ClearConnections(2);
		break;
	case OperatorType::ADD_CHILD_ELEMENT:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, DOWN, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("Element Name"));
		CREATE_CHILD_KEEP(2, XMLELEMENT_GUID, false, DOWN, MTEXT("New XML Element"));
		ClearConnections(3);
		break;
	case OperatorType::REMOVE_ELEMENT:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, DOWN, MTEXT("XML Element"));
		ClearConnections(1);
		break;
	case OperatorType::REMOVE_ATTRIBUTE:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, DOWN, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, DOWN, MTEXT("Attribute Name"));
		ClearConnections(2);
		break;
	case OperatorType::GET_PARENT:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, UP, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, XMLELEMENT_GUID, false, DOWN, MTEXT("Parent XML Element"));
		ClearConnections(2);
		break;
	case OperatorType::GET_FIRST_CHILD:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, UP, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, XMLELEMENT_GUID, false, DOWN, MTEXT("Child XML Element"));
		ClearConnections(2);
		break;
	case OperatorType::GET_NEXT_SIBLING:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, UP, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, XMLELEMENT_GUID, false, DOWN, MTEXT("Sibling XML Element"));
		ClearConnections(2);
		break;
	case OperatorType::QUERY_NODE:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, UP, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("XPath Query"));
		CREATE_CHILD_KEEP(2, XMLELEMENT_GUID, false, DOWN, MTEXT("Result"));
		CREATE_CHILD_KEEP(3, CHIP_GUID, true, DOWN, MTEXT("Called for Each Result"));
		ClearConnections(4);
		break;
	case OperatorType::QUERY_TEXT:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, UP, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("XPath Query"));
		CREATE_CHILD_KEEP(2, TEXT_GUID, false, DOWN, MTEXT("Result"));
		CREATE_CHILD_KEEP(3, CHIP_GUID, true, DOWN, MTEXT("Called for Each Result"));
		ClearConnections(4);
		break;
	case OperatorType::QUERY_VALUE:
		CREATE_CHILD_KEEP(0, XMLELEMENT_GUID, false, UP, MTEXT("XML Element"));
		CREATE_CHILD_KEEP(1, TEXT_GUID, false, UP, MTEXT("XPath Query"));
		CREATE_CHILD_KEEP(2, VALUE_GUID, false, DOWN, MTEXT("Result"));
		CREATE_CHILD_KEEP(3, CHIP_GUID, true, DOWN, MTEXT("Called for Each Result"));
		ClearConnections(4);
		break;
	default:
		ClearConnections();
		break;
	}
	RemoveMessage(UninitializedException());
}
