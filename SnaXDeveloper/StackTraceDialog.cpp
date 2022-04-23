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

#include "stdafx.h"
#include "StackTraceDialog.h"
#include <qgridlayout.h>
#include <qpushbutton.h>

using namespace m3d;



StackTraceDialog::StackTraceDialog(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	resize(900, 400);
	QGridLayout *layout = new QGridLayout(this);
	callStackWidget = new CallStackWidget(this);
	layout->addWidget(callStackWidget, 0, 0);
	QPushButton *okbutton = new QPushButton("OK", this);
	layout->addWidget(okbutton, 1, 0, 1, 1, Qt::AlignRight);
	connect(okbutton, &QPushButton::clicked, this, &QDialog::accept);
	_msgid = 0;
}

StackTraceDialog::~StackTraceDialog()
{
}

void StackTraceDialog::Init(unsigned msgid, QString msg, const CallStack &callStack)
{
	_msgid = msgid;

	callStackWidget->fill(callStack);

	setWindowTitle("Call Stack: " + msg);
}

void StackTraceDialog::flash()
{
	FLASHWINFO finfo;
	finfo.cbSize = sizeof( FLASHWINFO );
	finfo.hwnd = (HWND)winId();
	finfo.uCount = 6;
	finfo.dwTimeout = 50;
	finfo.dwFlags = FLASHW_CAPTION;
	::FlashWindowEx( &finfo );
}