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


template<typename T, typename S>
MapChip<T, S>::MapChip() 
{
}

template<typename T, typename S>
MapChip<T, S>::~MapChip()
{
}

template<typename T, typename S>
bool MapChip<T, S>::CopyChip(Chip *chip)
{
	MapChip *c = dynamic_cast<MapChip*>(chip);
	B_RETURN(ContainerChip::CopyChip(c));
	if (this->IsSaveContent())
		_map = c->_map;
	else
		_map.clear();
	this->SetUpdateStamp();
	_clearCache();
	return true;
}

template<typename T, typename S>
bool MapChip<T, S>::LoadChip(DocumentLoader &loader)
{
	B_RETURN(ContainerChip::LoadChip(loader));
	if (this->IsSaveContent()) {
		LOAD(MTEXT("map"), _map);
	}
	else
		ClearContainer();
	this->SetUpdateStamp();
	_clearCache();
	return true;
}

template<typename T, typename S>
bool MapChip<T, S>::SaveChip(DocumentSaver &saver) const
{
	B_RETURN(ContainerChip::SaveChip(saver));
	if (this->IsSaveContent())
		SAVE(MTEXT("map"), _map);
	return true;
}


template<typename T, typename S>
uint32 MapChip<T, S>::GetContainerSize()
{
	return (uint32)_map.size();
}

template<typename T, typename S>
void MapChip<T, S>::ClearContainer()
{
	_map.clear();
	this->SetUpdateStamp();
	_clearCache();
}

template<typename T, typename S>
bool MapChip<T, S>::HasElement(T key)
{
	return _map.find(key) != _map.end();
}

template<typename T, typename S>
bool MapChip<T, S>::AddElement(T key) 
{
	if (_map.insert(std::make_pair(key, S())).second) {
		_clearCache();
		this->SetUpdateStamp();
		return true;
	}
	return false;
}

template<typename T, typename S>
bool MapChip<T, S>::MoveElement(T oldKey, T newKey)
{
	auto n = _map.find(oldKey);
	if (n == _map.end())
		return false;
	if (_map.find(newKey) != _map.end())
		return false;
	if (!_map.insert(std::make_pair(newKey, n->second)).second)
		return false;
	_map.erase(n);
	return true;
}

template<typename T, typename S>
bool MapChip<T, S>::RemoveElement(T key)
{
	auto n = _map.find(key);
	if (n != _map.end()) {
		_map.erase(n);
		_clearCache();
		this->SetUpdateStamp();
		return true;
	}
	return false;
}

template<typename T, typename S>
const T* MapChip<T, S>::GetElementByIndex(uint32 index)
{
	if (index >= _map.size()) {
		_clearCache();
		return nullptr;
	}
	if (_cache != _map.end() && (index > _cacheIndex ? (index - _cacheIndex) : (_cacheIndex - index)) < (_map.size() / 2)) {} // It is fastest to iterate from cache!
	else if (index > _map.size() / 2) { // Iterate from back.
		_cache = _map.end();
		_cache--;
		_cacheIndex = (uint32)_map.size() - 1;
	}
	else { // Iterate from front.
		_cache = _map.begin();
		_cacheIndex = 0;
	}
	std::advance(_cache, int(index - _cacheIndex));
	_cacheIndex = index;
	return &_cache->first;
}

template<typename T, typename S>
const T* MapChip<T, S>::GetNextElement(T key)
{
	auto n = _map.upper_bound(key);
	if (n != _map.end())
		return &n->first;
	return nullptr;
}

template<typename T, typename S>
const T* MapChip<T, S>::GetPreviousElement(T key)
{
	auto n = _map.lower_bound(key);
	if (n == _map.begin() || n == _map.end())
		return nullptr;
	--n;
	return &n->first;
}

template<typename T, typename S>
void MapChip<T, S>::_clearCache()
{
	_cache = _map.end();
	_cacheIndex = 0;
}

template<typename T, typename S>
bool MapChip<T, S>::AddElement(T key, S val)
{
	if (_map.insert(std::make_pair(key, val)).second) {
		_clearCache();
		this->SetUpdateStamp();
		return true;
	}
	return false;
}

template<typename T, typename S>
const S* MapChip<T, S>::GetValueByKey(T key)
{
	auto n = _map.find(key);
	if (n == _map.end())
		return nullptr;
	return &n->second;
}

template<typename T, typename S>
const S* MapChip<T, S>::GetValueByIndex(uint32 index)
{
	GetElementByIndex(index); // update cache.
	if (_cache == _map.end())
		return nullptr;
	return &_cache->second;
}
