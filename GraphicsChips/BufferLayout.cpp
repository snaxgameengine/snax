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
#include "BufferLayout.h"
#include "SimpleLexer.h"

using namespace m3d;

#define RULE_VALUE 100
#define RULE_VECTOR 101
#define RULE_MATRIX 102
#define RULE_FLOATXXX 103
#define RULE_INTXXX 104
#define RULE_UINTXXX 105
#define RULE_BOOLXXX 106
#define RULE_DOUBLEXXX 107
#define RULE_OFFSET 108
#define RULE_INDEX_VALUE 109


BufferLayout::BufferLayout()
{
}

BufferLayout::~BufferLayout()
{
}

void _parseType(SimpleLexer &lex, BufferLayout::Item &itm, const SimpleLexer::Token &t)
{

}

bool BufferLayout::Init(String config, String *err)
{
	static List<SimpleLexer::Rule> rules;
	if (rules.empty()) {
		List<SimpleLexer::Rule> string_rules, r;

		string_rules.push_back(SimpleLexer::Rule(RULE_VALUE, std::regex("^value$", std::regex_constants::ECMAScript | std::regex_constants::icase)));
		string_rules.push_back(SimpleLexer::Rule(RULE_VECTOR, std::regex("^vector$", std::regex_constants::ECMAScript | std::regex_constants::icase)));
		string_rules.push_back(SimpleLexer::Rule(RULE_MATRIX, std::regex("^matrix$", std::regex_constants::ECMAScript | std::regex_constants::icase)));
		string_rules.push_back(SimpleLexer::Rule(RULE_FLOATXXX, std::regex("^float([1-4](x[1-4])?)?$", std::regex_constants::ECMAScript | std::regex_constants::icase)));
		string_rules.push_back(SimpleLexer::Rule(RULE_INTXXX, std::regex("^int([1-4](x[1-4])?)?$", std::regex_constants::ECMAScript | std::regex_constants::icase)));
		string_rules.push_back(SimpleLexer::Rule(RULE_UINTXXX, std::regex("^uint([1-4](x[1-4])?)?$", std::regex_constants::ECMAScript | std::regex_constants::icase)));
		string_rules.push_back(SimpleLexer::Rule(RULE_BOOLXXX, std::regex("^bool([1-4](x[1-4])?)?$", std::regex_constants::ECMAScript | std::regex_constants::icase)));
		string_rules.push_back(SimpleLexer::Rule(RULE_DOUBLEXXX, std::regex("^double([1-4](x[1-4])?)?$", std::regex_constants::ECMAScript | std::regex_constants::icase)));
		string_rules.push_back(SimpleLexer::Rule(RULE_OFFSET, std::regex("^offset$", std::regex_constants::ECMAScript | std::regex_constants::icase)));
		string_rules.push_back(SimpleLexer::Rule(RULE_IDENTIFICATOR, std::regex("^([_a-zA-Z][_a-zA-Z0-9]*)$")));

		r.push_back(SimpleLexer::Rule(RULE_INDEX_VALUE, std::regex("^[0-9]{1,7}", std::regex_constants::ECMAScript | std::regex_constants::icase)));

		r.push_back(SimpleLexer::Rule(RULE_LINEBREAK, std::regex("^([\\n]+)")));
		r.push_back(SimpleLexer::Rule(RULE_WHITESPACE, std::regex("^([ \\t]+)")));
//		r.push_back(SimpleLexer::Rule(RULE_SINGLE_LINE_COMMENT, std::regex("^(\\/\\/.*)")));
//		r.push_back(SimpleLexer::Rule(RULE_FLOAT, std::regex("^([-+]?[0-9]*\\.[0-9]+([eE][-+]?[0-9]+)?)")));
//		r.push_back(SimpleLexer::Rule(RULE_INTEGER, std::regex("^([-+]?[0-9]+)")));
		r.push_back(SimpleLexer::Rule(RULE_OPERATOR, std::regex("^([(){}[\\]=+\\-.,;:])"), [](const SimpleLexer::Rule &rule, const std::cmatch &rmatch) { return SimpleLexer::RuleID(rule.id + (int32)rmatch.begin()->str().at(0)); }));
//		r.push_back(SimpleLexer::Rule(RULE_QUOTED_STRING, std::regex("^(\"[^\"\\\\]*(?:\\\\.[^\"\\\\]*)*\")")));
		r.push_back(SimpleLexer::Rule(RULE_STRING, std::regex("^([_a-zA-Z0-9]+)"), string_rules));

//		rules.push_back(SimpleLexer::Rule(RULE_MULTILINE_COMMENT, std::regex("^(\\/\\*(.|\r|\n)*\\*\\/)")));
		rules.push_back(SimpleLexer::Rule(RULE_LINEBREAK, std::regex("(.*\\n?)"), r));
	}

	SimpleLexer lex;
	lex.Init(rules);

	_items.clear();
	_itemsByType[0].clear();
	_itemsByType[1].clear();
	_itemsByType[2].clear();
	_itemsByType[3].clear();
	_itemsByType[4].clear();
	_itemsByType[5].clear();
	_sizeInBytes = 0;

	if (!lex.Tokenize(config, MTEXT("BufferLayout"), err))
		return false;

	UINT64 offset = 0;
	Set<String> names;
	try {
		while (lex.PeekToken().id != RULE_EOF) {
			Item itm;

			SimpleLexer::Token t = lex.NextToken();

			{
				if (t.id == RULE_FLOATXXX) itm.dt = BufferLayout::Item::DataType::FLOAT;
				else if (t.id == RULE_INTXXX) itm.dt = BufferLayout::Item::DataType::INT;
				else if (t.id == RULE_UINTXXX) itm.dt = BufferLayout::Item::DataType::UINT;
				else if (t.id == RULE_BOOLXXX) itm.dt = BufferLayout::Item::DataType::BOOL;
				else if (t.id == RULE_DOUBLEXXX) itm.dt = BufferLayout::Item::DataType::DOUBLE;
				else if (t.id == RULE_VALUE) { itm.type = BufferLayout::Item::VALUE; itm.dt = BufferLayout::Item::DataType::FLOAT; itm.rows = 1; itm.columns = 1; }
				else if (t.id == RULE_VECTOR) { itm.type = BufferLayout::Item::VECTOR; itm.dt = BufferLayout::Item::DataType::FLOAT; itm.rows = 1; itm.columns = 4; }
				else if (t.id == RULE_MATRIX) { itm.type = BufferLayout::Item::MATRIX; itm.dt = BufferLayout::Item::DataType::FLOAT; itm.rows = 4; itm.columns = 4; }
				else
					throw lex.ErrorStr(t);
				if (itm.rows == 0 && itm.columns == 0) {
					uint32 L[] = { 4,3,4,5,6 };
					if (t.length == L[(uint32)itm.dt] + 3) { // eg float4x4 (rowsxcolumns)
						itm.type = BufferLayout::Item::MATRIX;
						itm.columns = (uint32)(t.str[L[(uint32)itm.dt] + 2] - '0');
						itm.rows = (uint32)(t.str[L[(uint32)itm.dt]] - '0');
					}
					else if (t.length > L[(uint32)itm.dt]) { // eg float4
						itm.type = BufferLayout::Item::VECTOR;
						itm.columns = (uint32)(t.str[L[(uint32)itm.dt]] - '0');
						itm.rows = 1;
					}
					else {
						itm.type = BufferLayout::Item::VALUE;
						itm.rows = itm.columns = 1; // eg float
					}
				}
			}

			t = lex.NextToken();
			if (t.id != RULE_IDENTIFICATOR) 
				throw lex.ErrorStr(t);
			itm.name = t.AsString();
			if (names.insert(itm.name).second == false)
				throw lex.ErrorStr(t, MTEXT("identifier is already specified!"));

			static uint32 DT_SIZE[] = { 4, 4, 4, 4, 8 };

			itm.size = DT_SIZE[(uint32)itm.dt];

			if (itm.type == Item::VECTOR)
				itm.size *= itm.columns;
			else if (itm.type == Item::MATRIX)
				itm.size = (itm.size * itm.rows) + (((itm.size * itm.rows) + 15) / 16) * 16 * (itm.columns - 1);

			if (lex.PeekToken().id == RULE_LBRACKET) {
				t = lex.NextToken();
				t = lex.NextToken();
				if (t.id != RULE_INDEX_VALUE || !strUtils::toNum(t.AsString(), itm.elements))
					throw lex.ErrorStr(t);
				t = lex.NextToken();
				if (t.id != RULE_RBRACKET)
					throw lex.ErrorStr(t);
				if (itm.type == Item::VALUE) itm.type = Item::VALUE_ARRAY;
				else if (itm.type == Item::VECTOR) itm.type = Item::VECTOR_ARRAY;
				else if (itm.type == Item::MATRIX) itm.type = Item::MATRIX_ARRAY;

				itm.size = itm.size + ((itm.size + 15) / 16) * 16 * (itm.elements - 1);
			}

			if (itm.size >= 16)
				offset = ((offset + 15) / 16) * 16;
			else if (offset % 16 + itm.size > 16)
				offset = ((offset + 15) / 16) * 16;
			//offset = ((offset + itm.size - 1) / itm.size) * itm.size;

			if (lex.PeekToken().id == RULE_COLON) {
				t = lex.NextToken();
				t = lex.NextToken();
				if (t.id != RULE_OFFSET)
					throw lex.ErrorStr(t);
				t = lex.NextToken();
				if (t.id != RULE_LPAREN)
					throw lex.ErrorStr(t);
				t = lex.NextToken();
				if (t.id != RULE_INDEX_VALUE || !strUtils::toNum(t.AsString(), itm.offset))
					throw lex.ErrorStr(t);
				t = lex.NextToken();
				if (t.id != RULE_RPAREN)
					throw lex.ErrorStr(t);
				if (itm.offset < offset)
					throw strUtils::ConstructString(MTEXT("Offset can not be smaller than %1 bytes for this item.")).arg(offset).string;
				offset = itm.offset;
				if (offset % 16 + std::min(itm.size, 16u) > 16)
					throw strUtils::ConstructString(MTEXT("Invalid offset.")).arg(std::min(itm.size, 16u)).string;
			}
			else {
				itm.offset = offset;
			}

			offset += itm.size;
			_items.push_back(itm);
			_itemsByType[itm.type].push_back(uint32(_items.size() - 1));
			
			t = lex.NextToken();
			if (t.id != RULE_SEMICOLON)
				throw lex.ErrorStr(t);
		}
	}
	catch (const String &e) {
		if (err)
			*err = e;
		return false;
	}

	_sizeInBytes = offset;

	return true;
}

