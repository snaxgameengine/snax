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
#include "ChipDialogs/StandardDialogPage.h"
#include "GraphicsChips/GraphicsState.h"
#include "ui_GraphicsState_Dlg2.h"
#include <qtextdocument.h>

namespace m3d
{


class GRAPHICSCHIPS_DLG_EXPORT GraphicsState_Dlg : public StandardDialogPage
{
	Q_OBJECT
	DIALOGDESC_DECL
public:
	GraphicsState_Dlg();
	~GraphicsState_Dlg();

	GraphicsState*GetChip() { return (GraphicsState*)DialogPage::GetChip(); }

	virtual void Init();

	virtual void OnCancel();
	virtual void AfterApply();

	virtual void Update();

private:
	Ui::GraphicsState_Dlg2 ui;
	
	GraphicsState::StateElementMap _initElements;
	GraphicsState::StateElementMap _elements;
	GraphicsState::StateElementMap _defaultElements;
	int32 _psoID;

	//virtual QSize sizeHint() const { return QSize(800, 600); }
	//virtual bool eventFilter(QObject *obj, QEvent *event) override;
	//	String _initSource;

	bool _blockSignals;

	QFont _labelFont;
	QFont _labelFontBold;

	QMap<QLineEdit*, QString> _texts;

	void _update_ui();
	void _update_rs_ui();
	void _update_dss_ui();
	void _update_bs_ui();
	void _update_chip();
	void _setIndex(QLabel *label, QComboBox *cb, const GraphicsState::StateElementMap &m, GraphicsState::Element e);
	void _setText(QLabel *label, QLineEdit *le, const GraphicsState::StateElementMap &m, GraphicsState::Element e);

	void keyPressEvent(QKeyEvent *evt);

private slots:
	void onSetRS();
	void onSetRS2();
	void onSetDSS();
	void onSetDSS2();
	void onSetBS();
	void onSetBS2();
	void onShowInheritedChanged();
	void onRTChanged(int32 id);
};


}