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
#include "EditorPageListWidget.h"
#include "M3DEngineExt/ClassExt.h"
#include "M3DEngineExt/ClassDiagram.h"
#include "M3DEngine/Document.h"
#include "M3DEngineExt/ClassView.h"
#include "EmbeddedChipDialogContainer.h"

using namespace m3d;


ClassViewPage::ClassViewPage(ClassExt *c) : EditorPage(EPT_CLASS)
{
	view = c->CreateView();
}

ClassViewPage::~ClassViewPage()
{
	view->GetClass()->DestroyView(view);
}

static unsigned editorPageViewCounter = 0;
static EditorPage emptyEditorPage = EditorPage();

EditorPageListWidget::EditorPageListWidget(QWidget *parent) : QComboBox(parent)
{
	connect(this, qOverload<int>(&EditorPageListWidget::currentIndexChanged), this, &EditorPageListWidget::onCurrentChanged);
}

EditorPageListWidget::~EditorPageListWidget()
{
	assert(count() == 0);
}

void EditorPageListWidget::copyFrom(EditorPageListWidget *w)
{
	assert(count() == 0);
	unsigned vidx = 0;
	int j = -1;
	for (int i = 0; i < w->count(); i++) {
		EditorPage *ep = static_cast<EditorPage*>(w->itemData(i).value<void*>());
		if (ep->type == EPT_CLASS) { // We do NOT copy dialogs as there are only one of them!
			ClassViewPage *p = static_cast<ClassViewPage*>(ep);
			ClassViewPage *page = _addClassPage(p->view->GetClass()); // Signals are blocked!
			page->view->SetFolder(p->view->GetFolder(), true);
			page->view->SetCameraPos(p->view->GetCameraPos());
			page->view->SetZoom(p->view->GetZoom());
			page->viewIndex = p->viewIndex;
			if (page->viewIndex > vidx) {
				vidx = page->viewIndex;
				j = i;
			}
		}
	}

	{
		QSignalBlocker sb(this);
		setCurrentIndex(j); // This will not signal if j is already current.
	}

	onCurrentChanged();
}


EditorPage *EditorPageListWidget::GetCurrentPage()
{
	if (currentIndex() == -1)
		return &emptyEditorPage;
	EditorPage *ep = static_cast<EditorPage*>(currentData().value<void*>());
	return ep;
}

ClassViewPage *EditorPageListWidget::OpenClassPage(ClassExt *c)
{
	for (int i = 0; i < count(); i++) {
		EditorPage *ep = static_cast<EditorPage*>(itemData(i).value<void*>());
		ClassViewPage *p = dynamic_cast<ClassViewPage*>(ep);
		if (p && p->view->GetClass() == c) {
			setCurrentIndex(i); // This will trigger onCurrentChanged()
			return p;
		}
	}

	ClassViewPage *page = _addClassPage(c);

	onCurrentChanged();

	return page;
}

ClassViewPage *EditorPageListWidget::_addClassPage(ClassExt *c)
{
	static const QIcon icn(":/EditorApp/Resources/graph.png");
	static const QIcon icnRG(":/EditorApp/Resources/class-diagram.png");

	QSignalBlocker sb(this);

	ClassViewPage *page = new ClassViewPage(c);
	bool isRG = (dynamic_cast<ClassDiagram*>(c) != nullptr); // Class diagram?
	QString label = TOQSTRING(c->GetName());
	if (c->IsDirty())
		label += "*";
	int idx = _getInsertIndex(label);
	insertItem(idx, isRG ? icnRG : icn, label, QVariant::fromValue(static_cast<void*>(page)));
	setItemData(idx, isRG ? "" : (c->GetDocument()->GetFileName().IsValid() ? TOQSTRING(c->GetDocument()->GetFileName().AsString()) : "Not saved"), Qt::ToolTipRole);
	setCurrentIndex(idx);

	return page;
}

ChipDialogPage *EditorPageListWidget::OpenChipDialogPage(EmbeddedChipDialogContainer *c)
{
	static QIcon chipIcon(":/EditorApp/Resources/chip.png");

	for (int i = 0; i < count(); i++) {
		EditorPage *ep = static_cast<EditorPage*>(itemData(i).value<void*>());
		ChipDialogPage *p = dynamic_cast<ChipDialogPage*>(ep);
		if (p && p->container == c) {
			setCurrentIndex(i); // This will trigger onCurrentChanged()
			return p;
		}
	}

	ChipDialogPage *page = new ChipDialogPage(c);

	{
		QSignalBlocker sb(this);

		QString label = c->GetTitle();
		int idx = _getInsertIndex(label);
		insertItem(idx, chipIcon, label, QVariant::fromValue((void*)page));
		setItemData(idx, label, Qt::ToolTipRole);
		setCurrentIndex(idx);
	}

	onCurrentChanged();

	return page;
}

