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
#include "StdChips/ClassInstanceRefChip.h"
#include "M3DEngine/ClassInstance.h"
#include "ui_ClassInstanceRefChip_Dlg.h"

namespace m3d
{



class STDCHIPS_DIALOGS_API ClassInstanceRefChip_Dlg : public StandardDialogPage
{
	Q_OBJECT
	DIALOGDESC_DECL
public:
	ClassInstanceRefChip_Dlg();
	~ClassInstanceRefChip_Dlg();

	ClassInstanceRefChip *GetChip() { return (ClassInstanceRefChip*)DialogPage::GetChip(); }

	virtual void Init();

	virtual void OnCancel();
	virtual void AfterApply();
	virtual void Refresh();

	virtual QSize preferredSize() const { return QSize(800, 600); }

protected:
	void _fillData();
	void _enableButtons();
	Chip *_getDataChipFromRow(int row);
	Chip *_getDataChip(ChipID cid);

private slots:
	void onNewInstance();
	void onClearReference();
	void onCopyReference();
	void onPasteReference();
	void onTableDoubleClick(int row, int column);
	void hideMembersClicked();
	void saveInstanceClicked();
	void preloadClicked();
	void nameChanged();
	void onCellChanged(int, int);
	void ownerClicked();

private:
	Ui::ClassInstanceRefChip_Dlg ui;

	bool _initSaveRef;
	bool _initPreload;
	ClassInstanceID _instanceID;
};


}