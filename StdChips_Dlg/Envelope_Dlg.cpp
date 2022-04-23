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
#include "Envelope_dlg.h"


using namespace m3d;

DIALOGDESC_DEF(Envelope_Dlg, ENVELOPE_GUID);

Envelope_Dlg::Envelope_Dlg()
{
	ui.setupUi(this);
	_newPointDialog = new QDialog(this);
	QGridLayout *l0 = new QGridLayout(_newPointDialog);
	QLabel *lbl0 = new QLabel("x:", _newPointDialog);
	QLabel *lbl1 = new QLabel("y:", _newPointDialog);
	QLineEdit *edit0 = new QLineEdit(_newPointDialog);
	edit0->setAlignment(Qt::AlignRight);
	edit0->setText("0.0");
	edit0->setObjectName("xEdit");
	lbl0->setBuddy(edit0);
	QLineEdit *edit1 = new QLineEdit(_newPointDialog);
	edit1->setAlignment(Qt::AlignRight);
	edit1->setText("0.0");
	edit1->setObjectName("yEdit");
	lbl1->setBuddy(edit1);
	QGridLayout *l1 = new QGridLayout(_newPointDialog);
	QPushButton *btn0 = new QPushButton("OK", _newPointDialog);
	btn0->setDefault(true);
	QPushButton *btn1 = new QPushButton("Cancel", _newPointDialog);
	l1->addWidget(btn0, 0, 0);
	l1->addWidget(btn1, 0, 1);
	l0->addWidget(lbl0, 0, 0);
	l0->addWidget(edit0, 0, 1);
	l0->addWidget(lbl1, 1, 0);
	l0->addWidget(edit1, 1, 1);
	l0->addLayout(l1, 2, 0, 1, 2);
	connect(btn0, &QPushButton::clicked, _newPointDialog, &QDialog::accept);
	connect(btn1, &QPushButton::clicked, _newPointDialog, &QDialog::reject);

	_spline = nullptr;
}

Envelope_Dlg::~Envelope_Dlg()
{
}

void Envelope_Dlg::Init()
{
	_initSpline = GetChip()->GetSpline();
	ui.graph->SetSpline(&GetChip()->GetSpline());
	ui.graph->viewAll();
}

void Envelope_Dlg::OnCancel()
{
	if (IsEdited())
		GetChip()->SetSpline(_initSpline);
}

void Envelope_Dlg::AfterApply()
{
	_initSpline = GetChip()->GetSpline();
}

void Envelope_Dlg::updateControlPointUI(uint32 i)
{
	float64 t = 0.0, c = 0.0, b = 0.0;
	if (i == -1) {
		ui.comboBox_cpType->setCurrentIndex(-1);
		ui.lineEdit_cpX->setText("");
		ui.lineEdit_cpY->setText("");
	}
	else {
		const ValueTCBSpline::ControlPoint &cp = ui.graph->GetSpline()->GetControlPoints()[i];
		ui.comboBox_cpType->setCurrentIndex(cp.type);
		ui.lineEdit_cpX->setText(QString::number(cp.x));
		ui.lineEdit_cpY->setText(QString::number(cp.y));
		t = cp.t;
		c = cp.c;
		b = cp.b;
	}
	ui.doubleSpinBox_cpT->blockSignals(true);
	ui.doubleSpinBox_cpT->setValue(t);
	ui.doubleSpinBox_cpT->blockSignals(false);
	ui.doubleSpinBox_cpC->blockSignals(true);
	ui.doubleSpinBox_cpC->setValue(c);
	ui.doubleSpinBox_cpC->blockSignals(false);
	ui.doubleSpinBox_cpB->blockSignals(true);
	ui.doubleSpinBox_cpB->setValue(b);
	ui.doubleSpinBox_cpB->blockSignals(false);
	ui.pushButton_Delete->setEnabled(i != -1);
	ui.comboBox_cpType->setEnabled(i != -1);
	ui.lineEdit_cpX->setEnabled(i != -1);
	ui.lineEdit_cpY->setEnabled(i != -1);
	ui.doubleSpinBox_cpT->setEnabled(i != -1);
	ui.doubleSpinBox_cpC->setEnabled(i != -1);
	ui.doubleSpinBox_cpB->setEnabled(i != -1);
	ui.pushButton_P->setEnabled(i != -1 && i > 0);
	ui.pushButton_N->setEnabled(i != -1 && i < ui.graph->GetSpline()->GetControlPoints().size() - 1);
}