EditorPage *EditorPageListWidget::FindPage(ClassExt *c) const
{
	for (int i = 0; i < count(); i++) {
		EditorPage *ep = static_cast<EditorPage*>(itemData(i).value<void*>());
		ClassViewPage *p = dynamic_cast<ClassViewPage*>(ep);
		if (p && p->view->GetClass() == c)
			return p;
	}
	return nullptr;
}

EditorPage *EditorPageListWidget::FindPage(EmbeddedChipDialogContainer *c) const 
{
	for (int i = 0; i < count(); i++) {
		EditorPage *ep = static_cast<EditorPage*>(itemData(i).value<void*>());
		ChipDialogPage *p = dynamic_cast<ChipDialogPage*>(ep);
		if (p && p->container == c)
			return p;
	}
	return nullptr;
}

void EditorPageListWidget::closePage(int idx)
{
	if (idx < 0 || idx >= count())
		return;

	EditorPage *ep = static_cast<EditorPage*>(itemData(idx).value<void*>());

	if (currentIndex() == idx) {
		int k = -1;
		unsigned maxViewIdx = 0;
		for (int j = 0; j < count(); j++) {
			EditorPage *p = static_cast<EditorPage*>(itemData(j).value<void*>());
			if (j != idx && p->viewIndex > maxViewIdx) {
				maxViewIdx = p->viewIndex;
				k = j;
			}
		}
		setCurrentIndex(k); // This will trigger onCurrentChanged()
	}

	{
		QSignalBlocker sb(this);
		removeItem(idx);
	}

	emit pageClosed(ep);

	delete ep;
}

void EditorPageListWidget::closePage(EditorPage *ep)
{
	for (int i = 0; i < count(); i++)
		if (static_cast<EditorPage*>(itemData(i).value<void*>()) == ep)
			return closePage(i);
}

void EditorPageListWidget::closeCurrentPage()
{
	int i = currentIndex();
	if (i != -1)
		closePage(i);
}

void EditorPageListWidget::closeAllButCurrentPage()
{
	for (int i = count(); i > 0; i--)
		if ((i - 1) != currentIndex())
			closePage(i - 1);
}

void EditorPageListWidget::closeAllPages()
{
	setCurrentIndex(-1); // This will trigger onCurrentChanged()

	while (count() > 0) {
		closePage(count() - 1);
	}
}

void EditorPageListWidget::onClassNameOrDirtyFlagChanged(ClassExt *c)
{
	QSignalBlocker sb(this);

	static const QIcon icn(":/EditorApp/Resources/graph.png");

	for (unsigned i = 0; i < count(); i++) {
		EditorPage *ep = static_cast<EditorPage*>(itemData(i).value<void*>());
		ClassViewPage *page = dynamic_cast<ClassViewPage*>(ep);
		if (page && page->view->GetClass() == c) {
			QString newLabel = TOQSTRING(c->GetName());
			if (c->IsDirty())
				newLabel += "*";
			bool isCurrentItem = currentIndex() == i;
			removeItem(i);
			i = _getInsertIndex(newLabel);
			insertItem(i, icn, newLabel, QVariant::fromValue(static_cast<void*>(page)));
			setItemData(i, c->GetDocument()->GetFileName().IsValid() ? TOQSTRING(c->GetDocument()->GetFileName().AsString()) : "Not saved", Qt::ToolTipRole);
			if (isCurrentItem)
				setCurrentIndex(i);
			break;
		}
	}
}

void EditorPageListWidget::onDocumentFileChange(Document *d)
{
	for (size_t j = 0; j < d->GetClasss().size(); j++) {
		ClassExt *c = dynamic_cast<ClassExt*>(d->GetClasss()[j]);
		for (unsigned i = 0; i < count(); i++) {
			EditorPage *ep = static_cast<EditorPage*>(itemData(i).value<void*>());
			ClassViewPage *page = dynamic_cast<ClassViewPage*>(ep);
			if (page && page->view->GetClass() == c) {
				setItemData(i, c->GetDocument()->GetFileName().IsValid() ? TOQSTRING(c->GetDocument()->GetFileName().AsString()) : "Not saved", Qt::ToolTipRole);
				break;
			}
		}
	}
}

void EditorPageListWidget::onCurrentChanged()
{
	EditorPage *ep = GetCurrentPage();
	if (ep)
		ep->viewIndex = ++editorPageViewCounter;
	else
		ep = &emptyEditorPage;

	emit pageChanged(ep);
}

int EditorPageListWidget::_getInsertIndex(QString name)
{
	for (int i = 0; i < count(); i++)
		if (itemText(i) > name)
			return i;
	return count();
}
