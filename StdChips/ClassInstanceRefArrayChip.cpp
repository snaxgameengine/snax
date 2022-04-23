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
#include "ClassInstanceRefArrayChip.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"
#include "M3DEngine/ClassInstance.h"
#include "Value.h"
#include "Text.h"
#include "InstanceData.h"
#include "M3DEngine/Engine.h"
#include "M3DEngine/ChipManager.h"
#include <algorithm>

using namespace m3d;


CHIPDESCV1_DEF(ClassInstanceRefArrayChip, MTEXT("Instance Ref Array"), CLASSINSTANCEREFARRAYCHIP_GUID, ARRAYCHIP_GUID);


ClassInstanceRefArrayChip::ClassInstanceRefArrayChip()
{
}

ClassInstanceRefArrayChip::~ClassInstanceRefArrayChip()
{
	_clear();
}

void ClassInstanceRefArrayChip::_clear()
{
	for (size_t i = 0; i < _array.size(); i++)
		if (_array[i].IsOwner())
			_array[i]->Release();
	_array.clear();
}

bool ClassInstanceRefArrayChip::CopyChip(Chip *chip)
{
	_clear();

	ClassInstanceRefArrayChip *c = dynamic_cast<ClassInstanceRefArrayChip*>(chip);
	B_RETURN(ArrayChip::CopyChip(c));

	_preload = c->_preload;

	if (IsSaveContent()) {
		_array.resize(c->_array.size());
		for (size_t i = 0; i < c->_array.size(); i++) {
			if (c->_array[i].IsOwner())
				_array[i] = c->_array[i]->Clone(this);
			else
				_array[i] = c->_array[i];
		}
	}

	return true;
}

bool ClassInstanceRefArrayChip::LoadChip(DocumentLoader &loader)
{
	_clear();
	B_RETURN(ArrayChip::LoadChip(loader));
	if (loader.GetDocumentVersion() >= Version(1,2,1,0)) // TODO: Remove!
		LOAD(MTEXT("preload"), _preload);
	if (IsSaveContent())
		LOAD(MTEXT("array"), _array);
	return true;
}

bool ClassInstanceRefArrayChip::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(ArrayChip::SaveChip(saver));
	SAVE(MTEXT("preload"), _preload);
	if (IsSaveContent())
		SAVE(MTEXT("array"), _array);
	return true;
}

void ClassInstanceRefArrayChip::OnDestroyDevice()
{
	for (size_t i = 0; i < _array.size(); i++)
		if (_array[i].IsOwner())
			_array[i]->OnDestroyDevice();
}

void ClassInstanceRefArrayChip::OnReleasingBackBuffer(RenderWindow *rw)
{
	for (size_t i = 0; i < _array.size(); i++)
		if (_array[i].IsOwner())
			_array[i]->OnReleasingBackBuffer(rw);
}

void ClassInstanceRefArrayChip::RestoreChip()
{
	for (size_t i = 0; i < _array.size(); i++) {
		_array[i].Prepare(this);
		if (_array[i].IsOwner())
			_array[i]->RestoreChip();
	}
}

void ClassInstanceRefArrayChip::AddDependencies(ProjectDependencies &deps)
{
	for (size_t i = 0; i < _array.size(); i++)
		if (_array[i].IsOwner())
			_array[i]->AddDependencies(deps);
}

Chip *ClassInstanceRefArrayChip::FindChip(ChipID chipID)
{
	Chip *c = Chip::FindChip(chipID);
	if (c)
		return c;
	for (size_t i = 0; i < _array.size(); i++)
		if (_array[i].IsOwner() && (c = _array[i]->FindChip(chipID)))
			break;
	return c;
}

uint32 ClassInstanceRefArrayChip::GetContainerSize() 
{
	return (uint32)_array.size(); 
}

void ClassInstanceRefArrayChip::SetContainerSize(uint32 size) 
{ 
	if (size == 0)
		return _clear();
	if (size < _array.size()) {
		for (size_t i = size; i < _array.size(); i++) {
			if (_array[i].IsOwner()) {
				_array[i]->Release();
			}
		}
	}
	_array.resize(size); 
}

void ClassInstanceRefArrayChip::ClearContainer() 
{ 
	_clear();
}

ClassInstanceRef ClassInstanceRefArrayChip::GetInstance(uint32 index)
{
	if (index < _array.size())
		return _array[index];
	return ClassInstanceRef();
}

void ClassInstanceRefArrayChip::CreateInstance(uint32 index, Class *instanceOf)
{
	if (index >= _array.size())
		return;
	if (_array[index].IsOwner())
		_array[index]->Release();
	_array[index] = ClassInstance::Create(instanceOf, this);
}

void ClassInstanceRefArrayChip::SetInstance(uint32 index, ClassInstanceRef ref, AssignType at)
{
	if (index >= _array.size())
		return;

	switch (at)
	{
	case AT_REF:
		if (_array[index] != ref) {
			ClearInstance(index);
			_array[index] = ClassInstanceRef(ref, false);
		}
		break;
	case AT_MOVE:
		if (_array[index] != ref) {
			ClearInstance(index);
			_array[index] = ClassInstanceRef(ref, false);
		}
		if (_array[index]) {
			_array[index].SetOwner(true);
			_array[index]->SetOwner(this);
		}
		break;
	case AT_COPY:
		if (ref)
			ref = ref->Clone(this);
		ClearInstance(index);
		_array[index] = ref;
		break;
	}
}

