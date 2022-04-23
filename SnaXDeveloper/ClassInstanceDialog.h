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
#include "ui_ClassInstanceDialog.h"
#include "M3DEngine/ChipDef.h"
#include "M3DCore/MMath.h"


Q_DECLARE_METATYPE(XMFLOAT4);


namespace m3d
{

class ClassInstance;
class ClassInstanceDialog;

class ClassInstanceDialogCallback
{
public:
	virtual void CloseDialog(ClassInstanceDialog *dialog) = 0;
	virtual void ShowChipDialog(Chip *chip) = 0;
	virtual void SetDirty(Class *clazz) = 0;
};


#define REFRESH_FREQ 500



class ClassInstanceDialog : public QDialog
{
	Q_OBJECT
public:
	ClassInstanceDialog(QWidget * parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
	~ClassInstanceDialog();

	void SetCallback(ClassInstanceDialogCallback *cb) { _callback = cb; }

	void Init(ClassInstance *instance);
	void Refresh();

	ClassInstance *GetInstance() const { return _instance; }

	void Flash();
	void UpdateTitle();

protected:
	Ui::ClassInstanceDialog ui;
	ClassInstance *_instance;
	ClassInstanceDialogCallback *_callback;

	long _lastRefresh;

	void _fillData();
	Chip *_getDataChipFromRow(int row);
	Chip *_getDataChip(ChipID cid);

private slots:
	void onTableDoubleClick(int row, int column);
	void hideMembersClicked();
	void onFinish(int);
	void openOwner();
	void nameChanged();
	void copyRef();
	void destroyRef();
	void onCellChanged(int row, int column);

};

}