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
#include "Chip.h"
#include "Engine.h"
#include "Class.h"
#include "ChipManager.h"
#include "DocumentLoader.h"
#include "DocumentSaver.h"
#include "Function.h"
#include "M3DCore/MemoryManager.h"
#include "StdChips/Shortcut.h"


using namespace m3d;


// Method calls using ChildPtr::-> should always be done as a single isolated statement.
// This is because the function stack does not reset directly after such a call, but after the full statement is complete
// and the destructor of ChildPtr::Call is executed. 
// This is quite a limitation, but there seems to be little I can do about it at the moment. There is no
// way I can intersept and do some code DIRECTLY after a function returns. (_penter/_pexit is no alternative).
// Hopefully c++ comes up with a solution to this soon.
// It is only in the case where ChildPtr is a chip returned from a function or a parameter that this is a problem, but since you
// at compile time does not know when this will be, you should always follow these rules. Else you may run into problems that may be hard
// to locate.


const ChipDesc Chip::DESC = {MTEXT("Chip"), CHIP_GUID, CHIP_GUID, ChipDesc::STANDARD, VERSION1, MTEXT("CHIP_FACTORY")};


std::atomic<ChipID> ids = 0;

#pragma warning( push )
#pragma warning( disable : 4355 ) // this in parameter list warning.
Chip::Chip() : _id(++ids), _clazz(nullptr), _owner(this), _editorData(nullptr), _typeIndex(InvalidChipTypeIndex), _lastHit(0), _function(nullptr), _childProvider(this), _updateStamp(0), _messages(nullptr)
{
	GenerateGuid(_globalID);
}
#pragma warning( pop )

Chip::~Chip() 
{
	if (_function)
		mmdelete(_function);
	_clearConnections();
}

void Chip::Release() const
{
	Chip *r = const_cast<Chip*>(this);
	r->_clearMessages();
	r->OnRelease();
	r->NotifyDestuction();
	mmdelete(this);
}
/*
void Chip::SetChildProvider(const Chip *chip)
{
	_childProvider = chip == 0 ? this : chip;
}
*/
void Chip::_removeChild(Chip *child)
{
	const ChildConnectionList &ccl = GetChildren();
	for (uint32 i = 0; i < ccl.size(); i++) {
		ChildConnection *cc = ccl[i];
		if (cc) {
			for (uint32 j = 0; j < cc->connections.size(); j++) {
				if (cc->connections[j].chip == child)
					_setChild(nullptr, i, j);
			}
		}
	}
}

bool Chip::_setChild(Chip *child, uint32 index, uint32 subIndex, bool insert)
{
	const ChildConnectionList &ccl = GetChildren();
	if (child == this || child && child->AsShortcut() && child->AsShortcut()->GetOriginal() == this) 
		return false; // Cannot link to ourself, directly or through a shortcut!
	if (index >= ccl.size())
		return false; // Invalid index
	ChildConnection *cc = ccl[index];
	if (!cc)
		return false; // No connection at index
	if (subIndex > 0 && cc->desc.connType != ChildConnectionDesc::GROWING)
		return false; // not growing connection!
	if (child && !engine->GetChipManager()->IsChipTypeSupported(cc->chipTypeIndex, child->GetChipTypeIndex())) // check type!
		return false; // type not supported at this connection
	if (insert && cc->desc.connType == ChildConnectionDesc::GROWING) {
		if (subIndex > cc->connections.size())
			cc->connections.resize(subIndex);
		cc->connections.insert(cc->connections.begin() + subIndex, child);
	}
	else {
		if (subIndex >= cc->connections.size())
			cc->connections.resize(subIndex + 1);
		cc->connections[subIndex] = child;
	}
	return true;
}

void Chip::_clearConnections(uint32 fromIndex)
{
	// Note: This function accesses _children directly, not through the provider!
	for (uint32 i = fromIndex; i < _children.size(); i++)
		_clearConnection(i);
	if (fromIndex == 0)
		_children.clear();
	else
		_children.resize(fromIndex);

	for (size_t i = 0; i < _connectionClones.size(); i++)
		_connectionClones[i]->_clearConnections(fromIndex);
}

void Chip::_clearConnection(uint32 index)
{
	// Note: This function accesses _children directly, not through the provider!
	if (index >= _children.size())
		return;
	ChildConnection *child = _children[index];
	if (child) {
		mmdelete(child);
		_children[index] = 0;
	}

	for (size_t i = 0; i < _connectionClones.size(); i++)
		_connectionClones[i]->_clearConnection(index);
}

void Chip::_setConnection(uint32 index, const ChildConnectionDesc &connection, bool keepChildren)
{
	// Note: This function accesses _children directly, not through the provider!
	if (index >= _children.size())
		_children.resize(index + 1);
	SubConnectionList subConns;
	if (keepChildren) {
		ChildConnection *lst = _children[index];
		if (lst)
			subConns = lst->connections;
	}
	_clearConnection(index);
	_children[index] = mmnew ChildConnection(connection, engine->GetChipManager()->GetChipTypeIndex(connection.type));

	for (uint32 i = 0; i < subConns.size(); i++)
		_setChild(subConns[i].chip, index, i, false);

	for (size_t i = 0; i < _connectionClones.size(); i++)
		_connectionClones[i]->_setConnection(index, connection, keepChildren);
}

