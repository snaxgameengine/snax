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

#include "stdafx.h"
#include "EditorWidget.h"
#include "DXEditorWidget.h"
#include <qtoolbutton.h>
#include "M3DEngineExt/ClassDiagram.h"
#include "M3DEngine/Chip.h"
#include "M3DEngineExt/ClassExt.h"
#include "M3DEngine/Document.h"
#include "M3DEngine/Function.h"
#include "M3DEngine/Engine.h"
#include "EmbeddedChipDialogContainer.h"
#include "EditorPageListWidget.h"
#include <qsortfilterproxymodel.h>
#include "EditorCollection.h"


using namespace m3d;


class DragButton : public QToolButton
{
public:
	EditorWidget *parent;

	DragButton(EditorWidget *parent = NULL) : QToolButton(parent), parent(parent)
	{
	}

	void mouseReleaseEvent(QMouseEvent * event) override 
	{
		QObject *w = QApplication::widgetAt(QCursor::pos());//event->globalPos()); // globalPos does not work with scaling/multiple monitors...
		while (w) {
			EditorWidget *ew = dynamic_cast<EditorWidget*>(w);
			if (ew) {
				if (ew != parent) {
					ew->moveFrom(parent);
				}
				break;
			}
			w = w->parent();
		}
		QApplication::restoreOverrideCursor();
	}
	void mousePressEvent(QMouseEvent * event) override
	{
		static QCursor drop(QCursor(QIcon(":/EditorApp/Resources/drop.png").pixmap(32)));
		QApplication::setOverrideCursor(drop);

	}
	void mouseMoveEvent(QMouseEvent * event) override
	{
		static QCursor drop(QCursor(QIcon(":/EditorApp/Resources/drop.png").pixmap(32)));
		static QCursor nodrop(QCursor(QIcon(":/EditorApp/Resources/no_drop.png").pixmap(32)));
		QObject *w = QApplication::widgetAt(QCursor::pos());//event->globalPos()); // globalPos does not work with scaling/multiple monitors...
		while (w) {
			EditorWidget *ew = dynamic_cast<EditorWidget*>(w);
			if (ew) {
				QApplication::changeOverrideCursor(drop);
				return;
			}
			w = w->parent();
		}
		QApplication::changeOverrideCursor(nodrop);
	}
};


