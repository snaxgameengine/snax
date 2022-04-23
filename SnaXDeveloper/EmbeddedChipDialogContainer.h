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
#include "ChipDialogs/ChipDialog.h"

namespace m3d
{

class EditorWidget;
class CustomSizeGrip;

class EmbeddedChipDialogContainer : public QWidget, public ChipDialogContainer
{
	Q_OBJECT
public:
	EmbeddedChipDialogContainer(EditorWidget *ew);
	~EmbeddedChipDialogContainer();

	QWidget *GetWidget() override { return this; }
	void Flash() override {}
	void Activate() override;
	void DeleteLater() override;
	void OnTitleChanged(QString s) override  { _title = s; }
	void SetChipDialog(ChipDialog *cd) override;
	void ReleaseChipDialog() override;
	void OpenDialog() override;
	unsigned GetEmbeddedID() const override;
	void OnPageChanged() override;

	bool IsMarkedForDestruction() const { return _markedForDestruction; }

	EditorWidget *_ew;
	bool _markedForDestruction;

	QString GetTitle() const;

protected:
	void resizeEvent(QResizeEvent * event) override;
	bool eventFilter(QObject *o, QEvent *e) override;

	QString _title;
	CustomSizeGrip *_csg;

public:
	void accept();
	void reject();

signals:
	void finished(int);
};

}
