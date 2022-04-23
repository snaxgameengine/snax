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
#include "GraphicsChips/GraphicsResourceChip.h"
#include "ui_GraphicsResource_Dlg.h"

namespace m3d
{


class GRAPHICSCHIPS_DLG_EXPORT GraphicsResource_Dlg : public StandardDialogPage
{
	Q_OBJECT
	DIALOGDESC_DECL
public:
	GraphicsResource_Dlg();
	~GraphicsResource_Dlg();

	GraphicsResourceChip*GetChip() { return dynamic_cast<GraphicsResourceChip*>(DialogPage::GetChip()); }

	virtual void Init();

	virtual void OnOK();

private:
	void _onSrvUiUpdated();
	void _onRtvUiUpdated();
	void _onDsvUiUpdated();
	void _onUavUiUpdated();

private:
	Ui::GraphicsResource_Dlg ui;

	bool _blockUiUpdates;

	SRVDesc _initSRV;
	RTVDesc _initRTV;
	DSVDesc _initDSV;
	UAVDesc _initUAV;

	QFont _labelFont;
	QFont _labelFontBold;

	SRVDesc _getSRVFromUI();
	RTVDesc _getRTVFromUI();
	DSVDesc _getDSVFromUI();
	UAVDesc _getUAVFromUI();

};
}