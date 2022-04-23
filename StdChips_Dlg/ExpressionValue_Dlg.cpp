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

#include "StdAfx.h"
#include "ExpressionValue_dlg.h"
#include "StdChips/ExpressionValue.h"
#include "ChipDialogs/FlowLayout.h"

using namespace m3d;


DIALOGDESC_DEF(ExpressionValue_Dlg, EXPRESSIONVALUE_GUID);


ExpressionChip_Dlg::ExpressionChip_Dlg()
{
	ui.setupUi(this);

	_msgTimer.setSingleShot(true);
	connect(&_msgTimer, &QTimer::timeout, this, [this]() { ui.label_msg->setText(_msg); });

	connect(ui.editor, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [this](int i)
	{ 
		if (i == -1) 
			return; 
		//QSignalBlocker b(ui.editor);
		QString name = ui.editor->currentText();
		QString s = ui.editor->currentData().toString();
		ui.editor->setCurrentIndex(-1);
		ui.editor->setCurrentText(s);
		this->GetChip()->GetChip()->SetName(FROMQSTRING(name));
	});

	struct Func
	{
		QString name;
		QString func;
		QString tooltip;
	};

	Func funcs[] = { 
		{ "abs", "abs(a)", "The absolute value of arg1." },
		{ "acos", "acos(a)", "The arc cosine of arg1, expressed in radians." },
		{ "asin", "asin(a)", "The arc sine of arg1, expressed in radians." },
		{ "atan", "atan(a)", "The arc tangent of arg1, expressed in radians." },
		{ "atan2", "atan2(a, b)", "The arc tangent of arg2/arg1, expressed in radians." },
		{ "ceil", "ceil(a)", "Rounds arg1 upward, returning the smallest integral value that is not less than arg1." },
		{ "clamp", "clamp(a, b, c)", "Clamps arg1 to be no less than arg2, and no greater than arg3." },
		{ "cos", "cos(a)", "The cosine of arg1 (radians)." },
		{ "cosh", "cosh(a)", "The hyperbolic cosine of arg1." },
		{ "deg", "deg(a)", "Converts an angle, arg1, given in radians to degrees." },
		{ "exp", "exp(a)", "The base-e exponential function of arg1, which is e raised to the power arg1." },
		{ "exp2", "exp2(a)", "The base-2 exponential function of arg1, which is 2 raised to the power arg1." },
		{ "floor", "floor(a)", "Rounds arg1 downward, returning the largest integral value that is not greater than arg1." },
		{ "fmod", "fmod(a, b)", "The floating-point remainder of arg1/arg2 (rounded towards zero)." },
		{ "frac", "frac(a)", "The fractional (or decimal) part of arg1; which is greater than or equal to 0 and less than 1." },
		{ "isinf", "isinf(a)", "1.0 if a component of arg1 is +INF, -1.0 if it is -INF, and 0.0 if it is not." },
		{ "isnan", "isnan(a)", "1.0 if a component of arg1 if NaN (Not a Number), 0.0 if not." },
		{ "ldexp", "ldexp(a, b)", "The result of multiplying arg1 (the significand) by 2 raised to the power of arg2 (the exponent)." },
		{ "lerp", "lerp(a, b, c)", "Performs a linear interpolation: arg1 + arg3 * (arg2 - arg1)." },
		{ "ln", "ln(a)", "The natural logarithm of arg1." },
		{ "log", "log(a)", "The natural logarithm of arg1." },
		{ "log10", "log10(a)", "The common (base-10) logarithm of arg1." },
		{ "log2", "log2(a)", "The binary (base-2) logarithm of arg1." },
		{ "mad", "mad(a, b, c)", "Performs an arithmetic multiply/add operation on three arguments: arg1 * arg2 + arg3." },
		{ "max", "max(a, b)", "The greater of arg1 and arg2." },
		{ "min", "min(a, b)", "The lesser of arg1 and arg2." },
		{ "pow", "pow(a, b)", "arg1 raised to the power arg2." },
		{ "rad", "rad(a)", "Converts an angle, arg1, given in degrees to radians." },
		{ "rand", "rand()", "A pseudo random value." },
		{ "round", "round(a)", "The integral value that is nearest to arg1, with halfway cases rounded away from zero." },
		{ "rsqrt", "rsqrt(a)", "The reciprocal of the square root of arg1: 1.0 / sqrt(arg1)." },
		{ "saturate", "saturate(a)", "Clamps arg1 within the range of 0 to 1." },
		{ "sign", "sign(a)", "-1.0 if arg1 is less than zero; 0.0 if arg1 equals zero; and 1.0 if arg1 is greater than zero." },
		{ "sin", "sin(a)", "The sine of arg1 (radians)." },
		{ "sinh", "sinh(a)", "The hyperbolic sine of arg1." },
		{ "smoothstep", "smoothstep(a, b, c)", "A smooth Hermite interpolation between 0 and 1, if arg3 is in the range [arg1, arg2]." },
		{ "sqrt", "sqrt(a)", "The square root of arg1." },
		{ "step", "step(a, b)", "This function uses the following formula: (arg2 >= arg1) ? 1 : 0." },
		{ "tan", "tan(a)", "The tangent of arg1 (radians)." },
		{ "tanh", "tanh(a)", "The hyperbolic tangent of arg1." },
		{ "trunc", "trunc(a)", "Rounds arg1 toward zero, returning the nearest integral value that is not larger in magnitude than arg1." }
	};

	Func ops[] = {
		{ "+", "+", "Addition or unary positive." },
		{ "-", "-", "Subtraction or unary negate." },
		{ "*", "*", "Multiplication." },
		{ "/", "/", "Division." },
		{ "%", "&", "Modulo." },
		{ "<<", "<<", "Bitwise shift left." },
		{ ">>", ">>", "Bitwise shift right." },
		{ "^", "^", "Bitwise exclusive OR." },
		{ "~", "~", "Bitwise invert." },
		{ "&", "&", "Bitwise AND." },
		{ "|", "|", "Bitwise OR." },
		{ "<", "<", "Less than." },
		{ "<=", "<=", "Less or equal than." },
		{ "==", "==", "Equals." },
		{ "!=", "!=", "Not equal." },
		{ ">", ">", "Greater than." },
		{ ">=", ">=", "Greater or equal than." },
		{ "&&", "&&", "And." },
		{ "||", "||", "Or." },
		{ "?:", "?!", "Ternary if-else." },
		{ "!", "!", "Not." }
	};

	Func operands[] = {
		{ "a", "a", "1st child connected to this chip." },
		{ "b", "b", "2nd child connected to this chip." },
		{ "c", "c", "3rd child connected to this chip." },
		{ "d", "d", "4th child connected to this chip." },
		{ "e", "e", "5th child connected to this chip." },
		{ "f", "f", "6th child connected to this chip." },
		{ "g", "g", "7th child connected to this chip." },
		{ "h", "h", "8th child connected to this chip." },
		{ "i", "i", "9th child connected to this chip." },
		{ "j", "j", "10th child connected to this chip." },
		{ "k", "k", "11th child connected to this chip." },
		{ "l", "l", "12th child connected to this chip." },
		{ "m", "m", "13th child connected to this chip." },
		{ "n", "n", "14th child connected to this chip." },
		{ "o", "o", "15th child connected to this chip." },
		{ "p", "p", "16th child connected to this chip." },
		{ "q", "q", "17th child connected to this chip." },
		{ "r", "r", "18th child connected to this chip." },
		{ "s", "s", "19th child connected to this chip." },
		{ "t", "t", "20th child connected to this chip." },
		{ "u", "u", "21th child connected to this chip." },
		{ "v", "v", "22th child connected to this chip." },
		{ "w", "w", "23th child connected to this chip." },
		{ "x", "x", "24th child connected to this chip." },
		{ "y", "y", "25th child connected to this chip." },
		{ "z", "z", "26th child connected to this chip." },
		{ "old", "old", "The previous value calculated by this chip." },
		{ "dt", "dt", "The time elapsed since the previous frame in seconds." },
		{ "pi", "pi", "Pi: 3,14159265359..." }
	};


	class Label : public QLabel
	{
	public:
		QString func;
		QLineEdit *e;
		Label(const Func &f, QLineEdit *e) : QLabel(f.name), func(f.func), e(e)
		{ 
			setStyleSheet("QLabel:hover { background-color:red; }");
			setToolTip(f.tooltip); 
		}
		virtual void mouseReleaseEvent(QMouseEvent * event)
		{
			if (e)
				e->insert(func);
		}
	};
	
	FlowLayout *fl = new FlowLayout();
	ui.groupBox_funcs->setLayout(fl);

	for (size_t i = 0; i < _countof(funcs); i++) {
		const auto &a = funcs[i];
		fl->addWidget(new Label(a, ui.editor->lineEdit()));
	}

	fl = new FlowLayout();
	ui.groupBox_ops->setLayout(fl);

	for (size_t i = 0; i < _countof(ops); i++) {
		const auto &a = ops[i];
		fl->addWidget(new Label(a, ui.editor->lineEdit()));
	}

	fl = new FlowLayout();
	ui.groupBox_operands->setLayout(fl);

	for (size_t i = 0; i < _countof(operands); i++) {
		const auto &a = operands[i];
		fl->addWidget(new Label(a, ui.editor->lineEdit()));
	}

}

ExpressionChip_Dlg::~ExpressionChip_Dlg()
{
}

void ExpressionChip_Dlg::Init()
{
	QSignalBlocker b(ui.editor);
	
	_initExp = GetChip()->GetExpression();
	ui.editor->setCurrentIndex(-1);
	ui.editor->setCurrentText(TOQSTRING(_initExp));
}

void ExpressionChip_Dlg::OnCancel()
{
	if (IsEdited())
		GetChip()->SetExpression(_initExp);
}

void ExpressionChip_Dlg::onValueChange(const QString &v)
{
	String s = FROMQSTRING(v);
	String msg;
	if (!GetChip()->SetExpression(s, &msg)) {
		_msg = TOQSTRING(msg);
		_msgTimer.start(500);
	}
	else {
		_msgTimer.stop();
		ui.label_msg->setText("");
	}
	SetDirty();
	StandardDialogPage::GetChip()->SetName(GetChip()->GetExpression());
}
