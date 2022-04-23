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
#include "Document.h"
#include "DocumentFileTypes.h"
#include "Engine.h"
#include "DocumentSaveLoadUtil.h"
#include "ClassManager.h"


using namespace m3d;


Document::Document(ClassFactory *factory) : _eventListener(nullptr), _factory(factory), _readOnly(false), _dirty(false), _startClass(nullptr), _recoverySaveTime(0)
{
}

Document::~Document()
{
	while (_clazzList.size() > 0)
		RemoveClass(_clazzList.back());
	assert(_clazzList.empty());
}

Class *Document::CreateClass()
{
	Class *cg = _factory->Create();
	if (!cg)
		return nullptr;
	_clazzList.push_back(cg);
	cg->SetDocument(this);
//	if (_eventListener)
//		_eventListener->ClassAdded(this, cg);
	return cg;
}

bool Document::MoveClass(Class *cg) 
{ 
	Document *oldDoc = cg->GetDocument();
	if (oldDoc) {
		if (oldDoc == this)
			return true; // No change
		if (!oldDoc->_removeClass(cg))
			return false; // could not be removed from old doc.
	}
	_clazzList.push_back(cg);
	cg->SetDocument(this);
	if (_eventListener)
		_eventListener->ClassMoved(cg, oldDoc, this);
	return true; 
}

bool Document::RemoveClass(Class *cg)
{
	if (!_removeClass(cg))
		return false;
	cg->Clear();
	mmdelete(cg);
	return true;
}

bool Document::_removeClass(Class *cg)
{
	size_t i = 0;
	for (; i < _clazzList.size(); i++) {
		if (_clazzList[i] == cg)
			break;
	}
	if (i == _clazzList.size())
		return false; // Not found
	_clazzList.erase(_clazzList.begin() + i);
//	if (_eventListener)
//		_eventListener->ClassRemoved(this, cg);
	return true;
}

void Document::SetDirty(bool dirty)
{
	if (dirty == false) { // If not dirty, mark classes as well!
		for (size_t i = 0; i < _clazzList.size(); i++)
			if (_clazzList[i]->IsDirty())
				_clazzList[i]->SetDirty(false);
	}
	if (dirty && _recoverySaveTime == 0) {
		_recoverySaveTime = engine->GetClockTime(); // Mark a timestamp, and after N seconds we will do a recovery save!
	}
	if (_dirty == dirty)
		return;
	_dirty = dirty;
	if (_eventListener)
		_eventListener->DirtyFlagChanged(this);
}

void Document::SetFileName(Path filename)
{
	if (filename == _filename)
		return;
	_filename = filename;
}

bool Document::SetStartClass(Class *cg)
{
	if (cg->GetDocument() != this)
		return false;
	_startClass = cg;
	return true;
}
