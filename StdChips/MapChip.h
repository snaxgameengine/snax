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

#include "SetChip.h"
#include "M3DCore/Containers.h"

namespace m3d
{

template<typename T, typename S>
class MapChip : public SetChipInterface<T>
{
CHIPDESC_DECL

public:
	typedef Map<T, S> MapType;

	MapChip();
	~MapChip();

	bool CopyChip(Chip *chip) override;
	bool LoadChip(DocumentLoader &loader) override;
	bool SaveChip(DocumentSaver &saver) const override;

	// Overrides from ContainerChip.
	uint32 GetContainerSize() override;
	void ClearContainer() override;

	bool HasElement(T key) override;
	bool AddElement(T key) override;
	bool MoveElement(T oldKey, T newKey);
	bool RemoveElement(T key) override;
	const T* GetElementByIndex(uint32 index) override;
	const T* GetNextElement(T key) override;
	const T* GetPreviousElement(T key) override;

	virtual bool AddElement(T key, S val);
	virtual const S *GetValueByKey(T key);
	virtual const S *GetValueByIndex(uint32 index);

	// Returns the internal container.
	virtual const MapType &GetMap() const { return _map; }

protected:
	MapType _map;

	typename MapType::iterator _cache;
	uint32 _cacheIndex = -1;

	void _clearCache();
};

#include "MapChip.inl"

}