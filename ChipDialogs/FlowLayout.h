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
#include <qlayout.h>
#include <qstyle.h>

namespace m3d
{
class CHIPDIALOGS_EXPORT FlowLayout : public QLayout
{
public:
	explicit FlowLayout(QWidget* parent, int32 margin = -1, int32 hSpacing = -1, int32 vSpacing = -1);
	explicit FlowLayout(int32 margin = -1, int32 hSpacing = -1, int32 vSpacing = -1);
	~FlowLayout();

	void addItem(QLayoutItem* item) override;
	int32 horizontalSpacing() const;
	int32 verticalSpacing() const;
	Qt::Orientations expandingDirections() const override;
	bool hasHeightForWidth() const override;
	int32 heightForWidth(int32) const override;
	int32 count() const override;
	QLayoutItem* itemAt(int32 index) const override;
	QSize minimumSize() const override;
	void setGeometry(const QRect& rect) override;
	QSize sizeHint() const override;
	QLayoutItem* takeAt(int32 index) override;

private:
	int32 doLayout(const QRect& rect, bool testOnly) const;
	int32 smartSpacing(QStyle::PixelMetric pm) const;

	QList<QLayoutItem*> itemList;
	int32 m_hSpace;
	int32 m_vSpace;
};

}