EditorWidget::EditorWidget(QWidget *parent) : QWidget(parent), _id(0)
{
	_pageMenu = new QMenu(this);
	_toolbar = new QToolBar(this);

	{
		_goBackAction = _toolbar->addAction(QIcon(":/EditorApp/Resources/back.png"), "Go Back", this, &EditorWidget::goBack);
		_goBackAction->setShortcut(QKeySequence::Back);
		_goBackAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		_goBackAction->setEnabled(false);
		addAction(_goBackAction);
		_goFwdAction = _toolbar->addAction(QIcon(":/EditorApp/Resources/forward.png"), "Go Forward", this, &EditorWidget::goForward);
		_goFwdAction->setShortcut(QKeySequence::Forward);
		_goFwdAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		_goFwdAction->setEnabled(false);
		addAction(_goFwdAction);
	}
	_toolbar->addSeparator();
	{
		QToolButton *btn = new DragButton(this);
		btn->setIcon(QIcon(":/EditorApp/Resources/move.png"));
		_toolbar->addWidget(btn);
	}
	{
		_menuBtn = new QToolButton(this);
		_menuBtn->setStyleSheet("QToolButton::menu-indicator{image:none;}");
		_menuBtn->setIcon(QIcon(":/EditorApp/Resources/menu.png"));
		_menuBtn->setArrowType(Qt::NoArrow);
		_menuBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
		_menuBtn->setPopupMode(QToolButton::InstantPopup);
		_menuBtn->setMenu(_pageMenu);
		_toolbar->addWidget(_menuBtn);
	}
	{
		_pageList = new EditorPageListWidget(this);
		_pageList->setMaximumSize(200, 16777215);
		_pageList->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
		_pageList->setMaxVisibleItems(25);
		_toolbar->addWidget(_pageList);
		connect(_pageList, &EditorPageListWidget::pageChanged, this, &EditorWidget::onPageChanged);
		connect(_pageList, &EditorPageListWidget::pageClosed, this, &EditorWidget::onPageClosed);
	}
	{
		_closePageAction = _toolbar->addAction(QIcon(":/EditorApp/Resources/close2.png"), "Close", _pageList, &EditorPageListWidget::closeCurrentPage);
		_closePageAction->setShortcut(QKeySequence::Close);
		_closePageAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
		_closePageAction->setEnabled(false);
		addAction(_closePageAction);
	}
	_toolbar->addSeparator();
	{
		_functionList = new QComboBox(this);
		_functionList->setMaximumSize(200, 16777215);
		_functionList->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
		_functionList->setMaxVisibleItems(25);
		_toolbar->addWidget(_functionList);
		connect(_functionList, qOverload<int>(&QComboBox::currentIndexChanged), this, &EditorWidget::gotoFunction);
	}
	{
		QWidget *spacer = new QWidget();
		spacer->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum));
		_toolbar->addWidget(spacer);
	}
	{
		_collapseAction = _toolbar->addAction("Close Window", this, &EditorWidget::closeEditor);
		QMenu *splitMenu = new QMenu(this);
		_splitVAction = splitMenu->addAction(QIcon(":/EditorApp/Resources/split-vertical.png"), "Split Vertically", this, &EditorWidget::splitVertical);
		_splitHAction = splitMenu->addAction(QIcon(":/EditorApp/Resources/split-horizontal.png"), "Split Horizontally", this, &EditorWidget::splitHorizontal);
		QToolButton *splitBtn = new QToolButton(this);
		splitBtn->setStyleSheet("QToolButton::menu-indicator{image:none;}");
		splitBtn->setIcon(QIcon(":/EditorApp/Resources/split-vertical.png"));
		splitBtn->setArrowType(Qt::NoArrow);
		splitBtn->setToolButtonStyle(Qt::ToolButtonIconOnly);
		splitBtn->setPopupMode(QToolButton::InstantPopup);
		splitBtn->setMenu(splitMenu);
		_toolbar->addWidget(splitBtn);
	}

	{
		connect(_pageMenu, &QMenu::aboutToShow, this, [&]() { setFocus(); });

		_pageMenu->addAction(ActionManager::instance().getAction(ActionID::SAVE));
		_pageMenu->addSeparator();
		_pageMenu->addAction(_closePageAction);
		_closeAllPagesAction = _pageMenu->addAction("Close All", _pageList, &EditorPageListWidget::closeAllPages);
		_closeAllPagesAction->setEnabled(false);
		_closeAllButThisPagesAction = _pageMenu->addAction("Close All But This", _pageList, &EditorPageListWidget::closeAllButCurrentPage);
		_closeAllButThisPagesAction->setEnabled(false);
		_pageMenu->addSeparator();
		_openContainingFolderAction = _pageMenu->addAction("Open Containing Folder", this, &EditorWidget::openContainingFolder);
		_openContainingFolderAction->setEnabled(false);
		_pageMenu->addSeparator();
		_pageMenu->addAction(ActionManager::instance().getAction(ActionID::ADD_CLASS));
		_pageMenu->addAction(ActionManager::instance().getAction(ActionID::IMPORT_CLASS));
	}

	_toolbar->setIconSize(QSize(16, 16));
	 
	QLabel *dummy = new QLabel();
	dummy->setText("Our life is full of empty space!");
	dummy->setAlignment(Qt::AlignCenter);
	dummy->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
	dummy->setFocusPolicy(Qt::WheelFocus);

	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	_dxEditorWidget = new DXEditorWidget(this);
	_dxEditorWidget->setObjectName(QStringLiteral("dxEditorWidget"));
	sizePolicy.setHeightForWidth(_dxEditorWidget->sizePolicy().hasHeightForWidth());
	_dxEditorWidget->setSizePolicy(sizePolicy);
	_dxEditorWidget->setMinimumSize(QSize(0, 0));
	_dxEditorWidget->setFocusPolicy(Qt::StrongFocus);
	_dxEditorWidget->setAcceptDrops(false);
	setFocusProxy(_dxEditorWidget);

	_tabWidget = new QTabWidget(this);
	QTabBar *tabBar = _tabWidget->findChild<QTabBar *>();
	tabBar->hide();
	_tabWidget->setObjectName("mainTabBar");
	_tabWidget->setStyleSheet("QTabWidget#mainTabBar::pane { border: 0; background-color: darkgray }");
	_tabWidget->addTab(dummy, "Empty");
	_tabWidget->addTab(_dxEditorWidget, "Class Editor");

	_verticalLayout = new QVBoxLayout(this);
	_verticalLayout->setSpacing(6);
	_verticalLayout->setContentsMargins(11, 11, 11, 11);
	_verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
	_verticalLayout->setContentsMargins(0, 0, 0, 0);
	_verticalLayout->addWidget(_toolbar);
	_verticalLayout->addWidget(_tabWidget);

	connect(_dxEditorWidget, &DXEditorWidget::showChipDialog, this, [=](Chip*c,bool b) { showChipDialog(c, b); } );
	connect(_dxEditorWidget, &DXEditorWidget::focusOnChip, this, [=](ClassExt*cl,Chip*c) { openClass(cl, c); } );
	connect(_dxEditorWidget, &DXEditorWidget::enterFolder, this, [=](ClassExt*cl,FolderID f,bool b) { openClassOnFolder(cl, f, b); } );
	connect(_dxEditorWidget, &DXEditorWidget::showClassDescriptionDialog, this, [=](ClassExt* cl) { showClassDescriptionDialog(cl); });
	connect(this, &EditorWidget::forceVSync, _dxEditorWidget, &DXEditorWidget::SetForceVSync);
	connect(this, &EditorWidget::newFrame, _dxEditorWidget, &DXEditorWidget::invalidateGraphics);
	connect(this, &EditorWidget::addClass, this, &EditorWidget::setAsCurrent);
	connect(this, &EditorWidget::importClass, this, &EditorWidget::setAsCurrent);
	connect(this, &EditorWidget::showChipDialog, this, [=](Chip*,bool) { setAsCurrent(); });
	connect(this, &EditorWidget::onClassNameOrDirtyFlagChanged, _pageList, &EditorPageListWidget::onClassNameOrDirtyFlagChanged);
	connect(this, &EditorWidget::onDocumentFileChange, _pageList, &EditorPageListWidget::onDocumentFileChange);
	connect(_dxEditorWidget, &DXEditorWidget::showClassDescriptionDialog, this, [=](ClassExt*) { setAsCurrent(); });

	EditorCollection::GetInstance()->AddEditorWidget(this);
}

