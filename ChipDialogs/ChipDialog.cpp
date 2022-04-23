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

#include "StdAfx.h"
#include "M3DEngine/Chip.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/Engine.h"
#include "DialogPage.h"
#include "ChipDialog.h"
#include "ChipDialogManager.h"


using namespace m3d;


ChipDialogDialog::ChipDialogDialog(QWidget *parent) : QDialog(parent, Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint)
{
	QGridLayout *l = new QGridLayout(this);
	l->setContentsMargins(0, 0, 0, 0);
}

ChipDialogDialog::~ChipDialogDialog()
{
}

void ChipDialogDialog::Flash()
{
	FLASHWINFO finfo;
	finfo.cbSize = sizeof(FLASHWINFO);
	finfo.hwnd = (HWND)winId();
	finfo.uCount = 6;
	finfo.dwTimeout = 50;
	finfo.dwFlags = FLASHW_CAPTION;
	::FlashWindowEx(&finfo);
}

void ChipDialogDialog::Activate()
{
	if (isMinimized())
		showNormal();
	activateWindow();
}

void ChipDialogDialog::DeleteLater()
{
	deleteLater();
}

void ChipDialogDialog::OnTitleChanged(QString s)
{
	setWindowTitle(s);
}

void ChipDialogDialog::SetChipDialog(ChipDialog *cd)
{
	_dlg = cd;

	cd->SetContainer(this);
	layout()->addWidget(cd);
	cd->SetTabBarHidden(false);

	connect(this, &ChipDialogDialog::finished, cd, &ChipDialog::onFinished);
	connect(cd, &ChipDialog::accept, this, &ChipDialogDialog::accept);
	connect(cd, &ChipDialog::reject, this, &ChipDialogDialog::reject);
}

void ChipDialogDialog::ReleaseChipDialog()
{
	disconnect(this, &ChipDialogDialog::finished, _dlg, &ChipDialog::onFinished);
	disconnect(_dlg, &ChipDialog::accept, this, &ChipDialogDialog::accept);
	disconnect(_dlg, &ChipDialog::reject, this, &ChipDialogDialog::reject);

	_dlg = nullptr;
}

void ChipDialogDialog::OpenDialog()
{
	show();
}

void ChipDialogDialog::OnPageChanged() 
{
	setMinimumSize(0, 0);
	_dlg->adjustSize();
	resize(_dlg->GetCurrentPageSize());
}

void ChipDialogDialog::resizeEvent(QResizeEvent * event)
{
	QDialog::resizeEvent(event);
	_dlg->SetCurrentPageSize(event->size());
}



