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
#include "SimpleDialogPage.h"
#include <functional>

namespace m3d
{


class CHIPDIALOGS_EXPORT SimpleTreeWidgetDialogPage : public SimpleDialogPage
{
public:
	SimpleTreeWidgetDialogPage(bool instantUpdateMode = true);
	~SimpleTreeWidgetDialogPage();

	RPtr AddItem(String text, RData data);
	RPtr AddItem(RPtr parentItem, String text, RData data);
	void SetInit(RData data, RData defaultData);
	RData GetCurrent();

	void sort(RSortOrder order = RSortOrder::AscendingOrder);

	void OnOK() override;
	void OnCancel() override;
	void AfterApply() override;

protected:
	virtual bool onSelectionChanged(RData data) { return false; }
	void SetSelectionChangedCallback(std::function<bool(RData)> cb) { _onSelectedChangedCallback = cb; }

private:
	void _onSelectionChanged();

	RPtr _treeWidget;
	std::function<bool(RData)> _onSelectedChangedCallback;

	RData _initData;
	RData _defaultData;

	bool _instantUpdateMode;
	bool _isInit;
};


}