void Chip::AddConnectionClone(Chip *chip)
{
	_connectionClones.push_back(chip);
}

void Chip::RemoveConnectionClone(Chip *chip)
{
	for (size_t i = 0; i < _connectionClones.size(); i++) {
		if (_connectionClones[i] == chip) {
			_connectionClones[i] = _connectionClones[_connectionClones.size() - 1];
			_connectionClones.pop_back();
			break;
		}
	}
}


ChipTypeIndex Chip::GetChipTypeIndex() const 
{ 
	if (_typeIndex == -1)
		_typeIndex = engine->GetChipManager()->GetChipTypeIndex(GetChipType());
	return _typeIndex; 
}

void Chip::SetConnection(uint32 index, const ChildConnectionDesc &connection, bool keepChildren)
{
	return _setConnection(index, connection, keepChildren);
}

bool Chip::SetChild(Chip *child, uint32 index, uint32 subIndex)
{
	return _setChild(child, index, subIndex, false);
}

bool Chip::InsertChild(Chip *child, uint32 index, uint32 subIndex)
{
	return _setChild(child, index, subIndex, true);
}

bool Chip::MoveChild(uint32 index, uint32 fromSubIndex, uint32 toSubIndex)
{
	const ChildConnectionList &ccl = GetChildren();
	if (index >= ccl.size())
		return false;
	ChildConnection *cc = ccl[index];
	if (!cc)
		return false;
	if (cc->desc.connType != ChildConnectionDesc::GROWING)
		return false; // not growing connection!
	if (fromSubIndex >= cc->connections.size() || toSubIndex > cc->connections.size())
		return false; // Invalid subindices!
	Chip *ss = cc->connections[fromSubIndex].chip;
	cc->connections.erase(cc->connections.begin() + fromSubIndex);
	if (toSubIndex > cc->connections.size()) 
		cc->connections.push_back(ss);
	else
		cc->connections.insert(cc->connections.begin() + toSubIndex, ss);
	return true;
}

void Chip::ClearConnections(uint32 fromIndex)
{
	return _clearConnections(fromIndex);
}

uint32 Chip::GetConnectionCount() const 
{ 
	return (uint32)GetChildren().size(); 
}

uint32 Chip::GetSubConnectionCount(uint32 index) const 
{ 
	const ChildConnectionList &ccl = GetChildren();
	if (index < ccl.size() && ccl[index])
	{
		if (ccl[index]->desc.connType != ChildConnectionDesc::MULTI)
		{
			return (uint32)ccl[index]->connections.size();
		}
		else 
		{
			Chip* chip = GetRawChild(index, 0);
			if (chip)
				return chip->GetMultiConnectionChildCount();
		}
	}
	return 0;
}

ChipChildPtr Chip::GetChild(uint32 index, uint32 subIndex) const
{
	// To prevent infinit loops within a function we check the function stack.
	// The limit must be set high enough for normal operations to pass through, but low enough to not freeze the app. The limit can be adjusted by the user.
	if (functionStack.CanIncrementRecordRef()) {
		//Chip *chip = GetRawChild(index, subIndex);
		//if (chip)
		//	return chip->GetChip();

		const ChildConnectionList& ccl = GetChildren();
		if (index >= ccl.size())
			return nullptr;
		ChildConnection* cc = ccl[index];
		if (!cc)
			return nullptr;
		if (cc->desc.connType != ChildConnectionDesc::MULTI) {
			if (subIndex >= cc->connections.size())
				return nullptr;
			SubConnection& sc = cc->connections[subIndex];
			sc.lastHit = engine->GetFrameTime();
			if (sc.chip) {
				sc.chip->_lastHit = sc.lastHit;
				return sc.chip->GetChip();
			}
		}
		else {
			SubConnection& sc = cc->connections[0];
			sc.lastHit = engine->GetFrameTime();
			if (sc.chip) {
				sc.chip->_lastHit = sc.lastHit;
				return sc.chip->GetMultiConnectionChip(subIndex);
			}
		}
	}
	else {
		Chip *c = const_cast<Chip*>(this); // Silly trix.. 
		if (functionStack.IsRunning())
			c->AddMessage(InfiniteLoopException());
		else
			c->AddMessage(GetChildNotAllowedException());
	}
	return ChipChildPtr();
}