UINT64 BufferLayout::GetBufferSize() const
{
	return _sizeInBytes;
}

String BufferLayout::GetConfig() const
{
	static const Char *DataTypeNames[] = {"bool", "int", "uint", "float", "double"};
	String config;
	UINT64 offset = 0;
	for (size_t i = 0; i < _items.size(); i++) {
		const Item &itm = _items[i];
		String dt;
		if (itm.type == Item::MATRIX || itm.type == Item::MATRIX_ARRAY)
			dt = strUtils::ConstructString("%1%2x%3").arg(DataTypeNames[(uint32)itm.dt]).arg(itm.rows).arg(itm.columns);
		else if (itm.type == Item::VECTOR || itm.type == Item::VECTOR_ARRAY)
			dt = strUtils::ConstructString("%1%2").arg(DataTypeNames[(uint32)itm.dt]).arg(itm.columns);
		else
			dt = DataTypeNames[(uint32)itm.dt];
		String line = strUtils::ConstructString("%1 %2").arg(dt).arg(itm.name);
		if (itm.elements > 0)
			line += strUtils::ConstructString("[%1]").arg(itm.elements);
		if (itm.size >= 16)
			offset = ((offset + 15) / 16) * 16;
		else
			offset = ((offset + itm.size - 1) / itm.size) * itm.size;
		if (itm.offset > offset)
			line += strUtils::ConstructString(" : offset(%1)").arg(itm.offset);
		config += line + ";\n";
		offset += itm.size;
	}
	return config;
}