EditorWidget::~EditorWidget()
{
	EditorCollection::GetInstance()->RemoveEditorWidget(this);
}

EditorWidget *EditorWidget::duplicate(QWidget *parent) const
{
	EditorWidget *ew = new EditorWidget(parent);

	// Copy page list and history!
	ew->_history = _history.duplicate();
	ew->_pageList->copyFrom(_pageList);

	return ew;
}

void EditorWidget::hightlight(bool b)
{
	if (b)
		_toolbar->setStyleSheet("QToolBar { background: yellow; }");
	else
		_toolbar->setStyleSheet("");
}

void EditorWidget::SetFPSProps(bool showFPS, bool limitFPS)
{
	if (_dxEditorWidget)
		_dxEditorWidget->SetFPSProps(showFPS, limitFPS);
}

void EditorWidget::onParented(bool closeable, int index, Qt::Orientation o)
{
	static QIcon l(":/EditorApp/Resources/collapse-left.png");
	static QIcon r(":/EditorApp/Resources/collapse-right.png");
	static QIcon u(":/EditorApp/Resources/collapse-up.png");
	static QIcon d(":/EditorApp/Resources/collapse-down.png");
	if (closeable) {
		if (o == Qt::Vertical)
			_collapseAction->setIcon(index == 0 ? u : d);
		else
			_collapseAction->setIcon(index == 0 ? l : r);
		_collapseAction->setVisible(true);
	}
	else
		_collapseAction->setVisible(false);
}

