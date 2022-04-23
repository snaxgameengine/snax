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
#include "EditorPageListWidget.h"
#include "M3DEngine/Chip.h"

namespace m3d
{

class ChipDialogContainer;

class EditorPageHistory : private DestructionObserver
{
public:
	struct Page
	{
		EditorPageType type;
		union
		{
			struct
			{
				ClassExt *clazz;
				Chip *chip; // The chip that got focus. Coords must also be set, but the chip's location overrides them.
				unsigned folder;
				float x;
				float y;
				float zoom;
				bool cameraMoved;
			} classView;
			struct
			{
				Chip *chip;
				unsigned activeTab;
			} chipDialog;
		};

		Page() : type(EPT_EMPTY), classView({ nullptr, nullptr, 0, 0.0f, 0.0f, 0.0f, false }) { }
		Page(ClassExt *clazz, Chip *chip, unsigned folder, float x, float y, float zoom, bool cameraMoved) : type(EPT_CLASS), classView({ clazz, chip, folder, x, y, zoom, cameraMoved }) { }
		Page(Chip *chip, unsigned activeTab) : type(EPT_CHIP), chipDialog({ chip, activeTab }) { }

		bool isEqual(const Page &pv) const
		{
			if (type != pv.type)
				return false;
			switch (type)
			{
			case EPT_CLASS:
				if (classView.clazz != pv.classView.clazz)
					return false;
				if (classView.chip != pv.classView.chip)
					return false;
				if (classView.chip == nullptr && (classView.folder != pv.classView.folder || classView.x != pv.classView.x || classView.y != pv.classView.y || classView.zoom != pv.classView.zoom))
					return false;
				break;
			case EPT_CHIP:
				if (chipDialog.chip != pv.chipDialog.chip)
					return false;
				if (chipDialog.activeTab != pv.chipDialog.activeTab)
					return false;
				break;
			}
			return true;
		}
	};


	EditorPageHistory();
	~EditorPageHistory();

	EditorPageHistory duplicate() const;

	const Page &getCurrentPage() const { return _currentPage; }
	const Page &goBackOrFwd(int n);
	void removeDialog(Chip *chip);
	void openDialog(Chip *chip, unsigned tab);
	void openClass(ClassExt *clazz, Chip *chip, unsigned folder, float x, float y, float zoom, bool cameraMoved);
	void updateCurrentClassPage(float x, float y, float zoom, unsigned folder);
	void onFolderRemoved(ClassExt *c, unsigned folderID);
	void onClassRemoved(ClassExt *c);

	bool canGoBack() const { return !_backPages.empty(); }
	bool canGoFwd() const { return !_fwdPages.empty(); }

private:
	QList<Page> _backPages;
	QList<Page> _fwdPages;
	Page _currentPage;

	void _newPageOpened(const Page &p);
	void _removeDuplicates();

	void OnDestruction(DestructionObservable *observable) override;


};


}
