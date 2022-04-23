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


#include "ClassExt.h"



namespace m3d
{
	/*
struct ChipEditorData2 : public ChipEditorData
{
	ChipEditorData2() : ChipEditorData(nullptr) {}
	ChipEditorData2(const ChipEditorData &rhs) : ChipEditorData(nullptr) { *this = rhs; }

	bool operator==(const ChipEditorData &rhs) const
	{
		return comment == rhs.comment && pos == rhs.pos;
	}

	ChipEditorData2 &operator=(const ChipEditorData &rhs)
	{
		comment = rhs.comment;
		pos = rhs.pos;
		return *this;
	}
};
*/
class ClassDiagram : public ClassExt
{
protected:
	static ClassDiagram *_singleton;

	// Got to have this struct to back up some chip data. I use this to see what is changing....
	struct RgExtra
	{
		String comment;
		Vector2 pos;
		Map<Shortcut*, Vector2> shortcuts;
		RgExtra() {}
		RgExtra(const Vector2 &p) : pos(p) {}
	};

	Map<ClassDiagramChip*, RgExtra> _mm;


public:
	ClassDiagram();
	~ClassDiagram();

	static void CreateSingleton();
	static void DestroySingleton();
	static ClassDiagram *Singleton() { return _singleton; }

	virtual bool CanAddFoldersAndBackgroundItems() const override { return false; }

	virtual Chip *AddChip(ChipTypeIndex chipGuid, bool initChip = true) override; // Only shortcuts!
	virtual bool RemoveChip(Chip *ch) override; // Only shortcuts!

	virtual void SetDirty(bool dirty) override;

	virtual ClassView *CreateView() override;
	virtual void DestroyView(ClassView *view) override;

	void OnClassAdded(Class *clazz);
	void OnClassRemoved(Class *clazz);

	void OnBaseClassAdded(Class *derived, Class *base);
	void OnBaseClassRemoved(Class *derived, Class *base);

	void SetConnectionMapping(Class *derived, const Map<Guid, std::pair<unsigned, Guid>> &baseClassMapping);

	virtual void OnShortcutSet(Shortcut *sc);

	virtual void UpdateRgChipData(Class *clazz);
};


}