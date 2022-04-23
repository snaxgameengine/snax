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


#include "Exports.h"
#include "M3DEngine/Chip.h"
#include "M3DEngine/ClassInstanceRef.h"
#include "ArrayChip.h"


namespace m3d
{


static const Guid CLASSINSTANCEREFARRAYCHIP_GUID = { 0xdd99a19d, 0xd2a2, 0x41a3, { 0xba, 0x1d, 0x53, 0xc2, 0xff, 0x21, 0x94, 0x46 } };

class STDCHIPS_API ClassInstanceRefArrayChip : public ArrayChip, public ClassInstanceRefOwner
{
	CHIPDESC_DECL;
public:
	ClassInstanceRefArrayChip();
	virtual ~ClassInstanceRefArrayChip();

	bool CopyChip(Chip *chip) override;
	bool LoadChip(DocumentLoader &loader) override;
	bool SaveChip(DocumentSaver &saver) const override;

	void OnDestroyDevice() override;
	void OnReleasingBackBuffer(RenderWindow *rw) override;
	void RestoreChip() override;
	void AddDependencies(ProjectDependencies &deps) override;
	Chip *FindChip(ChipID chipID) override;

	uint32 GetContainerSize() override;
	void SetContainerSize(uint32 size) override;
	void ClearContainer() override;

	void SetClass(Class* clazz) override;

	// AT_REF: Set the reference, but does not take ownership.
	// AT_MOVE: Sets the reference, and takes ownership.
	// AT_COPY: Copy the instance, and take ownership of new.
	enum AssignType { AT_REF, AT_MOVE, AT_COPY };

	// Returns the instance at given index.
	virtual ClassInstanceRef GetInstance(uint32 index);
	// Creates an instance at the given index.
	virtual void CreateInstance(uint32 index, Class *instanceOf);
	// Set instance at given index.
	virtual void SetInstance(uint32 index, ClassInstanceRef ref, AssignType at = AT_REF);
	// Clears instance at given index.
	virtual void ClearInstance(uint32 index);
	// Makes us the owner of instance at given index.
	virtual void MakeOwner(uint32 index);
	// Add elements an fill them with instances.
	virtual void AddElements(uint32 count, Class *instanceOf = nullptr);
	// Insert elements an fill them with instances.
	virtual void InsertElements(uint32 at, uint32 count, Class *instanceOf = nullptr);
	// Removes a series of elements.
	virtual void RemoveElements(uint32 at, uint32 count);
	// Sets a series to elements to null.
	virtual void ClearElements(uint32 at, uint32 count);
	// Moves a series of elements to another range.
	virtual void MoveElements(uint32 index, uint32 count, int npos);

	virtual void Sort(const List<InstanceData*> &keys, int dir = 1, uint32 start = 0, uint32 count = -1);

protected:
	List<ClassInstanceRef> _array;

	void _clear();
};

}