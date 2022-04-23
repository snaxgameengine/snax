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

namespace m3d
{


static const Guid CLASSINSTANCEREFCHIP_GUID = { 0xea094cf0, 0xcf56, 0x4f5e, { 0xba, 0xdc, 0x7a, 0x8b, 0xf2, 0xe5, 0xf2, 0x76 } };


class STDCHIPS_API ClassInstanceRefChip : public Chip, public ClassInstanceRefOwner
{
	CHIPDESC_DECL;
public:
	ClassInstanceRefChip();
	virtual ~ClassInstanceRefChip();

	bool CopyChip(Chip *chip) override;
	bool LoadChip(DocumentLoader &loader) override;
	bool SaveChip(DocumentSaver &saver) const override;

	void OnDestroyDevice() override;
	void OnReleasingBackBuffer(RenderWindow *rw) override;
	void RestoreChip() override;
	void AddDependencies(ProjectDependencies &deps) override;
	Chip *FindChip(ChipID chipID) override;

	void SetClass(Class* clazz) override;

	// AT_REF: Set the reference, but does not take ownership.
	// AT_MOVE: Sets the reference, and takes ownership.
	// AT_COPY: Copy the instance, and take ownership of new.
	enum AssignType { AT_REF, AT_MOVE, AT_COPY };

	// Returns our instance.
	virtual ClassInstanceRef GetInstance() { return _ref; }
	// Creates a new instance and makes us the owner.
	virtual void CreateInstance(Class *instanceOf);
	// Sets our instance ref.
	virtual void SetInstance(ClassInstanceRef ref, AssignType at = AT_REF);
	// Clears the reference.
	virtual void ClearInstance();
	// Makes us the owner of the instance.
	virtual void MakeOwner();
	// true if we should keep the reference through save/load.
	virtual bool IsSaveRef() const { return _saveRef; }
	// set if we should keep the reference through save/load.
	virtual void SetSaveRef(bool saveRef) { _saveRef = saveRef; }


protected:
	ClassInstanceRef _ref;
	bool _saveRef;

	void _clear();
};



}