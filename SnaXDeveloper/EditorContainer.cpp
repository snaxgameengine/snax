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
#include "EditorContainer.h"
#include "EditorWidget.h"
#include "EditorCollection.h"


using namespace m3d;



EditorContainer::EditorContainer(QWidget *parent) : QSplitter(parent)
{
	_closeEditorMapper = new QSignalMapper(this);
	connect(_closeEditorMapper, &QSignalMapper::mappedInt, this, &EditorContainer::closeEditor);
	_splitVerticalMapper = new QSignalMapper(this);
	connect(_splitVerticalMapper, &QSignalMapper::mappedInt, this, &EditorContainer::splitVertical);
	_splitHorizontalMapper = new QSignalMapper(this);
	connect(_splitHorizontalMapper, &QSignalMapper::mappedInt, this, &EditorContainer::splitHorizontal);
	setOpaqueResize(false);
}

EditorContainer::~EditorContainer()
{
}

void EditorContainer::init(EditorWidget *ew)
{
	if (count() != 0)
		return;
	if (ew == nullptr)
		ew = new EditorWidget(this);

	addWidget(ew);
	setCollapsible(0, false);
	_connect(ew, 0);
}

void EditorContainer::closeEditor(int id)
{
	if (count() < 2 || id > 1)
		return;
	EditorContainer *p = dynamic_cast<EditorContainer*>(parent());
	if (p) {
		int i = p->indexOf(this);
		QWidget *w = widget(1 - id);
		QList<int> sizes = p->sizes();
		setParent(nullptr);
		p->insertWidget(i, w);
		p->setSizes(sizes);
		p->setCollapsible(i, false);
		EditorWidget *ew = dynamic_cast<EditorWidget*>(w);
		if (ew)
			p->_connect(ew, i);
		deleteLater();
	}
	else {
		QWidget *w = widget(id);
		if (w) {
			EditorWidget *ew = dynamic_cast<EditorWidget*>(w);
			if (ew) {
				ew->closeAll();
			}
			w->setParent(nullptr);
			w->deleteLater();
		}
		EditorWidget *ew = dynamic_cast<EditorWidget*>(widget(0));
		if (ew) {
			ew->onParented(false);
			if (id == 0) {
				_closeEditorMapper->setMapping(ew, 0);
				_splitVerticalMapper->setMapping(ew, 0);
				_splitHorizontalMapper->setMapping(ew, 0);
			}
		}
		else {
			EditorContainer *c = dynamic_cast<EditorContainer*>(widget(0));
			if (c) {
				Qt::Orientation o = c->orientation();
				QList<int> s = c->sizes();
				int t = (o == Qt::Horizontal ? c->size().width() : c->size().height()) / 2;
				int u = (o == Qt::Horizontal ? size().width() : size().height()) / 2;
				s[0] = u * s[0] / t;
				s[1] = u * s[1] / t;
				c->setParent(nullptr);
				addWidget(c->widget(0));
				addWidget(c->widget(0));
				setOrientation(o);
				setSizes(s);
				EditorWidget *ew0 = dynamic_cast<EditorWidget*>(widget(0));
				if (ew0)
					_connect(ew0, 0);
				EditorWidget *ew1 = dynamic_cast<EditorWidget*>(widget(1));
				if (ew1)
					_connect(ew1, 1);
				c->deleteLater();
			}
		}
	}
}

void EditorContainer::splitVertical(int id)
{
	_split(id, Qt::Vertical);
}

void EditorContainer::splitHorizontal(int id)
{
	_split(id, Qt::Horizontal);
}

void EditorContainer::_split(int id, Qt::Orientation o)
{
	if (id > count())
		return;
	if (count() == 1) {
		setOrientation(o);
		EditorWidget *ew0 = dynamic_cast<EditorWidget*>(widget(0));
		EditorWidget *ew = ew0 ? ew0->duplicate(this) : new EditorWidget(this);
		int s = (o == Qt::Horizontal ? size().width() : size().height()) / 2;
		QList<int> sizes({ s, s });
		setSizes(sizes);
		setCollapsible(1, false);
		_connect(ew, 1);
		ew = dynamic_cast<EditorWidget*>(widget(0));
		if (ew)
			ew->onParented(true, 0, o);
	}
	else if (count() == 2) {
		EditorWidget *ew = dynamic_cast<EditorWidget*>(widget(id));
		if (!ew)
			return;
		_closeEditorMapper->removeMappings(ew);
		_splitVerticalMapper->removeMappings(ew);
		_splitHorizontalMapper->removeMappings(ew);
		disconnect(ew, &EditorWidget::closeEditor, _closeEditorMapper, qOverload<>(&QSignalMapper::map));
		disconnect(ew, &EditorWidget::splitVertical, _splitVerticalMapper, qOverload<>(&QSignalMapper::map));
		disconnect(ew, &EditorWidget::splitHorizontal, _splitHorizontalMapper, qOverload<>(&QSignalMapper::map));
		QList<int> s = sizes();
		EditorContainer *c = new EditorContainer();
		c->init(ew);
		insertWidget(id, c);
		setSizes(s);
		setCollapsible(id, false);
		c->_split(0, o);
	}
	EditorWidget *c = EditorCollection::GetInstance()->GetCurrentEditor();
	if (c)
		c->hightlight(true);
}

void EditorContainer::_connect(EditorWidget *ew, int index)
{
	ew->onParented(dynamic_cast<EditorContainer*>(parent()) != nullptr || count() > 1, index, orientation());
	_closeEditorMapper->setMapping(ew, index);
	_splitVerticalMapper->setMapping(ew, index);
	_splitHorizontalMapper->setMapping(ew, index);
	connect(ew, &EditorWidget::closeEditor,     _closeEditorMapper,     qOverload<>(&QSignalMapper::map));
	connect(ew, &EditorWidget::splitVertical,   _splitVerticalMapper,   qOverload<>(&QSignalMapper::map));
	connect(ew, &EditorWidget::splitHorizontal, _splitHorizontalMapper, qOverload<>(&QSignalMapper::map));
}