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

#include "MapChip.h"
#include "M3DEngine/ClassInstanceRef.h"
#include "M3DEngine/ClassInstance.h"

namespace m3d
{
	

template<typename T>
class ClassInstanceRefMapChip : public MapChip<T, ClassInstanceRef>, public ClassInstanceRefOwner
{
public:
	// AT_REF: Set the reference, but does not take ownership.
	// AT_MOVE: Sets the reference, and takes ownership.
	// AT_COPY: Copy the instance, and take ownership of new.
	enum AssignType { AT_REF, AT_MOVE, AT_COPY };

	ClassInstanceRefMapChip();
	~ClassInstanceRefMapChip();

	bool CopyChip(Chip *chip) override;
	bool LoadChip(DocumentLoader &loader) override;
	bool SaveChip(DocumentSaver &saver) const override;

	void OnDestroyDevice() override;
	void OnReleasingBackBuffer(RenderWindow *rw) override;
	void RestoreChip() override;
	void AddDependencies(ProjectDependencies &deps) override;
	Chip *FindChip(ChipID chipID) override;

	void SetClass(Class* clazz) override;

	void ClearContainer() override { return _clear(); }

	bool RemoveElement(T key) override;

	bool AddElement(T key, ClassInstanceRef val) override;

	// Create a new instance at given key. Overwrites if exist.
	virtual ClassInstanceRef CreateInstance(T key, Class *instanceOf);
	// Set element at given key. Overwrites is exist.
	virtual void SetInstance(T key, ClassInstanceRef ref, AssignType at = AT_REF);
	// Makes us the owner of the instance at given key, if exist.
	virtual void MakeOwner(T key);

protected:
	void _clear();

};

#include "ClassInstanceRefMapChip.inl"


static const Guid CLASSINSTANCEREFBYVALUEMAPCHIP_GUID = { 0xcc33e380, 0x0ebf, 0x485c, { 0xa1, 0x66, 0x79, 0xf8, 0x14, 0xe1, 0x99, 0xb1 } };
template class STDCHIPS_API ClassInstanceRefMapChip<value>;
typedef ClassInstanceRefMapChip<value> ClassInstanceRefByValueMapChip;

static const Guid CLASSINSTANCEREFBYTEXTMAPCHIP_GUID = { 0xb4f05438, 0x4f85, 0x4b6f, { 0x87, 0xc2, 0x29, 0x8f, 0x9b, 0x22, 0x5b, 0x40 } };
template class STDCHIPS_API ClassInstanceRefMapChip<String>;
typedef ClassInstanceRefMapChip<String> ClassInstanceRefByTextMapChip;



}