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
#include "ExpressionParser2.h"
#include "ExpressionParserTree.h"
#include "M3DCore/Containers.h"
#include "M3DCore/MString.h"

using namespace m3d;
using namespace m3d::expr;


ExpressionParser::ExpressionParser()
{
	_exp = nullptr;
	_callback = nullptr;
}

ExpressionParser::~ExpressionParser()
{
}

ExpressionParser &ExpressionParser::operator=(const ExpressionParser &rhs)
{
	_exp.release();
	if (rhs._exp)
		_exp.reset(rhs._exp->duplicate());
	_callback = rhs._callback;
	return *this;
}


//
// ============================================= FOR REFERENCE =============================================
//
// Precedence	Operator			Description													Associativity
// =================================================================================================
// 1			::					Scope resolution											Left-to-right
// 2			a++   a--			Suffix/postfix increment and decrement
// 				type()   type{}		Functional cast
// 				a()					Function call
// 				a[]					Subscript
// 				.   ->				Member access
// 3			++a   --a			Prefix increment and decrement								Right-to-left
// 				+a   -a				Unary plus and minus
// 				!   ~				Logical NOT and bitwise NOT
// 				(type)				C-style cast
// 				*a					Indirection (dereference)
// 				&a					Address-of
// 				sizeof				Size-of[note 1]
// 				new   new[]			Dynamic memory allocation
// 				delete   delete[]	Dynamic memory deallocation
// 4			.*   ->*			Pointer-to-member											Left-to-right
// 5			a*b   a/b   a%b		Multiplication, division, and remainder
// 6			a+b   a-b			Addition and subtraction
// 7			<<   >>				Bitwise left shift and right shift
// 8			<=>					Three-way comparison operator (since C++20)
// 9			<   <=				For relational operators < and <= respectively
// 				>   >=				For relational operators > and >= respectively
// 10			==   !=				For relational operators = and != respectively
// 11			&					Bitwise AND
// 12			^					Bitwise XOR (exclusive or)
// 13			|					Bitwise OR (inclusive or)
// 14			&&					Logical AND
// 15			||					Logical OR
// 16			a?b:c				Ternary conditional[note 2]									Right-to-left
// 				throw				throw operator
// 				=					Direct assignment (provided by default for C++ classes)
// 				+=   -=				Compound assignment by sum and difference
// 				*=   /=   %=		Compound assignment by product, quotient, and remainder
// 				<<=   >>=			Compound assignment by bitwise left shift and right shift
// 				&=   ^=   |=		Compound assignment by bitwise AND, XOR, and OR
// 17			,					Comma														Left-to-right

// func(...) obj[...] .member +u -u ! ~ (cast) * / % + - << >> < <= > >= == != & ^ | && || ?: , (parens) 1.234e[+-]5

