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
#include "PhysXRigidDynamic.h"
#include "PhysX.h"


namespace m3d
{


static const Guid PHYSXROPEACTOR_GUID = { 0x739a253a, 0xb545, 0x41a5, { 0x95, 0x59, 0xf8, 0x50, 0xb3, 0x72, 0xd4, 0xb9 } };



class PHYSXCHIPS_API PhysXRopeActor : public PhysXRigidDynamic
{
	CHIPDESC_DECL;
public:
	PhysXRopeActor();
	virtual ~PhysXRopeActor();

	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;

	virtual void CallChip() override {}

	virtual PxRigidDynamic *GetRigidDynamic() const override;

	virtual PxRigidDynamic *CreateActor(const PxTransform &pose) override { return nullptr; }
	virtual void DestroyActor() override {}

	enum Type {NONE, FIRST, LAST, INDEX };

	virtual Type GetType() const { return _type; }
	virtual void SetType(Type type);

protected:
	Type _type;
};



}