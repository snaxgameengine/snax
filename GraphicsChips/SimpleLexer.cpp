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
#include "SimpleLexer.h"

using namespace m3d;



void SimpleLexer::Init(const List<Rule> &rules)
{
	_rules = rules;
}

bool SimpleLexer::Tokenize(String txt, String filename, String *error)
{
	_text = txt;

	_filename = filename;

	_nextToken = 0;
	_tokens.clear();

	const Char * const str = _text.c_str();
	const Char *s = str;
	uint32 l = (uint32)_text.length();

	uint32 line = 0;
	uint32 row = 0;
	uint32 pos = 0;

	while (s < str + l) {
		Token t = _match(s, str + l, _rules, pos, line, row);
		
		if (t.id == RULE_INVALID) {
			if (error)
				*error = strUtils::ConstructString(MTEXT("%1(%2,%3-%3): Unrecognized token \'%4\'.\n")).arg(filename).arg(t.line + 1).arg(t.row + 1).arg(String(t.str));
			return false;
		}

		s += t.length;

		if (t.id == RULE_LINEBREAK) {
			line += (uint32)t.length;
			row = 0;
		}
		else if (t.id == RULE_MULTILINE_COMMENT) {
			for (uint32 i = 0; i < t.length; i++) {
				if (*(t.str + i) == '\n') {
					line += 1;
					row = 0;
				}
				else 
					row++;
			}
		}
		else {
			row += (uint32)t.length;
			if (t.id != RULE_WHITESPACE && t.id != RULE_SINGLE_LINE_COMMENT)
				_tokens.push_back(t);
		}
	}

	return true;
}

SimpleLexer::Token SimpleLexer::PrevToken() const
{
	if (_nextToken == 0)
		return Token(RULE_INVALID, nullptr, 0, 0, 0, 0);
	return _tokens[_nextToken - 1];
}

SimpleLexer::Token SimpleLexer::NextToken()
{
	if (_nextToken >= _tokens.size())
		return _tokens.size() ? Token(RULE_EOF, nullptr, _tokens.back().line, _tokens.back().row + _tokens.back().length, _tokens.back().pos + _tokens.back().length, 0) : Token(RULE_EOF, nullptr, 0, 0, 0, 0);
	return _tokens[_nextToken++];
}

SimpleLexer::Token SimpleLexer::PeekToken() const
{
	if (_nextToken >= _tokens.size())
		return _tokens.size() ? Token(RULE_EOF, nullptr, _tokens.back().line, _tokens.back().row + _tokens.back().length, _tokens.back().pos + _tokens.back().length, 0) : Token(RULE_EOF, nullptr, 0, 0, 0, 0);
	return _tokens[_nextToken];
}

SimpleLexer::Token SimpleLexer::_match(const Char *s, const Char *e, const List<Rule> &rules, uint32 pos, uint32 line, uint32 row)
{
	RuleID rid = RULE_INVALID;
	std::cmatch rmatch;	
	for (size_t i = 0; i < rules.size(); i++) {
		const Rule &rule = rules[i];
		if (std::regex_search(s, e, rmatch, rule.rule)) {
			if (!rule.subrules.empty()) 
				return _match(s, s + rmatch.length(), rule.subrules, pos, line, row);
			rid = rule.callback ? rule.callback(rule, rmatch) : rule.id;
			break;
		}
	}
	return Token(rid, s, line, row, pos, (uint32)rmatch.length());
}

String SimpleLexer::ErrorStr(const Token &t, String msg)
{
	if (t.id == RULE_EOF)
		return strUtils::ConstructString(MTEXT("%1(%2,%3): Unexpected end of file.\n")).arg(_filename).arg(t.line + 1).arg(t.row + 1);
	return strUtils::ConstructString(MTEXT("%1(%2,%3-%4): %5\n")).arg(_filename).arg(t.line + 1).arg(t.row + 1).arg(t.row + t.length).arg(strUtils::ConstructString(msg).arg(t.AsString()));
}