// Called when moving a page from another editor widget.
void EditorWidget::moveFrom(EditorWidget *ew)
{
	if (ew == this) 
		return;

	EditorPageHistory::Page p = ew->_history.getCurrentPage();

	if (p.type == EPT_CHIP)
	{
		// A chip dialog can only be open in one widget at a time. Remove the dialog from the widget we are moving from.
		ChipDialogPage *p = static_cast<ChipDialogPage*>(ew->_pageList->GetCurrentPage());
		EmbeddedChipDialogContainer *c = p->container;
		c->_markedForDestruction = true; // Prevent destruction of the container.
		ew->closeChipDialog(c, true); // Close the other view. Remove its history for the dialog.
		c->_markedForDestruction = false;
		c->_ew = this; // Move ownership of the container.
		_tabWidget->addTab(c, "");
	}

	_openPageFromHistory(p); // This will also update our history.
}

ChipDialogContainer *EditorWidget::createChipDialogContainer()
{
	// Create a new container and add it to our tab widget. We are the owner!
	EmbeddedChipDialogContainer *c = new EmbeddedChipDialogContainer(this);
	_tabWidget->addTab(c, "");
	return c;
}

void EditorWidget::openChipDialog(EmbeddedChipDialogContainer *c)
{
	// Open a chip dialog in a container owned by us.
	_pageList->OpenChipDialogPage(c);
}

void EditorWidget::closeChipDialog(ChipDialogContainer *c, bool removeFromHistory)
{
	EmbeddedChipDialogContainer *e = dynamic_cast<EmbeddedChipDialogContainer*>(c);
	if (!e)
		return;

	EditorPage *ep = _pageList->FindPage(e);
	if (ep)
		_pageList->closePage(ep); // Close the given page!

	if (removeFromHistory)
		_history.removeDialog(c->GetChipDialog()->GetChip()); // History is updated by now. It will not be current page!

	_goBackAction->setEnabled(_history.canGoBack());
	_goFwdAction->setEnabled(_history.canGoFwd());
}


void EditorWidget::onChipDialogPageChanged()
{
	EditorPage *ep = _pageList->GetCurrentPage();
	if (ep->type != EPT_CHIP)
		return;

	ChipDialogPage *p = static_cast<ChipDialogPage*>(ep);

	_functionList->blockSignals(true); // This should really not be neccessary!
	_functionList->setCurrentIndex(p->container->GetChipDialog()->GetCurrentPage());
	_functionList->blockSignals(false);

	_history.openDialog(p->container->GetChipDialog()->GetChip(), p->container->GetChipDialog()->GetCurrentPage());

	_goBackAction->setEnabled(_history.canGoBack());
	_goFwdAction->setEnabled(_history.canGoFwd());
}


bool _blockPageListEvents = false;
bool _blockHistoryUpdates = false;
bool _blockNewFrame = false;

template<typename T>
struct ScopedReset
{
	T s;
	T &t;
	ScopedReset(T &t, T v) : s(t), t(t) { t = v; }
	~ScopedReset() { t = s; }
};
typedef ScopedReset<bool> ScopedBoolReset;


void EditorWidget::goBack()
{
	ScopedBoolReset r(_blockHistoryUpdates, true); // History already updated!
	_openPageFromHistory(_history.goBackOrFwd(-1));
}

