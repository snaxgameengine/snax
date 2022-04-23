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

#include "Common.h"
#include <qtreewidget.h>
#include "M3DEngine/Chip.h"
#include "M3DEngine/GlobalDef.h"
#include "M3DEngine/ClassInstance.h"
#include "M3DEngine/Class.h"


namespace m3d
{


class CallStackWidget : public QTreeWidget
{
	Q_OBJECT
public:
	CallStackWidget(QWidget * parent = 0);
	virtual ~CallStackWidget();

	static void DumpCurrentCallStack(CallStack &callStack, ClassID lastClass = InvalidClassID, ChipID lastChip = InvalidChipID, bool isBreakPoint = false);

public:
	void fill(const CallStack &callStack);

private:
	void onItemDoubleClicked(QTreeWidgetItem *itm, int column);
	void showMenu(QPoint p);

private:
	QMenu *_menu;
	QAction *_showInstance;
	QAction *_showChip;

};


}