String BufferLayout::GetStruct(String name) const
{
	static const Char *TypeNames[] = { "float", "float4", "float4x4", "float", "float4", "float4x4" };
	String s = strUtils::ConstructString("struct %1\n{\n").arg(name);
	UINT64 offset = 0;
	UINT p = 0;
	for (size_t i = 0; i < _items.size(); i++) {
		const Item &itm = _items[i];
		if (itm.type == Item::VALUE)
			offset = ((offset + 3) / 4) * 4;
		else
			offset = ((offset + 15) / 16) * 16;
		if (itm.offset > offset) {
			if (itm.type == Item::VALUE) {}
			else
			{}
		}
		s += strUtils::ConstructString("    %1 %2").arg(TypeNames[itm.type]).arg(itm.name);
		if (itm.elements > 0)
			s += strUtils::ConstructString("[%1]").arg(itm.elements);
		s += ";\n";
		offset += itm.size;
	}
	s += "}";
	return s;
}

bool BufferLayout::IsSubsetOf(const BufferLayout &bl, String &msg) const
{
	if (_sizeInBytes > bl._sizeInBytes) {
		msg = String(MTEXT("Buffer too small."));
		return false;
	}
	for (size_t i = 0, j = 0; i < _items.size(); i++) {
		const Item &itm = _items[i];
		if (j == bl._items.size()) {
			msg = strUtils::ConstructString(MTEXT("Buffer is missing element \'%1\'.")).arg(itm.name);
			return false;
		}
		for (; j < bl._items.size(); j++) {
			const Item &jtm = bl._items[j];
			if (itm.offset == jtm.offset) {
				if (itm.name != jtm.name)
					msg = strUtils::ConstructString(MTEXT("Name of element (offset = %1 bytes) is not matching: \'%2\' vs \'%3\'.")).arg(itm.offset).arg(jtm.name).arg(itm.name);
				else if (itm.type != jtm.type || itm.dt != jtm.dt)
					msg = strUtils::ConstructString(MTEXT("Type mismatch of element \'%1\'.")).arg(itm.name);
				else if (itm.rows != jtm.rows || itm.columns != jtm.columns)
					msg = strUtils::ConstructString(MTEXT("Dimensions of element \'%1\' is not matching.")).arg(itm.name);
				else if (itm.elements > jtm.elements)
					msg = strUtils::ConstructString(MTEXT("Array size of element \'%1\' is too small.")).arg(itm.name);
				else
					break;
				return false;
			}
			else if (itm.offset < jtm.offset) {
				msg = strUtils::ConstructString(MTEXT("Offset of element \'%1\' is %2 bytes, but must be no more than %3 bytes.")).arg(jtm.name).arg(jtm.offset).arg(itm.offset);
				return false;
			}
		}
	}
	return true;
}

