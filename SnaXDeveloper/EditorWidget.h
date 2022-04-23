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
#include <QWidget.h>
#include "EditorPageListWidget.h"
#include "M3DEngine/GlobalDef.h"
#include "M3DCore/DestructionObserver.h"
#include "M3DEngine/Chip.h"
#include "M3DEngineExt/ClassView.h"
#include "EmbeddedChipDialogContainer.h"
#include "EditorPageHistory.h"
#include "ActionManager.h"
#include "M3DEngine/Function.h"
#include "M3DEngine/Document.h"

namespace m3d
{

class DXEditorWidget;
class EditorWidget;
class ClassExt;
class Document;
class Chip;
class EditorWidget;
class ChipDialogContainer;
class EmbeddedChipDialogContainer;

class ClassExt;
class ClassView;
class Function;
struct EditorPage;



class EditorWidget : public QWidget, public ActionListener
{
	Q_OBJECT
public:
	EditorWidget(QWidget *parent = NULL);
	~EditorWidget();

	DXEditorWidget *getDXEditorWidget() { return _dxEditorWidget; }

	EditorWidget *duplicate(QWidget *parent = nullptr) const;
	void onParented(bool closeable, int index = 0, Qt::Orientation o = Qt::Vertical);

	void moveFrom(EditorWidget *ew);

	unsigned getID() const { return _id; }
	void setID(unsigned id) { _id = id; }

	ChipDialogContainer *createChipDialogContainer();
	void openChipDialog(EmbeddedChipDialogContainer *c);
	void closeChipDialog(ChipDialogContainer *c, bool removeFromHistory);

	void onChipDialogPageChanged();

	Document *getCurrentDocument() const;

	void hightlight(bool b);

	void SetFPSProps(bool showFPS, bool limitFPS);

public:
	void goBack();
	void goForward();
	void openClass(ClassExt *c, Chip *ch) { return openClassWithZoom(c, ch, 0.0f); }
	void openClassWithZoom(ClassExt *c, Chip *ch, float zoom);
	void openClassOnFolder2(ClassExt *c, unsigned folderID, float x, float y, float zoom);
	void openClassOnFolder(ClassExt *c, unsigned folderID, bool inside);
	void onClassRemoved(ClassExt*);
	void onFolderRemoved(ClassExt*, unsigned folderID);
	void onFunctionAdded(Function*);
	void onFunctionChanged(Function*);
	void onFunctionRemoved(Function*);
	void save();
	void openContainingFolder();
	void closeAll();
	void onNewFrame();

private:
	void gotoFunction();
	void onPageChanged(EditorPage *newPage);
	void onPageClosed(EditorPage *ep);

signals:
	void pageChanged();
	void showChipDialog(Chip*, bool, unsigned=0);
	void onClassNameOrDirtyFlagChanged(ClassExt*);
	void onDocumentFileChange(Document*);
	void addClass();
	void importClass();
	void saveDocument(Document*);
	void saveDocumentAs(Document*);
	void forceVSync(bool);
	void newFrame();
	void closeEditor();
	void splitVertical();
	void splitHorizontal();
	void setAsCurrent();
	void showClassDescriptionDialog(ClassExt*);

protected:
	EditorPageHistory _history;

	void _openPageFromHistory(const EditorPageHistory::Page &pv);
	void _onPageChanged();
	void _fillFunctionList();

	ActionMask getActions() override;
	bool isActionEnabled(ActionID id) override;
	void triggered(ActionID id) override;

	QToolBar *_toolbar;
	QVBoxLayout *_verticalLayout;
	DXEditorWidget *_dxEditorWidget;
	EditorPageListWidget *_pageList;
	QComboBox *_functionList;
	QMenu *_pageMenu;
	QAction *_goBackAction;
	QAction *_goFwdAction;
	QToolButton *_menuBtn;
	QAction *_closePageAction;
	QAction *_collapseAction;
	QAction *_splitVAction;
	QAction *_splitHAction;
	QAction *_closeAllPagesAction;
	QAction *_closeAllButThisPagesAction;
	QAction *_openContainingFolderAction;
	EmbeddedChipDialogContainer *_chipDialogContainer;
	QTabWidget *_tabWidget;

	unsigned _id;

};

}