bool ExpressionParser::parse(const String &expression, String *msg)
{

	enum struct TokenID { NIL, VARIABLE, MEMBER_FIELD, NUMBER, FUNC, MEMBER_FUNC, CCAST, LPAREN, RPAREN, LBRACKET, RBRACKET, SUBSCRIPT, DOT, UMINUS, UPLUS, NOT, BWNOT, MULT, DIV, MOD, PLUS, MINUS, SHFT_LFT, SHFT_RGHT, LESS, LESSEQ, GREATER, GREATEREQ, EQUAL, NEQUAL, BWAND, BWXOR, BWOR, AND, OR, QUESTION, COLON, COMMA, TCOND };
	enum TokenClass { NIL = 1, OPERAND = 2, NUMBER, VARIABLE, MEMBER = 4, BOP = 4, LUOP = 8, RUOP = 16 };

	const uint32 PRECEDENCE[39] =    {0,0,0,0,2,2, 3,0,0,2,2,2,2, 3, 3, 3, 3,5,5,5,6,6,7,7,9,9,9,9,10,10,11,12,13,14,15,16,16,17,16};
	const uint32 ASSOCIATIVITY[39] = {0,0,0,0,1,1,(uint32)-1,0,0,1,1,1,1,(uint32)-1,(uint32)-1,(uint32)-1,(uint32)-1,1,1,1,1,1,1,1,1,1,1,1, 1, 1, 1, 1, 1, 1, 1,(uint32)-1,(uint32)-1, 1,(uint32)-1};

	struct Token
	{
		TokenID id;
		const Char *start;
		const Char *end;
		uint32 pCount;
		uint32 idx;
	};

	struct ExprException
	{
		const Char *msg;
		const Char *start;
		const Char *end;

		ExprException(const Char *msg, const Char *start = nullptr, const Char *end = nullptr) : msg(msg), start(start), end(end) {}
	};

#if defined(DEBUG) | defined(_DEBUG)
//#define DEBUGEXPR
#endif

	String ex = strUtils::toLower(expression);

	try {
		List<Token> tokens;
		const Char *c = ex.c_str();
		const Char *end = c + ex.size();
		auto peekNext = [&]() -> Char { if (c + 1 == end) return '\0'; return *(c + 1); };
		auto popNext = [&]() -> Char { ++c; return peekNext(); };
		auto prevToken = [&]() -> Token { if (tokens.empty()) return {TokenID::NIL, nullptr, nullptr, 0}; return tokens.back(); };
		for (; c < end; c++) {
			const Char *start = c;
			const Char t = *c;
			if (t == ' ' || t == '\t')
				continue;
			if (t == '_' || t >= 'a' && t <= 'z' || t >= 'A' && t <= 'Z') {
				for (Char s = peekNext(); s == '_' || s >= 'a' && s <= 'z' || s >= 'A' && s <= 'Z' || s >= '0' && s <= '9'; s = popNext());
				tokens.push_back( { prevToken().id == TokenID::DOT ? TokenID::MEMBER_FIELD : TokenID::VARIABLE, start, c, 0 } );
			}
			else if (t >= '0' && t <= '9' || t == '.' && peekNext() >= '0' && peekNext() <= '9') {
				if (t == '0' && (peekNext() == 'x' || peekNext() == 'X')) { // hex
					throw ExprException(MTEXT("Unexpected Character"), c + 1, c + 1);
				}
				else {
					bool dot = t == '.', e = false, reqNbr = dot;
					for (Char s = peekNext(); true; s = popNext()) {
						if (s >= '0' && s <= '9')
							reqNbr = false;
						else if (s == '.' && !dot && !e)
							dot = true;
						else if ((s == 'e' || s == 'E') && !e)
							e = reqNbr = true;
						else if ((s == '+' || s == '-') && e && reqNbr)
							e = reqNbr;
						else if (t == '.' || t == '_' || t >= 'a' && t <= 'z' || t >= 'A' && t <= 'Z')
							throw ExprException(MTEXT("Unexpected Character"), c, c);
						else if (reqNbr)
							throw ExprException(MTEXT("Unexpected Character"), c, c);
						else
							break;
					}
					tokens.push_back({ TokenID::NUMBER, start, c, 0 });
				}
			}
			else if (t == '<' && peekNext() == '<') {
				popNext();
				tokens.push_back( { TokenID::SHFT_LFT, start, c, 2 } );
			} // operand -u(
			else if (t == '>' && peekNext() == '>') {
				popNext();
				tokens.push_back({ TokenID::SHFT_RGHT, start, c, 2 });
			}// operand (
			else if (t == '<' && peekNext() == '=') {
				popNext();
				tokens.push_back({ TokenID::LESSEQ, start, c, 2 });
			}// operand (
			else if (t == '>' && peekNext() == '=') {
				popNext();
				tokens.push_back({ TokenID::GREATEREQ, start, c, 2 });
			}// operand (
			else if (t == '&' && peekNext() == '&') {
				popNext();
				tokens.push_back({ TokenID::AND, start, c, 2 });
			}// operand (
			else if (t == '|' && peekNext() == '|') {
				popNext();
				tokens.push_back({ TokenID::OR, start, c, 2 });
			}// operand (
			else if (t == '=' && peekNext() == '=') {
				popNext();
				tokens.push_back({ TokenID::EQUAL, start, c, 2 });
			}// operand (
			else if (t == '!' && peekNext() == '=') {
				popNext();
				tokens.push_back({ TokenID::NEQUAL, start, c, 2 });
			}// operand (
			else if (t == '-' || t == '+') {
				TokenID p = prevToken().id;
				if (p == TokenID::VARIABLE || p == TokenID::MEMBER_FIELD || p == TokenID::NUMBER || p == TokenID::RPAREN || p == TokenID::RBRACKET)
					tokens.push_back({ t == '+' ? TokenID::PLUS : TokenID::MINUS, start, c, 2 });// operand (
				else 
					tokens.push_back({ t == '+' ? TokenID::UPLUS : TokenID::UMINUS, start, c, 1 });// operand ( (not -u)
			}
			else {
				if (t == '(') {
					if (prevToken().id == TokenID::VARIABLE)
						tokens.back().id = TokenID::FUNC;
					else if (prevToken().id == TokenID::MEMBER_FIELD)
						tokens.back().id = TokenID::MEMBER_FUNC;
				}
				switch (t) 
				{
				case '!': tokens.push_back({TokenID::NOT, start, c, 1}); break;
				case '%': tokens.push_back({TokenID::MOD, start, c, 2}); break;
				case '&': tokens.push_back({TokenID::BWAND, start, c, 2}); break;
				case '(': tokens.push_back({TokenID::LPAREN, start, c, 0}); break;
				case ')': tokens.push_back({TokenID::RPAREN, start, c, 0}); break;//binary op
				case '*': tokens.push_back({TokenID::MULT, start, c, 2}); break;
				case ',': tokens.push_back({TokenID::COMMA, start, c, 2}); break;
				case '.': tokens.push_back({TokenID::DOT, start, c, 2}); break;// member
				case '/': tokens.push_back({TokenID::DIV, start, c, 2}); break;
				case ':': tokens.push_back({TokenID::COLON, start, c, 3}); break;
				case '<': tokens.push_back({TokenID::LESS, start, c, 2}); break;
				case '=': tokens.push_back({TokenID::EQUAL, start, c, 2}); break;
				case '>': tokens.push_back({TokenID::GREATER, start, c, 2}); break;
				case '?': tokens.push_back({TokenID::QUESTION, start, c, 3}); break;
				case '[': tokens.push_back({TokenID::LBRACKET, start, c, 2}); break;
				case ']': tokens.push_back({TokenID::RBRACKET, start, c, 2}); break;//binary op
				case '^': tokens.push_back({TokenID::BWXOR, start, c, 2}); break;
				case '|': tokens.push_back({TokenID::BWOR, start, c, 2}); break;
				case '~': tokens.push_back({TokenID::BWNOT, start, c, 1}); break;
				default: throw ExprException(MTEXT("Unrecognised Symbol"), c, c); // Invalid symbol!
				}
			}
		}

#ifdef DEBUGEXPR
		OutputDebugStringA(MTEXT("\n"));
		for (int i = 0; i < tokens.size(); i++) {
			String s = strUtils::ConstructString(MTEXT("%2    ")).arg((uint32)tokens[i].id).arg(String(tokens[i].start, (size_t)(tokens[i].end - tokens[i].start + 1)));
			OutputDebugStringA(s.c_str());
		}
		OutputDebugStringA(MTEXT("\n"));
#endif

		List<Token> operatorStack;
		List<Token> output;

		for (size_t i = 0; i < tokens.size(); i++) {
			tokens[i].idx = i;
			Token t = tokens[i];
			// If the incoming symbols is an operand, push out.
			if (t.id == TokenID::VARIABLE || t.id == TokenID::MEMBER_FIELD || t.id == TokenID::NUMBER)
				output.push_back(t);
			// If the incoming symbol is a left parenthesis, push it on the stack.
			else if (t.id == TokenID::LPAREN)
				operatorStack.push_back(t);
			//else if (t.id == TokenID::LBRACKET)
			//	operatorStack.push_back(t);
			// If the incoming symbol is a comma, pop operator stack until ( is found.
			else if (t.id == TokenID::COMMA) {
				while (true) {
					if (operatorStack.empty())
						throw ExprException(MTEXT("Unexpected Symbol"), t.start, t.end);
					if (operatorStack.back().id == TokenID::LPAREN) {
						operatorStack.back().pCount++; // Count the number of commas inside a (). Needed by n-nary operators!
						break;
					}
					output.push_back(operatorStack.back());
					operatorStack.pop_back();
				}
			}
			// If the incoming symbol is a right parenthesis: discard the right parenthesis, pop and push out the stack symbols until you see a left parenthesis.Pop the left parenthesis and discard it.
			else if (t.id == TokenID::RPAREN) {
				const Char *start = nullptr;
				while (true) {
					if (operatorStack.empty())
						throw ExprException(MTEXT("Unexpected Symbol"), t.start, t.end);
					if (operatorStack.back().id == TokenID::LPAREN) {
						start = operatorStack.back().start;
						uint32 pcount = operatorStack.back().pCount;
						if (operatorStack.back().idx < t.idx - 1)
							pcount++; // We had at least on parameter. Not ().
						operatorStack.pop_back();
						if (!operatorStack.empty() && (operatorStack.back().id == TokenID::FUNC || operatorStack.back().id == TokenID::MEMBER_FUNC))
							operatorStack.back().pCount = pcount;
						break;
					}
					output.push_back(operatorStack.back());
					operatorStack.pop_back();
				}
				// Identify a c-style cast.
				if (!output.empty() && output.back().id == TokenID::VARIABLE) {
					if (i < tokens.size() - 1) {
						Token tn = tokens[i + 1];
						if (tn.id == TokenID::VARIABLE || tn.id == TokenID::FUNC || tn.id == TokenID::NUMBER || tn.id == TokenID::LPAREN) { // + unary operators!
							operatorStack.push_back({TokenID::CCAST, start, t.end});
							output.pop_back();
						}
					}
				}
			}
			else if (t.id == TokenID::RBRACKET) {
				while (true) {
					if (operatorStack.empty())
						throw ExprException(MTEXT("Unexpected Symbol"), t.start, t.end);
					if (operatorStack.back().id == TokenID::LBRACKET) {
						output.push_back({TokenID::SUBSCRIPT, operatorStack.back().start, t.end, 2});
						operatorStack.pop_back();
						break;
					}
					output.push_back(operatorStack.back());
					operatorStack.pop_back();
				}
			}
			// If the incoming symbol is an operator and the stack is empty or contains a left parenthesis on top, push the incoming operator onto the stack.
			else if (operatorStack.empty() || operatorStack.back().id == TokenID::LPAREN)
				operatorStack.push_back(t);
			// If the incoming symbol is an operator and has either higher precedence than the operator on the top of the stack, or has the same precedence as the operator on the top of the stack and is right associative -- push it on the stack.
			else if (PRECEDENCE[(uint32)t.id] < PRECEDENCE[(uint32)operatorStack.back().id] || PRECEDENCE[(uint32)t.id] == PRECEDENCE[(uint32)operatorStack.back().id] && ASSOCIATIVITY[(uint32)t.id] == -1)
					operatorStack.push_back(t);
			// If the incoming symbol is function, and the last operator was a member-access-operator, push it onto the stack.
			else if (operatorStack.back().id == TokenID::DOT && t.id == TokenID::MEMBER_FUNC)
				operatorStack.push_back(t);
			else {
				// If the incoming symbol is an operator and has either lower precedence than the operator on the top of the stack, 
				// or has the same precedence as the operator on the top of the stack and is left associative-- continue to pop the stack until this is not true.Then, push the incoming operator.
				while (!operatorStack.empty() && (operatorStack.back().id != TokenID::LPAREN && operatorStack.back().id != TokenID::LBRACKET) && (PRECEDENCE[(uint32)t.id] > PRECEDENCE[(uint32)operatorStack.back().id] || PRECEDENCE[(uint32)t.id] == PRECEDENCE[(uint32)operatorStack.back().id] && ASSOCIATIVITY[(uint32)t.id] == 1)) {
					output.push_back(operatorStack.back());
					operatorStack.pop_back();
				}
				operatorStack.push_back(t);
			}
		}
		// At the end of the expression, pop and push out all operators on the stack. (No parentheses should remain.)
		while (!operatorStack.empty()) {
			if (operatorStack.back().id == TokenID::LPAREN)
				throw ExprException("Missing )", operatorStack.back().start, operatorStack.back().end);
			if (operatorStack.back().id == TokenID::LBRACKET)
				throw ExprException(MTEXT("Missing ]"), operatorStack.back().start, operatorStack.back().end);
			output.push_back(operatorStack.back());
			operatorStack.pop_back();
		}

#ifdef DEBUGEXPR
		for (int i = 0; i < output.size(); i++) {
			String s = strUtils::ConstructString(MTEXT("%2    ")).arg((uint32)output[i].id).arg(String(output[i].start, (size_t)(output[i].end - output[i].start + 1)));
			OutputDebugStringA(s.c_str());
		}
		OutputDebugStringA(MTEXT("\n\n"));
#endif

		ExprNodePtrVector nodes;

		for (size_t i = 0; i < output.size(); i++) {
			Token t = output[i];
			String name = strUtils::toLower(String(t.start, t.end - t.start + 1));
			ExprNodePtr node;

			uint32 pcount = t.pCount;

			switch (t.id) 
			{
			case TokenID::VARIABLE:
			{
				if (name == MTEXT("pi"))
					node.reset(ValueNode::create(XM_PI));
				else {
					uint32 idx = _callback->Register(name);
					if (idx == -1)
						throw ExprException(MTEXT("Invalid Identifier"), t.start, t.end);
					node.reset(CallbackOperatorNode::create(idx, _callback));
				}
				break;
			}
			case TokenID::MEMBER_FIELD: 
			{
				if ((i + 1) == output.size() || output[i + 1].id != TokenID::DOT)
					throw ExprException(MTEXT("Unexpected Token"), t.start, t.end); 
				// The only member we have is the swizzle!
				if (name.size() > 4)
					throw ExprException(MTEXT("Unrecognized Token"), t.start, t.end);
				uint32 mask = 0;
				for (size_t i = 0; i < 4; i++) {
					if (i < name.size()) {
						Char c = name[i];
						if (c == 'x');
						else if (c == 'y') mask |= (0x1 << (i * 8));
						else if (c == 'z') mask |= (0x2 << (i * 8));
						else if (c == 'w') mask |= (0x3 << (i * 8));
						else throw ExprException(MTEXT("Unrecognized Token"), t.start, t.end);
					}
					else
						mask |= 0xFF << (i * 8);
				}
				node.reset(SwizzleOperatorNode::create(mask));
				pcount = 1;
				i++; // Skip '.'
				break;
			}
			case TokenID::NUMBER: 
			{
				float64 nbr = 0.0;
				strUtils::toNum(name, nbr);
				node.reset(ValueNode::create(nbr));
				break;
			}
			case TokenID::FUNC:
			{
				static m3d::Map<String, ExprNodeFactoryFunc> functionMap = []() {
					m3d::Map<String, ExprNodeFactoryFunc> functionMap;
					functionMap.insert(std::make_pair(MTEXT("saturate"), &SaturateOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("sin"), &SinOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("cos"), &CosOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("tan"), &TanOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("asin"), &aSinOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("acos"), &aCosOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("atan"), &aTanOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("abs"), &AbsOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("sqrt"), &SqrtOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("rsqrt"), &RSqrtOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("floor"), &FloorOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("ceil"), &CeilOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("round"), &RoundOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("sinh"), &SinhOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("cosh"), &CoshOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("tanh"), &TanhOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("log10"), &Log10OperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("log"), &LogOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("ln"), &LogOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("log2"), &Log2OperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("exp"), &ExpOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("exp2"), &Exp2OperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("frac"), &FracOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("sign"), &SignOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("rad"), &RadOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("deg"), &DegOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("isnan"), &IsNaNOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("isinf"), &IsInfOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("trunc"), &TruncOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("all"), &AllOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("any"), &AnyOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("length"), &LengthOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("normalize"), &NormalizeOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("determinant"), &DeterminantOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("transpose"), &TransposeOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("inverse"), &InverseOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("min"), &MinOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("max"), &MaxOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("pow"), &PowOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("fmod"), &FmodOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("atan2"), &ATan2OperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("ldexp"), &LDExpOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("step"), &StepOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("dot"), &DotOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("cross"), &CrossOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("distance"), &DistanceOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("mul"), &MulFuncOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("reflect"), &ReflectOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("clamp"), &ClampOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("lerp"), &LerpOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("lit"), &LitOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("mad"), &MadOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("smoothstep"), &SmoothstepOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("refract"), &RefractOperatorNode::create));
					functionMap.insert(std::make_pair(MTEXT("rand"), &RandOperatorNode::create));
					return std::move(functionMap);
				}();
				
				auto a = functionMap.find(name);
				if (a != functionMap.end())
					node.reset((a->second)());
				else {
					m3d::Map<String, std::function<ExprNode * (uint32)>> nAryfunctionMap;
					nAryfunctionMap.insert(std::make_pair(MTEXT("vector"), [](uint32 pcount) -> ExprNode* { return VectorConstructorOperatorNode::create(4, pcount); }));
					nAryfunctionMap.insert(std::make_pair(MTEXT("vector3"), [](uint32 pcount) -> ExprNode* { return VectorConstructorOperatorNode::create(3, pcount); }));
					nAryfunctionMap.insert(std::make_pair(MTEXT("vector2"), [](uint32 pcount) -> ExprNode* { return VectorConstructorOperatorNode::create(2, pcount); }));

					auto a = nAryfunctionMap.find(name);
					if (a != nAryfunctionMap.end())
						node.reset((a->second)(pcount));
				}

				break;
			}
			case TokenID::MEMBER_FUNC: break;
			case TokenID::CCAST: break;
			case TokenID::LPAREN: break;
			case TokenID::RPAREN: break;
			case TokenID::LBRACKET: break;
			case TokenID::RBRACKET: break;
			case TokenID::SUBSCRIPT: node.reset(SubscriptOperatorNode::create()); break;
			case TokenID::DOT: break;
			case TokenID::UMINUS: node.reset(NegOperatorNode::create()); break;
			case TokenID::UPLUS: node.reset(PosOperatorNode::create()); break;
			case TokenID::NOT: node.reset(NotOperatorNode::create()); break;
			case TokenID::BWNOT: node.reset(BWNotOperatorNode::create()); break;
			case TokenID::MULT: node.reset(MulOperatorNode::create()); break;
			case TokenID::DIV: node.reset(DivOperatorNode::create()); break;
			case TokenID::MOD: node.reset(IntModOperatorNode::create()); break;
			case TokenID::PLUS: node.reset(AddOperatorNode::create()); break;
			case TokenID::MINUS: node.reset(SubOperatorNode::create()); break;
			case TokenID::SHFT_LFT: node.reset(ShiftLeftOperatorNode::create()); break;
			case TokenID::SHFT_RGHT: node.reset(ShiftRightOperatorNode::create()); break;
			case TokenID::LESS: node.reset(LessOperatorNode::create()); break;
			case TokenID::LESSEQ: node.reset(LessEqOperatorNode::create()); break;
			case TokenID::GREATER: node.reset(GreaterOperatorNode::create()); break;
			case TokenID::GREATEREQ: node.reset(GreaterEqOperatorNode::create()); break;
			case TokenID::EQUAL: node.reset(EqOperatorNode::create()); break;
			case TokenID::NEQUAL: node.reset(NotEqOperatorNode::create()); break;
			case TokenID::BWAND: node.reset(BWAndOperatorNode::create()); break;
			case TokenID::BWXOR: node.reset(BWXOrOperatorNode::create()); break;
			case TokenID::BWOR: node.reset(BWOrOperatorNode::create()); break;
			case TokenID::AND: node.reset(AndOperatorNode::create()); break;
			case TokenID::OR: node.reset(OrOperatorNode::create()); break;
				// TODO: ** power operator! (Use PowOprOperatorNode!)
			case TokenID::QUESTION: break; 
			case TokenID::COLON: 
			{
				if ((i + 1) == output.size() || output[i + 1].id != TokenID::QUESTION)
					throw ExprException(MTEXT("Unexpected Token"), t.start, t.end);
				node.reset(IfThenElseOperatorNode::create());
				i++; // Skip '?'
				break;
			}
			case TokenID::COMMA: continue; // Skip comma!
			case TokenID::TCOND: node.reset(IfThenElseOperatorNode::create()); break;
			default:;
			}
			if (!node)
				throw ExprException(MTEXT("Unexpected Token"), t.start, t.end);

			if (node->isOperator() && ((OperatorNode*)node.get())->pcount() != pcount)
				throw ExprException(MTEXT("Wrong Number of Arguments"), t.start, t.end);

			nodes.push_back(std::move(node));
		}

		if (nodes.empty())
			throw ExprException(MTEXT("Empty Expression"));

		_exp = std::move(nodes.back());
		if (!_exp->build(nodes) || !nodes.empty())
			throw ExprException(MTEXT("Invalid Expression"));
	}
	catch(const ExprException &e) {
		String message = e.msg;
		if (e.start && e.end)
			message = strUtils::ConstructString(MTEXT("[%1, %2] %3: %4.")).arg(e.start - ex.c_str() + 1).arg(e.end - ex.c_str() + 2).arg(e.msg).arg(String(e.start, e.end - e.start + 1));
		else
			message = strUtils::ConstructString(MTEXT("[%1, %2] %3: %4.")).arg(1).arg(ex.length() + 1).arg(e.msg).arg(ex);
		if (msg)
			*msg = message;
#ifdef DEBUGEXPR
		OutputDebugStringA(message.c_str());
		OutputDebugStringA(MTEXT("\n"));
#endif
		_exp.reset();
	}



	return _exp != nullptr;
}

ExprValue ExpressionParser::calculate() const
{
	if (_exp) {
		const OperandNode *n = (*_exp)();
		if (n) {
			if (n->asValue())
				return ExprValue(n->asValue()->value());
			else if (n->asVector())
				return ExprValue(n->asVector()->vector());
			else if (n->asMatrix())
				return ExprValue(n->asMatrix()->matrix());
		}
	}
	return ExprValue();
}