ChipDialog::ChipDialog() : QWidget(), _container(nullptr), _chip(nullptr), _dialogManager(nullptr), _commentChanged(false), _lastRefresh(0), _currentPage(nullptr)
{
	setFocusPolicy(Qt::NoFocus);

	_commentTab = new QWidget();
	_commentEdit = new QTextEdit(_commentTab);
	_commentEdit->setMinimumSize(QSize(0, 0));
	_commentEdit->setBaseSize(QSize(0, 0));
	_commentEdit->setWhatsThis(MTEXT("This text field lets you add a comment to the chip, eg for documentation purposes."));
	_commentEdit->setAcceptRichText(false);
	QLayout *editLayout = new QHBoxLayout(_commentTab);
	QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sizePolicy.setHorizontalStretch(0);
	sizePolicy.setVerticalStretch(0);
	sizePolicy.setHeightForWidth(_commentEdit->sizePolicy().hasHeightForWidth());
	_commentEdit->setSizePolicy(sizePolicy);
	editLayout->addWidget(_commentEdit);

	_tabWidget = new QTabWidget(this);
	_tabWidget->addTab(_commentTab, QString(MTEXT("Comments")));
	_tabWidget->setWhatsThis(MTEXT("Select the property page for the chip type you want to edit."));

	_embedButton = new QPushButton(this);
	_embedButton->setText(MTEXT("Embed/Release"));
	_embedButton->setAutoDefault(false);

	_spacerItem = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum);

	_okButton = new QPushButton(this);
	_okButton->setToolTip(MTEXT("Accept changes and close dialog box."));
	_okButton->setWhatsThis(MTEXT("Applies the settings and closes the dialog box."));
	_okButton->setText(MTEXT("OK"));
	_okButton->setAutoDefault(true);
	_okButton->setDefault(true);

	_cancelButton = new QPushButton(this);
	_cancelButton->setToolTip(MTEXT("Undo changes. WARNING: Not all changes may be undoable!"));
	_cancelButton->setWhatsThis(MTEXT("Undo changes made in the dialog. WARNING: Not all changes may be undoable!"));
	_cancelButton->setText(MTEXT("Cancel"));

	_applyButton = new QPushButton(this);
	_applyButton->setWhatsThis(MTEXT("Applies the settings, but do not close the dialog box."));
	_applyButton->setText(MTEXT("Apply"));

	_horizontalLayout = new QHBoxLayout();
	_horizontalLayout->addWidget(_embedButton);
	_horizontalLayout->addItem(_spacerItem);
	_horizontalLayout->addWidget(_okButton);
	_horizontalLayout->addWidget(_cancelButton);
	_horizontalLayout->addWidget(_applyButton);

	_gridLayout = new QGridLayout(this);
	_gridLayout->addWidget(_tabWidget, 0, 0, 1, 1);
	_gridLayout->addLayout(_horizontalLayout, 1, 0, 1, 1);

	QWidget::setTabOrder(_okButton, _cancelButton);
	QWidget::setTabOrder(_cancelButton, _applyButton);
	QWidget::setTabOrder(_applyButton, _tabWidget);
	QWidget::setTabOrder(_tabWidget, _commentEdit);

	connect(_tabWidget, &QTabWidget::currentChanged, this, &ChipDialog::onPageChange);
	connect(_applyButton, &QPushButton::clicked, this, &ChipDialog::onApply);
	connect(_commentEdit, &QTextEdit::textChanged, this, &ChipDialog::onCommentChanged);
	connect(_okButton, &QPushButton::clicked, this, &ChipDialog::accept);
	connect(_cancelButton, &QPushButton::clicked, this, &ChipDialog::reject);
	connect(_embedButton, &QPushButton::clicked, this, &ChipDialog::onEmbed);
}

ChipDialog::~ChipDialog()
{
}

void ChipDialog::AddPage(DialogPage *page, String name, QSize size, PageData::MaximizedState maximized)
{
	if (!page)
		return;

	QWidget *qw = page->GetWidget();

	page->SetChipDialog(this);

	// qw is parented to tabWidget and destroyed when tabWidget is destroyed.
	int32 i = _tabWidget->insertTab(_tabWidget->count() - 1, qw, TOQSTRING(name));
	
	_pageMap.insert(std::make_pair(qw, PageData(page, size, maximized)));
}

void ChipDialog::Init(QSize defaultCommentPageSize, QSize *sizeHint)
{
	QWidget *fd = _tabWidget->widget(_tabWidget->count() - 1);
	_pageMap.insert(std::make_pair(_tabWidget->widget(_tabWidget->count() - 1), PageData(nullptr, defaultCommentPageSize)));
	_commentEdit->setText(TOQSTRING(GetDialogManager()->GetComment(GetChip())));
	_commentChanged = false;
	

	for (int32 i = 0, j = _tabWidget->count() - 1; i < j; i++) {
		auto n = _pageMap.find(_tabWidget->widget(j - i - 1)); // Iterate from base chips first!
		assert(n != _pageMap.end());
		assert(n->second.page != nullptr);
		try {
			n->second.page->Init();
		}
		catch (...) {
			msg(FATAL, String(MTEXT("The Init() method for the dialog page \'")) + FROMQSTRING(_tabWidget->tabText(j - i - 1)) + MTEXT("\' throwed an exception."), _chip);
			_pageMap.erase(_tabWidget->widget(j - i - 1));
			_tabWidget->removeTab(j - i - 1);
		}
	}

	_title = TOQSTRING(GetChip()->GetName()) + MTEXT(" Properties");
	if (_container)
		_container->OnTitleChanged(_title);

	if (_tabWidget->count() == 1) // Only comment page?
		onPageChange(0); // Ensure size is set!
	else
		_tabWidget->setCurrentIndex(0);

	if (sizeHint && _container->AsQDialog())
		*sizeHint = _container->AsQDialog()->sizeHint();//->window()->sizeHint();
}

