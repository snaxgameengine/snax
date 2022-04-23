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
#include "XMLElement.h"


using namespace m3d;


CHIPDESCV1_DEF(XMLElement, MTEXT("XML Element"), XMLELEMENT_GUID, CHIP_GUID);


XMLElement::XMLElement()
{
	_node = nullptr;
}

XMLElement::~XMLElement()
{
	_clearNode();
}

bool XMLElement::NewXML(String rootname)
{
	ClearNode();
	xmlDocPtr d = xmlNewDoc(BAD_CAST MTEXT("1.0"));
	if (!d)
		return false;
	xmlNodePtr n = xmlNewNode(NULL, BAD_CAST rootname.c_str());
	if (!n) {
		xmlFreeDoc(d);
		return false;	    
	}
	xmlDocSetRootElement(d, n);
	if (!SetNode(n)) {
		xmlFreeDoc(d);
		return false;
	}
	return true;
}

bool XMLElement::LoadXMLFromFile(String filename)
{
	ClearNode();
	xmlDocPtr d = xmlParseFile(filename.c_str());
	if (!d)
		return false;
	xmlNodePtr root = xmlDocGetRootElement(d);
	if (!root || !SetNode(root)) {
		xmlFreeDoc(d);
		return false;
	}
	return true;
}

bool XMLElement::LoadXMLFromText(String xmlText)
{
	ClearNode();
	xmlDocPtr d = xmlParseDoc(BAD_CAST xmlText.c_str());
	if (!d)
		return false;
	xmlNodePtr root = xmlDocGetRootElement(d);
	if (!root || !SetNode(root)) {
		xmlFreeDoc(d);
		return false;
	}
	return true;
}

bool XMLElement::SaveXMLToFile(String filename, bool saveEntireDoc)
{
	if (!_node)
		return false;
	if (saveEntireDoc || xmlDocGetRootElement(_node->doc) == _node) { // save entire doc
		return xmlSaveFormatFile(filename.c_str(), _node->doc, 1) > 0;
	}
	// save subtree
	xmlBufferPtr buffer = xmlBufferCreate();
	int n = 0;
	if (xmlNodeDump(buffer, _node->doc, _node, 0, 1) > 0) {
		FILE *f = nullptr;
		errno_t err = fopen_s(&f, filename.c_str(), MTEXT("w")); // TODO: add header (<?xml version=MTEXT("1.0") encoding=MTEXT("utf-8")?>)? (xmlBufferAddHead)
		if (err == 0) {
			n = xmlBufferDump(f, buffer);
			fclose(f);
		}
	}
	xmlBufferFree(buffer);
	return n > 0;
}

bool XMLElement::SaveXMLToText(String &result, bool saveEntireDoc)
{
	if (!_node)
		return false;
	if (saveEntireDoc || xmlDocGetRootElement(_node->doc) == _node) { // save entire doc
		xmlChar *data = 0;
		int size = 0;
		xmlDocDumpFormatMemory(_node->doc, &data, &size, 1);
		if (data) {
			result = (Char*)data;
			xmlFree(data);
			return true;
		}
	}
	else { // save subtree
		xmlBufferPtr buffer = xmlBufferCreate();
		xmlNodeDump(buffer, _node->doc, _node, 0, 1);
		const xmlChar *data = xmlBufferContent(buffer);
		if (data) {
			result = (Char*)data;
			xmlBufferFree(buffer);
			return true; // TODO: add header? (<?xml version=MTEXT("1.0") encoding=MTEXT("utf-8")?>)? (xmlBufferAddHead)
		}
		xmlBufferFree(buffer);
	}
	return false;
}

bool XMLElement::RemoveElement()
{
	if (!_node)
		return false;
	Doc *d = (Doc*)_node->doc->_private;
	for (const auto &n : d->elements) { // Iterate all elements belonging to current document...
		if (n == this)
			continue;
		for (xmlNodePtr m = n->GetNode(); m != nullptr; m = m->parent) { // Iterate all parents..
			if (m == _node) { // Is the 'n' element a child of us?
				n->ClearNode(); // Clear its chip!
				break;
			}
		}
	}
	if (d->elements.size() == 1)
		ClearNode(); // This will destroy the entire document!
	else {
		// Destroy node and all children!
		xmlNodePtr n = _node;
		ClearNode();
		xmlUnlinkNode(n);
		xmlFreeNode(n);
	}
	return true;
}

xmlXPathObjectPtr XMLElement::DoXPathQuery(String query)
{
	if (!_node)
		return nullptr;
	XMLElement::Doc *d = (XMLElement::Doc*)_node->doc->_private;
	if (!d->context) {
		d->context = xmlXPathNewContext(_node->doc);
	}
	d->context->node = _node;
	return xmlXPathEvalExpression( BAD_CAST query.c_str(), d->context); // Remember to free result with xmlFree!
}

bool XMLElement::SetNode(xmlNodePtr node)
{
	if (_node && (!node || node->doc != _node->doc))
		ClearNode();
	if (node) {
		if (node->type != XML_ELEMENT_NODE)
			return false; // invalid type!
		_node = node;
		SetUpdateStamp();
		if (!_node->doc->_private)
			_node->doc->_private = new Doc();
		Doc *d = (Doc*)_node->doc->_private;
		d->elements.insert(this);
	}
	return true;
}

void XMLElement::ClearNode()
{
	_clearNode();
	SetUpdateStamp();
}

void XMLElement::_clearNode()
{
	if (_node) {
		Doc *d = (Doc*)_node->doc->_private;
		d->elements.erase(this);
		if (d->elements.empty()) {
			if (d->context)
				xmlXPathFreeContext(d->context);
			xmlFreeDoc(_node->doc);
			delete d;
		}
		_node = nullptr;
	}
}
