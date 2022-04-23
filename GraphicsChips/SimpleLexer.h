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
#include <regex>


namespace m3d
{

#define RULE_INVALID 0
#define RULE_EOF 1
#define RULE_LINEBREAK 2
#define RULE_WHITESPACE 3
#define RULE_STRING 4
#define RULE_FLOAT 5
#define RULE_INTEGER 6
#define RULE_HEXINT 7
#define RULE_QUOTED_STRING 8
#define RULE_SINGLE_LINE_COMMENT 9
#define RULE_MULTILINE_COMMENT 10
#define RULE_IDENTIFICATOR 49
#define RULE_OPERATOR 50
#define RULE_LPAREN (RULE_OPERATOR + '(')
#define RULE_RPAREN (RULE_OPERATOR + ')')
#define RULE_LCBRACKET (RULE_OPERATOR + '{')
#define RULE_RCBRACKET (RULE_OPERATOR + '}')
#define RULE_LBRACKET (RULE_OPERATOR + '[')
#define RULE_RBRACKET (RULE_OPERATOR + ']')
#define RULE_ASSIGN (RULE_OPERATOR + '=')
#define RULE_PLUS (RULE_OPERATOR + '+')
#define RULE_MINUS (RULE_OPERATOR + '-')
#define RULE_DOT (RULE_OPERATOR + '.')
#define RULE_COMMA (RULE_OPERATOR + ',')
#define RULE_SEMICOLON (RULE_OPERATOR + ';')
#define RULE_COLON (RULE_OPERATOR + ':')







class GRAPHICSCHIPS_API SimpleLexer
{
public:
	typedef uint32 RuleID;

	struct Token
	{
		RuleID id;
		const Char *str;
		uint32 line;
		uint32 row;
		uint32 pos;
		uint32 length;

		Token() : id(RULE_INVALID), str(nullptr), line(0), row(0), pos(0), length(0) {}
		Token(RuleID id, const Char *str, uint32 line, uint32 row, uint32 pos, uint32 length) : id(id), str(str), line(line), row(row), pos(pos), length(length) {}

		String AsString() const { return String(str, length); }
		String AsQuotedString() const { return String(str + 1, length - 2); }
	};

	struct Rule;

	typedef RuleID (*RuleCallback)(const Rule &rule, const std::cmatch &rmatch);

	struct Rule
	{
		RuleID id;
		std::regex rule;
		List<Rule> subrules;
		RuleCallback callback;

		Rule() : id(RULE_INVALID), callback(nullptr) {}
		Rule(RuleID id, std::regex rule, const List<Rule> &subrules) : id(id), rule(rule), subrules(subrules), callback(nullptr) {}
		Rule(RuleID id, std::regex rule, RuleCallback callback = nullptr) : id(id), rule(rule), callback(callback) {}
	};

	typedef List<Token> TokenList;

	SimpleLexer() {}
	~SimpleLexer() {}

	void Init(const List<Rule> &rules);

	bool Tokenize(String txt, String filename, String *error);

	Token PrevToken() const;
	Token NextToken();
	Token PeekToken() const;

	String ErrorStr(const Token &t, String msg = MTEXT("Unexpected token: \'%1\'."));

private:
	Token _match(const Char *s, const Char *e, const List<Rule> &rules, uint32 pos, uint32 line, uint32 row);

	List<Rule> _rules;

	String _text;
	uint32 _nextToken;
	TokenList _tokens;
	String _filename;

};


}