void ChipDialog::SetFocusToFirstItem()
{
	if (_tabWidget->count() == 0)
		return;

	QWidget *w = _tabWidget->widget(_tabWidget->currentIndex())->nextInFocusChain();
	for (; w && w != _tabWidget->widget(_tabWidget->currentIndex()) && w->focusPolicy() == Qt::NoFocus; w = w->nextInFocusChain()) {}
	if (w)
		w->setFocus(Qt::TabFocusReason);
}

void ChipDialog::Update()
{
	for (int32 i = 0, j = _tabWidget->count() - 1; i < j; i++) {
		auto n = _pageMap.find(_tabWidget->widget(j - i - 1)); // Iterate from base chips first!
		assert(n != _pageMap.end());
		DialogPage *dp = n->second.page;
		assert(dp != nullptr);
		try {
			dp->Update();
		}
		catch (...) {
			msg(FATAL, String(MTEXT("The Update() method for the dialog page \'")) + FROMQSTRING(_tabWidget->tabText(j - i - 1)) + MTEXT("\' throwed an exception."), _chip);
		}
	}
	// Refresh
	int32 ct = engine->GetClockTime();
	if (abs(ct - _lastRefresh) / (float32(CLOCKS_PER_SEC) / 1000.0f) > REFRESH_FREQ) {
		_lastRefresh = ct;
		auto n = _pageMap.find(_tabWidget->currentWidget());
		if (n != _pageMap.end()) {
			try {
				if (n->second.page)
					n->second.page->Refresh();
			}
			catch (...) {
				msg(FATAL, String(MTEXT("The Refresh() method for the dialog page \'")) + FROMQSTRING(_tabWidget->tabText(_tabWidget->currentIndex())) + MTEXT("\' throwed an exception."), _chip);
			}
		}
	}
}

void ChipDialog::onFinished(int32 result)
{
	if (_currentPage) {
		try {
			_currentPage->OnLeave();
		}
		catch (...) {
			msg(FATAL, String(MTEXT("The OnLeave() method for the dialog page \'")) + FROMQSTRING(_tabWidget->tabText(_tabWidget->currentIndex())) + MTEXT("\' throwed an exception."), _chip);
		}
	}
	// 17.11.20: Update comment first so function tooltip in project tree is updated!
	//           This does not solve the problem where we update the comment without changing the function, leaving the tooltip unchanged... (TODO)
	if (result == QDialog::Accepted) {
		if (_commentChanged) {
			GetDialogManager()->SetComment(GetChip(), FROMQSTRING(_commentEdit->toPlainText()));
			_dialogManager->SetDirty(_chip->GetClass());
		}
	}
	for (int32 i = 0, j = _tabWidget->count() - 1; i < j; i++) {
		auto n = _pageMap.find(_tabWidget->widget(j - i - 1)); // Iterate from base chips first!
		assert(n != _pageMap.end());
		DialogPage *dp = n->second.page;
		assert(dp != nullptr);
		if (result == QDialog::Accepted) {
			try {
				dp->OnOK();
			}
			catch (...) {
				msg(FATAL, String(MTEXT("The OnOK() method for the dialog page \'")) + FROMQSTRING(_tabWidget->tabText(j - i - 1)) + MTEXT("\' throwed an exception."), _chip);
			}
		}
		else {
			try {
				dp->OnCancel();
			}
			catch (...) {
				msg(FATAL, String(MTEXT("The OnCancel() method for the dialog page \'")) + FROMQSTRING(_tabWidget->tabText(j - i - 1)) + MTEXT("\' throwed an exception."), _chip);
			}
		}
	}
	if (_container->AsQDialog() == nullptr || !_container->AsQDialog()->isModal()) // We cannot close a modal dialog here!
		GetDialogManager()->CloseChipDialog(GetChip());
}

