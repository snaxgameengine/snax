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

#include "pch.h"
#include "ClassDiagram.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngine/ChipManager.h"
#include "ClassDiagramView.h"
#include "StdChips/ClassDiagramChip.h"
#include "StdChips/Shortcut.h"


using namespace m3d;



ClassDiagram *ClassDiagram::_singleton = nullptr;

void ClassDiagram::CreateSingleton()
{
	if (!_singleton)
		_singleton = mmnew ClassDiagram();
}

void ClassDiagram::DestroySingleton()
{
	if (_singleton) {
		_singleton->Clear();
		mmdelete(_singleton);
		_singleton = nullptr;
	}
}

ClassDiagram::ClassDiagram() : ClassExt()
{
	SetName(MTEXT("Class Diagram"));
}

ClassDiagram::~ClassDiagram()
{
}

Chip *ClassDiagram::AddChip(ChipTypeIndex chipGuid, bool initChip)
{
	if (engine->GetChipManager()->GetChipTypeIndex(SHORTCUT_GUID) != chipGuid)
		return nullptr;
	return ClassExt::AddChip(chipGuid, initChip);
}

bool ClassDiagram::RemoveChip(Chip *ch) 
{ 
	if (ch && ch->AsShortcut()) {
		ClassDiagramChip* cdc = dynamic_cast<ClassDiagramChip*>(ch->AsShortcut()->GetOriginal());
		auto n = _mm.find(cdc);
		if (n != _mm.end())
			n->second.shortcuts.erase(ch->AsShortcut());
		return ClassExt::RemoveChip(ch);
	}
	return false; 
}

void ClassDiagram::SetDirty(bool dirty)
{
	if (!dirty)
		return;

	// A change has been made in the relation graph. Mark affected classes dirty!

	for (auto &n : _mm) {
		ClassExt *cgi = (ClassExt*)n.first->GetCG();
		assert(cgi);
		if (n.second.pos != n.first->GetChipEditorData()->pos || n.second.comment != n.first->GetChipEditorData()->comment) {
			n.second.pos = n.first->GetChipEditorData()->pos;
			n.second.comment = n.first->GetChipEditorData()->comment;
			cgi->SetDirty();
		}
		for (auto &m : n.second.shortcuts) {
			if (m.first->GetChipEditorData()->pos != m.second) {
				m.second = m.first->GetChipEditorData()->pos;
				cgi->SetDirty();
			}
		}
	}
}

void ClassDiagram::OnShortcutSet(Shortcut *sc)
{
	ClassExt::OnShortcutSet(sc);
	ClassDiagramChip *ch = dynamic_cast<ClassDiagramChip*>(sc->GetOriginal());
	assert(ch);
	_mm.find(ch)->second.shortcuts.insert(std::make_pair(sc, Vector2(-1e10,-1e10))); // <== Note: SetDirty() is called when shortcut is inserted manually, updating the pos there.
}

void ClassDiagram::OnClassAdded(Class *cg)
{
	ClassExt *cge = dynamic_cast<ClassExt*>(cg);
	assert(cge);
	ClassDiagramChip *ch = (ClassDiagramChip*)ClassExt::AddChip(engine->GetChipManager()->GetChipTypeIndex(CLASSDIAGRAMCHIP_GUID));
	assert(ch);
	ch->SetCG(cge);
	ch->GetChipEditorData()->pos = Vector2(((float32)rand() / RAND_MAX - 0.5f) * 10.0f, ((float32)rand() / RAND_MAX - 0.5f) * 10.0f);
	cge->SetClassDiagramChip((ClassDiagramChip*)ch);
	_mm.insert(std::make_pair(ch, RgExtra(ch->GetChipEditorData()->pos)));
}

