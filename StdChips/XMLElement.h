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

#pragma once

#include "Exports.h"
#include "M3DEngine/Chip.h"
#include <libxml/xmlreader.h>
#include <libxml/xmlsave.h>
#include <libxml/xmlstring.h>
#include <libxml/xpath.h>

namespace m3d
{


static const Guid XMLELEMENT_GUID = { 0x93864d7c, 0x616c, 0x485b, { 0x82, 0x11, 0x93, 0x7a, 0x4f, 0x87, 0x69, 0xda } };


class STDCHIPS_API XMLElement : public Chip
{
	CHIPDESC_DECL;
public:
	XMLElement();
	virtual ~XMLElement();

	virtual bool NewXML(String rootname);
	virtual bool LoadXMLFromFile(String filename);
	virtual bool LoadXMLFromText(String xmlText);
	virtual bool SaveXMLToFile(String filename, bool saveEntireDoc = false);
	virtual bool SaveXMLToText(String &result, bool saveEntireDoc = false);

	virtual bool RemoveElement();

	virtual xmlXPathObjectPtr DoXPathQuery(String query);

	virtual xmlNodePtr GetNode() const { return _node; }
	virtual bool SetNode(xmlNodePtr node);
	virtual void ClearNode();

protected:
	struct Doc
	{
		Set<XMLElement*> elements;
		xmlXPathContextPtr context;
		Doc() : context(nullptr) {}
	};

	xmlNodePtr _node;

	void _clearNode();
};



}