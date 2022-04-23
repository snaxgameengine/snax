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
#include "GraphicsChips/Renderable.h"
#include "ui_Renderable_Dlg.h"

namespace m3d
{


class GRAPHICSCHIPS_DLG_EXPORT Renderable_Dlg : public StandardDialogPage
{
	Q_OBJECT
	DIALOGDESC_DECL
public:
	Renderable_Dlg();
	~Renderable_Dlg();

	Renderable *GetChip() { return (Renderable*)DialogPage::GetChip(); }

	virtual void Init();

	virtual void OnOK();
	virtual void OnCancel();
	virtual void AfterApply();

private:
	Ui::Renderable_Dlg ui;

	QStringList _semantics;

	List<std::pair<uint32, String>> _initSpecificSubsets;
	bool _initWholeObjectCulling;
	bool _initSubsetCulling;
	AxisAlignedBox _initBB;

	void _fillBB(const AxisAlignedBox &aabb);
	void _getSubsets(List<std::pair<uint32, String>> &subsets);

private slots:
	void updateFromDrawable();
	void calculateBB();
	void moveSsUp();
	void moveSsDown();
	void bbEdited();
	void ssSelectionChanged();
	void ssItemChanged(QListWidgetItem*);
	void cullingChanged();
	

};

}