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
#include "PhysX.h"

namespace m3d
{


static const Guid PHYSXROPE_GUID = { 0x62ea0e5d, 0x90c8, 0x4898, { 0xae, 0x64, 0x14, 0xb0, 0x5a, 0x95, 0xbd, 0x58 } };



class PHYSXCHIPS_API PhysXRope : public Chip, public PhysXSceneObject
{
	CHIPDESC_DECL;
public:
	PhysXRope();
	virtual ~PhysXRope();

/*	virtual bool InitChip() override;
	virtual bool CopyChip(Chip *chip) override;
	virtual bool LoadChip(DocumentLoader &loader) override;
	virtual bool SaveChip(DocumentSaver &saver) const override;
*/
	virtual void CallChip() override;

	virtual void OnSceneDestroyed() override { DestroyRope(); }

	virtual void DestroyRope();

	virtual const List<PxRigidDynamic*> &GetActors() const { return _actors; }
	virtual const List<PxJoint*> &GetJoints() const { return _joints; }

protected:
	virtual void OnRelease() { DestroyRope(); }

	List<PxRigidDynamic*> _actors;
	List<PxJoint*> _joints;

};

}