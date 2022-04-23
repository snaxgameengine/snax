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

#include "ContainerChip.h"
#include "Value.h"
#include "Text.h"
#include "M3DCore/Containers.h"
#include "M3DEngine/DocumentSaveLoadUtil.h"

namespace m3d
{

template<typename T>
class SetChipInterface : public ContainerChip
{
CHIPDESC_DECL
public:
	// Returns true is given element exist.
	virtual bool HasElement(T key) = 0;
	// Add element with given key. Returns true if inserted, false if exist!
	virtual bool AddElement(T key) = 0;
	// Renames an element. Returns false if oldKEy does not exist or newKey is already in the map!
	virtual bool MoveElement(T oldKey, T newKey) = 0;
	// Removes element at given key, if exist - returns true if so.
	virtual bool RemoveElement(T key) = 0;
	// Returns element at given index. nullptr if not found. Random access of this is slow. Sequencial access is ok.
	virtual const T *GetElementByIndex(uint32 index) = 0;
	// Returns the next element after element with given key. nullptr if not found.
	virtual const T *GetNextElement(T key) = 0;
	// Returns the previous element after element with given key. nullptr if not found.
	virtual const T *GetPreviousElement(T key) = 0;
};


template<typename T>
class SetChip : public SetChipInterface<T>
{
public:
	typedef Set<T> SetType;
	
	SetChip();
	~SetChip();

	bool CopyChip(Chip *chip) override;
	bool LoadChip(DocumentLoader &loader) override;
	bool SaveChip(DocumentSaver &saver) const override;

	// Overrides from ContainerChip.
	uint32 GetContainerSize() override;
	void ClearContainer() override;
	
	bool HasElement(T key) override;
	bool AddElement(T key) override;
	bool MoveElement(T oldKey, T newKey) override;
	bool RemoveElement(T key) override;
	const T *GetElementByIndex(uint32 index) override;
	const T* GetNextElement(T key) override;
	const T* GetPreviousElement(T key) override;
	// Returns the internal container.
	virtual const SetType &GetSet() const { return _set; }

protected:
	SetType _set;

	typename SetType::iterator _cache;
	uint32 _cacheIndex = -1;

	void _clearCache();
};

#include "SetChip.inl"

static const Guid VALUESET_GUID = { 0xb4f05438, 0x4f85, 0x4b6f, { 0x87, 0xc2, 0x29, 0x8f, 0x9b, 0x22, 0x5b, 0x41 } };
template class STDCHIPS_API SetChipInterface<value>;
//template class STDCHIPS_API Set<value>;
template class STDCHIPS_API SetChip<value>;
typedef SetChip<value> ValueSetChip;



static const Guid TEXTSET_GUID = { 0x21867ae7, 0x2aba, 0x4cb4, { 0xa3, 0x76, 0x11, 0xdd, 0x0d, 0x85, 0xac, 0x5e } };
template class STDCHIPS_API SetChipInterface<String>;
//template class STDCHIPS_API Set<String>;
template class STDCHIPS_API SetChip<String>;
typedef SetChip<String> TextSetChip;

}