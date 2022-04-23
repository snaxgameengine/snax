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
#include "M3DCore/MString.h"
#include "M3DCore/Containers.h"
#include "M3DEngine/GlobalDef.h"
#include <qglobal.h>
#include <qdialog.h>
#include <qtextedit.h>
#include <qgridlayout.h>
#include <qtablewidget.h>

namespace m3d
{

class Chip;
class ChipDialogManager;
class DialogPage;
class ChipDialog;

struct PageData
{
	enum MaximizedState { MAX_UNSET, MAX_FALSE, MAX_TRUE };
	DialogPage *page;
	QSize size;
	MaximizedState maximized;
	PageData(DialogPage *page = nullptr, QSize size = QSize(-1, -1), MaximizedState maximized = MAX_UNSET) : page(page), size(size), maximized(maximized) {}
};

typedef Map<QWidget*, PageData> PageMap;

#define REFRESH_FREQ 500

class ChipDialogContainer
{
public:
	ChipDialogContainer() : _dlg(nullptr) {}
	virtual ~ChipDialogContainer() {}

	ChipDialog *GetChipDialog() { return _dlg; }

	virtual QWidget *GetWidget() = 0;
	virtual QDialog *AsQDialog() { return nullptr; }
	virtual void Flash() = 0;
	virtual void Activate() = 0;
	virtual void DeleteLater() = 0;
	virtual void OnTitleChanged(QString s) = 0;
	virtual void SetChipDialog(ChipDialog *cd) = 0;
	virtual void ReleaseChipDialog() = 0;
	virtual void OpenDialog() = 0;
	virtual uint32 GetEmbeddedID() const = 0;
	virtual void OnPageChanged() = 0;

protected:
	ChipDialog *_dlg;
};

class ChipDialogDialog : public QDialog, public ChipDialogContainer
{
public:
	ChipDialogDialog(QWidget *parent);
	~ChipDialogDialog();

	QWidget *GetWidget() override { return this; }
	QDialog *AsQDialog() override { return this; }
	void Flash() override;
	void Activate() override;
	void DeleteLater() override;
	void OnTitleChanged(QString s) override;
	void SetChipDialog(ChipDialog *cd) override;
	void ReleaseChipDialog() override;
	void OpenDialog() override;
	uint32 GetEmbeddedID() const override { return 0; }
	void OnPageChanged() override;

protected:
	void resizeEvent(QResizeEvent * event) override;

};

class CHIPDIALOGS_EXPORT ChipDialog : public QWidget
{
	Q_OBJECT
public:
	ChipDialog();
	~ChipDialog();

	// Returns the assosiated chip.
	Chip *GetChip() const { return _chip; }
	// Called by ChipManager only!
	void SetChip(Chip *ch) { _chip = ch; }
	// Returns the dialog manager.
	ChipDialogManager *GetDialogManager() const { return _dialogManager; }
	// Called by ChipManager only!
	void SetDialogManager(ChipDialogManager *dm) { _dialogManager = dm; }
	// Called by ChipManager only!
	void AddPage(DialogPage *page, String name, QSize size, PageData::MaximizedState maximized);
	// Called by ChipManager only!
	void Init(QSize defaultCommentPageSize, QSize *sizeHint = nullptr);
	// Called by ChipManager only!
	void Update();
	// Sets input focus to the first item in the current page.
	void SetFocusToFirstItem();
	// Changes to the dialogs comment page.
	void OpenCommentPage();
	// show/hide comment page
	void HideCommentPage(bool hide);
	// Called by ChipManager only!
	PageData GetPageData(ChipTypeIndex chipType);
	// Called by ChipManager only!
	QSize GetCommentPageSize();
	// Called by ChipManager only!
	void DestroyDeviceObject();
	// Hides the Apply button
	void HideApplyButton();
	// 
	void SetTabBarHidden(bool hide);
	// Sets the container for this dialog.
	void SetContainer(ChipDialogContainer *c);
	// Gets the container (dialog box or embedded container) holding this dialog.
	ChipDialogContainer *GetContainer() const { return _container; }
	//
	QString GetTitle() const { return _title; }

	// Returns the number of pages, including comment page.
	uint32 GetPageCount() const;
	// Returns the index of the current page. Left to right, last is comment page.
	uint32 GetCurrentPage() const;
	// Set current page by index.
	void SetCurrentPage(uint32 index);
	// Gets the title of the given page.
	QString GetPageTitle(uint32 index) const;
	// Get the expected size of the current page!
	QSize GetCurrentPageSize() const;
	// Store the size of the current page.
	void SetCurrentPageSize(QSize size);
	// When embedded: Maximized or not?
	bool IsCurrentPageMaximized() const;
	// When embedded: Set maximized or not!
	void SetCurrentPageMaximized(bool maximized);

signals:
	// User pressed OK.
	void accept();
	// User pressed Cancel.
	void reject();

protected:
	QGridLayout *_gridLayout;
	QTabWidget *_tabWidget;
	QWidget *_commentTab;
	QTextEdit *_commentEdit;
	QHBoxLayout *_horizontalLayout;
	QPushButton *_embedButton;
	QSpacerItem *_spacerItem;
	QPushButton *_okButton;
	QPushButton *_cancelButton;
	QPushButton *_applyButton;

	ChipDialogContainer *_container;
	Chip *_chip;
	ChipDialogManager *_dialogManager;
	bool _commentChanged;
	PageMap _pageMap;
	int32 _lastRefresh;
	DialogPage *_currentPage;
	QString _title;

	void resizeEvent(QResizeEvent * event) override {}

public:
	void onFinished(int32);
	void onPageChange(int32);
	void onApply();
	void onCommentChanged();
	void onEmbed();
};


}