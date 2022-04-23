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
#include <qcombobox.h>

namespace m3d
{


class ClassExt;
class ClassView;
class Document;
class EmbeddedChipDialogContainer;

enum EditorPageType { EPT_EMPTY, EPT_CLASS, EPT_CHIP };

struct EditorPage
{
	const EditorPageType type;
	unsigned viewIndex; // This is to find the last on shown when you close a page.

	EditorPage(EditorPageType type = EPT_EMPTY) : type(type), viewIndex(0) {}
	virtual ~EditorPage() {}
};

struct ClassViewPage : EditorPage
{
	ClassViewPage(ClassExt *c);
	~ClassViewPage();

	ClassView *view;
};

struct ChipDialogPage : EditorPage
{
	ChipDialogPage(EmbeddedChipDialogContainer *c) : EditorPage(EPT_CHIP), container(c) {}
	~ChipDialogPage() {}

	EmbeddedChipDialogContainer *container;
};

class EditorPageListWidget : public QComboBox
{
	Q_OBJECT 
public:
	EditorPageListWidget(QWidget *parent = NULL);
	~EditorPageListWidget();

	void copyFrom(EditorPageListWidget *w);

	EditorPage *GetCurrentPage();

	// Create a new page or open the existing one.
	ClassViewPage *OpenClassPage(ClassExt *c);
	ChipDialogPage *OpenChipDialogPage(EmbeddedChipDialogContainer *c);

	// Each class/dialog can only have one page!
	EditorPage *FindPage(ClassExt *c) const;
	EditorPage *FindPage(EmbeddedChipDialogContainer *c) const;


public:
	void onClassNameOrDirtyFlagChanged(ClassExt *c);
	void onDocumentFileChange(Document *d);
	void closePage(int idx);
	void closePage(EditorPage *ep);
	void closeCurrentPage();
	void closeAllButCurrentPage();
	void closeAllPages();

signals:
	// Emitted whenever current page changes!
	void pageChanged(EditorPage *newPage);
	// Emitted when a page is closed. If it was current, pageChanged() is emitted first!
	void pageClosed(EditorPage *ep);

protected:
	void onCurrentChanged();

protected:
	int _getInsertIndex(QString name);
	ClassViewPage *_addClassPage(ClassExt *c);
};

}
