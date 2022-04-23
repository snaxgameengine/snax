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
#include "ExpressionParser.h"
#include "ExpressionParserNodes.h"
#include "ExpressionParserTree.h"
#include "M3DCore/Containers.h"
#include "M3DEngine/Engine.h"

using namespace m3d;


static m3d::Map<String, ExprNodeFactFunc> tokenMap;


ExpressionParser::ExpressionParser()
{
	if (!tokenMap.size()) {
		tokenMap.insert(std::make_pair(MTEXT("+"), &AddExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("-"), &SubExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("*"), &MulOprExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("/"), &DivExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("^"), &PowOprExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("%"), &IntModExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("-u"), &NegExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("+u"), &PosExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("!"), &NotExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("<"), &LessExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("<="), &LessEqExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT(">"), &GreaterExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT(">="), &GreaterEqExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("=="), &EqExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("!="), &NotEqExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("&&"), &AndExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("||"), &OrExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("?"), &IfThenElseExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("min"), &MinExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("max"), &MaxExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("clamp"), &ClampExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("saturate"), &SaturateExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("sin"), &SinExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("cos"), &CosExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("tan"), &TanExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("asin"), &aSinExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("acos"), &aCosExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("atan"), &aTanExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("abs"), &AbsExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("sqrt"), &SqrtExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("floor"), &FloorExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("ceil"), &CeilExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("round"), &RoundExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("sinh"), &SinhExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("cosh"), &CoshExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("tanh"), &TanhExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("log"), &LogExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("ln"), &LnExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("pow"), &PowExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("exp"), &ExpExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("fmod"), &FmodExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("frac"), &FracExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("lerp"), &LerpExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("sign"), &SignExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("rad"), &RadExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("deg"), &DegExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("isnan"), &IsNaNExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("isinf"), &IsInfExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("rand"), &RandExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("pi"), &PiExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("dot"), &DotExprNode::create));
		tokenMap.insert(std::make_pair(MTEXT("mul"), &MulExprNode::create));
	}

	_exp = nullptr;
	_callback = nullptr;
}

ExpressionParser::~ExpressionParser()
{
	if (_exp)
		delete _exp;
}

ExpressionParser &ExpressionParser::operator=(const ExpressionParser &rhs)
{
	if (_exp)
		delete _exp;
	_exp = nullptr;
	if (rhs._exp)
		_exp = rhs._exp->duplicate();
	_callback = rhs._callback;
	return *this;
}

