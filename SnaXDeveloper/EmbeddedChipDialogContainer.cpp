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
#include "EmbeddedChipDialogContainer.h"
#include "EditorContainer.h"
#include "EditorWidget.h"

using namespace m3d;


namespace m3d
{ 
class CustomSizeGrip : public QWidget
{
public:
	CustomSizeGrip(QWidget *parent = nullptr) : QWidget(parent)
	{
		setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));
		setCursor(Qt::SizeFDiagCursor);
	}

	QSize sizeHint() const
	{
		QStyleOption opt(0);
		opt.initFrom(this);
		return style()->sizeFromContents(QStyle::CT_SizeGrip, &opt, QSize(13, 13), this);
	}

	void paintEvent(QPaintEvent *)
	{
		QPainter painter(this);
		QStyleOptionSizeGrip opt;
		opt.initFrom(this);
		opt.corner = Qt::Corner::BottomRightCorner;
		style()->drawControl(QStyle::CE_SizeGrip, &opt, &painter, this);
	}
};
}


EmbeddedChipDialogContainer::EmbeddedChipDialogContainer(EditorWidget *ew) : _ew(ew), _markedForDestruction(false)
{
	setAutoFillBackground(true);

	QGridLayout *l = new QGridLayout(this);
	l->setContentsMargins(0, 0, 0, 0);
	l->setSpacing(0);

	_csg = new CustomSizeGrip(this);
	_csg->installEventFilter(this);
	l->addWidget(_csg, 1, 1, Qt::AlignRight | Qt::AlignBottom);
}

EmbeddedChipDialogContainer::~EmbeddedChipDialogContainer()
{
	assert(_markedForDestruction == true);
}

void EmbeddedChipDialogContainer::Activate()
{
	_ew->openChipDialog(this);
}

void EmbeddedChipDialogContainer::DeleteLater()
{
	_markedForDestruction = true;
	_ew->closeChipDialog(this, false);
	setParent(nullptr);
	deleteLater();
}

void EmbeddedChipDialogContainer::SetChipDialog(ChipDialog *cd)
{
	_dlg = cd;
	cd->SetContainer(this);
	((QGridLayout*)layout())->addWidget(cd, 0, 0);
	cd->SetTabBarHidden(true);
	
	connect(this, &EmbeddedChipDialogContainer::finished, cd, &ChipDialog::onFinished);
	connect(cd, &ChipDialog::accept, this, &EmbeddedChipDialogContainer::accept);
	connect(cd, &ChipDialog::reject, this, &EmbeddedChipDialogContainer::reject);
}

void EmbeddedChipDialogContainer::ReleaseChipDialog()
{
	_markedForDestruction = true;
	_ew->closeChipDialog(this, true);
}

void EmbeddedChipDialogContainer::OpenDialog()
{
	_ew->openChipDialog(this);
}

unsigned EmbeddedChipDialogContainer::GetEmbeddedID() const
{
	return _ew->getID();
}

void EmbeddedChipDialogContainer::OnPageChanged()
{
	if (_dlg->IsCurrentPageMaximized()) {
		setMinimumSize(0, 0);
		setMaximumSize(1000000, 1000000);
	}
	else {
		setMinimumSize(_dlg->GetCurrentPageSize());
		setMaximumSize(_dlg->GetCurrentPageSize());
	}
	_ew->onChipDialogPageChanged();
}

void EmbeddedChipDialogContainer::resizeEvent(QResizeEvent * event)
{
	QWidget::resizeEvent(event);
	if (!_dlg->IsCurrentPageMaximized())
		_dlg->SetCurrentPageSize(event->size());
}


void EmbeddedChipDialogContainer::accept()
{
	emit finished(QDialog::Accepted);
}

void EmbeddedChipDialogContainer::reject()
{
	emit finished(QDialog::Rejected);
}

QString EmbeddedChipDialogContainer::GetTitle() const
{
	return _dlg->GetTitle();
}

bool EmbeddedChipDialogContainer::eventFilter(QObject *o, QEvent *e)
{
	QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);

	if (o == _csg && me != nullptr)
	{
		static QSize oldSize = QSize();
		static bool dragging = false;

		switch (static_cast<int>(e->type()))
		{
		case QEvent::MouseButtonPress:
			if (me->button() == Qt::LeftButton) {
				oldSize = _dlg->GetCurrentPageSize();
				dragging = true;
			}
			else
				dragging = false;
			break;
		case QEvent::MouseMove:
			if (dragging) {
				Q_ASSERT(me != nullptr);
				QPoint p = _csg->mapToParent(me->pos());
				QSize ms = layout()->minimumSize();
				QSize s = QSize(qMax(ms.width(), p.x()), qMax(ms.height(), p.y()));
				QSize mySize = ((QWidget*)parent())->size();
				s = QSize(qMin(s.width(), mySize.width()), qMin(s.height(), mySize.height()));
				if (s.width() * 100 > mySize.width() * 95 && s.height() * 100 > mySize.height() * 95) { // Dragging big?
					if (!_dlg->IsCurrentPageMaximized()) { // Going maximized!
						_dlg->SetCurrentPageMaximized(true);
						_dlg->SetCurrentPageSize(oldSize);
					}
				}
				else {
					if (_dlg->IsCurrentPageMaximized())
						_dlg->SetCurrentPageMaximized(false);
					_dlg->SetCurrentPageSize(s);
				}
				OnPageChanged();
			}
			break;
		case QEvent::MouseButtonRelease:
			dragging = false;
			break;
		case QEvent::MouseButtonDblClick:
			if (me->button() == Qt::LeftButton)
			{
				GetChipDialog()->SetCurrentPageMaximized(!GetChipDialog()->IsCurrentPageMaximized());
				OnPageChanged();
			}
			break;
		}
	}

	return QWidget::eventFilter(o, e);
}