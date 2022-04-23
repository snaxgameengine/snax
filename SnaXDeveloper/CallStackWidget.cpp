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
#include "CallStackWidget.h"
#include <qheaderview.h>
#include <qapplication.h>
#include <qmenu.h>
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngine/Function.h"
#include "AppSignals.h"

using namespace m3d;



CallStackWidget::CallStackWidget(QWidget * parent) : QTreeWidget(parent)
{
	header()->reset();
	headerItem()->setText(0, "Frame");
	headerItem()->setText(1, "Depth");
	headerItem()->setText(2, "Instance");
	headerItem()->setText(3, "Class");
	headerItem()->setText(4, "Chip");
	setIndentation(0);
	setSortingEnabled(false);
	header()->setStretchLastSection(true);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setColumnWidth(0, 60);
	setColumnWidth(1, 60);
	setColumnWidth(2, 200);
	setColumnWidth(3, 200);

	_menu = new QMenu(this);
	_showInstance = _menu->addAction("Show Instance Dialog");
	_showChip = _menu->addAction("Show Chip Dialog");

	connect(&AppSignals::instance(), &AppSignals::setCallStack, this, &CallStackWidget::fill);

	connect(this, &CallStackWidget::itemDoubleClicked, this, &CallStackWidget::onItemDoubleClicked);
	connect(this, &CallStackWidget::customContextMenuRequested, this, &CallStackWidget::showMenu);
}

CallStackWidget::~CallStackWidget()
{
}

void CallStackWidget::DumpCurrentCallStack(CallStack &callStack, ClassID lastClass, ChipID lastChip, bool isBreakPoint)
{
	FunctionStackTrace trace;
	functionStack.DumpStackTrace(trace);
	callStack.clear();

	if (trace.size() > 0) { // Do we have a stack trace?
		Class *startCG = engine->GetClassManager()->GetStartClass();
		callStack.push_back(CallStackRecord(startCG ? startCG->GetStartChip() : nullptr)); // Add project start chip
		ClassInstance *instance = nullptr; // Always starting with no instance!
		for (size_t i = 1; i < trace.size(); i++) { // Iterate stack trace. Skipping first.
			const FunctionStackTraceRecord &r = trace[trace.size() - i - 1]; // Iterate back->front
			if (callStack.back().chip == CGCID(r.caller)) // Previous record is the same?
				callStack.back().type = r.caller->AsFunctionCall() ? CallStackRecord::FUNCTION_CALL : CallStackRecord::PARAMETER; // Update the icon only!
			else
				callStack.push_back(CallStackRecord(r.caller, instance, r.caller->AsFunctionCall() ? CallStackRecord::FUNCTION_CALL : CallStackRecord::PARAMETER)); // Add function call or parameter call!
			callStack.push_back(CallStackRecord(r.called, instance = r.instance, r.caller->AsFunctionCall() ? CallStackRecord::Type(CallStackRecord::STATIC_FUNCTION + (uint32)r.called->GetFunction()->GetType()) : CallStackRecord::CHIP)); // Add called function or chip.
		}
		if (lastChip != InvalidChipID && callStack.back().chip.chipID != lastChip)
			callStack.push_back(CallStackRecord(CGCID(lastClass, lastChip), instance, isBreakPoint ? CallStackRecord::BREAK_POINT : CallStackRecord::CHIP)); // Add reported chip if it exist.
	}
}