bool ExpressionParser::Parse(const String &expression)
{
//	 0. ,
//	 1. ()
//	 2. func
//	 3. ! + - (unary)
//	 4. ^
//	 5. * / %
//	 6. + -
//	 7. < <= > >=
//	 8. == !=
//	 9. &&
//	10. ||
//	11. ?:

	if (_exp)
		delete _exp;
	_exp = 0;

	if (expression.size() == 0)
		return false;

	List<String> tokens;


	const Char *c = expression.c_str();
	bool b = false;
	while (*c) {
		uint32 i = (uint32)strcspn(c, MTEXT("+-/*!=<>()^%&|,.?: "));
		if (i == 0)
			i = 1;
		String token;
		token.append(c, i);
		if (token != MTEXT(" ")) {
			if (tokens.size() && token == MTEXT("=") && (tokens.back() == MTEXT("!") || tokens.back() == MTEXT("=") || tokens.back() == MTEXT(">") || tokens.back() == MTEXT("<")))
				tokens.back() += MTEXT("="); // operator ==
			else if (tokens.size() && token == MTEXT("&") && tokens.back() == MTEXT("&"))
				tokens.back() += MTEXT("&"); // operator &&
			else if (tokens.size() && token == MTEXT("|") && tokens.back() == MTEXT("|"))
				tokens.back() += MTEXT("|"); // operator ||
			else if (tokens.size() && (token[0] >= L'0' && token[0] <= L'9' && tokens.back()[tokens.back().size() - 1] == L'.' || tokens.back()[0] >= L'0' && tokens.back()[0] <= L'9' && token == MTEXT(".")))
				tokens.back() += token; // Numbers eg 343.34, .34
			else
				tokens.push_back(token);
		}
		c += i;
	}

	List<ExprNode*> outList;
	List<ExprNode*> tmpList;
	tmpList.push_back(0);
	ExprNode *last = 0;

	bool ok = true;

	for (uint32 i = 0; i < tokens.size(); i++) {
		if ((!last || last->isOperator()) && (i == 0 || tokens[i - 1] != MTEXT(")")) && (tokens[i] == MTEXT("+") || tokens[i] == MTEXT("-"))) {
			tokens[i] += MTEXT("u"); // unary +/-
		}
		if (tokens[i] == MTEXT("(")) {
			tmpList.push_back(0);
			last = 0;
			continue;
		}
		if (tokens[i] == MTEXT(")")) {
			while (tmpList.back()) {
				outList.push_back(tmpList.back());
				tmpList.pop_back();
			}
			if (tmpList.size() == 1) {
				ok = false;
				break;
			}
			tmpList.pop_back();
			last = 0;
			continue;
		}
		if (tokens[i] == MTEXT(",")) {
			while (tmpList.back()) {
				outList.push_back(tmpList.back());
				tmpList.pop_back();
			}
			last = 0;
			if (tmpList.size() == 1) {
				ok = false;
				break;
			}
			continue;
		}
		if (tokens[i] == MTEXT(":")) {
			while (tmpList.back() && dynamic_cast<IfThenElseExprNode*>(tmpList.back()) == 0) {
				outList.push_back(tmpList.back());
				tmpList.pop_back();
			}
			last = 0;
			if (dynamic_cast<IfThenElseExprNode*>(tmpList.back()) == 0) {
				ok = false;
				break;
			}
			continue;
		}
		if (tokens[i] == MTEXT(".")) {
			continue;
		}

		auto n = tokenMap.find(tokens[i]);
		ExprNode *en = 0;
		if (n == tokenMap.end()) { // Not found, number or variable...
			value f = 0.0f;
			if (strUtils::toNum(tokens[i], f)) { // number?
				en = ConstExprNode::create(f);
			}
			else if (i > 0 && tokens[i - 1] == MTEXT(".")) {
				String s = tokens[i];
				en = MemberExprNode::create(s); 
			}
			else {// variable
				uint32 index = _callback ? _callback->Register(tokens[i]) : -1;
				if (index == -1) {
					ok = false;
					break;
				}
				en = CallbackExprNode::create(index, _callback);
			}
		}
		else
			en = (*n->second)();

		// is operand: last!=0 && last is operand => error!
		// is function: next != ( => error!
		if ((!en->isOperator() && last && !last->isOperator()) || (en->isOperator() && en->precedence() == 2 && (i == (tokens.size() - 1) || tokens[i + 1] != MTEXT("(")))) {
			delete en;
			ok = false;
			break;
		}
		if (en->isOperator()) {
			while (tmpList.back() != 0 && tmpList.back()->precedence() <= en->precedence()) { // supporting ^ (pow): '<=' should be '<' for this operator?? (right-to-left)
				outList.push_back(tmpList.back());
				tmpList.pop_back();
			}
			tmpList.push_back(en);
		}		
		else
			outList.push_back(en);
		last = en;
	}

	for (;tmpList.back(); tmpList.pop_back())
		outList.push_back(tmpList.back());

	ExprNode *root = outList.size() ? outList.back() : 0;

	if (!ok || tmpList.size() != 1 || !root || !root->build(outList) || outList.size() > 0) {
		if (ok && tmpList.size() == 1 && root)
			delete root;

		for (uint32 i = 0; i < outList.size(); i++)
			if (outList[i])
				delete outList[i];
		for (uint32 i = 0; i < tmpList.size(); i++)
			if (tmpList[i])
				delete tmpList[i];

		return false;
	}

	_exp = root;

	return true;
}

ExprValue ExpressionParser::Calculate() const 
{ 
	if (_exp) {
		OutputDebugStringA((_exp->hlsl() + MTEXT("\n")).c_str());
		return (*_exp)(); 
	}
	return ExprValue();
}