void ChipDialog::onPageChange(int32 i)
{
	if (_currentPage) {
		try {
			_currentPage->OnLeave();
		}
		catch (...) {
			msg(FATAL, String(MTEXT("The OnLeave() method for the dialog page \'")) + FROMQSTRING(_tabWidget->tabText(_tabWidget->currentIndex())) + MTEXT("\' throwed an exception."), _chip);
		}
	}

	for (int32 j = 0; j < _tabWidget->count(); j++) {
		_tabWidget->widget(j)->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
		}

	if (i == -1)
		return;

	_tabWidget->widget(i)->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	QWidget *w = _tabWidget->currentWidget();
	auto n = _pageMap.find(w);
	assert(n != _pageMap.end());

	_tabWidget->updateGeometry();
/*	if (n->second.size.width() == -1)
		_container->Resize(/*_container->AsQDialog()->* /sizeHint(), n->second.page->IsEmbedMaximized());
	else
		_container->Resize(n->second.size, n->second.maximized);
*/
	_currentPage = n->second.page;
	if (_currentPage) {
		try {
			_currentPage->OnEnter();
		}
		catch (...) {
			msg(FATAL, String(MTEXT("The OnEnter() method for the dialog page \'")) + FROMQSTRING(_tabWidget->tabText(_tabWidget->currentIndex())) + MTEXT("\' throwed an exception."), _chip);
		}
	}

	_container->OnPageChanged();

	// Make sure it is updated!
//	_refresh(n->second.page);
}

/*	virtual void focusInEvent(QFocusEvent * event);

void ChipDialog::focusInEvent(QFocusEvent * event)
{
	QDialog::focusInEvent(event);
}*/

void ChipDialog::onApply()
{
	for (int32 i = 0, j = _tabWidget->count() - 1; i < j; i++) {
		const auto n = _pageMap.find(_tabWidget->widget(j - i - 1)); // Iterate from base chips first!
		assert(n != _pageMap.end());
		DialogPage *dp = n->second.page;
		assert(dp != nullptr);
		try {
			dp->OnApply();
		}
		catch (...) {
			msg(FATAL, String(MTEXT("The OnApply() method for the dialog page \'")) + FROMQSTRING(_tabWidget->tabText(j - i - 1)) + MTEXT("\' throwed an exception."), _chip);
		}
		dp->ResetEditFlag();
	}
	for (int32 i = 0, j = _tabWidget->count() - 1; i < j; i++) {
		auto n = _pageMap.find(_tabWidget->widget(j - i - 1)); // Iterate from base chips first!
		assert(n != _pageMap.end());
		DialogPage *dp = n->second.page;
		assert(dp != nullptr);
		try {
			dp->AfterApply();
		}
		catch (...) {
			msg(FATAL, String(MTEXT("The AfterApply() method for the dialog page \'")) + FROMQSTRING(_tabWidget->tabText(j - i - 1)) + MTEXT("\' throwed an exception."), _chip);
		}
	}
	if (_commentChanged) {
		GetDialogManager()->SetComment(GetChip(), FROMQSTRING(_commentEdit->toPlainText()));
		_dialogManager->SetDirty(_chip->GetClass());
		_commentChanged = false;
	}
	_title = TOQSTRING(GetChip()->GetName()) + MTEXT(" Properties");
	if (_container)
		_container->OnTitleChanged(_title);
}

void ChipDialog::OpenCommentPage()
{
	if (_tabWidget->widget(_tabWidget->count() - 1) == _commentTab)
		_tabWidget->setCurrentIndex(_tabWidget->count() - 1);
}

void ChipDialog::HideCommentPage(bool hide)
{
	int32 i = _tabWidget->indexOf(_commentTab);
	if (hide && i != -1)
		_tabWidget->removeTab(i);
	if (!hide && i == -1)
		_tabWidget->insertTab(0, _commentTab, MTEXT("Comments"));
}

void ChipDialog::onCommentChanged()
{
	_commentChanged = true;
}

void ChipDialog::onEmbed()
{
	_dialogManager->ToggleEmbedding(this);
}

