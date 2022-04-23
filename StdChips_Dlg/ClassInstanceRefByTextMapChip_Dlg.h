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
#include "StdChips/ClassInstanceRefMapChip.h"
#include "M3DEngine/ClassInstance.h"
#include "ui_ClassInstanceRefByTextMapChip_Dlg.h"

namespace m3d
{



class STDCHIPS_DIALOGS_API ClassInstanceRefByTextMapChip_Dlg : public StandardDialogPage
{
	Q_OBJECT
		DIALOGDESC_DECL
public:
	ClassInstanceRefByTextMapChip_Dlg();
	~ClassInstanceRefByTextMapChip_Dlg();

	ClassInstanceRefByTextMapChip* GetChip() { return (ClassInstanceRefByTextMapChip*)DialogPage::GetChip(); }

	virtual void Init();
	virtual void OnCancel();
	virtual void AfterApply();
	virtual void Refresh();

	virtual QSize preferredSize() const { return QSize(800, 600); }

	virtual void showDataDialog(uint32 row, ClassInstanceID instanceID, ChipID dataID);

	virtual void contextMenuEvent(QContextMenuEvent* event) override;

	Ui::ClassInstanceRefByTextMapChip_Dlg ui;

private:
	void _setDataItem(Chip* data, ClassInstanceID instanceID, int row, int column);
	bool _getInstance(int row, ClassInstanceRef &ref);
	Chip* _getDataChip(ClassInstanceRef instance, ChipID dataID);
	void _setInstance(int row, String key, ClassInstanceRef ref);
	void _addRows(int from, int count);
	void _removeRows(int from, int count);
	void _clearRows(int from, int count);
	QPair<int, int> _getSelection();
	bool _getKeyAndTypeOfNewInstance(String& key, Class** instanceOf);
	Map<Guid, int>::iterator _addColumn(const Guid& globalID, int baseCount, QString name, bool underscored);
	void _setColumnInOrder(int column);
	void _swapRows(uint32 a, uint32 b);
	String _keyAtRow(int row);
	ClassInstanceID _instanceIDAtRow(int row);
	bool _isRowValid(int row);


	Map<Guid, int> _dataTypeMap; // Map from data guid to logical column nr
	bool _initSaveRef;
	bool _initPreload;

private slots:
	void onTableDoubleClick(int row, int column);
	void hideMembersClicked();
	void preloadClicked();
	void addElement();
	void removeRows();
	void createInstance();
	void clearRef();
	void copyRef();
	void pasteRef();
	void updateButtons();
	void onTableCellChanged(int row, int column);
	void saveInstanceClicked();
	void refresh();

};

}
