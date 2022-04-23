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


#include <iterator>

template<typename T>
SetChip<T>::SetChip()
{
}

template<typename T>
SetChip<T>::~SetChip()
{
}

template<typename T>
bool SetChip<T>::CopyChip(Chip *chip)
{
	SetChip *c = dynamic_cast<SetChip*>(chip);
	B_RETURN(ContainerChip::CopyChip(c));
	if (this->IsSaveContent())
		_set = c->_set;
	else
		_set.clear();
	this->SetUpdateStamp();
	_clearCache();
	return true;
}

template<typename T>
bool SetChip<T>::LoadChip(DocumentLoader &loader)
{
	B_RETURN(ContainerChip::LoadChip(loader));
	if (this->IsSaveContent()) {
		LOAD(MTEXT("set"), _set);
	}
	else
		ClearContainer();
	this->SetUpdateStamp();
	_clearCache();
	return true;
}

template<typename T>
bool SetChip<T>::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(ContainerChip::SaveChip(saver));
	if (this->IsSaveContent())
		SAVE(MTEXT("set"), _set);
	return true;
}


template<typename T>
uint32 SetChip<T>::GetContainerSize()
{
	return (uint32)_set.size();
}

template<typename T>
void SetChip<T>::ClearContainer()
{
	_set.clear();
	this->SetUpdateStamp();
	_clearCache();
}

template<typename T>
bool SetChip<T>::HasElement(T key)
{
	return _set.find(key) != _set.end();
}

template<typename T>
bool SetChip<T>::AddElement(T key) 
{
	if (_set.insert(key).second) {
		_clearCache();
		this->SetUpdateStamp();
		return true;
	}
	return false;
}

template<typename T>
bool SetChip<T>::MoveElement(T oldKey, T newKey)
{
	auto n = _set.find(oldKey);
	if (n == _set.end())
		return false;
	if (_set.find(newKey) != _set.end())
		return false;
	_set.insert(newKey);
	_set.erase(n);
	return true;
}

template<typename T>
bool SetChip<T>::RemoveElement(T key)
{
	auto n = _set.find(key);
	if (n != _set.end()) {
		_set.erase(n);
		_clearCache();
		this->SetUpdateStamp();
		return true;
	}
	return false;
}

template<typename T>
const T *SetChip<T>::GetElementByIndex(uint32 index)
{
	if (index >= _set.size())
		return nullptr;
	if (_cache != _set.end() && (index > _cacheIndex ? (index - _cacheIndex) : (_cacheIndex - index)) < (_set.size() / 2)) {} // It is fastest to iterate from cache!
	else if (index > _set.size() / 2) { // Iterate from back.
		_cache = _set.end();
		--_cache;
		_cacheIndex = (uint32)_set.size() - 1;
	}
	else { // Iterate from front.
		_cache = _set.begin();
		_cacheIndex = 0;
	}
	std::advance(_cache, int(index - _cacheIndex));
	_cacheIndex = index;
	return &*_cache;
}

template<typename T>
const T* SetChip<T>::GetNextElement(T key)
{
	auto n = _set.upper_bound(key);
	if (n != _set.end())
		return &*n;
	return nullptr;
}

template<typename T>
const T* SetChip<T>::GetPreviousElement(T key)
{
	auto n = _set.lower_bound(key);
	if (n == _set.begin() || n == _set.end())
		return nullptr;
	--n;
	return &*n;
}

template<typename T>
void SetChip<T>::_clearCache()
{
	_cache = _set.end();
	_cacheIndex = -1;
}