bool BufferLayout::IsCompatible(const BufferLayout &bl) const
{
	for (size_t i = 0, j = 0; i < _items.size(); i++) {
		const Item &itm = _items[i];
		for (; j < bl._items.size(); j++) {
			const Item &jtm = bl._items[j];
			if (jtm.offset < itm.offset) {
				if ((jtm.offset + jtm.size) > itm.offset)
					return false;
				continue;
			}
			else if (jtm.offset > itm.offset) {
				if ((itm.offset + itm.size) > jtm.offset)
					return false;
				break;
			}
			else {
				if (itm.type != jtm.type || itm.dt != jtm.dt || itm.rows != jtm.rows || itm.columns != jtm.columns || strUtils::compareNoCase(itm.name, jtm.name) != 0)
					return false;
				break;
			}
		}
	}
	return true;
}

bool BufferLayout::IsEqual(const BufferLayout &bl) const
{
	if (_sizeInBytes != bl._sizeInBytes || _items.size() != bl._items.size())
		return false;
	for (size_t i = 0; i < _items.size(); i++) {
		const Item &itm = _items[i], jtm = bl._items[i];
		if (itm.type != jtm.type || itm.dt != jtm.dt || itm.rows != jtm.rows || itm.columns != jtm.columns || itm.elements != jtm.elements || itm.offset != jtm.offset || itm.size != jtm.size || itm.name != jtm.name)
			return false;
	}
	return true;
}

BufferLayout BufferLayout::Merge(const BufferLayout &bl) const
{
	if (!IsCompatible(bl))
		return BufferLayout();
	BufferLayout r;
	size_t j = 0;
	for (size_t i = 0; i < _items.size(); i++) {
		const Item &itm = _items[i];
		for (; j <= bl._items.size(); j++) {
			if (j == bl._items.size()) {
				r._items.push_back(itm);
				r._itemsByType[itm.type].push_back(uint32(r._items.size() - 1));
				break;
			}
			const Item &jtm = bl._items[j];
			if (itm.offset < jtm.offset) {
				r._items.push_back(itm);
				r._itemsByType[itm.type].push_back(uint32(r._items.size() - 1));
				break;
			}
			else if (jtm.offset < itm.offset) {
				r._items.push_back(jtm);
				r._itemsByType[jtm.type].push_back(uint32(r._items.size() - 1));
			}
			else {
				Item ktm = itm;
				ktm.rows = std::max(itm.rows, jtm.rows);
				ktm.columns = std::max(itm.columns, jtm.columns);
				ktm.elements = std::max(itm.elements, jtm.elements);
				ktm.size = std::max(itm.size, jtm.size);
				r._items.push_back(ktm);
				r._itemsByType[ktm.type].push_back(uint32(r._items.size() - 1));
				j++;
				break;
			}
		}
	}
	for (; j < bl._items.size(); j++) {
		const Item &jtm = bl._items[j];
		r._items.push_back(jtm);
		r._itemsByType[jtm.type].push_back(uint32(r._items.size() - 1));
	}
	r._sizeInBytes = std::max(_sizeInBytes, bl._sizeInBytes);
	return r;
}