void ClassInstanceRefArrayChip::ClearInstance(uint32 index)
{
	if (index >= _array.size())
		return;
	if (_array[index].IsOwner())
		_array[index]->Release();
	else
		_array[index].Reset();
}

void ClassInstanceRefArrayChip::MakeOwner(uint32 index)
{
	if (index >= _array.size())
		return;
	if (!_array[index])
		return;
	if (!_array[index].IsOwner()) {
		_array[index]->SetOwner(this);
		_array[index].SetOwner(true);
	}
}

void ClassInstanceRefArrayChip::AddElements(uint32 count, Class *instanceOf)
{
	SetContainerSize((uint32)_array.size() + count);
	if (instanceOf) {
		for (size_t i = _array.size() - count; i < _array.size(); i++)
			CreateInstance((uint32)i, instanceOf);
	}
}

void ClassInstanceRefArrayChip::InsertElements(uint32 at, uint32 count, Class *instanceOf)
{
	if (at > _array.size())
		return;
	_array.insert(_array.begin() + at, count, ClassInstanceRef());
	if (instanceOf) {
		for (size_t i = at; i < at + count; i++)
			CreateInstance((uint32)i, instanceOf);
	}
}

void ClassInstanceRefArrayChip::RemoveElements(uint32 at, uint32 count)
{
	if (at >= _array.size())
		return;
	count = (uint32)std::min(_array.size() - at, (size_t)count);
	for (size_t i = at; i < at + count; i++)
		if (_array[i].IsOwner())
			_array[i]->Release();
	_array.erase(_array.begin() + at, _array.begin() + at + count - 1);
}

void ClassInstanceRefArrayChip::ClearElements(uint32 at, uint32 count)
{
	if (at >= _array.size())
		return;
	count = (uint32)std::min(_array.size() - at, (size_t)count);
	for (size_t i = at; i < at + count; i++) {
		if (_array[i].IsOwner())
			_array[i]->Release();
		else
			_array[i].Reset();
	}
}

void ClassInstanceRefArrayChip::MoveElements(uint32 index, uint32 count, int npos)
{
	if (npos == 0)
		return;
	if (npos < 0 && !(index + count <= _array.size() && index >= (size_t)-npos))
		return;
	if (npos > 0 && !(index + count + (size_t)npos <= _array.size()))
		return;

	if (npos < 0)
	{
		std::rotate(_array.begin() + index - npos, _array.begin() + index, _array.begin() + index + count);
	}
	else
	{
		std::rotate(_array.rbegin() + index - npos, _array.rbegin() + index, _array.rbegin() + index + count);
	}
}

struct ClassInstanceRefArrayCmp 
{
	typedef bool(*Func)(const Chip *a, const Chip *b);
	struct Key
	{
		InstanceData *d;
		Func f;
	};
	List<Key> keys;

	bool operator()(const ClassInstanceRef &a, const ClassInstanceRef &b)
	{   
		if (a && b) {
			for (size_t i = 0; i < keys.size(); i++) {
				const Key &k = keys[i];
				Chip *ca = a->GetData(k.d), *cb = b->GetData(k.d);
				if (ca && cb) {
					if ((*k.f)(ca, cb))
						return true;
					if ((*k.f)(cb, ca))
						return false;
				}
				else
					return ca != nullptr;
			}
			return a->GetRuntimeID() < b->GetRuntimeID();
		}
		return a;
	}

	static bool value_less(const Chip *a, const Chip *b) { return ((Value*)a)->GetChipValue() < ((Value*)b)->GetChipValue(); }
	static bool value_greater(const Chip *a, const Chip *b) { return ((Value*)b)->GetChipValue() < ((Value*)a)->GetChipValue(); }
	static bool text_less(const Chip *a, const Chip *b) { return ((Text*)a)->GetChipText() < ((Text*)b)->GetChipText(); }
	static bool text_greater(const Chip *a, const Chip *b) { return ((Text*)b)->GetChipText() < ((Text*)a)->GetChipText(); }
};

void ClassInstanceRefArrayChip::Sort(const List<InstanceData*> &keys, int dir, uint32 start, uint32 count)
{
	if (_array.empty() || start >= _array.size() - 1 || count == 0 || dir == 0)
		return;
	count = std::min(count, (uint32)_array.size() - start);
	ClassInstanceRefArrayCmp l;
	for (size_t i = 0; i < keys.size(); i++) {
		if (!keys[i])
			continue;
		ClassInstanceRefArrayCmp::Key k;
		k.d = keys[i];
		if (engine->GetChipManager()->IsChipTypeSupported(VALUE_GUID, keys[i]->GetChipType()))
			k.f = dir > 0 ? ClassInstanceRefArrayCmp::value_less : ClassInstanceRefArrayCmp::value_greater;
		else if (engine->GetChipManager()->IsChipTypeSupported(TEXT_GUID, keys[i]->GetChipType()))
			k.f = dir > 0 ? ClassInstanceRefArrayCmp::text_less : ClassInstanceRefArrayCmp::text_greater;
		else
			continue;
		l.keys.push_back(k);
	}
	if (l.keys.empty())
		return;
	std::sort(&_array[start], &_array[start + count - 1] + 1, l);
}

void ClassInstanceRefArrayChip::SetClass(Class* clazz)
{
	Chip::SetClass(clazz);
	for (size_t i = 0; i < _array.size(); i++) {
		auto &a = _array[i];
		if (a && a.IsOwner())
			a->SetOwner(this);
	}
}

