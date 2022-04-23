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
#include "M3DCore/Containers.h"
#include "M3DEngine/DocumentFileTypes.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/Environment.h"
#include <qmessagebox.h>


namespace m3d
{

class ClassExt;


static void _enumerateDirty(List<Document*> &dirtyList)
{
	dirtyList.clear();
	const DocumentPtrSet &m = engine->GetDocumentManager()->GetDocuments();
	for (const auto &n : m)
		if (n->IsDirty())
			dirtyList.push_back(n);
}

static bool _save(Document *doc)
{
	ScopedOverrideCursor oc(Qt::WaitCursor);
	bool b = engine->GetDocumentManager()->SaveDocument(doc);
	if (!b)
		return false;
//	doc->SetDirty(false);
	return true;
}

static bool _saveAs(Document *doc)
{
	ScopedOverrideCursor oc(Qt::WaitCursor);
	bool b = engine->GetDocumentManager()->SaveDocumentAs(doc);
	if (!b)
		return false;
//	doc->SetDirty(false);
	return true;
}

static bool _saveThese(List<Document*>& toSave)
{
	Set<Document*> libDocs;
	for (unsigned i = 0; i < toSave.size(); i++) {
		Document* doc = toSave[i];
		Path p = doc->GetFileName();
		if (p.IsValid() && !engine->GetEnvironment()->IsPathInsideProjectRootFolder(p) && engine->GetEnvironment()->IsPathInsideLibraryFolder(p)) {
			if (!doc->IsAllowLibraryUpdate())
				libDocs.insert(doc);
		}
	}
	if (!libDocs.empty()) {
		bool allConfirmed = false;
		for (const auto &n : libDocs) {
			Document* doc = n;
			if (!allConfirmed) {
				int r = QMessageBox::question((QWidget*)nullptr, "Save Library Document", QString("Do you really want to save the library document \'%1\'?").arg(TOQSTRING(doc->GetFileName().AsString())), (QMessageBox::StandardButtons)((libDocs.size() > 1 ? (QMessageBox::YesToAll | QMessageBox::NoToAll) : QMessageBox::NoButton) | QMessageBox::Yes | QMessageBox::No), QMessageBox::No);
				if (r == QMessageBox::YesToAll)
					allConfirmed = true;
				else if (r == QMessageBox::NoToAll)
					break;
				if (r != QMessageBox::No) {
					doc->SetAllowLibraryUpdate(true);
				}
			}
			else
				doc->SetAllowLibraryUpdate(true);
		}
	}
		
	for (unsigned i = 0; i < toSave.size(); i++) {
		auto n = libDocs.find(toSave[i]);
		if (n != libDocs.end() && !(*n)->IsAllowLibraryUpdate())
			continue;
		if (!_save(toSave[i]))
			return false; // Cancel operation!
	}
	return true;
}


static bool _saveDirty()
{
	List<Document*> dirtyList;
	_enumerateDirty(dirtyList);
	return _saveThese(dirtyList);
}



static QString _getFilterFromFileType(QString ext)
{
	for (unsigned i = 0; i < (uint32)DocumentFileTypes::FileType::FILE_TYPE_COUNT; i++) {
		QString e = TOQSTRING(DocumentFileTypes::FILE_TYPES[i].ext);
		if (e.compare(ext, Qt::CaseInsensitive) == 0)
			return TOQSTRING(DocumentFileTypes::FILE_TYPES[i].desc) + " (*." + e + ")";
	}
	return QString();
}

static QString _getFilter()
{
	QString r;
	for (unsigned i = 0; i < (uint32)DocumentFileTypes::FileType::FILE_TYPE_COUNT; i++) {
		if (i > 0)
			r += ";;";
		r += TOQSTRING(DocumentFileTypes::FILE_TYPES[i].desc) + " (*." + TOQSTRING(DocumentFileTypes::FILE_TYPES[i].ext) + ")";
	}
	return r;
}

static QString _getFilterCommon()
{
	QString r = "SnaX Documents (*.";
	for (unsigned i = 0; i < (uint32)DocumentFileTypes::FileType::FILE_TYPE_COUNT; i++) {
		if (i > 0)
			r += " *.";
		r += TOQSTRING(DocumentFileTypes::FILE_TYPES[i].ext);
	}
	return r + ")";
}


}