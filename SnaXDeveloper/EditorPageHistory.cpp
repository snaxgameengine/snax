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
#include "EditorPageHistory.h"
#include "EmbeddedChipDialogContainer.h"
#include "M3DEngineExt/ClassExt.h"

using namespace m3d;


EditorPageHistory::EditorPageHistory()
{
}

EditorPageHistory::~EditorPageHistory()
{
}

EditorPageHistory EditorPageHistory::duplicate() const
{
	EditorPageHistory h;
	h._backPages = _backPages;
	h._fwdPages = _fwdPages;

	// Remove all chip dialogs. Function is called when duplicating an editor. Chip dialogs can only have one!
	auto f = [&h](QList<Page> &lst) { 
		for (QList<Page>::iterator itr = lst.begin(); itr != lst.end();)  {
			if (itr->type == EPT_CLASS && itr->classView.chip != nullptr)
				itr->classView.chip->AddDestructionObserver(&h);
			if (itr->type == EPT_CHIP)
				itr = lst.erase(itr); // Only one chip dialog!!
			else
				itr++;
		}
	};

	f(h._backPages);
	f(h._fwdPages);
	if (_currentPage.type != EPT_CHIP)
		h._currentPage = _currentPage; // Keep the same page if not a chip, else blank page!

	h._removeDuplicates();

	return h;
}

const EditorPageHistory::Page &EditorPageHistory::goBackOrFwd(int n)
{
	if (n == 0)
		return _currentPage;

	auto f = [&](QList<Page> &a, QList<Page> &b, unsigned n) {
		if ((unsigned)a.size() < n)
			return; // Can't go back/forward!

		Page p;

		for (unsigned i = 0; i < n; i++) {
			p = a.back();
			a.pop_back();

			if (_currentPage.type != EPT_EMPTY)
				b.push_back(_currentPage);

			_currentPage = p;
		}
	};

	if (n < 0)
		f(_backPages, _fwdPages, (unsigned)-n);
	else
		f(_fwdPages, _backPages, (unsigned)n);

	_removeDuplicates();

	return _currentPage;
}

void EditorPageHistory::removeDialog(Chip *chip)
{
	auto f = [chip](QList<Page> &lst) 
	{
		for (QList<Page>::iterator itr = lst.begin(); itr != lst.end(); ) {
			if (itr->type == EPT_CHIP && itr->chipDialog.chip == chip)
				itr = lst.erase(itr);
			else
				itr++;
		}
	};

	f(_backPages);
	f(_fwdPages);

	if (_currentPage.type == EPT_CHIP && _currentPage.chipDialog.chip == chip)
		_currentPage = Page();

	_removeDuplicates();
}

void EditorPageHistory::openDialog(Chip *chip, unsigned pageIdx)
{
	Page p = Page(chip, pageIdx);

	if (!_currentPage.isEqual(p))
		_newPageOpened(p);
}

void EditorPageHistory::openClass(ClassExt *clazz, Chip *chip, unsigned folder, float x, float y, float zoom, bool cameraMoved)
{
	Page p = Page(clazz, chip, folder, x, y, zoom, cameraMoved);

	if (!_currentPage.isEqual(p))
		_newPageOpened(p);
}

void EditorPageHistory::updateCurrentClassPage(float x, float y, float zoom, unsigned folder)
{
	if (_currentPage.type == EPT_CLASS) {
		_currentPage.classView.x = x;
		_currentPage.classView.y = y;
		_currentPage.classView.zoom = zoom;
		_currentPage.classView.folder = folder;
	}
}

void EditorPageHistory::onFolderRemoved(ClassExt *clazz, unsigned folderID)
{
	// Remove the folder from the history, if there is no chip set that could override it!
	auto f = [clazz, folderID](QList<Page> &lst) 
	{
		for (QList<Page>::iterator itr = lst.begin(); itr != lst.end();) {
			if (itr->type == EPT_CLASS && itr->classView.clazz == clazz && itr->classView.folder == folderID) {
				if (itr->classView.chip == nullptr)
					itr = lst.erase(itr);
				else
					(itr++)->classView.folder = InvalidFolderID; // Mark the folder as invalid. The chip will override it anyway.
			}
			else
				itr++;
		}
	};

	f(_backPages);
	f(_fwdPages);

	// Note: _currentPageVisit may still point to an invalid folder..
	if (_currentPage.type == EPT_CLASS && _currentPage.classView.clazz == clazz && _currentPage.classView.folder == folderID) {
		// The same will be set in the ClassView when the active folder is removed.
		_currentPage.classView.folder = MainFolderID;
		_currentPage.classView.x = 0.0f;
		_currentPage.classView.y = 0.0f;
	}

	_removeDuplicates();
}

void EditorPageHistory::onClassRemoved(ClassExt *c)
{
	auto f = [c](QList<Page> &lst) 
	{
		for (QList<Page>::iterator itr = lst.begin(); itr != lst.end();) {
			if (itr->type == EPT_CLASS && itr->classView.clazz == c)
				itr = lst.erase(itr);
			else
				itr++;
		}
	};

	f(_backPages);
	f(_fwdPages);

	if (_currentPage.type == EPT_CLASS && _currentPage.classView.clazz == c)
		_currentPage = Page(); // Set blank page!

	_removeDuplicates();
}

void EditorPageHistory::OnDestruction(DestructionObservable *observable)
{
	Chip *c = dynamic_cast<Chip*>(observable);
	if (!c)
		return;

	removeDialog(c);

	auto f = [c](QList<Page> &lst)
	{
		for (QList<Page>::iterator itr = lst.begin(); itr != lst.end();) {
			if (itr->type == EPT_CHIP && itr->chipDialog.chip == c)
				itr = lst.erase(itr);
			else if (itr->type == EPT_CLASS && itr->classView.chip == c) {
				itr->classView.chip = nullptr;
				if (itr->classView.folder == InvalidFolderID)
					itr = lst.erase(itr);
				else
					itr++;
			}
			else
				itr++;
		}
	};

	f(_backPages);
	f(_fwdPages);

	if (_currentPage.type == EPT_CLASS && _currentPage.classView.chip == c)
		_currentPage.classView.chip = nullptr;
	if (_currentPage.type == EPT_CHIP && _currentPage.chipDialog.chip == c)
		_currentPage = Page();

	_removeDuplicates();
}

void EditorPageHistory::_newPageOpened(const Page &p)
{
	if (_currentPage.type != EPT_EMPTY) {
		_backPages.push_back(_currentPage);
		while (_backPages.size() > 50)
			_backPages.pop_back();
	}
	_currentPage = p;
	_fwdPages.clear();
	_removeDuplicates();
	if (p.type == EPT_CLASS && p.classView.chip != nullptr)
		p.classView.chip->AddDestructionObserver(this);
	else if (p.type == EPT_CHIP)
		p.chipDialog.chip->AddDestructionObserver(this);
}

void EditorPageHistory::_removeDuplicates()
{
	auto f = [](QList<Page> &lst, Page &p)
	{
		for (QList<Page>::iterator itr = lst.begin(); itr != lst.end();) {
			if (p.isEqual(*itr))
				itr = lst.erase(itr);
			else
				p = *(itr++);
		}
	};

	Page p = _currentPage;
	f(_backPages, p);

	p = _currentPage;
	f(_fwdPages, p);
}