void CallStackWidget::fill(const CallStack &callStack)
{
	// Idx	Depth	Instance		CG			Chip Name
	// 0	0		-				Default		[CH] ProjectStart
	// 1	0		-				Default		[FC] Test->Func
	// 2	1		MyInstance (2)	Test		[ F] Func
	// 3	1		MyInstance (2)	Test		[ P] Param
	// 4	0		-				Default		[CH] SomeValue
	// 5	0		-				Default		[CH] Reported ch

	static const QIcon icn[7] = 
	{
		QIcon(":/EditorApp/Resources/chip.png"), 
		QIcon(":/EditorApp/Resources/func-call.png"),
		QIcon(":/EditorApp/Resources/static-func.png"),
		QIcon(":/EditorApp/Resources/nonvirt-func.png"),
		QIcon(":/EditorApp/Resources/virt-func.png"),
		QIcon(":/EditorApp/Resources/parameter.png"), 
		QIcon(":/EditorApp/Resources/break-point.png") 
	};

	clear();

	unsigned level = 0;

	for (size_t i = 0; i < callStack.size(); i++) {
		const CallStackRecord &r = callStack[i];
		QTreeWidgetItem *itm = new QTreeWidgetItem();
		itm->setText(0, QString::number(i + 1));
		itm->setText(1, QString::number(level));
		if (r.type == CallStackRecord::FUNCTION_CALL)
			level++;
		else if (r.type == CallStackRecord::PARAMETER)
			level--;
		assert(level != -1);
		Chip *chip = engine->GetClassManager()->FindChip(r.chip.chipID, r.chip.clazzID);
		if (chip) {
			if (chip->GetClass())
				itm->setText(3, TOQSTRING(chip->GetClass()->GetName()));
			else
				itm->setText(3, "?");
			itm->setText(4, TOQSTRING(chip->GetName()));
		}
		else {
			itm->setText(3, "?");
			itm->setText(4, "?");
		}
		if (r.instance.instanceID != InvalidClassInstanceID) {
			ClassInstance *instance = engine->GetClassManager()->FindInstance(r.instance.instanceID, r.instance.clazzID);
			if (instance) {
				itm->setText(2, QString::number(instance->GetRuntimeID()) + " - " + (instance->GetName().empty() ? "Unnamed" : TOQSTRING(instance->GetName())));
			}
			else {
				itm->setText(2, "?");
			}
		}
		else
			itm->setText(2, "-");
		itm->setIcon(4, icn[r.type]);
		itm->setData(2, Qt::UserRole + 0, (unsigned)r.instance.clazzID);
		itm->setData(2, Qt::UserRole + 1, (unsigned)r.instance.instanceID);
		itm->setData(3, Qt::UserRole + 0, (unsigned)r.chip.clazzID);
		itm->setData(4, Qt::UserRole + 0, (unsigned)r.chip.clazzID);
		itm->setData(4, Qt::UserRole + 1, (unsigned)r.chip.chipID);
		insertTopLevelItem(0, itm);
	}
}

void CallStackWidget::onItemDoubleClicked(QTreeWidgetItem *itm, int column)
{
/*	if (column == 2) {
		ClassID cgid(itm->data(2, Qt::UserRole + 0).toUInt());
		ClassInstanceID instanceID(itm->data(2, Qt::UserRole + 1).toUInt());
		ClassInstance *instance = engine->GetClassManager()->FindInstance(instanceID, cgid);
		if (instance)
			emit openInstanceDialog(instance);
		else
			QApplication::beep();
	}
	else if (column == 4) {*/
		ClassID cgid(itm->data(4, Qt::UserRole + 0).toUInt());
		ChipID chipID(itm->data(4, Qt::UserRole + 1).toUInt());
		Chip *chip = engine->GetClassManager()->FindChip(chipID, cgid);
		if (chip)
			AppSignals::instance().openClass((ClassExt*)chip->GetClass(), chip);
		else
			QApplication::beep();
//	}
}

void CallStackWidget::showMenu(QPoint p)
{
	int column = columnAt(p.x());
	QTreeWidgetItem *itm = itemAt(p);
	if (!itm)
		return;

	ClassID cgid(itm->data(2, Qt::UserRole + 0).toUInt());
	ClassInstanceID instanceID(itm->data(2, Qt::UserRole + 1).toUInt());
	ClassInstance *instance = engine->GetClassManager()->FindInstance(instanceID, cgid);

	ClassID cgidC(itm->data(4, Qt::UserRole + 0).toUInt());
	ChipID chipID(itm->data(4, Qt::UserRole + 1).toUInt());
	Chip *chip = engine->GetClassManager()->FindChip(chipID, cgidC);

	_showInstance->setEnabled(instance != nullptr);
	_showChip->setEnabled(chip != nullptr);

	if (_menu->exec(viewport()->mapToGlobal(p)) == _showInstance) {
		AppSignals::instance().openInstanceDialog(instance);
	}
	else if (_menu->exec(viewport()->mapToGlobal(p)) == _showChip) {
		AppSignals::instance().openChipDialog(chip);
	}
}