void EditorWidget::goForward()
{
	ScopedBoolReset r(_blockHistoryUpdates, true); // History already updated!
	_openPageFromHistory(_history.goBackOrFwd(1));
}

void EditorWidget::openClassWithZoom(ClassExt *c, Chip *ch, float zoom) 
{
	if (!c)
		return;

	ClassViewPage *p = nullptr;

	{
		ScopedBoolReset r(_blockPageListEvents, true); // We handle this event ourself!
		p = _pageList->OpenClassPage(c);
	}

	{
		ScopedBoolReset r(_blockNewFrame, true); // RestoreChips can cause newFrame() to be called!
		ScopedOverrideCursor oc(Qt::WaitCursor);
		c->RestoreChips();
	}

	if (p) {
		// Focus on the given chip, if any!
		if (ch)
			p->view->FocusOnChip(ch->GetID());
		// Set the zoom if given.
		if (zoom != 0.0f) 
			p->view->SetZoom(zoom);

		if (!_blockHistoryUpdates) {
			_history.openClass(c, ch, p->view->GetFolder(), p->view->GetCameraPos().x, p->view->GetCameraPos().y, p->view->GetZoom(), false);
		}
		else
			_history.updateCurrentClassPage(p->view->GetCameraPos().x, p->view->GetCameraPos().y, p->view->GetZoom(), p->view->GetFolder());
	}

	_onPageChanged();

	setFocus();
}

void EditorWidget::openClassOnFolder2(ClassExt *c, unsigned folderID, float x, float y, float zoom) 
{
	if (!c)
		return;

	ClassViewPage *p = nullptr;

	{
		ScopedBoolReset r(_blockPageListEvents, true);
		p = _pageList->OpenClassPage(c);
	}

	{
		ScopedBoolReset r(_blockNewFrame, true);
		ScopedOverrideCursor oc(Qt::WaitCursor);
		c->RestoreChips();
	}

	if (p) {
		p->view->SetFolder(folderID, true);
		p->view->SetCameraPos(Vector2(x, y));
		p->view->SetZoom(zoom);

		if (!_blockHistoryUpdates) {
			_history.openClass(c, nullptr, p->view->GetFolder(), p->view->GetCameraPos().x, p->view->GetCameraPos().y, p->view->GetZoom(), false);
		}
	}

	_onPageChanged();
}

void EditorWidget::openClassOnFolder(ClassExt *c, unsigned folderID, bool inside) 
{
	if (!c)
		return;
	
	ClassViewPage *p = nullptr;

	{
		ScopedBoolReset r(_blockPageListEvents, true);
		p = _pageList->OpenClassPage(c);
	}

	{
		ScopedBoolReset r(_blockNewFrame, true);
		ScopedOverrideCursor oc(Qt::WaitCursor);
		c->RestoreChips();
	}

	if (p) {
		// Focus on given folder.
		p->view->SetFolder((FolderID)folderID, inside);
		if (!_blockHistoryUpdates) {
			_history.openClass(c, nullptr, p->view->GetFolder(), p->view->GetCameraPos().x, p->view->GetCameraPos().y, p->view->GetZoom(), false);
		}
	}

	_onPageChanged();
}

void EditorWidget::onClassRemoved(ClassExt *cg) 
{
	EditorPage *ep = _pageList->FindPage(cg);
	if (ep)
		_pageList->closePage(ep); // Close the given page!

	_history.onClassRemoved(cg);

	_goBackAction->setEnabled(_history.canGoBack());
	_goFwdAction->setEnabled(_history.canGoFwd());
}

void EditorWidget::onFolderRemoved(ClassExt*cg, unsigned folderID)
{
	_history.onFolderRemoved(cg, folderID);

	_goBackAction->setEnabled(_history.canGoBack());
	_goFwdAction->setEnabled(_history.canGoFwd());
}

