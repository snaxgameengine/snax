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
#include "ClassChip.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ClassManager.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/Class.h"
#include "M3DEngine/DocumentManager.h"
#include "M3DEngine/Document.h"
#include "M3DEngine/Environment.h"

using namespace m3d;


CHIPDESCV1_DEF(ClassChip, MTEXT("Class"), CLASSCHIP_GUID, CHIP_GUID);


ClassChip::ClassChip() : _preload(true), _clazzGlobalID(NullGUID), _clazz(nullptr)
{
}

ClassChip::~ClassChip()
{
	if (_clazz)
		_clazz->UnregisterClassChip(this);
}

bool ClassChip::CopyChip(Chip *chip)
{
	ClassChip *c = dynamic_cast<ClassChip*>(chip);
	B_RETURN(Chip::CopyChip(c));
	_preload = c->_preload;
	_filename = c->_filename;
	_clazzGlobalID = c->_clazzGlobalID;
	if (_clazz)
		_clazz->UnregisterClassChip(this);
	_clazz = nullptr; // _clazz is set when asked!
	return true;
}

bool ClassChip::LoadChip(DocumentLoader &loader)
{
	B_RETURN(Chip::LoadChip(loader));
	LOAD(MTEXT("classId|cg"), _clazzGlobalID);
	if (loader.GetDocumentVersion() >= Version(1, 2, 6, 0)) { // TODO: Remove!
		LOAD(MTEXT("preload"), _preload);
	}
	else {
		LOAD(MTEXT("preload"), (uint32&)_preload);
	}
	String filenameStr;
	LOAD(MTEXT("filename"), filenameStr);
	if (!filenameStr.empty())
		_filename = loader.GetEnvironment()->ResolveDocumentPath(filenameStr, GetClass()->GetDocument()->GetFileName());
	if (_clazz)
		_clazz->UnregisterClassChip(this);
	_clazz = nullptr; // _clazz is set when asked!
	if (_filename.IsFile() && _preload && loader.IsLoadRelatedDocumentsAsync())
	{
		Path fn = engine->GetDocumentManager()->PreloadDocument(_filename, &_clazzGlobalID);
		if (fn.IsValid() && fn != _filename && GetClass()) { // The file we are actually loading is different from the one we expected?
			msg(WARN, strUtils::ConstructString(MTEXT("Class-Chip in class \'%1\' (%4) tried find class \'%3\' from \'%2\', but found it in \'%5\'. Please resave!")).arg(GetClass()->GetName()).arg(_filename.AsString()).arg(GuidToString(_clazzGlobalID)).arg(GetClass()->GetDocument()->GetFileName().GetName()).arg(fn.AsString()));
			_filename = fn;
			GetClass()->SetDirty(); // Mark class as dirty so we can encourage user to save it with the updated filename!
		}
	}
	
	return true;
}

bool ClassChip::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(Chip::SaveChip(saver));
	SAVE(MTEXT("classId"), _clazzGlobalID);
	SAVE(MTEXT("preload"), _preload);
	Path p;
	if (_clazz)
		p = _clazz->GetDocument()->GetFileName();
	else
		p = _filename;
	String filenameStr;
	if (p.IsValid())
		filenameStr = saver.GetEnvironment()->CreateDocumentPath(p, GetClass()->GetDocument()->GetFileName());
	SAVE(MTEXT("filename"), filenameStr);
	return true;
}

Class *ClassChip::GetCG()
{
	if (_clazz)
		return _clazz;

	if (_clazzGlobalID != NullGUID) {
		//ClearError();
		_clazz = engine->GetClassManager()->GetClass(_clazzGlobalID);
		if (!_clazz) {
//			if (_autoLoad != NOAUTOLOAD) {
				if (_filename.IsFile()) {
					engine->GetDocumentManager()->GetDocument(_filename);
					_clazz = engine->GetClassManager()->GetClass(_clazzGlobalID);
				}
//			}
		}
	}
	else
		AddMessage(UninitializedException());

	if (_clazz)
		_clazz->RegisterClassChip(this);
	return _clazz;
}

void ClassChip::SetCG(Class *cg)
{
	if (_clazz == cg)
		return;
	if (_clazz)
		_clazz->UnregisterClassChip(this);
	_clazz = cg;
	if (_clazz)
		_clazz->RegisterClassChip(this);
	_clazzGlobalID = _clazz ? _clazz->GetGuid() : NullGUID;
	_filename = Path();
}