void ClassDiagram::OnClassRemoved(Class *cg)
{
	ClassExt *cge = dynamic_cast<ClassExt*>(cg);
	assert(cge);
	assert(engine->GetClassManager()->GetClass(cge->GetID()) == nullptr);
	Chip *ch = cge->GetClassDiagramChip();
	if (ch)
		ClassExt::RemoveChip(ch);
	_mm.erase((ClassDiagramChip*)ch);
	cge->SetClassDiagramChip((ClassDiagramChip*)nullptr);
}

void ClassDiagram::OnBaseClassAdded(Class *derived, Class *base)
{
	ClassExt *derivedE = dynamic_cast<ClassExt*>(derived);
	ClassExt *baseE = dynamic_cast<ClassExt*>(base);
	assert(derivedE && baseE);

	ClassDiagramChip *chBase = baseE->GetClassDiagramChip();
	ClassDiagramChip *chDerived = derivedE->GetClassDiagramChip();
	assert(chBase && chDerived);

	chDerived->OnBaseClassAdded(chBase);
}

void ClassDiagram::OnBaseClassRemoved(Class *derived, Class *base)
{
	ClassExt *derivedE = dynamic_cast<ClassExt*>(derived);
	ClassExt *baseE = dynamic_cast<ClassExt*>(base);
	assert(derivedE && baseE);

	ClassDiagramChip *chBase = baseE->GetClassDiagramChip();
	ClassDiagramChip *chDerived = derivedE->GetClassDiagramChip();
	assert(chBase && chDerived);

	chDerived->OnBaseClassRemoved(chBase);
}

void ClassDiagram::SetConnectionMapping(Class *derived, const Map<Guid, std::pair<unsigned, Guid>> &baseGraphMapping)
{
	ClassExt *cg = dynamic_cast<ClassExt*>(derived);

	assert(cg);

	ClassDiagramChip *ch = cg->GetClassDiagramChip();

	assert(ch);

	ch->UnlinkChildren(); // Remove default connections

	Set<Chip*> s;

	for (const auto &n : cg->GetBaseClasses()) { // Iterate all base graphs...
		ClassExt *base = dynamic_cast<ClassExt*>(n);
		assert(base);
		ClassDiagramChip *chBase = base->GetClassDiagramChip();
		assert(chBase);

		auto p = baseGraphMapping.find(base->GetGuid());
		if (p != baseGraphMapping.end()) { // base graph found in mapping?
			Chip *linkTo = chBase;
			if (chBase->GetGlobalID() != p->second.second) {
				List<Shortcut*> shortcuts = chBase->GetShortcuts();
				for (unsigned i = 0; i < shortcuts.size(); i++) {
					if (shortcuts[i]->GetGlobalID() == p->second.second) {
						linkTo = shortcuts[i];
						break;
					}
				}
			}
			if (ch->GetRawChild(p->second.first) == nullptr) // connection available?
				ch->OnBaseClassAdded(linkTo, p->second.first);
			else
				s.insert(linkTo); // The base chip is added later to an available connection!
		}
	}

	for (const auto &n : s) {
		ch->OnBaseClassAdded(n);
/*		for (unsigned i = 0; i < ch->GetSubConnectionCount(0) + 1; i++) {
			if (ch->GetRawChild(0, i) == nullptr) {
				bool b = ch->SetChild(n->get(), 0, i); // This should always return true!
				assert(b);
			}
		}
*/	}
}

void ClassDiagram::UpdateRgChipData(Class *cg)
{
	ClassExt *ext = (ClassExt*)cg;
	ClassDiagramChip *ch = ext->GetClassDiagramChip();
	assert(ch);
	auto n = _mm.find(ch);
	assert(n != _mm.end());
	n->second.pos = ch->GetChipEditorData()->pos;
	n->second.comment = ch->GetChipEditorData()->comment;
	for (auto &m : n->second.shortcuts) {
		m.second = m.first->GetChipEditorData()->pos;
	}
}

ClassView *ClassDiagram::CreateView()
{
	return mmnew ClassDiagramView(this);
}

void ClassDiagram::DestroyView(ClassView *view)
{
	mmdelete(view);
}