uint32 BufferLayout::GetNumberOf(Item::Type type) const
{
	return (uint32)_itemsByType[type].size();
}

String BufferLayout::GetNameOf(Item::Type type, uint32 idx) const
{
	return _items[_itemsByType[type][idx]].name;
}

uint32 BufferLayout::GetElementsInArray(Item::Type type, uint32 idx) const
{
	return _items[_itemsByType[type][idx]].elements;
}

void BufferLayout::SetValue(uint32 idx, BYTE *buffer, const double *v) const
{
	const Item &itm = _items[_itemsByType[Item::VALUE][idx]];
	switch (itm.dt)
	{
	case Item::DataType::BOOL: *reinterpret_cast<BOOL*>(buffer + itm.offset) = (*v != 0.0) ? TRUE : FALSE; break;
	case Item::DataType::INT: *reinterpret_cast<INT*>(buffer + itm.offset) = (INT)*v; break;
	case Item::DataType::UINT: *reinterpret_cast<UINT*>(buffer + itm.offset) = (UINT)*v; break;
	case Item::DataType::FLOAT: *reinterpret_cast<FLOAT*>(buffer + itm.offset) = (float)*v; break;
	case Item::DataType::DOUBLE: *reinterpret_cast<DOUBLE*>(buffer + itm.offset) = *v; break;
	}
}