Chip *Chip::GetRawChild(uint32 index, uint32 subIndex) const
{
	const ChildConnectionList &ccl = GetChildren();
	if (index >= ccl.size())
		return nullptr;
	ChildConnection *cc = ccl[index];
	if (!cc)
		return nullptr;
	if (subIndex >= cc->connections.size())
		return nullptr;
	SubConnection &sc = cc->connections[subIndex];
	sc.lastHit = engine->GetFrameTime();
	if (sc.chip)
		sc.chip->_lastHit = sc.lastHit;
	return sc.chip; 
	// (if implementing run-time const)
//	Chip *ch = cc->connections[subIndex];
//	if (ch->IsConst() && cc->desc.dataDirection != UP)
//		ch = nullptr;
//	return ch;
}

void Chip::RemoveEmptyConnections()
{
	const ChildConnectionList &ccl = GetChildren();
	for (uint32 i = 0; i < ccl.size(); i++) {
		ChildConnection *cc = ccl[i];
		if (!cc)
			continue;
		SubConnectionList cl = cc->connections;
		cc->connections.clear();
		for (uint32 j = 0; j < cl.size(); j++) {
			if (!cl[j].chip)
				continue;
			cc->connections.push_back(cl[j]);
		}
	}
}

void Chip::RemoveChild(Chip *child)
{
	_removeChild(child);
}

ChipChildPtr Chip::GetChip() 
{ 
	//Touch();
	return ChipChildPtr(this); 
}

void Chip::Run()
{
	Touch();
	ChipChildPtr ch = GetChip();
	if (ch)
		ch->CallChip();
}

bool Chip::InitChip()
{
	if (_name.length() == 0) // Don't override name set explicitly by contructor!
		SetName(GetChipDesc().name);
	return true;
}

bool Chip::CopyChip(Chip *chip)
{
	if (!chip)
		return false;
	SetName(chip->GetName());
	Refresh.SetRefreshMode(chip->Refresh.GetRefreshMode());
	return true;
}

void Chip::Touch()
{
	_lastHit = engine->GetFrameTime();
}

uint32 Chip::GetMessageHitCount(const ChipMessage &msg) const
{
	if (_messages) {
		for (size_t i = 0; i < _messages->size(); i++)
			if (_messages->at(i).msg == _messages->at(i).msg)
				return _messages->at(i).hitCount;
	}
	return 0;
}

void Chip::AddMessage(const ChipMessage &msg)
{
	if (!_messages)
		_messages = mmnew ChipMessageList();
	for (size_t i = 0; i < _messages->size(); i++) {
		if (_messages->at(i).msg == msg.msg) {
			_messages->at(i).hitCount++;
			return;
		}
	}
	_messages->push_back(msg);
	_messages->back().hitCount = 1;
	engine->ChipMessageAdded(this, _messages->back());
}

void Chip::RemoveMessage(const ChipMessage &msg)
{
	if (_messages) {
		for (size_t i = 0; i < _messages->size(); i++) {
			if (_messages->at(i).msg == msg.msg) {
				engine->ChipMessageRemoved(this, _messages->at(i));
				if (i < _messages->size() - 1)
					_messages->at(i) = _messages->back();
				_messages->pop_back();
				break;
			}
		}
		if (_messages->empty()) {
			mmdelete(_messages);
			_messages = nullptr;
		}
	}
}

void Chip::ClearMessages()
{
	_clearMessages();
}

void Chip::_clearMessages()
{
	if (_messages) {
		while (_messages->size()) {
			engine->ChipMessageRemoved(this, _messages->back());
			_messages->pop_back();
		}
		mmdelete(_messages);
		_messages = nullptr;
	}
}

void Chip::AddException(const ChipException &exp)
{
	if (exp.chip) {
		if (exp.chip != this) {
			exp.chip->AddMessage(ChipMessage(exp));
			AddMessage(ChipMessage(MTEXT("ExceptionCaught"), strUtils::ConstructString(MTEXT("An exception thrown from a Chip named \'%1\' was caught. Please check the message log for details!")).arg(exp.chip->GetName()), WARN));
		}
		else
			AddMessage(ChipMessage(exp));
	}
	else {
		AddMessage(ChipMessage(exp));
		AddMessage(ChipMessage(MTEXT("AnonymousException"), MTEXT("An exception not linked to any Chip was caught. Please check the message log for details!"), WARN));
	}
}

bool Chip::CanCreateFunction() const
{
	return _clazz && _clazz->GetChip(GetID()) == this;
}

Function *Chip::CreateFunction()
{
	if (_function || !CanCreateFunction())
		return _function;
	return _function = mmnew Function(this);
}

void Chip::RemoveFunction()
{
	mmdelete(_function);
	_function = nullptr;
}

void Chip::SetName(String name) 
{ 
	if (_name == name)
		return; // No change!
	_name = name;
	if (_function)
		_function->Set(_name);
}

List<Shortcut*> Chip::GetShortcuts() const
{
	List<Shortcut*> r;
	const Set<DestructionObserver*> &o = GetDestructionObservers();
	for (DestructionObserver* n : o)
	{
		Shortcut* sc = dynamic_cast<Shortcut*>(n);
		if (sc)
			r.push_back(sc);
	}
	return r;
}

void Chip::SetClass(Class* clazz)
{ 
	_clazz = clazz; 
}