void Envelope_Dlg::controlPointEdited()
{
	uint32 i = ui.graph->GetCurrentControlPoint();
	if (i < ui.graph->GetSpline()->GetControlPoints().size()) {
		ValueTCBSpline::ControlPoint cp = ui.graph->GetSpline()->GetControlPoints()[i];
		cp.type = ui.comboBox_cpType->currentIndex() == 0 ? cp.STEP : ui.comboBox_cpType->currentIndex() == 1 ? cp.LINEAR : cp.TCB;
		bool ok;
		float64 tmp;
		tmp = ui.lineEdit_cpX->text().toDouble(&ok);
		if (ok)
			cp.x = tmp;
		tmp = ui.lineEdit_cpY->text().toDouble(&ok);
		if (ok)
			cp.y = tmp;
		cp.t = ui.doubleSpinBox_cpT->value();
		cp.c = ui.doubleSpinBox_cpC->value();
		cp.b = ui.doubleSpinBox_cpB->value();
		GetChip()->GetSpline().UpdateControlPoint(i, cp);
		const ValueTCBSpline::ControlPoint &updated = ui.graph->GetSpline()->GetControlPoints()[i];
		ui.lineEdit_cpX->setText(QString::number(updated.x));
		ui.lineEdit_cpY->setText(QString::number(updated.y));
		SetDirty();
	}
	ui.graph->repaint();
}

void Envelope_Dlg::addControlPoint()
{
	while (true) {
		QLineEdit *xEdit = _newPointDialog->findChild<QLineEdit*>("xEdit");
		QLineEdit *yEdit = _newPointDialog->findChild<QLineEdit*>("yEdit");
		Q_ASSERT(xEdit && yEdit);
		xEdit->selectAll();
		xEdit->setFocus();
		int r = _newPointDialog->exec();
		if (r == QDialog::Accepted) {
			bool ok1, ok2;
			value x = (value)xEdit->text().toDouble(&ok1);
			value y = (value)yEdit->text().toDouble(&ok2);
			if (!ok1 || !ok2) {
				QMessageBox::critical(this, "Insertion Failed", "Invalid input.");
				continue;
			}
			if (addControlPoint(x, y))
				break;
		}
		else
			break;
	}
}

bool Envelope_Dlg::addControlPoint(value x, value y)
{
	uint32 i = GetChip()->GetSpline().AddControlPoint(ValueTCBSpline::ControlPoint(x, y));
	if (i == -1)
		QMessageBox::critical(this, "Insertion Failed", "Cannot insert control point at given x-coordinate.");
	else {
		ui.graph->setCurrentControlPoint(i);
		SetDirty();
	}
	return i != -1;
}

void Envelope_Dlg::moveCurrentControlPoint(value dx, value dy)
{
	uint32 i = ui.graph->GetCurrentControlPoint();
	if (i < ui.graph->GetSpline()->GetControlPoints().size()) {
		ValueTCBSpline::ControlPoint cp = ui.graph->GetSpline()->GetControlPoints()[i];
		cp.x += dx;
		cp.y += dy;
		GetChip()->GetSpline().UpdateControlPoint(i, cp);
		const ValueTCBSpline::ControlPoint &updated = ui.graph->GetSpline()->GetControlPoints()[i];
		ui.lineEdit_cpX->setText(QString::number(updated.x));
		ui.lineEdit_cpY->setText(QString::number(updated.y));
		SetDirty();
	}
	ui.graph->repaint();
}

void Envelope_Dlg::deleteCurrentControlPoint()
{
	uint32 i = ui.graph->GetCurrentControlPoint();
	if (i < ui.graph->GetSpline()->GetControlPoints().size()) {
		GetChip()->GetSpline().RemoveControlPoint(i);
		i = std::min((size_t)i + 1, GetChip()->GetSpline().GetControlPoints().size()) - 1;
		ui.graph->setCurrentControlPoint(i);
		SetDirty();
	}
	ui.graph->repaint();
}