struct __DOUBLE4
{
	double x, y, z, w;
	__DOUBLE4(const XMFLOAT4 &v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
};

struct __DOUBLE4X4
{
	__DOUBLE4 x, y, z, w;
	__DOUBLE4X4(const XMFLOAT4X4 &m) : x(XMFLOAT4(m.m[0])), y(XMFLOAT4(m.m[1])), z(XMFLOAT4(m.m[2])), w(XMFLOAT4(m.m[3])) {}
};

struct __DOUBLE2
{
	double x, y;
	__DOUBLE2(const XMFLOAT4& v) : x(v.x), y(v.y) {}
};

struct __DOUBLE2X4
{
	__DOUBLE2 x, y, z, w;
	__DOUBLE2X4(const XMFLOAT4X4& m) : x(XMFLOAT4(m.m[0])), y(XMFLOAT4(m.m[1])), z(XMFLOAT4(m.m[2])), w(XMFLOAT4(m.m[3])) {}
};

struct __BOOL4X4
{
	XMINT4 x, y, z, w;
	__BOOL4X4(const XMFLOAT4X4 &m) :
		x(XMINT4(m.m[0][0] != 0.0 ? TRUE : FALSE, m.m[0][1] != 0.0 ? TRUE : FALSE, m.m[0][2] != 0.0 ? TRUE : FALSE, m.m[0][3] != 0.0 ? TRUE : FALSE)),
		y(XMINT4(m.m[1][0] != 0.0 ? TRUE : FALSE, m.m[1][1] != 0.0 ? TRUE : FALSE, m.m[1][2] != 0.0 ? TRUE : FALSE, m.m[1][3] != 0.0 ? TRUE : FALSE)),
		z(XMINT4(m.m[2][0] != 0.0 ? TRUE : FALSE, m.m[2][1] != 0.0 ? TRUE : FALSE, m.m[2][2] != 0.0 ? TRUE : FALSE, m.m[2][3] != 0.0 ? TRUE : FALSE)),
		w(XMINT4(m.m[3][0] != 0.0 ? TRUE : FALSE, m.m[3][1] != 0.0 ? TRUE : FALSE, m.m[3][2] != 0.0 ? TRUE : FALSE, m.m[3][3] != 0.0 ? TRUE : FALSE)) {}
}; 

struct __INT4X4
{
	XMINT4 x, y, z, w;
	__INT4X4(const XMFLOAT4X4 &m) :
		x(XMINT4((INT)m.m[0][0], (INT)m.m[0][1], (INT)m.m[0][2], (INT)m.m[0][3])), 
		y(XMINT4((INT)m.m[1][0], (INT)m.m[1][1], (INT)m.m[1][2], (INT)m.m[1][3])),
		z(XMINT4((INT)m.m[2][0], (INT)m.m[2][1], (INT)m.m[2][2], (INT)m.m[2][3])),
		w(XMINT4((INT)m.m[3][0], (INT)m.m[3][1], (INT)m.m[3][2], (INT)m.m[3][3])) {}
};

struct __UINT4X4
{
	XMUINT4 x, y, z, w;
	__UINT4X4(const XMFLOAT4X4 &m) :
		x(XMUINT4((UINT)m.m[0][0], (UINT)m.m[0][1], (UINT)m.m[0][2], (UINT)m.m[0][3])),
		y(XMUINT4((UINT)m.m[1][0], (UINT)m.m[1][1], (UINT)m.m[1][2], (UINT)m.m[1][3])),
		z(XMUINT4((UINT)m.m[2][0], (UINT)m.m[2][1], (UINT)m.m[2][2], (UINT)m.m[2][3])),
		w(XMUINT4((UINT)m.m[3][0], (UINT)m.m[3][1], (UINT)m.m[3][2], (UINT)m.m[3][3])) {}
};

void BufferLayout::SetVector(uint32 idx, BYTE *buffer, const XMFLOAT4 *v) const
{
	const Item &itm = _items[_itemsByType[Item::VECTOR][idx]];
	switch (itm.dt)
	{
	case Item::DataType::BOOL: { XMINT4 t(v->x != 0.0 ? TRUE : FALSE, v->y != 0.0 ? TRUE : FALSE, v->z != 0.0 ? TRUE : FALSE, v->w != 0.0 ? TRUE : FALSE);  std::memcpy(reinterpret_cast<BOOL*>(buffer + itm.offset), &t, sizeof(INT) * itm.columns); break; }
	case Item::DataType::INT: { XMINT4 t((INT)v->x, (INT)v->y, (INT)v->z, (INT)v->w); std::memcpy(reinterpret_cast<INT*>(buffer + itm.offset), &t, sizeof(INT) * itm.columns); break; }
	case Item::DataType::UINT: { XMUINT4 t((UINT)v->x, (UINT)v->y, (UINT)v->z, (UINT)v->w); std::memcpy(reinterpret_cast<UINT*>(buffer + itm.offset), &t, sizeof(UINT) * itm.columns); break; }
	case Item::DataType::FLOAT: std::memcpy(reinterpret_cast<FLOAT*>(buffer + itm.offset), v, sizeof(FLOAT) * itm.columns); break;
	case Item::DataType::DOUBLE: { __DOUBLE4 t(*v);  std::memcpy(reinterpret_cast<DOUBLE*>(buffer + itm.offset), &t, sizeof(DOUBLE) * itm.columns); break; }
	}
}

void BufferLayout::SetMatrix(uint32 idx, BYTE *buffer, const XMFLOAT4X4 *m) const
{
	XMFLOAT4X4 mt;
	XMStoreFloat4x4(&mt, XMMatrixTranspose(XMLoadFloat4x4(m)));
	const Item &itm = _items[_itemsByType[Item::MATRIX][idx]];
	switch (itm.dt)
	{
	case Item::DataType::BOOL: { __BOOL4X4 t(mt);  std::memcpy(reinterpret_cast<BOOL*>(buffer + itm.offset), &t, sizeof(BOOL) * 4 * (itm.columns - 1) + sizeof(BOOL) * itm.rows); break; }
	case Item::DataType::INT: { __INT4X4 t(mt);  std::memcpy(reinterpret_cast<INT*>(buffer + itm.offset), &t, sizeof(INT) * 4 * (itm.columns - 1) + sizeof(INT) * itm.rows); break; }
	case Item::DataType::UINT: { __UINT4X4 t(mt);  std::memcpy(reinterpret_cast<UINT*>(buffer + itm.offset), &t, sizeof(UINT) * 4 * (itm.columns - 1) + sizeof(UINT) * itm.rows); break; }
	case Item::DataType::FLOAT: std::memcpy(reinterpret_cast<FLOAT*>(buffer + itm.offset), &mt, sizeof(FLOAT) * 4 * (itm.columns - 1) + sizeof(FLOAT) * itm.rows); break;
	case Item::DataType::DOUBLE: 
	{
		if (itm.rows > 2) {
			__DOUBLE4X4 t(mt);
			std::memcpy(reinterpret_cast<DOUBLE*>(buffer + itm.offset), (const void*)&t, sizeof(DOUBLE) * 4 * (itm.columns - 1) + sizeof(DOUBLE) * itm.rows);
		}
		else {
			__DOUBLE2X4 t(mt);
			std::memcpy(reinterpret_cast<DOUBLE*>(buffer + itm.offset), (const void*)&t, sizeof(DOUBLE) * 2 * (itm.columns - 1) + sizeof(DOUBLE) * itm.rows);
		}
		break;
	}
	}
}

void BufferLayout::SetValueArray(uint32 idx, BYTE *buffer, uint32 count, const double *v) const
{
	const Item& itm = _items[_itemsByType[Item::VALUE_ARRAY][idx]];
	switch (itm.dt)
	{
	case Item::DataType::BOOL:
	{
		for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
			BOOL q = v[i] != 0 ? TRUE : FALSE;
			std::memcpy(reinterpret_cast<BOOL*>(buffer + itm.offset + sizeof(BOOL) * 4 * i), &q, sizeof(BOOL));
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	case Item::DataType::INT:
	{
		for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
			INT q = (INT)v[i];
			std::memcpy(reinterpret_cast<INT*>(buffer + itm.offset + sizeof(INT) * 4 * i), &q, sizeof(INT));
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	case Item::DataType::UINT:
	{
		for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
			UINT q = (UINT)v[i];
			std::memcpy(reinterpret_cast<UINT*>(buffer + itm.offset + sizeof(UINT) * 4 * i), &q, sizeof(UINT));
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	case Item::DataType::FLOAT:
	{
		for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
			FLOAT q = (FLOAT)v[i];
			std::memcpy(reinterpret_cast<FLOAT*>(buffer + itm.offset + sizeof(FLOAT) * 4 * i), &q, sizeof(FLOAT));
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	case Item::DataType::DOUBLE:
	{
		for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
			std::memcpy(reinterpret_cast<DOUBLE*>(buffer + itm.offset + sizeof(DOUBLE) * 2 * i), v + i, sizeof(DOUBLE));
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	}
}

void BufferLayout::SetVectorArray(uint32 idx, BYTE *buffer, uint32 count, const XMFLOAT4 *v) const
{
	const Item& itm = _items[_itemsByType[Item::VECTOR_ARRAY][idx]];
	switch (itm.dt)
	{
	case Item::DataType::BOOL:
	{
		for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
			const XMFLOAT4* w = v + i;
			BOOL q[4] = { w->x ? TRUE : FALSE, w->y ? TRUE : FALSE, w->z ? TRUE : FALSE, w->w ? TRUE : FALSE };
			std::memcpy(reinterpret_cast<BOOL*>(buffer + itm.offset + sizeof(BOOL) * 4 * i), q, sizeof(BOOL)* itm.columns);
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	case Item::DataType::INT:
	{
		for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
			const XMFLOAT4* w = v + i;
			INT q[4] = { (INT)w->x, (INT)w->y, (INT)w->z, (INT)w->w };
			std::memcpy(reinterpret_cast<INT*>(buffer + itm.offset + sizeof(INT) * 4 * i), q, sizeof(INT)* itm.columns);
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	case Item::DataType::UINT: 
	{
		for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
			const XMFLOAT4* w = v + i;
			UINT q[4] = { (UINT)w->x, (UINT)w->y, (UINT)w->z, (UINT)w->w };
			std::memcpy(reinterpret_cast<UINT*>(buffer + itm.offset + sizeof(UINT) * 4 * i), q, sizeof(UINT) * itm.columns);
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	case Item::DataType::FLOAT:
	{
		if (itm.columns == 4) {
			std::memcpy(reinterpret_cast<FLOAT*>(buffer + itm.offset), v, sizeof(FLOAT) * 4 * count);
		}
		else {
			for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
				std::memcpy(reinterpret_cast<FLOAT*>(buffer + itm.offset + sizeof(FLOAT) * 4 * i), v + i, sizeof(FLOAT) * itm.columns);
			}
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	case Item::DataType::DOUBLE:
	{
		for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
			const XMFLOAT4* w = v + i;
			__DOUBLE4 t(*w);
			std::memcpy(reinterpret_cast<DOUBLE*>(buffer + itm.offset + sizeof(DOUBLE) * (itm.columns > 2 ? 4 : 2) * i), &t, sizeof(DOUBLE)* itm.columns);
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	}
}

void BufferLayout::SetMatrixArray(uint32 idx, BYTE *buffer, uint32 count, const XMFLOAT4X4 *m) const
{
	const Item &itm = _items[_itemsByType[Item::MATRIX_ARRAY][idx]];
	switch (itm.dt)
	{
	case Item::DataType::BOOL:
	{
		for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
			XMFLOAT4X4 mt;
			XMStoreFloat4x4(&mt, XMMatrixTranspose(XMLoadFloat4x4(m + i)));
			__BOOL4X4 t(mt);
			std::memcpy(reinterpret_cast<BOOL*>(buffer + itm.offset + sizeof(BOOL) * 4 * itm.columns * i), &t, sizeof(BOOL) * 4 * (itm.columns - 1) + sizeof(BOOL) * itm.rows);
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	case Item::DataType::INT:
	{
		for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
			XMFLOAT4X4 mt;
			XMStoreFloat4x4(&mt, XMMatrixTranspose(XMLoadFloat4x4(m + i)));
			__INT4X4 t(mt);
			std::memcpy(reinterpret_cast<INT*>(buffer + itm.offset + sizeof(INT) * 4 * itm.columns * i), &t, sizeof(INT) * 4 * (itm.columns - 1) + sizeof(INT) * itm.rows);
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	case Item::DataType::UINT:
	{
		for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
			XMFLOAT4X4 mt;
			XMStoreFloat4x4(&mt, XMMatrixTranspose(XMLoadFloat4x4(m + i)));
			__UINT4X4 t(mt);
			std::memcpy(reinterpret_cast<UINT*>(buffer + itm.offset + sizeof(UINT) * 4 * itm.columns * i), &t, sizeof(UINT) * 4 * (itm.columns - 1) + sizeof(UINT) * itm.rows);
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	case Item::DataType::FLOAT:
	{
		for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
			XMFLOAT4X4 mt;
			XMStoreFloat4x4(&mt, XMMatrixTranspose(XMLoadFloat4x4(m + i)));
			std::memcpy(reinterpret_cast<FLOAT*>(buffer + itm.offset + sizeof(FLOAT) * 4 * itm.columns * i), &mt, sizeof(FLOAT) * 4 * (itm.columns - 1) + sizeof(FLOAT) * itm.rows);
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	case Item::DataType::DOUBLE:
	{
		for (uint32 i = 0, j = std::min(count, itm.elements); i < j; i++) {
			XMFLOAT4X4 mt;
			XMStoreFloat4x4(&mt, XMMatrixTranspose(XMLoadFloat4x4(m + i)));
			if (itm.rows > 2)
			{
				__DOUBLE4X4 t(mt);
				std::memcpy(reinterpret_cast<DOUBLE*>(buffer + itm.offset + sizeof(DOUBLE) * 4 * itm.columns * i), (const void*)&t, sizeof(DOUBLE) * 4 * (itm.columns - 1) + sizeof(DOUBLE) * itm.rows);
			}
			else
			{
				__DOUBLE2X4 t(mt);
				std::memcpy(reinterpret_cast<DOUBLE*>(buffer + itm.offset + sizeof(DOUBLE) * 2 * itm.columns * i), (const void*)&t, sizeof(DOUBLE) * 2 * (itm.columns - 1) + sizeof(DOUBLE) * itm.rows);
			}
		}
		if (count < itm.elements) {
			// We should maybe clear any remaining space in the array, but probably no need if the array is already cleaned...
		}
		break;
	}
	}
}





BufferLayoutManager &BufferLayoutManager::GetInstance()
{
	static BufferLayoutManager mgr;
	return mgr;
}

BufferLayoutManager::BufferLayoutManager()
{
}

BufferLayoutManager::~BufferLayoutManager()
{
}

BufferLayoutID BufferLayoutManager::RegisterLayout(String config, String *err)
{
	BufferLayout bl;
	if (!bl.Init(config, err))
		return InvalidBufferLayoutID;
	String cleanConfig = bl.GetConfig();
	auto n = _layoutMap.find(cleanConfig);
	if (n != _layoutMap.end())
		return n->second;
	_layouts.push_back(bl);
	n = _layoutMap.insert(std::make_pair(cleanConfig, (BufferLayoutID)_layouts.size())).first;
	return n->second;
}

const BufferLayout *BufferLayoutManager::GetLayout(BufferLayoutID id)
{
	if (id == InvalidBufferLayoutID || id > _layouts.size())
		return nullptr;
	return &_layouts[id - 1];
}

bool BufferLayoutManager::IsSubsetOf(BufferLayoutID isThis, BufferLayoutID subsetOfThis, String &msg)
{
	if (isThis == InvalidBufferLayoutID || subsetOfThis == InvalidBufferLayoutID)
		return false;
	UINT64 id = UINT64(isThis) << 32 | UINT64(subsetOfThis);
	auto n = _subsetCache.find(id);
	if (n != _subsetCache.end()) {
		if (!n->second.b)
			msg = n->second.msg;
		return n->second.b;
	}
	bool b = _layouts[isThis - 1].IsSubsetOf(_layouts[subsetOfThis - 1], msg);
	_subsetCache[id] = {b, msg};
	return b;
}