PageData ChipDialog::GetPageData(ChipTypeIndex chipType)
{
	for (const auto &n : _pageMap) {
		if (n.second.page && n.second.page->GetChipTypeIndex() == chipType)
			return n.second;
	}
	return PageData();
}

QSize ChipDialog::GetCommentPageSize()
{
	auto n = _pageMap.find(_commentTab);
	assert(n != _pageMap.end());
	return n->second.size;
}

void ChipDialog::DestroyDeviceObject()
{
	for (int32 i = 0, j = _tabWidget->count() - 1; i < j; i++) {
		auto n = _pageMap.find(_tabWidget->widget(j - i - 1));
		assert(n != _pageMap.end());
		n->second.page->DestroyDeviceObject();
	}
}

void ChipDialog::HideApplyButton()
{
	_applyButton->setVisible(false);
}

void ChipDialog::SetTabBarHidden(bool hide)
{
	QTabBar *tabBar = _tabWidget->findChild<QTabBar *>();
	if (tabBar->isHidden() != hide)
		tabBar->setHidden(hide);
}

void ChipDialog::SetContainer(ChipDialogContainer *c)
{
	setParent(c->GetWidget());
	_container = c;

	if (dynamic_cast<ChipDialogDialog*>(c) != nullptr) {
		_embedButton->setText(MTEXT("Embed"));
		_embedButton->setToolTip(MTEXT("Embeds the dialog into the main application window."));
		_dialogManager->SetDefaultEmbedding(_chip->GetChipDesc().type, 0);
	}
	else {
		_embedButton->setText(MTEXT("Release"));
		_embedButton->setToolTip(MTEXT("Releases the dialog from the main application window."));
		_dialogManager->SetDefaultEmbedding(_chip->GetChipDesc().type, c->GetEmbeddedID());
	}
}

uint32 ChipDialog::GetPageCount() const
{
	return _tabWidget->count();
}

uint32 ChipDialog::GetCurrentPage() const 
{
	return _tabWidget->currentIndex(); 
}

void ChipDialog::SetCurrentPage(uint32 index)
{
	_tabWidget->setCurrentIndex(index);
}

QString ChipDialog::GetPageTitle(uint32 index) const
{
	return _tabWidget->tabText(index);
}

QSize ChipDialog::GetCurrentPageSize() const
{
	auto n = _pageMap.find(_tabWidget->currentWidget());

	if (n == _pageMap.end())
		return QSize();

	if (n->second.size.width() == -1)
	{
		QSize t = minimumSizeHint();
		if (n->second.page)
		{
			QSize s = n->second.page->GetWidget()->sizeHint();
			if (s.width() > 0)
				t.setWidth(s.width());
			if (s.height() > 0)
				t.setHeight(s.height());
		}
		return t;
	}
	
	return n->second.size;
}

void ChipDialog::SetCurrentPageSize(QSize size)
{
	auto n = _pageMap.find(_tabWidget->currentWidget());

	if (n != _pageMap.end())
		n->second.size = size;
}

bool ChipDialog::IsCurrentPageMaximized() const
{
	auto n = _pageMap.find(_tabWidget->currentWidget());

	if (n == _pageMap.end())
		return false;

	if (n->second.maximized == PageData::MAX_UNSET) {
		DialogPage *dp = dynamic_cast<DialogPage*>(n->first);
		if (dp)
			return dp->IsEmbedMaximized();
		else
			return false; // Comment page!
	}

	return n->second.maximized == PageData::MAX_TRUE;
}

void ChipDialog::SetCurrentPageMaximized(bool maximized)
{
	auto n = _pageMap.find(_tabWidget->currentWidget());

	if (n != _pageMap.end())
		n->second.maximized = maximized ? PageData::MAX_TRUE : PageData::MAX_FALSE;
}


/*
void ChipDialog::ReleasePages()
{
	_releasing = true;
	while (_tabWidget->count() > 1) {
		const PageMap::Node *n = _pageMap.find(_tabWidget->widget(0)); // Iterate from base chips first!
		assert(n != _pageMap.nil());
		DialogPage *dp = n->get().second.page;
		assert(dp != nullptr);
		_tabWidget->removeTab(0);
		dp->Release();
	}
}
*/