void EditorWidget::onFunctionAdded(Function*f)
{
	if (_pageList->currentIndex() == -1)
		return;
	ClassViewPage *p = dynamic_cast<ClassViewPage*>(_pageList->GetCurrentPage());
	if (p == nullptr)
		return;
	if (p->view->GetClass() != f->GetChip()->GetClass())
		return;
	_fillFunctionList();
}

void EditorWidget::onFunctionChanged(Function*f)
{
	onFunctionAdded(f); // Identical functionality!
}

void EditorWidget::onFunctionRemoved(Function*f) 
{
	onFunctionAdded(f); // Identical functionality!
}

void EditorWidget::openContainingFolder()
{
	ClassViewPage *p = dynamic_cast<ClassViewPage*>(_pageList->GetCurrentPage());
	if (!p)
		return;
	if (p->view->GetClass()->GetDocument() && p->view->GetClass()->GetDocument()->GetFileName().IsFile())
		ShellExecuteA(0, MTEXT("explore"), p->view->GetClass()->GetDocument()->GetFileName().GetDirectory().AsString().c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

void EditorWidget::closeAll()
{
	_pageList->closeAllPages();
}

void EditorWidget::onNewFrame()
{
	if (_blockNewFrame) 
		return;

	EditorPage *ep = _pageList->GetCurrentPage();
	if (ep->type == EPT_CLASS) {
		emit newFrame(); // This will update the editor graphics view!

		ClassViewPage *p = static_cast<ClassViewPage*>(ep);
		
		// Note: We assume _currentPageVisit has the correct folder set!

		const EditorPageHistory::Page &pv = _history.getCurrentPage();

		assert(pv.type == EPT_CLASS);

		if (pv.classView.x != p->view->GetCameraPos().x || pv.classView.y != p->view->GetCameraPos().y) { // Has the camera moved?
			if (pv.classView.cameraMoved) { // Has the camera moved earlier?
				// Update the current history with new camera position
				_history.updateCurrentClassPage(p->view->GetCameraPos().x, p->view->GetCameraPos().y, p->view->GetZoom(), p->view->GetFolder());
			}
			else {
				_history.openClass(p->view->GetClass(), nullptr, p->view->GetFolder(), p->view->GetCameraPos().x, p->view->GetCameraPos().y, p->view->GetZoom(), true);
				_goBackAction->setEnabled(_history.canGoBack());
				_goFwdAction->setEnabled(_history.canGoFwd());
			}
			_functionList->setCurrentIndex(-1);
		}
	}
}

void EditorWidget::save()
{
	ClassViewPage *p = dynamic_cast<ClassViewPage*>(_pageList->GetCurrentPage());
	if (!p)
		return;
	if (p->view->GetClass()->GetDocument())
		emit saveDocument(p->view->GetClass()->GetDocument());
}

void EditorWidget::_openPageFromHistory(const EditorPageHistory::Page &p)
{
	switch (p.type)
	{
	case EPT_CLASS:
	{
		if (p.classView.chip != nullptr)
			openClassWithZoom(p.classView.clazz, p.classView.chip, p.classView.zoom);
		else
			openClassOnFolder2(p.classView.clazz, p.classView.folder, p.classView.x, p.classView.y, p.classView.zoom);
	}
	break;
	case EPT_CHIP:
	{
		emit showChipDialog(p.chipDialog.chip, false, getID());
		ChipDialogPage *page = dynamic_cast<ChipDialogPage*>(_pageList->GetCurrentPage());
		if (page && page->container->GetChipDialog()->GetChip() == p.chipDialog.chip)
			_functionList->setCurrentIndex(p.chipDialog.activeTab);
	}
	break;
	}
}

void EditorWidget::_onPageChanged()
{
	EditorPage *newPage = _pageList->GetCurrentPage();

	switch (newPage->type)
	{
	case EPT_EMPTY:
	{
		_tabWidget->setCurrentIndex(0);
		_dxEditorWidget->setView(nullptr);
		_closePageAction->setEnabled(false);
		_closeAllButThisPagesAction->setEnabled(false);
		_openContainingFolderAction->setEnabled(false);
	}
	break;
	case EPT_CLASS:
	{
		m3d::ClassViewPage *p = (m3d::ClassViewPage*)newPage;
		_tabWidget->setCurrentIndex(1);
		_dxEditorWidget->setView(p->view);
		_closePageAction->setEnabled(true);
		_closeAllButThisPagesAction->setEnabled(_pageList->count() > 1);
		_openContainingFolderAction->setEnabled(p->view->GetClass()->GetDocument() && p->view->GetClass()->GetDocument()->GetFileName().IsFile());
	}
	break;
	case EPT_CHIP:
	{
		m3d::ChipDialogPage *p = (m3d::ChipDialogPage*)newPage;
		int i = _tabWidget->indexOf(p->container);
		_tabWidget->setCurrentIndex(i);
		_closePageAction->setEnabled(true);
		_closeAllButThisPagesAction->setEnabled(_pageList->count() > 1);
		_openContainingFolderAction->setEnabled(false);
	}
	break;
	}

	ActionManager::instance().updateAction(ActionID::SAVE);

	_closeAllPagesAction->setEnabled(_pageList->count() > 0);

	_fillFunctionList();

	_goBackAction->setEnabled(_history.canGoBack());
	_goFwdAction->setEnabled(_history.canGoFwd());

	emit pageChanged();
}

Document *EditorWidget::getCurrentDocument() const
{
	ClassViewPage *p = dynamic_cast<ClassViewPage*>(_pageList->GetCurrentPage());
	if (!p)
		return nullptr;
	return p->view->GetClass()->GetDocument();
}

void EditorWidget::onPageChanged(EditorPage *newPage)
{
	// This slot is called by _pageList only.
	if (_blockPageListEvents)
		return;
	
	if (!_blockHistoryUpdates) {
		EditorPage *ep = _pageList->GetCurrentPage();

		switch (ep->type) 
		{
		case EPT_CLASS:
		{
			ClassViewPage *p = static_cast<ClassViewPage*>(ep);
			_history.openClass(p->view->GetClass(), nullptr, p->view->GetFolder(), p->view->GetCameraPos().x, p->view->GetCameraPos().y, p->view->GetZoom(), false);
			break;
		}
		case EPT_CHIP:
		{
			ChipDialogPage *p = static_cast<ChipDialogPage*>(ep);
			_history.openDialog(p->container->GetChipDialog()->GetChip(), p->container->GetChipDialog()->GetCurrentPage());
			break;
		}
		}
	}

	_onPageChanged();
}

void EditorWidget::onPageClosed(EditorPage *ep)
{
	switch (ep->type)
	{
	case EPT_CLASS:
		break;
	case EPT_CHIP:
		{
			ChipDialogPage *p = (ChipDialogPage*)ep;
			if (!p->container->IsMarkedForDestruction())
				p->container->GetChipDialog()->reject();
			int i = _tabWidget->indexOf(p->container);
			if (i != -1)
				_tabWidget->removeTab(i);
		}
		break;
	}
}


void EditorWidget::gotoFunction() 
{
	QSignalBlocker blockPageList(_functionList);

	if (_functionList->currentIndex() == -1)
		return;

	EditorPage *ep = _pageList->GetCurrentPage();

	switch (ep->type)
	{
	case EPT_CLASS:
		{
			ClassViewPage *p = static_cast<ClassViewPage*>(ep);
			ChipID cid = (ChipID)_functionList->currentData().toInt();
			Chip *c = p->view->GetClass()->FindChip(cid);
			if (c)
				openClass(p->view->GetClass(), c);
		}
		break;
	case EPT_CHIP:
		{
			ChipDialogPage *p =static_cast<ChipDialogPage*>(ep);
			p->container->GetChipDialog()->SetCurrentPage(_functionList->currentIndex());
		}
		break;
	}
}

void EditorWidget::_fillFunctionList()
{
	QSignalBlocker blockPageList(_functionList);

	_functionList->clear();

	EditorPage *ep = _pageList->GetCurrentPage();

	if (ep == nullptr)
		return;

	switch (ep->type)
	{
	case EPT_CLASS:
		{
			static const QIcon icons[8] = { QIcon(":/EditorApp/Resources/static-func.png"), QIcon(":/EditorApp/Resources/nonvirt-func.png"), QIcon(":/EditorApp/Resources/virt-func.png"), QIcon(":/EditorApp/Resources/parameter.png"), QIcon(":/EditorApp/Resources/func-call.png"), QIcon(":/EditorApp/Resources/instance-data.png"), QIcon(":/EditorApp/Resources/function-data.png"), QIcon(":/EditorApp/Resources/break-point.png") };

			ClassViewPage *p = static_cast<ClassViewPage*>(ep);
			const FunctionPtrSet &functions = p->view->GetClass()->GetFunctions();
			QMultiMap<QString, Function*> functionByName;
			for (const auto &n : functions)
				functionByName.insert(TOQSTRING(n->GetChip()->GetName()), n);
			for (QMultiMap<QString, Function*>::const_iterator itr = functionByName.begin(); itr != functionByName.end(); itr++) {
				_functionList->addItem(icons[(uint32)itr.value()->GetType()], itr.key(), (int)itr.value()->GetChip()->GetID());
				const ParameterConnectionSet &parameters = itr.value()->GetParameters();
				QMap<int, Parameter*> paramsByIndex;
				for (const auto &n : parameters)
					paramsByIndex.insert(n.c, n.p);
				int count = 1;
				for (QMap<int, Parameter*>::const_iterator jtr = paramsByIndex.begin(); jtr != paramsByIndex.end(); jtr++) {
					Chip *p = (Chip*)jtr.value();
					_functionList->addItem(icons[3], TOQSTRING(((count++ < paramsByIndex.size() ? u8"\x251c " : u8"\x2514 ") + p->GetName())), (int)p->GetID());
				}
			}
			if (_history.getCurrentPage().type == EPT_CLASS && _history.getCurrentPage().classView.chip != nullptr)
				_functionList->setCurrentIndex(_functionList->findData((int)_history.getCurrentPage().classView.chip->GetID()));
			else
				_functionList->setCurrentIndex(-1);
		}
		break;
	case EPT_CHIP:
		{
			static const QIcon icons[2] = { QIcon(":/EditorApp/Resources/prop-dialog.png"), QIcon(":/EditorApp/Resources/note.png") };

			ChipDialogPage *p = static_cast<ChipDialogPage*>(ep);
			ChipDialog *cd = p->container->GetChipDialog();
			for (unsigned i = 0, j = cd->GetPageCount(); i < j; i++) {
				_functionList->addItem(icons[i < j - 1 ? 0 : 1], cd->GetPageTitle(i));
			}
			_functionList->setCurrentIndex(cd->GetCurrentPage());
		}
		break;
	default:
		break;
	}
}

ActionMask EditorWidget::getActions() 
{
	emit setAsCurrent(); // This is kind of a hack, but this function is called when a child of us gains focus. That is, make us the current editor!

	return ActionMask(ActionID::SAVE) | ActionMask(ActionID::SAVE_AS);
}

bool EditorWidget::isActionEnabled(ActionID id)
{
	EditorPage *p = _pageList->GetCurrentPage();
	bool saveable = p && p->type == EPT_CLASS && ((ClassViewPage*)p)->view->GetClass()->GetDocument();
	return saveable;
}

void EditorWidget::triggered(ActionID id)
{
	EditorPage *p = _pageList->GetCurrentPage();
	Document *d = p && p->type == EPT_CLASS ? ((ClassViewPage*)p)->view->GetClass()->GetDocument() : nullptr;
	if (!d)
		return;
	if (id == ActionID::SAVE)
		emit saveDocument(d);
	else if (id == ActionID::SAVE_AS)
		emit saveDocumentAs(d);
}


